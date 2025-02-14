#ifndef ZMQ_DEALER_HPP
#define ZMQ_DEALER_HPP

#include <libcpp/net/zmq/zmq_chan.hpp>

namespace libcpp
{

using zmq_option = zmqpp::socket_option;

class zmq_dealer
{
public:
    zmq_dealer(zmqpp::context& ctx, const std::string& id)
        : ctx_{ctx}
        , sock_{ctx, zmqpp::socket_type::dealer}
        , loop_{}
        , w_ch_{ctx}
    {
        sock_.set(zmqpp::socket_option::identity, id);
    }
    virtual ~zmq_dealer()
    {
        sock_.close();
    };

public:
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

    inline bool unsafe_send(zmqpp::message& msg, bool dont_block = false)
    {
        return sock_.send(msg, dont_block);
    }

    inline bool unsafe_send(const std::string& dst, const std::string& content, bool dont_block = false)
    {
        zmqpp::message msg{dst, content};
        return unsafe_send(msg, dont_block);
    }

    inline bool send(zmqpp::message& msg)
    {
        zmqpp::message tmp;
        return (w_ch_ << msg) && (w_ch_ >> tmp) && sock_.send(tmp);
    }

    inline bool send(const std::string& dst, const std::string& content)
    {
        zmqpp::message msg{dst, content};
        return send(msg);
    }

    inline bool recv(zmqpp::message& msg)
    {
        return sock_.receive(msg);
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

protected:
    zmqpp::context&  ctx_;
    zmqpp::socket    sock_;
    zmqpp::loop      loop_;
    libcpp::zmq_chan w_ch_;
};

}

#endif