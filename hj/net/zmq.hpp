#ifndef ZMQ_HPP
#define ZMQ_HPP

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include <zmq.h>

#ifndef HJ_ZMQ_THREAD_AFFINITY_CHECK_ENABLE
#define HJ_ZMQ_THREAD_AFFINITY_CHECK_ENABLE 1
#endif

namespace hj::zmq
{

namespace cmd
{
// constexpr uint8_t DATA = 0;
constexpr uint8_t STOP = 1;
} // namespace cmd

enum class io_status
{
    ok,
    would_block,
    interrupted,
    closed
};

struct multipart_result
{
    io_status status;
    size_t    frames;

    [[nodiscard]] explicit operator bool() const noexcept
    {
        return status == io_status::ok;
    }

    friend bool operator==(const multipart_result &lhs, io_status rhs) noexcept
    {
        return lhs.status == rhs;
    }

    friend bool operator==(io_status lhs, const multipart_result &rhs) noexcept
    {
        return lhs == rhs.status;
    }

    friend bool operator!=(const multipart_result &lhs, io_status rhs) noexcept
    {
        return lhs.status != rhs;
    }

    friend bool operator!=(io_status lhs, const multipart_result &rhs) noexcept
    {
        return lhs != rhs.status;
    }
};

// -----------------------------------------------------------------------------
// Exception Handling
// -----------------------------------------------------------------------------
class zmq_error : public std::runtime_error
{
  public:
    explicit zmq_error(std::string op)
        : zmq_error(std::move(op), zmq_errno())
    {
    }

    zmq_error(std::string op, int errnum)
        : std::runtime_error(format_message(op, errnum))
        , _op(std::move(op))
        , _errnum(errnum)
    {
    }

    [[nodiscard]] const std::string &operation() const noexcept { return _op; }
    [[nodiscard]] int         error_number() const noexcept { return _errnum; }
    [[nodiscard]] const char *error_name() const noexcept
    {
        switch(_errnum)
        {
            case EAGAIN:
                return "EAGAIN";
            case ENOTSUP:
                return "ENOTSUP";
            case EPROTONOSUPPORT:
                return "EPROTONOSUPPORT";
            case ENOBUFS:
                return "ENOBUFS";
            case ENETDOWN:
                return "ENETDOWN";
            case EADDRINUSE:
                return "EADDRINUSE";
            case EINPROGRESS:
                return "EINPROGRESS";
            case ENOTSOCK:
                return "ENOTSOCK";
            case EMSGSIZE:
                return "EMSGSIZE";
            case EAFNOSUPPORT:
                return "EAFNOSUPPORT";
            case ENETUNREACH:
                return "ENETUNREACH";
            case ECONNREFUSED:
                return "ECONNREFUSED";
            case EINTR:
                return "EINTR";
            case EFAULT:
                return "EFAULT";
            case EINVAL:
                return "EINVAL";
            case EBADF:
                return "EBADF";
#ifdef EHAUSKEEP
            case EHAUSKEEP:
                return "EHAUSKEEP";
#endif
#ifdef ETERM
            case ETERM:
                return "ETERM";
#endif
#ifdef EMTHREAD
            case EMTHREAD:
                return "EMTHREAD";
#endif
            default:
                return "UNKNOWN_ERROR";
        }
    }

  private:
    static std::string format_message(const std::string &op, int errnum)
    {
        std::ostringstream ss;
        ss << op << " failed:\n"
           << "  errno=" << errnum << "\n"
           << "  detail=" << zmq_strerror(errnum);
        return ss.str();
    }

  private:
    std::string _op;
    int         _errnum;
};

class context : public std::enable_shared_from_this<context>
{
  public:
    using ptr = std::shared_ptr<context>;

    static ptr create() { return std::make_shared<context>(); }

    context()
        : _ctx(zmq_ctx_new())
    {
        if(!_ctx)
            throw zmq_error("zmq_ctx_new failed");
    }

    context(const context &)            = delete;
    context &operator=(const context &) = delete;

    context(context &&other) noexcept
        : _ctx(other._ctx)
    {
        other._ctx = nullptr;
    }

