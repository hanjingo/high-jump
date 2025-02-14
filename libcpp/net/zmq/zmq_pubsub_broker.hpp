#ifndef ZMQ_PUBSUB_BROKER_HPP
#define ZMQ_PUBSUB_BROKER_HPP

#include <string>
#include <libcpp/net/zmq/zmq_chan.hpp>

namespace libcpp
{

using zmq_option = zmqpp::socket_option;

static const std::string UNFILT(1, 0x01);

class zmq_pub_sub_broker
{
public:
    zmq_pub_sub_broker(zmqpp::context& ctx)
        : ctx_{ctx}
        , loop_{}
        , xpub_sock_{ctx, zmqpp::socket_type::xpub}
        , xsub_sock_{ctx, zmqpp::socket_type::xsub}
    {
        xpub_sock_.set(zmqpp::socket_option::xpub_verbose, 1); // set all pass

        xsub_sock_.send(UNFILT);
    }
    virtual ~zmq_pub_sub_broker()
    {
        xpub_sock_.close();
        xsub_sock_.close();
    }

    inline void bind(const std::string xpub_addr, const std::string xsub_addr)
    {
        xpub_sock_.bind(xpub_addr);
        xsub_sock_.bind(xsub_addr);

        loop_.add(xsub_sock_, std::bind(&zmq_pub_sub_broker::on_xsub, this, std::ref(xpub_sock_), std::ref(xsub_sock_)));
        loop_.add(xpub_sock_, std::bind(&zmq_pub_sub_broker::on_xpub, this, std::ref(xpub_sock_), std::ref(xsub_sock_)));
    }

    inline void loop_start()
    {
        loop_.start();
    }

public:
    virtual bool on_xsub(zmqpp::socket& xpub_sock, zmqpp::socket& xsub_sock)
    {
        while (true) {
            zmqpp::message msg;
            if (!xsub_sock.receive(msg, true)) {
                break;
            }

            if (!xpub_sock.send(msg)) { // route method
                assert(false);
                return false;
            }
        }
        return true;
    }
    virtual bool on_xpub(zmqpp::socket& xpub_sock, zmqpp::socket& xsub_sock)
    {
        while (true) {
            zmqpp::message msg;
            if (!xpub_sock.receive(msg, true)) {
                break;
            }

            std::string topic(1, 0x01);   // self subscribe
            topic.append(msg.get(0));

            if (!xsub_sock.send(topic)) { // route subscribe
                assert(false);
                return false;
            }
        }
        return true;
    }

    inline zmqpp::socket& xpub_socket()
    {
        return xpub_sock_;
    }

    inline zmqpp::socket& xsub_socket()
    {
        return xsub_sock_;
    }

private:
    zmqpp::context&   ctx_;
    zmqpp::loop       loop_;
    zmqpp::socket     xpub_sock_;
    zmqpp::socket     xsub_sock_;
};

}

#endif