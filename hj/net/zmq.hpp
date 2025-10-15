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

#include <string>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <vector>
#include <functional>
#include <unordered_map>

#include <zmq.h>

namespace hj
{
namespace zmq
{

class context
{
  public:
    context()
        : _ctx(zmq_ctx_new())
    {
        if(!_ctx)
            throw std::runtime_error("zmq_ctx_new failed");
    }
    context(const context &) = delete;
    context(context &&other) noexcept
        : _ctx(other._ctx)
    {
        other._ctx = nullptr;
    }

    ~context()
    {
        if(_ctx)
            zmq_ctx_term(_ctx);
    }

    context &operator=(const context &) = delete;
    context &operator=(context &&other) noexcept
    {
        if(this == &other)
            return *this;

        if(_ctx)
            zmq_ctx_term(_ctx);

        _ctx       = other._ctx;
        other._ctx = nullptr;
        return *this;
    }
    void *get() const { return _ctx; }

  private:
    void *_ctx;
}; // context

class message
{
  public:
    message()
    {
        if(zmq_msg_init(&_msg) != 0)
            throw std::runtime_error("zmq_msg_init failed");
    }
    explicit message(size_t size)
    {
        if(zmq_msg_init_size(&_msg, size) != 0)
            throw std::runtime_error("zmq_msg_init_size failed");
    }
    message(const message &) = delete;
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
    size_t size() const noexcept { return zmq_msg_size(&_msg); }

  private:
    zmq_msg_t _msg;
}; // message

class socket
{
  public:
    socket(void *ctx, int type)
        : _sock(zmq_socket(ctx, type))
    {
        if(!_sock)
            throw std::runtime_error("zmq_socket failed");
    }
    socket(const socket &) = delete;
    socket(socket &&other) noexcept
        : _sock(other._sock)
    {
        other._sock = nullptr;
    }

    ~socket()
    {
        if(_sock)
            zmq_close(_sock);
    }

    socket &operator=(const socket &) = delete;
    socket &operator=(socket &&other) noexcept
    {
        if(this != &other)
        {
            if(_sock)
                zmq_close(_sock);
            _sock       = other._sock;
            other._sock = nullptr;
        }
        return *this;
    }

    inline void *get() const noexcept { return _sock; }

  private:
    void *_sock;
};

class chan
{
  public:
    explicit chan(void *ctx, int flags = ZMQ_DONTWAIT)
        : _flags{flags}
        , _ctx{ctx}
        , _sock_in{zmq_socket(ctx, ZMQ_PAIR)}
        , _sock_out{zmq_socket(ctx, ZMQ_PAIR)}
    {
        if(!_sock_in || !_sock_out)
            assert(false);

        std::ostringstream ss;
        ss << std::hex << this;
        std::string id = "inproc://" + ss.str();
        if(0 != zmq_bind(_sock_in, id.c_str())
           || 0 != zmq_connect(_sock_out, id.c_str()))
        {
            zmq_close(_sock_in);
            _sock_in = nullptr;

            zmq_close(_sock_out);
            _sock_out = nullptr;

            assert(false);
        }

        zmq_msg_init(&_buf_in);
        zmq_msg_init(&_buf_out);
    }
    ~chan()
    {
        if(_sock_in != nullptr)
            zmq_close(_sock_in);

        if(_sock_out != nullptr)
            zmq_close(_sock_out);

        zmq_msg_close(&_buf_in);
        zmq_msg_close(&_buf_out);
    }

    chan(const chan &)            = delete;
    chan &operator=(const chan &) = delete;
    chan(chan &&)                 = default;
    chan &operator=(chan &&)      = default;

    inline bool operator>>(zmq_msg_t &dst)
    {
        return zmq_msg_recv(&dst, _sock_out, _flags) >= 0;
    }

    inline bool operator>>(std::string &dst)
    {
        int nbytes = zmq_msg_recv(&_buf_out, _sock_out, _flags);
        if(nbytes < 0)
            return false;

        dst.reserve(nbytes);
        dst.assign(static_cast<char *>(zmq_msg_data(&_buf_out)), nbytes);
        return true;
    }