    ~context() noexcept
    {
        if(_ctx)
        {
            shutdown();
            terminate();
        }
    }

    context &operator=(context &&other) noexcept
    {
        if(this != &other)
        {
            if(_ctx)
            {
                shutdown();
                terminate();
            }

            _ctx       = other._ctx;
            other._ctx = nullptr;
        }
        return *this;
    }

    void shutdown() noexcept
    {
        if(_ctx)
            zmq_ctx_shutdown(_ctx);
    }

    void terminate() noexcept
    {
        if(_ctx)
        {
            zmq_ctx_term(_ctx);
            _ctx = nullptr;
        }
    }

    void *get() const noexcept { return _ctx; }

  private:
    void *_ctx{nullptr};
};

class message
{
  public:
    message()
    {
        if(zmq_msg_init(&_msg) != 0)
            throw zmq_error("zmq_msg_init failed");
    }

    explicit message(size_t size)
    {
        if(zmq_msg_init_size(&_msg, size) != 0)
            throw zmq_error("zmq_msg_init_size failed");
    }

    explicit message(std::string_view sv)
    {
        if(zmq_msg_init_size(&_msg, sv.size()) != 0)
            throw zmq_error("zmq_msg_init_size failed");
        if(!sv.empty())
            std::memcpy(data(), sv.data(), sv.size());
    }

    message(void *data_ptr, size_t size, zmq_free_fn *ffn, void *hint = nullptr)
    {
        if(zmq_msg_init_data(&_msg, data_ptr, size, ffn, hint) != 0)
            throw zmq_error("zmq_msg_init_data failed");
    }

    template <typename Deleter,
              typename = std::enable_if_t<
                  std::is_invocable_v<std::decay_t<Deleter>, void *>
                  || std::is_invocable_v<std::decay_t<Deleter>>>>
    message(void *data_ptr, size_t size, Deleter &&deleter)
    {
        using DecayedDeleter = std::decay_t<Deleter>;
        static_assert(
            std::is_nothrow_invocable_v<DecayedDeleter, void *>
                || std::is_nothrow_invocable_v<DecayedDeleter>,
            "hj::zmq::message custom deleter must be noexcept callable!");
        auto *del_ptr = new DecayedDeleter(std::forward<Deleter>(deleter));

        auto free_fn = [](void *data, void *hint) noexcept {
            auto *d = static_cast<DecayedDeleter *>(hint);
            if constexpr(std::is_invocable_v<DecayedDeleter, void *>)
                (*d)(data);
            else
                (*d)();

            delete d;
        };

        if(zmq_msg_init_data(&_msg, data_ptr, size, free_fn, del_ptr) != 0)
        {
            delete del_ptr;
            throw zmq_error("zmq_msg_init_data with custom deleter failed");
        }
    }

    template <typename T>
    message(std::shared_ptr<T> buffer, void *data_ptr, size_t size)
        : message(data_ptr, size, [buf = std::move(buffer)](void *) mutable {
            buf.reset();
        })
    {
    }

    template <typename Container>
    message(std::shared_ptr<Container> container)
        : message(
              container,
              const_cast<void *>(static_cast<const void *>(container->data())),
              container->size() * sizeof(typename Container::value_type))
    {
    }

    message(const message &)            = delete;
    message &operator=(const message &) = delete;

    message(message &&rhs) noexcept
    {
        zmq_msg_init(&_msg);
        zmq_msg_move(&_msg, &rhs._msg);
    }

    ~message() noexcept { zmq_msg_close(&_msg); }

    message &operator=(message &&rhs) noexcept
    {
        if(this != &rhs)
        {
            zmq_msg_close(&_msg);
            zmq_msg_move(&_msg, &rhs._msg);
        }
        return *this;
    }

    zmq_msg_t       *get() noexcept { return &_msg; }
    const zmq_msg_t *get() const noexcept { return &_msg; }
    void            *data() noexcept { return zmq_msg_data(&_msg); }
    const void      *data() const noexcept
    {
        return zmq_msg_data(const_cast<zmq_msg_t *>(&_msg));
    }
    size_t           size() const noexcept { return zmq_msg_size(&_msg); }
    std::string_view to_string_view() const noexcept
    {
        return std::string_view(static_cast<const char *>(data()), size());
    }

