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
        : ctx_{ctx}
        , back_{zmq_socket(ctx, ZMQ_XPUB)}
        , front_{zmq_socket(ctx, ZMQ_XSUB)}
    {}
    virtual ~zmq_pubsub_broker()
    {
        if (back_ != nullptr)
        {
            zmq_close(back_);
            back_ = nullptr;
        }
        if (back_ != nullptr)
        {
            zmq_close(front_);
            front_ = nullptr;
        }
    }

    inline int bind(const std::string& xpub_addr, const std::string& xsub_addr)
    {
        int ret = zmq_bind(back_, xpub_addr.c_str());
        if (ret != 0)
            return ret;

        ret = zmq_bind(front_, xsub_addr.c_str());
        if (ret != 0)
            return ret;

        return 0;
    }

    inline int proxy(void *capture = nullptr)
    {
        return zmq_proxy(front_, back_, capture);
    }

private:
    void*     ctx_;
    void*     back_;
    void*     front_;
};

}

#endif