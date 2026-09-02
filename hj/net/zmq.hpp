/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025-2026 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ZMQ_HPP
#define ZMQ_HPP

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <zmq.h>

namespace hj
{
namespace zmq
{

// -----------------------------------------------------------------------------
// Exception Handling
// -----------------------------------------------------------------------------
class zmq_error : public std::runtime_error
{
  public:
    explicit zmq_error(const std::string &prefix)
        : std::runtime_error(prefix + ": " + zmq_strerror(zmq_errno()))
        , _errnum(zmq_errno())
    {
    }

    explicit zmq_error(const char *prefix)
        : std::runtime_error(std::string(prefix) + ": "
                             + zmq_strerror(zmq_errno()))
        , _errnum(zmq_errno())
    {
    }

    zmq_error(const std::string &prefix, int errnum)
        : std::runtime_error(prefix + ": " + zmq_strerror(errnum))
        , _errnum(errnum)
    {
    }

    int error_number() const noexcept { return _errnum; }

  private:
    int _errnum;
};

// Helper macro/function for internal error checking
inline void check_error(int result, const char *msg)
{
    if(result < 0)
    {
        if(errno != EAGAIN && errno != EINTR)
            throw zmq_error(msg);
    }
}

// -----------------------------------------------------------------------------
// Context (Managed with Shared Pointer)
// -----------------------------------------------------------------------------
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
            zmq_ctx_term(_ctx);
    }

    context &operator=(context &&other) noexcept
    {
        if(this != &other)
        {
            if(_ctx)
                zmq_ctx_term(_ctx);
            _ctx       = other._ctx;
            other._ctx = nullptr;
        }
        return *this;
    }

    void *get() const noexcept { return _ctx; }

  private:
    void *_ctx;
};

// -----------------------------------------------------------------------------
// Control Signal / Poison Pill Enum
// -----------------------------------------------------------------------------
enum class control_cmd : uint8_t
{
    DATA = 0,
    STOP = 1
};

// -----------------------------------------------------------------------------
// Message
// -----------------------------------------------------------------------------
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

    // Zero-Copy Custom Deallocator Message Construction
    message(void *data_ptr, size_t size, zmq_free_fn *ffn, void *hint = nullptr)
    {
        if(zmq_msg_init_data(&_msg, data_ptr, size, ffn, hint) != 0)
            throw zmq_error("zmq_msg_init_data failed");
    }

    message(const message &)            = delete;
    message &operator=(const message &) = delete;

    message(message &&rhs) noexcept
    {
        zmq_msg_init(&_msg);
        zmq_msg_move(&_msg, &rhs._msg);
    }

    ~message() noexcept { zmq_msg_close(&_msg); }

    // FIXED: Removed invalid zmq_msg_init call to fix memory leak
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

// -----------------------------------------------------------------------------
// Base Socket Class
// -----------------------------------------------------------------------------
class socket_base
{
  public:
    socket_base(context::ptr ctx, int type)
        : _ctx(std::move(ctx))
        , _sock(nullptr)
    {
        if(!_ctx)
            throw std::invalid_argument("context pointer cannot be null");

        _sock = zmq_socket(_ctx->get(), type);
        if(!_sock)
            throw zmq_error("zmq_socket failed");
    }

    virtual ~socket_base() noexcept
    {
        if(_sock)
            zmq_close(_sock);
    }

    socket_base(const socket_base &)            = delete;
    socket_base &operator=(const socket_base &) = delete;

    socket_base(socket_base &&other) noexcept
        : _ctx(std::move(other._ctx))
        , _sock(other._sock)
    {
        other._sock = nullptr;
    }

    socket_base &operator=(socket_base &&other) noexcept
    {
        if(this != &other)
        {
            if(_sock)
                zmq_close(_sock);
            _ctx        = std::move(other._ctx);
            _sock       = other._sock;
            other._sock = nullptr;
        }
        return *this;
    }

    void set_linger(int ms)
    {
        if(set_opt(ZMQ_LINGER, ms) != 0)
            throw zmq_error("set_opt ZMQ_LINGER failed");
    }

    int set_opt(int opt, int value)
    {
        return zmq_setsockopt(_sock, opt, &value, sizeof(value));
    }

    int set_opt(int opt, const void *value, size_t len)
    {
        return zmq_setsockopt(_sock, opt, value, len);
    }

    int get_opt(int opt, void *value, size_t *len) const
    {
        return zmq_getsockopt(_sock, opt, value, len);
    }

    void *get() const noexcept { return _sock; }