  private:
    zmq_msg_t _msg;
};

class socket
{
  public:
    socket(context::ptr ctx, int type)
        : _ctx(std::move(ctx))
        , _sock(nullptr)
        , _owner_thread_id(std::this_thread::get_id())
    {
        if(!_ctx)
            throw std::invalid_argument("context pointer cannot be null");

        _sock = zmq_socket(_ctx->get(), type);
        if(!_sock)
            throw zmq_error("zmq_socket failed");
    }

    virtual ~socket() noexcept
    {
        if(_sock)
            zmq_close(_sock);
    }

    socket(const socket &)            = delete;
    socket &operator=(const socket &) = delete;

    socket(socket &&other) noexcept
        : _ctx(std::move(other._ctx))
        , _sock(other._sock)
        , _owner_thread_id(other._owner_thread_id)
    {
        other._sock      = nullptr;
        _owner_thread_id = std::this_thread::get_id();
    }

    socket &operator=(socket &&other) noexcept
    {
        if(this != &other)
        {
            if(_sock)
                zmq_close(_sock);

            _ctx             = std::move(other._ctx);
            _sock            = other._sock;
            _owner_thread_id = std::this_thread::get_id();
            other._sock      = nullptr;
        }
        return *this;
    }

    void bind_to_current_thread() noexcept
    {
        _owner_thread_id = std::this_thread::get_id();
    }

    void bind(const std::string &addr)
    {
        check_thread_affinity();
        if(zmq_bind(_sock, addr.c_str()) != 0)
            throw zmq_error("zmq_bind failed");
    }

    void connect(const std::string &addr)
    {
        check_thread_affinity();
        if(zmq_connect(_sock, addr.c_str()) != 0)
            throw zmq_error("zmq_connect failed");
    }

    void disconnect(const std::string &addr)
    {
        check_thread_affinity();
        if(zmq_disconnect(_sock, addr.c_str()) != 0)
            throw zmq_error("zmq_disconnect failed");
    }

    void set_linger(int ms)
    {
        check_thread_affinity();
        if(set_opt(ZMQ_LINGER, ms) != 0)
            throw zmq_error("set_opt ZMQ_LINGER failed");
    }

    int set_opt(int opt, int value)
    {
        check_thread_affinity();
        return zmq_setsockopt(_sock, opt, &value, sizeof(value));
    }

    int set_opt(int opt, const void *value, size_t len)
    {
        check_thread_affinity();
        return zmq_setsockopt(_sock, opt, value, len);
    }

    int get_opt(int opt, void *value, size_t *len) const
    {
        check_thread_affinity();
        return zmq_getsockopt(_sock, opt, value, len);
    }

    void *get() const noexcept
    {
        check_thread_affinity();
        return _sock;
    }

    io_status send(message &&msg, int flags = 0)
    {
        check_thread_affinity();
        while(true)
        {
            if(zmq_msg_send(msg.get(), _sock, flags) >= 0)
                return io_status::ok;

            int ec = zmq_errno();
            if(ec == EINTR)
                return io_status::interrupted;
            if(ec == EAGAIN)
                return io_status::would_block;
            if(ec == ETERM || ec == ENOTSOCK)
                return io_status::closed;

            throw zmq_error("zmq_msg_send failed", ec);
        }
    }

    io_status recv(message &msg, int flags = 0)
    {
        check_thread_affinity();
        while(true)
        {
            if(zmq_msg_recv(msg.get(), _sock, flags) >= 0)
                return io_status::ok;

            int ec = zmq_errno();
            if(ec == EINTR)
                return io_status::interrupted;
            if(ec == EAGAIN)
                return io_status::would_block;
            if(ec == ETERM || ec == ENOTSOCK)
                return io_status::closed;

            throw zmq_error("zmq_msg_recv failed", ec);
        }
    }

