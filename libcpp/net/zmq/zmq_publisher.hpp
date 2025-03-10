#ifndef ZMQ_PUBLISHER_HPP
#define ZMQ_PUBLISHER_HPP

#include <zmq.h>
#include <libcpp/net/zmq/zmq_chan.hpp>

namespace libcpp
{

class zmq_publisher
{
public:
    zmq_publisher(void* ctx)
        : _ctx{ctx}
        , _sock{zmq_socket(ctx, ZMQ_PUB)}
        , _ch{zmq_chan(ctx)}
    {
    }
    ~zmq_publisher()
    {
        zmq_close(_sock);
		_sock = nullptr;
    }

    inline int set_opt(const int opt, const int value)
    {
        return zmq_setsockopt(_sock, opt, &value, sizeof(value));
    }

    inline int bind(const std::string& addr)
    {
        return zmq_bind(_sock, addr.c_str());
    }

    inline int bind_broker(const std::string& addr)
    {
        return zmq_connect(_sock, addr.c_str());
    }

    inline int unsafe_pub(const std::string& buf, bool dont_block = false)
    {
        return zmq_send(_sock, buf.data(), buf.size(), (dont_block ? ZMQ_NOBLOCK : 0));
    }

    int pub(const std::string& buf, bool dont_block = false)
    {
        _ch << buf;
        std::string tmp;
        _ch >> tmp;
        return zmq_send(_sock, tmp.data(), tmp.size(), (dont_block ? ZMQ_NOBLOCK : 0));
    }

private:
    void*               _ctx;
    void*               _sock;
    libcpp::zmq_chan    _ch;
};

}

#endif