    // Multi-part Sending Support
    bool send_multipart(const std::vector<std::string_view> &parts,
                        int                                  flags = 0)
    {
        for(size_t i = 0; i < parts.size(); ++i)
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
                if(errno == EINTR)
                    continue;
                if(errno == EAGAIN)
                    return false;
                throw zmq_error("zmq_send failed in send_multipart");
            }
        }
        return true;
    }

    // Multi-part Receiving Support
    bool recv_multipart(std::vector<std::string> &dst, int flags = 0)
    {
        dst.clear();
        while(true)
        {
            message msg;
            int     rc = -1;
            while(true)
            {
                rc = zmq_msg_recv(msg.get(), _sock, flags);
                if(rc >= 0)
                    break;
                if(errno == EINTR)
                    continue;
                if(errno == EAGAIN)
                    return false;
                throw zmq_error("zmq_msg_recv failed in recv_multipart");
            }

            dst.emplace_back(static_cast<char *>(msg.data()), msg.size());

            int64_t more     = 0;
            size_t  more_len = sizeof(more);
            if(zmq_getsockopt(_sock, ZMQ_RCVMORE, &more, &more_len) != 0)
                throw zmq_error("getsockopt ZMQ_RCVMORE failed");

            if(!more)
                break;
        }
        return true;
    }

  protected:
    context::ptr _ctx;
    void        *_sock;
};

// Generic Socket
class socket : public socket_base
{
  public:
    socket(context::ptr ctx, int type)
        : socket_base(std::move(ctx), type)
    {
    }
};

// -----------------------------------------------------------------------------
// Channel Read End (r_chan)
// -----------------------------------------------------------------------------
class r_chan : public socket_base
{
  public:
    explicit r_chan(context::ptr       ctx,
                    const std::string &addr,
                    int                flags       = 0,
                    int                max_retries = 100)
        : socket_base(std::move(ctx), ZMQ_PAIR)
        , _flags{flags}
        , _addr{addr}
    {
        // Robust inproc connection handling with retry logic
        int retries = 0;
        while(0 != zmq_connect(_sock, addr.c_str()))
        {
            if(errno == ECONNREFUSED && retries++ < max_retries)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                continue;
            }
            throw zmq_error("zmq_connect failed for r_chan");
        }
    }

    bool recv(message &dst)
    {
        while(true)
        {
            int rc = zmq_msg_recv(dst.get(), _sock, _flags);
            if(rc >= 0)
                return true;
            if(errno == EINTR)
                continue;
            if(errno == EAGAIN)
                return false;
            throw zmq_error("r_chan recv message failed");
        }
    }

    bool recv(std::string &dst)
    {
        message msg;
        if(recv(msg))
        {
            dst.assign(static_cast<const char *>(msg.data()), msg.size());
            return true;
        }
        return false;
    }

    inline bool operator>>(message &dst) { return recv(dst); }
    inline bool operator>>(std::string &dst) { return recv(dst); }

  private:
    int         _flags;
    std::string _addr;
};

// -----------------------------------------------------------------------------
// Channel Write End (w_chan)
// -----------------------------------------------------------------------------
class w_chan : public socket_base
{
  public:
    explicit w_chan(context::ptr ctx, int flags = 0)
        : socket_base(ctx, ZMQ_PAIR)
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

    bool send(message &src)
    {
        while(true)
        {
            if(zmq_msg_send(src.get(), _sock, _flags) >= 0)
                return true;
            if(errno == EINTR)
                continue;
            if(errno == EAGAIN)
                return false;
            throw zmq_error("w_chan send message failed");
        }
    }

    bool send(std::string_view src)
    {
        message msg(src);
        return send(msg);
    }

    inline bool operator<<(message &src) { return send(src); }
    inline bool operator<<(std::string_view src) { return send(src); }

    inline r_chan make_r_chan(int flags = 0) const
    {
        return r_chan(_ctx, _addr, flags);
    }

    const std::string &address() const noexcept { return _addr; }

  private:
    int         _flags;
    std::string _addr;
};

// Pair Channel Helper Factory
inline std::pair<w_chan, r_chan> make_pair_channel(context::ptr ctx)
{
    w_chan writer(ctx);
    r_chan reader = writer.make_r_chan();
    return {std::move(writer), std::move(reader)};
}

// -----------------------------------------------------------------------------
// Broker
// -----------------------------------------------------------------------------
class broker
{
  public:
    broker(socket_base &back, socket_base &front)
        : _back{back.get()}
        , _front{front.get()}
    {
    }

    ~broker() noexcept = default;

    broker(const broker &)            = delete;
    broker &operator=(const broker &) = delete;

    broker(broker &&rhs) noexcept
        : _back{rhs._back}
        , _front{rhs._front}
    {
        rhs._back  = nullptr;
        rhs._front = nullptr;
    }