    multipart_result send_multipart(const std::vector<std::string_view> &parts,
                                    int flags = 0)
    {
        check_thread_affinity();
        size_t i = 0;
        for(; i < parts.size(); ++i)
        {
            int send_flags = flags | ((i + 1 < parts.size()) ? ZMQ_SNDMORE : 0);
            while(true)
            {
                int rc = zmq_send(_sock,
                                  parts[i].data(),
                                  parts[i].size(),
                                  send_flags);
                if(rc >= 0)
                    break;

                int ec = zmq_errno();
                if(ec == EINTR)
                    return {io_status::interrupted, i};
                if(ec == EAGAIN)
                    return {io_status::would_block, i};
                if(ec == ETERM || ec == ENOTSOCK)
                    return {io_status::closed, i};

                throw zmq_error("zmq_send failed in send_multipart", ec);
            }
        }

        return {io_status::ok, parts.size()};
    }

    multipart_result send_multipart(std::vector<message> &&parts, int flags = 0)
    {
        check_thread_affinity();
        size_t i = 0;
        for(; i < parts.size(); ++i)
        {
            int send_flags = flags | ((i + 1 < parts.size()) ? ZMQ_SNDMORE : 0);
            while(true)
            {
                int rc = zmq_msg_send(parts[i].get(), _sock, send_flags);
                if(rc >= 0)
                    break;

                int ec = zmq_errno();
                if(ec == EINTR)
                    return {io_status::interrupted, i};
                if(ec == EAGAIN)
                    return {io_status::would_block, i};
                if(ec == ETERM || ec == ENOTSOCK)
                    return {io_status::closed, i};

                throw zmq_error(
                    "zmq_msg_send failed in send_multipart(vector<message>&&)",
                    ec);
            }
        }

        return {io_status::ok, parts.size()};
    }

    multipart_result recv_multipart(std::vector<std::string> &dst,
                                    int                       flags = 0)
    {
        check_thread_affinity();
        dst.clear();
        size_t i = 0;
        while(true)
        {
            message msg;
            while(true)
            {
                int rc = zmq_msg_recv(msg.get(), _sock, flags);
                if(rc >= 0)
                    break;

                int ec = zmq_errno();
                if(ec == EINTR)
                    return {io_status::interrupted, i};
                if(ec == EAGAIN)
                    return {io_status::would_block, i};
                if(ec == ETERM || ec == ENOTSOCK)
                    return {io_status::closed, i};

                throw zmq_error("zmq_msg_recv failed in recv_multipart", ec);
            }

            dst.emplace_back(static_cast<char *>(msg.data()), msg.size());
            ++i;

            int64_t more     = 0;
            size_t  more_len = sizeof(more);
            if(zmq_getsockopt(_sock, ZMQ_RCVMORE, &more, &more_len) != 0)
                throw zmq_error("getsockopt ZMQ_RCVMORE failed");

            if(!more)
                break;
        }

        return {io_status::ok, i};
    }

    multipart_result recv_multipart(std::vector<message> &dst, int flags = 0)
    {
        check_thread_affinity();
        dst.clear();
        size_t i = 0;
        while(true)
        {
            message msg;
            while(true)
            {
                int rc = zmq_msg_recv(msg.get(), _sock, flags);
                if(rc >= 0)
                    break;

                int ec = zmq_errno();
                if(ec == EINTR)
                    return {io_status::interrupted, i};
                if(ec == EAGAIN)
                    return {io_status::would_block, i};
                if(ec == ETERM || ec == ENOTSOCK)
                    return {io_status::closed, i};

                throw zmq_error(
                    "zmq_msg_recv failed in recv_multipart(vector<message>&)",
                    ec);
            }

            dst.push_back(std::move(msg));
            ++i;

            int64_t more     = 0;
            size_t  more_len = sizeof(more);
            if(zmq_getsockopt(_sock, ZMQ_RCVMORE, &more, &more_len) != 0)
                throw zmq_error("getsockopt ZMQ_RCVMORE failed");

            if(!more)
                break;
        }

        return {io_status::ok, i};
    }

