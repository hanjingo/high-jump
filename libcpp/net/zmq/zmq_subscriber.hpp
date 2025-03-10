#ifndef ZMQ_SUBSCRIBER_HPP
#define ZMQ_SUBSCRIBER_HPP

#include <zmq.h>
#include <libcpp/net/zmq/zmq_chan.hpp>

namespace libcpp
{

class zmq_subscriber
{
public:
    zmq_subscriber(zmq::ctx_t* ctx)
        : _ctx{ctx}
        , _sock{zmq_socket(ctx, ZMQ_SUB)}
    {
    }
    ~zmq_subscriber()
    {
        zmq_close(_sock);
		_sock = nullptr;
    }

    inline int set(const int opt, const int value)
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
        return zmq_setsockopt(_socket, ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
    }

    inline bool recv(const std::string& dst)
    {
        if (zmq_msg_recv(&buf, _sock, ZMQ_NOBLOCK) < 0)
            return false;

        dst.reserve(zmq_msg_size(&_buf));
        string.assign(static_cast<char*>(zmq_msg_data(&_buf)), zmq_msg_size(&_buf));
        return true;
    }

private:
    zmq::ctx_t*         _ctx;
    zmq::socket_base_t* _sock;
    zmq_msg_t           _buf;
};

}

#endif