    inline bool operator<<(zmq_msg_t &src)
    {
        return zmq_msg_send(&src, _sock_in, _flags) >= 0;
    }

    inline bool operator<<(const std::string &src)
    {
        if(zmq_msg_init_size(&_buf_in, src.size()) != 0)
            return false;

        memcpy(zmq_msg_data(&_buf_in), src.data(), src.size());
        return zmq_msg_send(&_buf_in, _sock_in, _flags) >= 0;
    }

  private:
    int       _flags;
    void     *_ctx;
    void     *_sock_in;
    void     *_sock_out;
    zmq_msg_t _buf_out;
    zmq_msg_t _buf_in;
}; // chan

class broker
{
  public:
    broker(void *back, void *front)
        : _back{back}
        , _front{front}
    {
    }
    virtual ~broker()
    {
        if(_back != nullptr)
            _back = nullptr;

        if(_front != nullptr)
            _front = nullptr;
    }

    broker(const broker &)            = delete;
    broker &operator=(const broker &) = delete;
    broker(broker &&)                 = delete;
    broker &operator=(broker &&)      = delete;

    inline int bind(const std::string &back_addr, const std::string &front_addr)
    {
        int ret = zmq_bind(_back, back_addr.c_str());
        if(ret != 0)
            return ret;

        ret = zmq_bind(_front, front_addr.c_str());
        if(ret != 0)
            return ret;

        return 0;
    }

    inline int proxy(void *capture = nullptr)
    {
        return zmq_proxy(_front, _back, capture);
    }

  private:
    void *_back;
    void *_front;
}; // broker

class consumer
{
  public:
    consumer(void *ctx)
        : _ctx{ctx}
        , _sock{zmq_socket(ctx, ZMQ_PULL)}
    {
        zmq_msg_init(&_buf);
    }
    ~consumer()
    {
        zmq_close(_sock);
        _sock = nullptr;

        zmq_msg_close(&_buf);
    }

    consumer(const consumer &)            = delete;
    consumer &operator=(const consumer &) = delete;
    consumer(consumer &&)                 = delete;
    consumer &operator=(consumer &&)      = delete;

    inline int set_opt(const int opt, const int value)
    {
        return zmq_setsockopt(_sock, opt, &value, sizeof(value));
    }

    inline int connect(const std::string &addr)
    {
        return zmq_connect(_sock, addr.c_str());
    }

    inline int disconnect(const std::string &addr)
    {
        return zmq_disconnect(_sock, addr.c_str());
    }

    int pull(std::string &dst, int flags = 0)
    {
        zmq_msg_init(&_buf);
        int nbytes = zmq_msg_recv(&_buf, _sock, flags);
        if(nbytes < 0)
            return nbytes;

        dst.reserve(nbytes);
        dst.assign(static_cast<char *>(zmq_msg_data(&_buf)), nbytes);
        return nbytes;
    }

    inline int pull(zmq_msg_t &data, int flags = 0)
    {
        return zmq_msg_recv(&data, _sock, flags);
    }

  private:
    void     *_ctx;
    void     *_sock;
    zmq_msg_t _buf;
}; // consumer

class producer
{
  public:
    producer(void *ctx)
        : _ctx{ctx}
        , _sock{zmq_socket(ctx, ZMQ_PUSH)}
        , _ch{zmq::chan(ctx)}
    {
        zmq_msg_init(&_buf);
    }
    ~producer()
    {
        zmq_close(_sock);
        _sock = nullptr;

        zmq_msg_close(&_buf);
    }

    producer(const producer &)            = delete;
    producer &operator=(const producer &) = delete;
    producer(producer &&)                 = delete;
    producer &operator=(producer &&)      = delete;

    inline int set_opt(const int opt, const int value)
    {
        return zmq_setsockopt(_sock, opt, &value, sizeof(value));
    }

    inline int bind(const std::string &addr)
    {
        return zmq_bind(_sock, addr.c_str());
    }

    inline int push(const std::string &str, const int flags = 0)
    {
        return zmq_send(_sock, str.data(), str.size(), flags);
    }

    inline int push(zmq_msg_t &data, int flags = 0)
    {
        return zmq_msg_send(&data, _sock, flags);
    }