  protected:
    void check_thread_affinity() const
    {
#if HJ_ZMQ_THREAD_AFFINITY_CHECK_ENABLE
        if(_owner_thread_id != std::thread::id()
           && _owner_thread_id != std::this_thread::get_id())
        {
            throw std::logic_error("ZeroMQ Socket Thread Affinity Violation: "
                                   "Socket accessed from different thread!");
        }
#endif
    }

  protected:
    context::ptr    _ctx;
    void           *_sock;
    std::thread::id _owner_thread_id;
};

class r_chan : public socket
{
  public:
    explicit r_chan(context::ptr ctx, const std::string &addr, int flags = 0)
        : socket(std::move(ctx), ZMQ_PAIR)
        , _flags{flags}
        , _addr{addr}
    {
        if(0 != zmq_connect(_sock, _addr.c_str()))
            throw zmq_error("zmq_connect failed for r_chan");
    }

    io_status recv(message &dst) { return socket::recv(dst, _flags); }

    io_status recv(std::string &dst)
    {
        message   msg;
        io_status st = recv(msg);
        if(st == io_status::ok)
            dst.assign(static_cast<const char *>(msg.data()), msg.size());

        return st;
    }

    inline io_status operator>>(message &dst) { return recv(dst); }
    inline io_status operator>>(std::string &dst) { return recv(dst); }

  private:
    int         _flags;
    std::string _addr;
};

class w_chan : public socket
{
  public:
    explicit w_chan(context::ptr ctx, int flags = 0)
        : socket(ctx, ZMQ_PAIR)
        , _flags{flags}
    {
        static std::atomic<uint64_t> chan_counter{0};
        uint64_t                     id = ++chan_counter;

        std::ostringstream ss;
        ss << "inproc://chan_" << id;
        _addr = ss.str();

        if(0 != zmq_bind(_sock, _addr.c_str()))
            throw zmq_error("zmq_bind failed for w_chan");
    }

    io_status send(message &&src)
    {
        return socket::send(std::move(src), _flags);
    }

    io_status send(std::string_view src)
    {
        message msg(src);
        return send(std::move(msg));
    }

    inline io_status operator<<(message &&src) { return send(std::move(src)); }
    inline io_status operator<<(std::string_view src) { return send(src); }

    inline r_chan make_r_chan(int flags = 0) const
    {
        return r_chan(_ctx, _addr, flags);
    }

    const std::string &address() const noexcept { return _addr; }

  private:
    int         _flags;
    std::string _addr;
};

inline std::pair<w_chan, r_chan> make_pair_channel(context::ptr ctx)
{
    w_chan writer(ctx);
    r_chan reader = writer.make_r_chan();
    return {std::move(writer), std::move(reader)};
}

class broker
{
  public:
    broker(context::ptr ctx, socket &&back, socket &&front)
        : _ctx(std::move(ctx))
        , _back(std::move(back))
        , _front(std::move(front))
        , _ctrl_server(_ctx, ZMQ_PAIR)
        , _running(false)
    {
        static std::atomic<uint64_t> broker_counter{0};
        _ctrl_addr = "inproc://broker_ctrl_" + std::to_string(++broker_counter);
        _ctrl_server.bind(_ctrl_addr);
    }

    ~broker() { stop(); }

    broker(const broker &)            = delete;
    broker &operator=(const broker &) = delete;

    broker(broker &&) noexcept            = default;
    broker &operator=(broker &&) noexcept = default;

    const socket &backend() const noexcept { return _back; }
    const socket &frontend() const noexcept { return _front; }

    [[nodiscard]] bool is_running() const noexcept
    {
        return _running.load(std::memory_order_acquire);
    }

    void bind(const std::string &back_addr, const std::string &front_addr)
    {
        _back.bind(back_addr);
        _front.bind(front_addr);
    }

