#ifndef ZMQ_ROUTER_HPP
#define ZMQ_ROUTER_HPP

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include <zmq.h>
#include <zmqpp/zmqpp.hpp>
#include <zmqpp/proxy.hpp>
#include <zmqpp/poller.hpp>
#include <zmqpp/loop.hpp>

namespace libcpp
{

using zmq_option = zmqpp::socket_option;
class zmq_router
{
public:
    zmq_router(zmqpp::context& ctx)
        : ctx_{ctx}
        , loop_{}
        , sock_{ctx, zmqpp::socket_type::router}
    {}
    virtual ~zmq_router()
    {
        sock_.close();
    }

    template<typename T>
    inline void set(const zmq_option opt, T value)
    {
        sock_.set(opt, value);
    }

    inline void bind(const std::string& addr)
    {
        sock_.bind(addr);

        loop_.add(sock_, std::bind(&zmq_router::on_route, this));
    }

    inline void loop_start()
    {
        loop_.start();
    }

    inline zmqpp::socket& socket()
    {
        return sock_;
    }

public:
    virtual bool on_route()
    {
        zmqpp::message msg;
        if (!sock_.receive(msg, true)) {
            assert(false);
            return false;
        }

        sock_.send(msg.get(1), zmqpp::socket::send_more);
        return sock_.send(msg, true);
    }

private:
    zmqpp::context&   ctx_;
    zmqpp::loop       loop_;
    zmqpp::socket     sock_;
};

}

#endif