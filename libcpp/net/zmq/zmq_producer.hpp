#ifndef ZMQ_PRODUCER_HPP
#define ZMQ_PRODUCER_HPP

#include <zmq.h>
#include <libcpp/net/zmq/zmq_chan.hpp>

namespace libcpp
{

class zmq_producer
{
public:
    zmq_producer(void* ctx)
        : _ctx{ctx}
        , _sock{zmq_socket(ctx, ZMQ_PUSH)}
        , _ch{zmq_chan(ctx)}
    {
        zmq_msg_init(&_buf);
    }
    ~zmq_producer()
    {
        zmq_close(_sock);
		_sock = nullptr;

        zmq_msg_close(&_buf);
    }

    inline int set_opt(const int opt, const int value)
    {
        return zmq_setsockopt(_sock, opt, &value, sizeof(value));
    }

    inline int bind(const std::string& addr)
    {
        return zmq_bind(_sock, addr.c_str());
    }

    inline int push(const std::string& str, const int flags = 0)
    {
        return zmq_send(_sock, str.data(), str.size(), flags);
    }

    inline int push(zmq_msg_t& data, int flags = 0)
    {
        return zmq_msg_send(&data, _sock, flags);
    }

    int safe_push(const std::string& str, int flags = 0)
    {
        _ch << str;
        _ch >> _buf;
        return zmq_msg_send(&_buf, _sock, flags);
    }

    int safe_push(zmq_msg_t& data, int flags = 0)
    {
        _ch << data;
        _ch >> _buf;
        return zmq_msg_send(&_buf, _sock, flags);
    }

private:
    void*               _ctx;
    void*               _sock;
    libcpp::zmq_chan    _ch;
    zmq_msg_t           _buf;
};

}

#endif