    /**
     * @brief Runs the steerable proxy loop with an internal control channel.
     * 
     * @note THIS IS A BLOCKING CALL. The current thread enters an event loop.
     *       Call stop() from another thread to trigger a graceful shutdown.
     */
    void proxy(socket *capture = nullptr)
    {
        bool expected = false;
        if(!_running.compare_exchange_strong(expected,
                                             true,
                                             std::memory_order_acq_rel))
            throw std::logic_error("broker::proxy() is already running!");

        struct guard
        {
            std::atomic<bool> &flag;
            ~guard() { flag.store(false, std::memory_order_release); }
        } running_guard{_running};

        while(true)
        {
            int rc = zmq_proxy_steerable(_front.get(),
                                         _back.get(),
                                         capture ? capture->get() : nullptr,
                                         _ctrl_server.get());

            if(rc == 0)
                return; // Gracefully stopped by "TERMINATE" command

            int ec = zmq_errno();
            if(ec == EINTR)
                continue;
            if(ec == ETERM || ec == ENOTSOCK)
                return;

            throw zmq_error("zmq_proxy_steerable failed", ec);
        }
    }

    void stop() noexcept
    {
        if(!is_running())
            return;

        try
        {
            socket ctrl_client(_ctx, ZMQ_PAIR);
            ctrl_client.connect(_ctrl_addr);
            message msg("TERMINATE");
            ctrl_client.send(std::move(msg));
        }
        catch(...)
        {
        }
    }

  private:
    context::ptr      _ctx;
    socket            _back;
    socket            _front;
    socket            _ctrl_server;
    std::string       _ctrl_addr;
    std::atomic<bool> _running;
};

class consumer : public socket
{
  public:
    explicit consumer(context::ptr ctx)
        : socket(std::move(ctx), ZMQ_PULL)
    {
    }

    std::optional<std::string> pull_string(int flags = 0)
    {
        message msg;
        if(pull(msg, flags) != io_status::ok)
            return std::nullopt;

        return std::string(static_cast<const char *>(msg.data()), msg.size());
    }

    io_status pull(message &data, int flags = 0)
    {
        return socket::recv(data, flags);
    }

    void safe_pull(w_chan &ch, uint8_t stop_cmd = cmd::STOP)
    {
        ch.bind_to_current_thread();
        while(true)
        {
            message   buf;
            io_status status = recv(buf, 0);
            if(status == io_status::closed || status == io_status::interrupted)
                break;

            if(status == io_status::would_block)
                continue;

            bool is_stop = false;
            if(buf.size() == sizeof(uint8_t))
            {
                uint8_t wire_cmd = *static_cast<const uint8_t *>(buf.data());
                if(wire_cmd == stop_cmd)
                    is_stop = true;
            }

            if(ch.send(std::move(buf)) != io_status::ok)
                break;

            if(is_stop)
                break;
        }
    }
};

class producer : public socket
{
  public:
    explicit producer(context::ptr ctx)
        : socket(std::move(ctx), ZMQ_PUSH)
    {
    }

    io_status push(message &&data, int flags = 0)
    {
        return socket::send(std::move(data), flags);
    }

    io_status push(std::string_view str, int flags = 0)
    {
        message msg(str);
        return push(std::move(msg), flags);
    }

    void safe_push(r_chan &ch, uint8_t stop_cmd = cmd::STOP)
    {
        ch.bind_to_current_thread();
        while(true)
        {
            message   buf;
            io_status status = ch.recv(buf);
            if(status == io_status::closed || status == io_status::interrupted)
                break;

            if(status == io_status::would_block)
                continue;

            bool is_stop = false;
            if(buf.size() == sizeof(uint8_t))
            {
                uint8_t wire_cmd = *static_cast<const uint8_t *>(buf.data());
                if(wire_cmd == stop_cmd)
                    is_stop = true;
            }

            if(push(std::move(buf), 0) != io_status::ok)
                break;

            if(is_stop)
                break;
        }
    }
};

class publisher : public socket
{
  public:
    explicit publisher(context::ptr ctx)
        : socket(std::move(ctx), ZMQ_PUB)
    {
    }

    io_status pub(message &&data, int flags = 0)
    {
        return socket::send(std::move(data), flags);
    }

    io_status pub(std::string_view str, int flags = 0)
    {
        message msg(str);
        return pub(std::move(msg), flags);
    }

