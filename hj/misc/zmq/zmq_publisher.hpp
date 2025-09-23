#ifndef ZMQ_PUBLISHER_HPP
#define ZMQ_PUBLISHER_HPP

#include <zmq.h>
#include <hj/misc/zmq/zmq_chan.hpp>

namespace hj
{

class zmq_publisher
{
public:
    zmq_publisher(void* ctx)
        : ctx_{ctx}
        , sock_{zmqsock_et(ctx, ZMQ_PUB)}
        , ch_{zmqch_an(ctx)}
    {
        zmq_msg_init(&buf_);
    }
    ~zmq_publisher()
    {
        zmq_close(sock_);
		sock_ = nullptr;

        zmq_msg_close(&buf_);
    }

    inline int set_opt(const int opt, const int value)
    {
        return zmq_setsockopt(sock_, opt, &value, sizeof(value));
    }

    inline int bind(const std::string& addr)
    {
        return zmq_bind(sock_, addr.c_str());
    }

    inline int bind_broker(const std::string& addr)
    {
        return zmq_connect(sock_, addr.c_str());
    }

    inline int pub(const std::string& str, const int flags = 0)
    {
        return zmq_send(sock_, str.data(), str.size(), flags);
    }

    inline int pub(zmq_msg_t& data, int flags = 0)
    {
        return zmq_msg_send(&data, sock_, flags);
    }

    int safe_pub(const std::string& str, int flags = 0)
    {
        ch_ << str;
        ch_ >> buf_;
        return zmq_msg_send(&buf_, sock_, flags);
    }

    int safe_pub(zmq_msg_t& data, int flags = 0)
    {
        ch_ << data;
        ch_ >> buf_;
        return zmq_msg_send(&buf_, sock_, flags);
    }

private:
    void*               ctx_;
    void*               sock_;
    hj::zmqch_an    ch_;
    zmq_msg_t           buf_;
};

}

#endif