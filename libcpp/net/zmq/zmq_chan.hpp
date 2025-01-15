#ifndef ZMQ_CHAN_HPP
#define ZMQ_CHAN_HPP

#include <string>
#include <sstream>

#include <zmq.h>
#include <zmqpp/zmqpp.hpp>
#include <zmqpp/proxy.hpp>
#include <zmqpp/poller.hpp>
#include <zmqpp/loop.hpp>

#define MAX_ID_LEN 64

namespace libcpp
{

class zmq_chan
{
public:
    explicit zmq_chan(zmqpp::context& ctx)
        : ctx_{ctx}
        , in_{ctx, zmqpp::socket_type::pair}
        , out_{ctx, zmqpp::socket_type::pair}
    {
        std::ostringstream ss;
        ss << std::hex << this;
        std::string id = "inproc://" + ss.str();

        in_.bind(id);
        out_.connect(id);
    }
    ~zmq_chan()
    {
        in_.close();
        out_.close();
    };

    inline bool operator>>(zmqpp::message& msg)
    {
        return out_.receive(msg);
    }

    inline bool operator <<(zmqpp::message& msg)
    {
        return in_.send(msg);
    }

private:
    zmqpp::context& ctx_;
    zmqpp::socket   in_;
    zmqpp::socket   out_;
};

}

#endif