    void safe_pub(r_chan &ch, uint8_t stop_cmd = cmd::STOP)
    {
        ch.bind_to_current_thread();
        while(true)
        {
            message   buf;
            io_status status = ch.recv(buf);
            if(status == io_status::closed || status == io_status::interrupted)
                break;

            if(status == io_status::would_block)
                continue;

            bool is_stop = false;
            if(buf.size() == sizeof(uint8_t))
            {
                uint8_t wire_cmd = *static_cast<const uint8_t *>(buf.data());
                if(wire_cmd == stop_cmd)
                    is_stop = true;
            }

            if(pub(std::move(buf), 0) != io_status::ok)
                break;

            if(is_stop)
                break;
        }
    }
};

class subscriber : public socket
{
  public:
    explicit subscriber(context::ptr ctx)
        : socket(std::move(ctx), ZMQ_SUB)
    {
    }

    void sub(std::string_view topic)
    {
        if(set_opt(ZMQ_SUBSCRIBE, topic.data(), topic.size()) != 0)
            throw zmq_error("subscriber sub failed");
    }

    void unsub(std::string_view topic)
    {
        if(set_opt(ZMQ_UNSUBSCRIBE, topic.data(), topic.size()) != 0)
            throw zmq_error("subscriber unsub failed");
    }

    std::optional<std::string> recv_string(int flags = 0)
    {
        message msg;
        if(recv(msg, flags) != io_status::ok)
            return std::nullopt;

        return std::string(static_cast<const char *>(msg.data()), msg.size());
    }

    io_status recv(message &data, int flags = 0)
    {
        return socket::recv(data, flags);
    }

    void safe_recv(w_chan &ch, uint8_t stop_cmd = cmd::STOP)
    {
        ch.bind_to_current_thread();
        while(true)
        {
            message   buf;
            io_status status = recv(buf, 0);
            if(status == io_status::closed || status == io_status::interrupted)
                break;

            if(status == io_status::would_block)
                continue;

            bool is_stop = false;
            if(buf.size() == sizeof(uint8_t))
            {
                uint8_t wire_cmd = *static_cast<const uint8_t *>(buf.data());
                if(wire_cmd == stop_cmd)
                    is_stop = true;
            }

            if(ch.send(std::move(buf)) != io_status::ok)
                break;

            if(is_stop)
                break;
        }
    }
};

class poller
{
  public:
    struct event_entry;
    using callback_t = std::function<void(const event_entry &)>;
    using user_data_t =
        std::variant<std::monostate, uintptr_t, void *, callback_t>;

    struct event_entry
    {
        void       *sock;
        zmq_fd_t    fd;
        short       events;
        short       revents;
        user_data_t user_data;

        [[nodiscard]] constexpr bool readable() const noexcept
        {
            return (revents & ZMQ_POLLIN) != 0;
        }

        [[nodiscard]] constexpr bool writable() const noexcept
        {
            return (revents & ZMQ_POLLOUT) != 0;
        }

        [[nodiscard]] constexpr bool error() const noexcept
        {
            return (revents & ZMQ_POLLERR) != 0;
        }

        template <typename T>
        [[deprecated("Using raw pointers in poller is unsafe and error-prone. "
                     "Use as_tag() or callback_t instead.")]]
        T *as_ptr() const
        {
            if(auto pval = std::get_if<void *>(&user_data))
                return static_cast<T *>(*pval);
            return nullptr;
        }

        template <typename T = uintptr_t>
        T as_tag() const
        {
            if(auto pval = std::get_if<uintptr_t>(&user_data))
                return static_cast<T>(*pval);
            return T{};
        }
    };

  public:
    poller() = default;

    void add(const socket &sock,
             uintptr_t     tag,
             short         events = ZMQ_POLLIN | ZMQ_POLLERR)
    {
        _items.push_back({sock.get(), 0, events, 0});
        _user_datas.push_back(tag);
    }

    void add(zmq_fd_t fd, uintptr_t tag, short events)
    {
        _items.push_back({nullptr, fd, events, 0});
        _user_datas.push_back(tag);
    }

