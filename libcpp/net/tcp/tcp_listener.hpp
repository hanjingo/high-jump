#ifndef TCP_LISTENER_HPP
#define TCP_LISTENER_HPP

#include <thread>
#include <chrono>
#include <iostream>
#include <functional>

#include <boost/asio.hpp>

#include <libcpp/net/tcp/tcp_session.hpp>

namespace libcpp
{

class tcp_listener
{
public:
    using signal_t          = int;
    using signal_set_t      = boost::asio::signal_set;

    using io_t              = boost::asio::io_context;
    using io_work_t         = boost::asio::io_service::work;
    using err_t             = boost::system::error_code;
    using address_t         = boost::asio::ip::address;

    using sock_t            = boost::asio::ip::tcp::socket;
    using acceptor_t        = boost::asio::ip::tcp::acceptor;
    using endpoint_t        = boost::asio::ip::tcp::endpoint;

    using opt_no_delay      = boost::asio::ip::tcp::no_delay;
    using opt_send_buf_sz   = boost::asio::ip::tcp::socket::send_buffer_size;
    using opt_recv_buf_sz   = boost::asio::ip::tcp::socket::receive_buffer_size;
    using opt_reuse_addr    = boost::asio::ip::tcp::socket::reuse_address;

    using accept_handler_t  = std::function<void(const err_t&, tcp_session*)>;
    using signal_handler_t  = std::function<void(const err_t&, signal_t)>;

public:
    tcp_listener()
        : sigs_{io_}
    {
    }
    virtual ~tcp_listener()
    {
        close();
    }

    inline void poll()
    {
        io_.run();
    }

    inline void loop_start()
    {
        io_work_t work{io_};
        io_.run();
    }

    inline void loop_end()
    {
        io_.stop();
    }

    template<typename T>
    inline void set_option(T opt)
    {
        if (acceptor_ != nullptr && acceptor_->is_open()) {
            acceptor_->set_option(opt);
        }
    }

    tcp_session* accept(uint16_t port)
    {
        endpoint_t ep{boost::asio::ip::tcp::v4(), port};
        return accept(ep);
    }

    tcp_session* accept(const char* ip, uint16_t port)
    {
        endpoint_t ep{address_t::from_string(ip), port};
        return accept(ep);
    }

    tcp_session* accept(const endpoint_t& ep)
    {
        if (binded_endpoint_ == ep) {
            return accept();
        }
        if (acceptor_ != nullptr) {
            acceptor_->close();
        }

        acceptor_        = new acceptor_t(io_, ep);
        binded_endpoint_ = ep;
        return accept();
    }

    tcp_session* accept()
    {
        assert(acceptor_ != nullptr);

        err_t err;
        auto sock = new sock_t(io_);
        acceptor_->accept(*sock, err);
        if (err.failed()) {
            delete sock;
            return nullptr;
        }

        auto net = new tcp_session(sock);
        net->set_conn_status(true);
        return net;
    }

    void async_accept(uint16_t port, accept_handler_t&& fn)
    {
        endpoint_t ep{boost::asio::ip::tcp::v4(), port};
        async_accept(ep, std::move(fn));
    }

    void async_accept(const char* ip, uint16_t port, accept_handler_t&& fn)
    {
        endpoint_t ep{address_t::from_string(ip), port};
        async_accept(ep, std::move(fn));
    }

    void async_accept(endpoint_t ep, accept_handler_t&& fn)
    {
        if (binded_endpoint_ == ep) {
            async_accept(std::move(fn));
            return;
        }
        if (acceptor_ != nullptr) {
            acceptor_->close();
        }

        acceptor_        = new acceptor_t(io_, ep);
        binded_endpoint_ = ep;
        async_accept(std::move(fn));
    }

    void async_accept(accept_handler_t&& fn)
    {
        assert(acceptor_ != nullptr);

        auto sock = new sock_t(io_);
        acceptor_->async_accept(*sock, [&](const err_t & err) {
            auto net = new tcp_session(sock);

            if (!err.failed()) {
                fn(err, net);
            }
        });
    }

    inline void add_signal(signal_t sig, signal_handler_t&& fn)
    {
        sigs_.add(sig);
        sigs_.async_wait(fn);
    }

    void remove_signal(signal_t sig)
    {
        sigs_.remove(sig);
    }

    void close()
    {
        if (acceptor_ != nullptr) {
            acceptor_->close();
            delete acceptor_;
            acceptor_ = nullptr;
        }

        loop_end();
    }

private:
    io_t                 io_;
    signal_set_t         sigs_;

    acceptor_t*          acceptor_ = nullptr;
    endpoint_t           binded_endpoint_;
};

}

#endif