    broker &operator=(broker &&rhs) noexcept
    {
        if(this != &rhs)
        {
            _back      = rhs._back;
            _front     = rhs._front;
            rhs._back  = nullptr;
            rhs._front = nullptr;
        }
        return *this;
    }

    void bind(const std::string &back_addr, const std::string &front_addr)
    {
        if(zmq_bind(_back, back_addr.c_str()) != 0)
            throw zmq_error("broker bind backend failed");

        if(zmq_bind(_front, front_addr.c_str()) != 0)
            throw zmq_error("broker bind frontend failed");
    }

    void proxy(socket_base *capture = nullptr)
    {
        while(true)
        {
            int rc =
                zmq_proxy(_front, _back, capture ? capture->get() : nullptr);
            if(rc == 0)
                return;
            if(errno == EINTR)
                continue;
            throw zmq_error("zmq_proxy execution failed");
        }
    }

  private:
    void *_back;
    void *_front;
};

// -----------------------------------------------------------------------------
// Consumer (PULL)
// -----------------------------------------------------------------------------
class consumer : public socket_base
{
  public:
    explicit consumer(context::ptr ctx)
        : socket_base(std::move(ctx), ZMQ_PULL)
    {
    }

    void connect(const std::string &addr)
    {
        if(zmq_connect(_sock, addr.c_str()) != 0)
            throw zmq_error("consumer connect failed");
    }

    void disconnect(const std::string &addr)
    {
        if(zmq_disconnect(_sock, addr.c_str()) != 0)
            throw zmq_error("consumer disconnect failed");
    }

    std::optional<std::string> pull_string(int flags = 0)
    {
        message msg;
        if(!pull(msg, flags))
            return std::nullopt;
        return std::string(static_cast<const char *>(msg.data()), msg.size());
    }

    bool pull(message &data, int flags = 0)
    {
        while(true)
        {
            int rc = zmq_msg_recv(data.get(), _sock, flags);
            if(rc >= 0)
                return true;
            if(errno == EINTR)
                continue;
            if(errno == EAGAIN)
                return false;
            throw zmq_error("consumer pull failed");
        }
    }

    void safe_pull(w_chan &ch, control_cmd stop_cmd = control_cmd::STOP)
    {
        while(true)
        {
            message buf;
            if(!pull(buf, 0))
                break;

            bool is_stop = false;
            if(buf.size() == sizeof(control_cmd))
            {
                control_cmd cmd;
                std::memcpy(&cmd, buf.data(), sizeof(control_cmd));
                if(cmd == stop_cmd)
                    is_stop = true;
            }

            if(!ch.send(buf))
                break;

            if(is_stop)
                break;
        }
    }
};

// -----------------------------------------------------------------------------
// Producer (PUSH)
// -----------------------------------------------------------------------------
class producer : public socket_base
{
  public:
    explicit producer(context::ptr ctx)
        : socket_base(std::move(ctx), ZMQ_PUSH)
    {
    }

    void bind(const std::string &addr)
    {
        if(zmq_bind(_sock, addr.c_str()) != 0)
            throw zmq_error("producer bind failed");
    }

    bool push(std::string_view str, int flags = 0)
    {
        message msg(str);
        return push(msg, flags);
    }

    bool push(message &data, int flags = 0)
    {
        while(true)
        {
            int rc = zmq_msg_send(data.get(), _sock, flags);
            if(rc >= 0)
                return true;
            if(errno == EINTR)
                continue;
            if(errno == EAGAIN)
                return false;
            throw zmq_error("producer push failed");
        }
    }

    void safe_push(r_chan &ch, control_cmd stop_cmd = control_cmd::STOP)
    {
        while(true)
        {
            message buf;
            if(!ch.recv(buf))
                break;

            bool is_stop = false;
            if(buf.size() == sizeof(control_cmd))
            {
                control_cmd cmd;
                std::memcpy(&cmd, buf.data(), sizeof(control_cmd));
                if(cmd == stop_cmd)
                    is_stop = true;
            }

            if(!push(buf, 0))
                break;

            if(is_stop)
                break;
        }
    }
};

// -----------------------------------------------------------------------------
// Publisher (PUB)
// -----------------------------------------------------------------------------
class publisher : public socket_base
{
  public:
    explicit publisher(context::ptr ctx)
        : socket_base(std::move(ctx), ZMQ_PUB)
    {
    }

    void bind(const std::string &addr)
    {
        if(zmq_bind(_sock, addr.c_str()) != 0)
            throw zmq_error("publisher bind failed");
    }

    void connect(const std::string &addr)
    {
        if(zmq_connect(_sock, addr.c_str()) != 0)
            throw zmq_error("publisher connect failed");
    }