    int safe_push(const std::string &str, int flags = 0)
    {
        _ch << str;
        _ch >> _buf;
        return zmq_msg_send(&_buf, _sock, flags);
    }

    int safe_push(zmq_msg_t &data, int flags = 0)
    {
        _ch << data;
        _ch >> _buf;
        return zmq_msg_send(&_buf, _sock, flags);
    }

  private:
    void         *_ctx;
    void         *_sock;
    hj::zmq::chan _ch;
    zmq_msg_t     _buf;
}; // producer

class publisher
{
  public:
    publisher(void *ctx)
        : _ctx{ctx}
        , _sock{zmq_socket(ctx, ZMQ_PUB)}
        , _ch{hj::zmq::chan(ctx)}
    {
        zmq_msg_init(&_buf);
    }
    ~publisher()
    {
        zmq_close(_sock);
        _sock = nullptr;

        zmq_msg_close(&_buf);
    }

    publisher(const publisher &)            = delete;
    publisher &operator=(const publisher &) = delete;
    publisher(publisher &&)                 = delete;
    publisher &operator=(publisher &&)      = delete;

    inline int set_opt(const int opt, const int value)
    {
        return zmq_setsockopt(_sock, opt, &value, sizeof(value));
    }

    inline int bind(const std::string &addr)
    {
        return zmq_bind(_sock, addr.c_str());
    }

    inline int bind_broker(const std::string &addr)
    {
        return zmq_connect(_sock, addr.c_str());
    }

    inline int pub(const std::string &str, const int flags = 0)
    {
        return zmq_send(_sock, str.data(), str.size(), flags);
    }

    inline int pub(zmq_msg_t &data, int flags = 0)
    {
        return zmq_msg_send(&data, _sock, flags);
    }

    int safe_pub(const std::string &str, int flags = 0)
    {
        _ch << str;
        _ch >> _buf;
        return zmq_msg_send(&_buf, _sock, flags);
    }

    int safe_pub(zmq_msg_t &data, int flags = 0)
    {
        _ch << data;
        _ch >> _buf;
        return zmq_msg_send(&_buf, _sock, flags);
    }

  private:
    void         *_ctx;
    void         *_sock;
    hj::zmq::chan _ch;
    zmq_msg_t     _buf;
}; // publisher

class subscriber
{
  public:
    subscriber(void *ctx)
        : _ctx{ctx}
        , _sock{zmq_socket(ctx, ZMQ_SUB)}
    {
        zmq_msg_init(&_buf);
    }
    ~subscriber()
    {
        zmq_close(_sock);
        _sock = nullptr;

        zmq_msg_close(&_buf);
    }

    subscriber(const subscriber &)            = delete;
    subscriber &operator=(const subscriber &) = delete;
    subscriber(subscriber &&)                 = delete;
    subscriber &operator=(subscriber &&)      = delete;

    inline int set_opt(const int opt, const int value)
    {
        return zmq_setsockopt(_sock, opt, &value, sizeof(value));
    }

    inline int connect(const std::string &addr)
    {
        return zmq_connect(_sock, addr.c_str());
    }

    inline int disconnect(const std::string &addr)
    {
        return zmq_disconnect(_sock, addr.c_str());
    }

    inline int sub(const std::string &topic)
    {
        return zmq_setsockopt(_sock,
                              ZMQ_SUBSCRIBE,
                              topic.c_str(),
                              topic.length());
    }

    int recv(std::string &dst, int flags = 0)
    {
        zmq_msg_init(&_buf);
        int nbytes = zmq_msg_recv(&_buf, _sock, flags);
        if(nbytes < 0)
            return nbytes;

        dst.reserve(nbytes);
        dst.assign(static_cast<char *>(zmq_msg_data(&_buf)), nbytes);
        return nbytes;
    }

    inline int recv(zmq_msg_t &data, int flags = 0)
    {
        return zmq_msg_recv(&data, _sock, flags);
    }

  private:
    void     *_ctx;
    void     *_sock;
    zmq_msg_t _buf;
}; // subscriber

} // namespace zmq
} // namespace hj

#endif