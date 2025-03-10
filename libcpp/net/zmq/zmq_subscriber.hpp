#ifndef ZMQ_SUBSCRIBER_HPP
#define ZMQ_SUBSCRIBER_HPP

#include <zmq.h>
#include <iostream>
#include <libcpp/net/zmq/zmq_chan.hpp>

namespace libcpp
{

class zmq_subscriber
{
public:
    zmq_subscriber(void* ctx)
        : _ctx{ctx}
        , _sock{zmq_socket(ctx, ZMQ_SUB)}
    {
    }
    ~zmq_subscriber()
    {
        zmq_close(_sock);
		_sock = nullptr;
    }

    inline int set_opt(const int opt, const int value)
    {
        return zmq_setsockopt(_sock, opt, &value, sizeof(value));
    }

    inline int connect(const std::string& addr)
    {
        return zmq_connect(_sock, addr.c_str());
    }

    inline int disconnect(const std::string& addr)
    {
        return zmq_disconnect(_sock, addr.c_str());
    }

    inline int sub(const std::string& topic)
    {
        return zmq_setsockopt(_sock, ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
    }

    int recv(std::string& dst, bool dont_block = false)
    {
        int len = zmq_recv(_sock, &_buf, zmq_msg_size(&_buf), (dont_block ? ZMQ_NOBLOCK : 0));
        if (len < 0)
            return len;

        dst.reserve(len);
        dst.assign(static_cast<char*>(zmq_msg_data(&_buf)), len);
        return len;
    }

private:
    void*            _ctx;
    void*            _sock;
    zmq_msg_t        _buf;
};

}

#endif