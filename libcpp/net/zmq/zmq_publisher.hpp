#ifndef ZMQ_PUBLISHER_HPP
#define ZMQ_PUBLISHER_HPP

#include <libcpp/net/zmq/zmq_chan.hpp>

namespace libcpp
{

using zmq_option = zmqpp::socket_option;
class zmq_publisher
{
public:
    zmq_publisher(zmqpp::context& ctx)
        : ctx_{ctx}
        , sock_{ctx, zmqpp::socket_type::pub}
        , w_ch_{ctx}
    {
    }
    ~zmq_publisher()
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
    }

    inline void bind_broker(const std::string& broker_addr, bool do_ack = false)
    {
        sock_.connect(broker_addr);
    }

    inline bool unsafe_pub(zmqpp::message& msg, bool dont_block = false)
    {
        return sock_.send(msg, dont_block);
    }

    inline bool pub(zmqpp::message& msg, bool dont_block = false)
    {
        zmqpp::message tmp;
        return (w_ch_ << msg) && (w_ch_ >> tmp) && sock_.send(tmp, dont_block);
    }

    inline zmqpp::socket& socket()
    {
        return sock_;
    }

private:
    zmqpp::context&  ctx_;
    zmqpp::socket    sock_;
    libcpp::zmq_chan w_ch_;
};

}

#endif