    bool pub(std::string_view str, int flags = 0)
    {
        message msg(str);
        return pub(msg, flags);
    }

    bool pub(message &data, int flags = 0)
    {
        while(true)
        {
            int rc = zmq_msg_send(data.get(), _sock, flags);
            if(rc >= 0)
                return true;
            if(errno == EINTR)
                continue;
            if(errno == EAGAIN)
                return false;
            throw zmq_error("publisher pub failed");
        }
    }

    void safe_pub(r_chan &ch, control_cmd stop_cmd = control_cmd::STOP)
    {
        while(true)
        {
            message buf;
            if(!ch.recv(buf))
                break;

            bool is_stop = false;
            if(buf.size() == sizeof(control_cmd))
            {
                control_cmd cmd;
                std::memcpy(&cmd, buf.data(), sizeof(control_cmd));
                if(cmd == stop_cmd)
                    is_stop = true;
            }

            if(!pub(buf, 0))
                break;

            if(is_stop)
                break;
        }
    }
};

// -----------------------------------------------------------------------------
// Subscriber (SUB)
// -----------------------------------------------------------------------------
class subscriber : public socket_base
{
  public:
    explicit subscriber(context::ptr ctx)
        : socket_base(std::move(ctx), ZMQ_SUB)
    {
    }

    void connect(const std::string &addr)
    {
        if(zmq_connect(_sock, addr.c_str()) != 0)
            throw zmq_error("subscriber connect failed");
    }

    void disconnect(const std::string &addr)
    {
        if(zmq_disconnect(_sock, addr.c_str()) != 0)
            throw zmq_error("subscriber disconnect failed");
    }

    void sub(std::string_view topic)
    {
        if(zmq_setsockopt(_sock, ZMQ_SUBSCRIBE, topic.data(), topic.size())
           != 0)
            throw zmq_error("subscriber sub failed");
    }

    void unsub(std::string_view topic)
    {
        if(zmq_setsockopt(_sock, ZMQ_UNSUBSCRIBE, topic.data(), topic.size())
           != 0)
            throw zmq_error("subscriber unsub failed");
    }

    std::optional<std::string> recv_string(int flags = 0)
    {
        message msg;
        if(!recv(msg, flags))
            return std::nullopt;
        return std::string(static_cast<const char *>(msg.data()), msg.size());
    }

    bool recv(message &data, int flags = 0)
    {
        while(true)
        {
            int rc = zmq_msg_recv(data.get(), _sock, flags);
            if(rc >= 0)
                return true;
            if(errno == EINTR)
                continue;
            if(errno == EAGAIN)
                return false;
            throw zmq_error("subscriber recv failed");
        }
    }

    void safe_recv(w_chan &ch, control_cmd stop_cmd = control_cmd::STOP)
    {
        while(true)
        {
            message buf;
            if(!recv(buf, 0))
                break;

            bool is_stop = false;
            if(buf.size() == sizeof(control_cmd))
            {
                control_cmd cmd;
                std::memcpy(&cmd, buf.data(), sizeof(control_cmd));
                if(cmd == stop_cmd)
                    is_stop = true;
            }

            if(!ch.send(buf))
                break;

            if(is_stop)
                break;
        }
    }
};

// -----------------------------------------------------------------------------
// Standard Poller (Built on libzmq zmq_poll API)
// -----------------------------------------------------------------------------
class poller
{
  public:
    struct event_entry
    {
        void    *socket;
        zmq_fd_t fd;
        short    events;
        short    revents;
        void    *user_data;
    };

    poller() = default;

    template <typename T = void>
    void add(const socket_base &sock,
             T                 *user_data = nullptr,
             short              events    = ZMQ_POLLIN | ZMQ_POLLERR)
    {
        _items.push_back({sock.get(), 0, events, 0});
        _user_datas.push_back(static_cast<void *>(user_data));
    }

    template <typename T = void>
    void add(zmq_fd_t fd, T *user_data, short events)
    {
        _items.push_back({nullptr, fd, events, 0});
        _user_datas.push_back(static_cast<void *>(user_data));
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

            if(rc > 0)
            {
                for(size_t i = 0; i < _items.size(); ++i)
                {
                    if(_items[i].revents != 0)
                    {
                        active_events.push_back({_items[i].socket,
                                                 _items[i].fd,
                                                 _items[i].events,
                                                 _items[i].revents,
                                                 _user_datas[i]});
                    }
                }
            }
            return rc;
        }
    }

  private:
    std::vector<zmq_pollitem_t> _items;
    std::vector<void *>         _user_datas;
};

} // namespace zmq
} // namespace hj

#endif // ZMQ_HPP