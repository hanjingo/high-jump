#ifndef ZMQ_SUBSCRIBER_HPP
#define ZMQ_SUBSCRIBER_HPP

#include <zmq.h>
#include <iostream>
#include <libcpp/misc/zmq/zmq_chan.hpp>

namespace libcpp
{

class zmq_subscriber
{
public:
    zmq_subscriber(void* ctx)
        : ctx_{ctx}
        , sock_{zmqsock_et(ctx, ZMQ_SUB)}
    {
        zmq_msg_init(&buf_);
    }
    ~zmq_subscriber()
    {
        zmq_close(sock_);
		sock_ = nullptr;

        zmq_msg_close(&buf_);
    }

    inline int set_opt(const int opt, const int value)
    {
        return zmq_setsockopt(sock_, opt, &value, sizeof(value));
    }

    inline int connect(const std::string& addr)
    {
        return zmq_connect(sock_, addr.c_str());
    }

    inline int disconnect(const std::string& addr)
    {
        return zmq_disconnect(sock_, addr.c_str());
    }

    inline int sub(const std::string& topic)
    {
        return zmq_setsockopt(sock_, ZMQ_SUBSCRIBE, topic.c_str(), topic.length());
    }

    int recv(std::string& dst, int flags = 0)
    {
        zmq_msg_init(&buf_);
        int nbytes = zmq_msg_recv(&buf_, sock_, flags);
        if (nbytes < 0)
            return nbytes;

        dst.reserve(nbytes);
        dst.assign(static_cast<char*>(zmq_msg_data(&buf_)), nbytes);
        return nbytes;
    }

    inline int recv(zmq_msg_t& data, int flags = 0)
    {
        return zmq_msg_recv(&data, sock_, flags);
    }

private:
    void*            ctx_;
    void*            sock_;
    zmq_msg_t        buf_;
};

}

#endif