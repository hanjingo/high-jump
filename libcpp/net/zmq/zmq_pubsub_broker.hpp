#ifndef ZMQ_PUBSUB_BROKER_HPP
#define ZMQ_PUBSUB_BROKER_HPP

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <iostream>

#include <libcpp/net/zmq/zmq_chan.hpp>

namespace libcpp
{

class zmq_pubsub_broker
{
public:
    zmq_pubsub_broker(void* ctx)
        : _ctx{ctx}
        , _back{zmq_socket(ctx, ZMQ_XPUB)}
        , _front{zmq_socket(ctx, ZMQ_XSUB)}
    {}
    virtual ~zmq_pubsub_broker()
    {
        if (_back != nullptr)
        {
            zmq_close(_back);
            _back = nullptr;
        }
        if (_back != nullptr)
        {
            zmq_close(_front);
            _front = nullptr;
        }
    }

    inline int bind(const std::string& xpub_addr, const std::string& xsub_addr)
    {
        int ret = zmq_bind(_back, xpub_addr.c_str());
        if (ret != 0)
            return ret;

        ret = zmq_bind(_front, xsub_addr.c_str());
        if (ret != 0)
            return ret;

        return 0;
    }

    inline int proxy(void *capture = nullptr)
    {
        return zmq_proxy(_front, _back, capture);
    }

private:
    void*     _ctx;
    void*     _back;
    void*     _front;
};

}

#endif