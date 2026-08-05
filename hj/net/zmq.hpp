/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
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
#include <cstring>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
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

    int error_number() const noexcept { return _errnum; }

  private:
    int _errnum;
};

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

    message &operator=(message &&rhs) noexcept
    {
        if(this != &rhs)
        {
            zmq_msg_close(&_msg);
            zmq_msg_init(&_msg);
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
// Base Socket Class (Handles Option, Lifetime and Multipart Messaging)
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

    void set_linger(int ms) { set_opt(ZMQ_LINGER, ms); }

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
                return false;
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
            zmq_msg_t msg;
            if(zmq_msg_init(&msg) != 0)
                return false;

            int rc = zmq_msg_recv(&msg, _sock, flags);
            if(rc < 0)
            {
                zmq_msg_close(&msg);
                if(errno == EINTR)
                    continue;
                return false;
            }

            dst.emplace_back(static_cast<char *>(zmq_msg_data(&msg)),
                             zmq_msg_size(&msg));

            int64_t more     = 0;
            size_t  more_len = sizeof(more);
            zmq_getsockopt(_sock, ZMQ_RCVMORE, &more, &more_len);
            zmq_msg_close(&msg);

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
    explicit r_chan(context::ptr ctx, const std::string &addr, int flags = 0)
        : socket_base(std::move(ctx), ZMQ_PAIR)
        , _flags{flags}
        , _addr{addr}
    {
        if(0 != zmq_connect(_sock, addr.c_str()))
            throw zmq_error("zmq_connect failed for r_chan");
    }

    inline bool operator>>(zmq_msg_t &dst)
    {
        while(true)
        {
            int rc = zmq_msg_recv(&dst, _sock, _flags);
            if(rc >= 0)
                return true;
            if(errno == EINTR)
                continue;

            zmq_msg_close(&dst);
            return false;
        }
    }

    inline bool operator>>(std::string &dst)
    {
        zmq_msg_t msg;
        if(zmq_msg_init(&msg) != 0)
            return false;

        while(true)
        {
            int nbytes = zmq_msg_recv(&msg, _sock, _flags);
            if(nbytes >= 0)
            {
                dst.assign(static_cast<char *>(zmq_msg_data(&msg)), nbytes);
                zmq_msg_close(&msg);
                return true;
            }
            if(errno == EINTR)
                continue;

            zmq_msg_close(&msg);
            return false;
        }
    }

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

    inline bool operator<<(zmq_msg_t &src)
    {
        while(true)
        {
            if(zmq_msg_send(&src, _sock, _flags) >= 0)
                return true;
            if(errno == EINTR)
                continue;

            zmq_msg_close(&src);
            return false;
        }
    }

    inline bool operator<<(std::string_view src)
    {
        zmq_msg_t msg;
        if(zmq_msg_init_size(&msg, src.size()) != 0)
            return false;

        if(!src.empty())
            std::memcpy(zmq_msg_data(&msg), src.data(), src.size());

        return *this << msg;
    }

    inline r_chan make_r_chan(int flags = 0) const
    {
        return r_chan(_ctx, _addr, flags);
    }

  private:
    int         _flags;
    std::string _addr;
};

// -----------------------------------------------------------------------------
// Broker
// -----------------------------------------------------------------------------
class broker
{
  public:
    broker(void *back, void *front)
        : _back{back}
        , _front{front}
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

    inline int bind(const std::string &back_addr, const std::string &front_addr)
    {
        int ret = zmq_bind(_back, back_addr.c_str());
        if(ret != 0)
            return ret;

        return zmq_bind(_front, front_addr.c_str());
    }

    inline int proxy(void *capture = nullptr)
    {
        while(true)
        {
            int rc = zmq_proxy(_front, _back, capture);
            if(rc == 0)
                return 0;
            if(errno == EINTR)
                continue;
            return rc;
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

    inline int connect(const std::string &addr)
    {
        return zmq_connect(_sock, addr.c_str());
    }

    inline int disconnect(const std::string &addr)
    {
        return zmq_disconnect(_sock, addr.c_str());
    }

    inline int pull(std::string &dst, int flags = 0)
    {
        zmq_msg_t msg;
        if(zmq_msg_init(&msg) != 0)
            return -1;

        while(true)
        {
            int nbytes = zmq_msg_recv(&msg, _sock, flags);
            if(nbytes >= 0)
            {
                dst.assign(static_cast<char *>(zmq_msg_data(&msg)), nbytes);
                zmq_msg_close(&msg);
                return nbytes;
            }
            if(errno == EINTR)
                continue;

            zmq_msg_close(&msg);
            return -1;
        }
    }

    inline int pull(zmq_msg_t &data, int flags = 0)
    {
        while(true)
        {
            int rc = zmq_msg_recv(&data, _sock, flags);
            if(rc >= 0 || errno != EINTR)
                return rc;
        }
    }

    inline void safe_pull(w_chan &ch, const int flags = 0)
    {
        while(true)
        {
            zmq_msg_t buf;
            if(zmq_msg_init(&buf) != 0)
                break;

            if(pull(buf, flags) < 0)
            {
                zmq_msg_close(&buf);
                break;
            }

            bool is_poison_pill = (zmq_msg_size(&buf) == 0);
            if(!ch.operator<<(buf))
            {
                zmq_msg_close(&buf);
                break;
            }

            if(is_poison_pill)
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

    inline int bind(const std::string &addr)
    {
        return zmq_bind(_sock, addr.c_str());
    }

    inline int push(std::string_view str, const int flags = 0)
    {
        while(true)
        {
            int rc = zmq_send(_sock, str.data(), str.size(), flags);
            if(rc >= 0 || errno != EINTR)
                return rc;
        }
    }

    inline int push(zmq_msg_t &data, int flags = 0)
    {
        while(true)
        {
            int rc = zmq_msg_send(&data, _sock, flags);
            if(rc >= 0 || errno != EINTR)
                return rc;
        }
    }

    inline void safe_push(r_chan &ch, const int flags = 0)
    {
        while(true)
        {
            zmq_msg_t buf;
            if(zmq_msg_init(&buf) != 0)
                break;

            if(!ch.operator>>(buf))
            {
                zmq_msg_close(&buf);
                break;
            }

            bool is_poison_pill = (zmq_msg_size(&buf) == 0);

            if(push(buf, flags) < 0)
            {
                zmq_msg_close(&buf);
                break;
            }

            if(is_poison_pill)
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

    inline int bind(const std::string &addr)
    {
        return zmq_bind(_sock, addr.c_str());
    }

    inline int bind_broker(const std::string &addr)
    {
        return zmq_connect(_sock, addr.c_str());
    }

    inline int pub(std::string_view str, const int flags = 0)
    {
        while(true)
        {
            int rc = zmq_send(_sock, str.data(), str.size(), flags);
            if(rc >= 0 || errno != EINTR)
                return rc;
        }
    }

    inline int pub(zmq_msg_t &data, int flags = 0)
    {
        while(true)
        {
            int rc = zmq_msg_send(&data, _sock, flags);
            if(rc >= 0 || errno != EINTR)
                return rc;
        }
    }

    inline void safe_pub(r_chan &ch, const int flags = 0)
    {
        while(true)
        {
            zmq_msg_t buf;
            if(zmq_msg_init(&buf) != 0)
                break;

            if(!ch.operator>>(buf))
            {
                zmq_msg_close(&buf);
                break;
            }

            bool is_poison_pill = (zmq_msg_size(&buf) == 0);

            if(pub(buf, flags) < 0)
            {
                zmq_msg_close(&buf);
                break;
            }

            if(is_poison_pill)
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

    inline int connect(const std::string &addr)
    {
        return zmq_connect(_sock, addr.c_str());
    }

    inline int disconnect(const std::string &addr)
    {
        return zmq_disconnect(_sock, addr.c_str());
    }

    inline int sub(std::string_view topic)
    {
        return zmq_setsockopt(_sock, ZMQ_SUBSCRIBE, topic.data(), topic.size());
    }

    inline int sub(const void *topic, size_t len)
    {
        return zmq_setsockopt(_sock, ZMQ_SUBSCRIBE, topic, len);
    }

    inline int unsub(std::string_view topic)
    {
        return zmq_setsockopt(_sock,
                              ZMQ_UNSUBSCRIBE,
                              topic.data(),
                              topic.size());
    }

    inline int unsub(const void *topic, size_t len)
    {
        return zmq_setsockopt(_sock, ZMQ_UNSUBSCRIBE, topic, len);
    }

    inline int recv(std::string &dst, int flags = 0)
    {
        zmq_msg_t msg;
        if(zmq_msg_init(&msg) != 0)
            return -1;

        while(true)
        {
            int nbytes = zmq_msg_recv(&msg, _sock, flags);
            if(nbytes >= 0)
            {
                dst.assign(static_cast<char *>(zmq_msg_data(&msg)), nbytes);
                zmq_msg_close(&msg);
                return nbytes;
            }
            if(errno == EINTR)
                continue;

            zmq_msg_close(&msg);
            return -1;
        }
    }

    inline int recv(zmq_msg_t &data, int flags = 0)
    {
        while(true)
        {
            int rc = zmq_msg_recv(&data, _sock, flags);
            if(rc >= 0 || errno != EINTR)
                return rc;
        }
    }

    inline void safe_recv(w_chan &ch, const int flags = 0)
    {
        while(true)
        {
            zmq_msg_t buf;
            if(zmq_msg_init(&buf) != 0)
                break;

            if(recv(buf, flags) < 0)
            {
                zmq_msg_close(&buf);
                break;
            }

            bool is_poison_pill = (zmq_msg_size(&buf) == 0);

            if(!ch.operator<<(buf))
            {
                zmq_msg_close(&buf);
                break;
            }

            if(is_poison_pill)
                break;
        }
    }
};

// -----------------------------------------------------------------------------
// Poller
// -----------------------------------------------------------------------------
class poller
{
  public:
    poller()  = default;
    ~poller() = default; // Non-virtual destructor

    poller(const poller &)            = delete;
    poller &operator=(const poller &) = delete;

    poller(poller &&other) noexcept
        : _items(std::move(other._items))
        , _user_datas(std::move(other._user_datas))
    {
    }

    poller &operator=(poller &&other) noexcept
    {
        if(this != &other)
        {
            _items      = std::move(other._items);
            _user_datas = std::move(other._user_datas);
        }
        return *this;
    }

    inline size_t size() const noexcept { return _items.size(); }

    template <typename T = void>
    inline int add(const socket_base &sock,
                   T                 *user_data = nullptr,
                   short              events    = ZMQ_POLLIN | ZMQ_POLLERR)
    {
        return _add(sock.get(), 0, events, static_cast<void *>(user_data));
    }

    template <typename T = void>
    inline int add(zmq_fd_t fd, T *user_data, short events)
    {
        return _add(nullptr, fd, events, static_cast<void *>(user_data));
    }

    inline int modify(const socket_base &sock, short events)
    {
        return _modify(sock.get(), 0, events);
    }

    inline int modify(zmq_fd_t fd, short events)
    {
        return _modify(nullptr, fd, events);
    }

    inline int remove(const socket_base &sock)
    {
        return _remove(sock.get(), 0);
    }
    inline int remove(zmq_fd_t fd) { return _remove(nullptr, fd); }

    inline int poll(long timeout_ms = -1)
    {
        if(_items.empty())
            return 0;

        while(true)
        {
            int rc = zmq_poll(_items.data(),
                              static_cast<int>(_items.size()),
                              timeout_ms);
            if(rc >= 0 || errno != EINTR)
                return rc;
        }
    }

    inline int wait(long timeout_ms = -1) { return poll(timeout_ms); }

    template <typename T = void>
    inline int wait_for(T **user_data, short *events, long timeout_ms = -1)
    {
        int rc = poll(timeout_ms);
        if(rc <= 0)
            return rc;

        for(size_t i = 0; i < _items.size(); ++i)
        {
            if(_items[i].revents == 0)
                continue;

            if(user_data)
                *user_data = static_cast<T *>(_user_datas[i]);
            if(events)
                *events = _items[i].revents;
            break;
        }
        return rc;
    }

    inline const std::vector<zmq_pollitem_t> &items() const noexcept
    {
        return _items;
    }

    inline void *user_data(size_t index) const noexcept
    {
        return _user_datas[index];
    }

  private:
    inline int
    _add(void *socket_ptr, zmq_fd_t fd, short events, void *user_data)
    {
        zmq_pollitem_t item;
        item.socket  = socket_ptr;
        item.fd      = fd;
        item.events  = events;
        item.revents = 0;

        _items.push_back(item);
        _user_datas.push_back(user_data);
        return 0;
    }

    inline int _modify(void *socket_ptr, zmq_fd_t fd, short events)
    {
        for(size_t i = 0; i < _items.size(); ++i)
        {
            if((socket_ptr && _items[i].socket == socket_ptr)
               || (!socket_ptr && _items[i].fd == fd))
            {
                _items[i].events = events;
                return 0;
            }
        }
        return -1;
    }

    inline int _remove(void *socket_ptr, zmq_fd_t fd)
    {
        for(size_t i = 0; i < _items.size(); ++i)
        {
            if((socket_ptr && _items[i].socket == socket_ptr)
               || (!socket_ptr && _items[i].fd == fd))
            {
                _items.erase(_items.begin() + i);
                _user_datas.erase(_user_datas.begin() + i);
                return 0;
            }
        }
        return -1;
    }

    std::vector<zmq_pollitem_t> _items;
    std::vector<void *>         _user_datas;
};

} // namespace zmq
} // namespace hj

#endif // ZMQ_HPP