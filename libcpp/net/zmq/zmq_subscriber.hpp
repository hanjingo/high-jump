#ifndef ZMQ_SUBSCRIBER_HPP
#define ZMQ_SUBSCRIBER_HPP

#include <libcpp/net/zmq/zmq_chan.hpp>

namespace libcpp
{

using zmq_option = zmqpp::socket_option;

class zmq_subscriber
{
public:
    zmq_subscriber(zmqpp::context& ctx)
        : ctx_{ctx}
        , loop_{}
        , sock_{ctx, zmqpp::socket_type::sub}
    {
    }
    ~zmq_subscriber()
    {
        sock_.close();
    }

    template<typename T>
    inline void set(const zmq_option opt, T value)
    {
        sock_.set(opt, value);
    }

    inline void connect(const std::string& addr, bool do_ack = false)
    {
        sock_.connect(addr);
    }

    inline void disconnect(const std::string& addr)
    {
        sock_.disconnect(addr);
    }

    inline void sub(const std::string& topic)
    {
        sock_.subscribe(topic);
    }

    inline bool recv(zmqpp::message& msg)
    {
        return sock_.receive(msg);
    }

    inline zmqpp::message recv()
    {
        zmqpp::message msg;
        sock_.receive(msg);
        return msg;
    }

    inline void loop_start()
    {
        loop_.start();
    }

    inline bool add_loop_event(std::function<bool()>&& callable)
    {
        loop_.add(sock_, std::move(callable));
        return true;
    }

    inline zmqpp::socket& socket()
    {
        return sock_;
    }

private:
    zmqpp::context& ctx_;
    zmqpp::loop     loop_;
    zmqpp::socket   sock_;
};

}

#endif