    template <typename T = void>
    [[deprecated(
        "Passing raw pointers to poller::add is dangerous due to dangling "
        "pointer risks. Prefer using uintptr_t tag or callback_t.")]]
    void add(const socket &sock,
             T            *user_data,
             short         events = ZMQ_POLLIN | ZMQ_POLLERR)
    {
        _items.push_back({sock.get(), 0, events, 0});
        _user_datas.push_back(static_cast<void *>(user_data));
    }

    template <typename T = void>
    [[deprecated(
        "Passing raw pointers to poller::add is dangerous due to dangling "
        "pointer risks. Prefer using uintptr_t tag or callback_t.")]]
    void add(zmq_fd_t fd, T *user_data, short events)
    {
        _items.push_back({nullptr, fd, events, 0});
        _user_datas.push_back(static_cast<void *>(user_data));
    }

    void add(const socket &sock,
             callback_t    cb,
             short         events = ZMQ_POLLIN | ZMQ_POLLERR)
    {
        _items.push_back({sock.get(), 0, events, 0});
        _user_datas.push_back(std::move(cb));
    }

    void add(zmq_fd_t fd, callback_t cb, short events)
    {
        _items.push_back({nullptr, fd, events, 0});
        _user_datas.push_back(std::move(cb));
    }

    bool remove(const socket &sock) { return remove_impl(sock.get(), 0, true); }

    bool remove(zmq_fd_t fd) { return remove_impl(nullptr, fd, false); }

    bool modify(const socket &sock, short events)
    {
        return modify_impl(sock.get(), 0, true, events);
    }

    bool modify(zmq_fd_t fd, short events)
    {
        return modify_impl(nullptr, fd, false, events);
    }

    void clear()
    {
        _items.clear();
        _user_datas.clear();
    }

    int poll(std::vector<event_entry> &active_events, long timeout_ms = -1)
    {
        active_events.clear();
        if(_items.empty())
            return 0;

        while(true)
        {
            int rc = zmq_poll(_items.data(),
                              static_cast<int>(_items.size()),
                              timeout_ms);
            if(rc < 0)
            {
                if(errno == EINTR)
                    continue;
                if(errno == ETIMEDOUT)
                    return 0;
                throw zmq_error("zmq_poll failed");
            }

            if(rc == 0)
                return rc;

            for(size_t i = 0; i < _items.size(); ++i)
            {
                if(_items[i].revents == 0)
                    continue;

                active_events.push_back({_items[i].socket,
                                         _items[i].fd,
                                         _items[i].events,
                                         _items[i].revents,
                                         _user_datas[i]});
            }
            return rc;
        }
    }

    int poll_and_dispatch(long timeout_ms = -1)
    {
        std::vector<event_entry> events;
        int                      rc = poll(events, timeout_ms);
        if(rc <= 0)
            return rc;

        for(auto &ev : events)
        {
            if(auto pcb = std::get_if<callback_t>(&ev.user_data))
            {
                if(*pcb)
                    (*pcb)(ev);
            }
        }
        return rc;
    }

  private:
    bool remove_impl(void *sock_ptr, zmq_fd_t fd, bool is_sock)
    {
        for(size_t i = 0; i < _items.size(); ++i)
        {
            if((is_sock && _items[i].socket == sock_ptr)
               || (!is_sock && _items[i].fd == fd))
            {
                _items.erase(_items.begin() + static_cast<ptrdiff_t>(i));
                _user_datas.erase(_user_datas.begin()
                                  + static_cast<ptrdiff_t>(i));
                return true;
            }
        }
        return false;
    }

    bool modify_impl(void *sock_ptr, zmq_fd_t fd, bool is_sock, short events)
    {
        for(size_t i = 0; i < _items.size(); ++i)
        {
            if((is_sock && _items[i].socket == sock_ptr)
               || (!is_sock && _items[i].fd == fd))
            {
                _items[i].events = events;
                return true;
            }
        }
        return false;
    }

  private:
    std::vector<zmq_pollitem_t> _items;
    std::vector<user_data_t>    _user_datas;
};

} // namespace hj::zmq

#endif // ZMQ_HPP