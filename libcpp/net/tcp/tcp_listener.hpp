#ifndef TCP_LISTENER_HPP
#define TCP_LISTENER_HPP

#include <thread>
#include <chrono>
#include <iostream>
#include <functional>
#include <atomic>
#include <memory>

#include <boost/asio.hpp>

#include <libcpp/net/tcp/tcp_socket.hpp>

namespace libcpp
{

class tcp_listener
{
public:
    using io_t              = tcp_socket::io_t;
    using io_work_t         = tcp_socket::io_work_t;
    using err_t             = tcp_socket::err_t;
    using address_t         = tcp_socket::address_t;

    using sock_t            = tcp_socket::sock_t;
    using endpoint_t        = tcp_socket::endpoint_t;
    using acceptor_t        = boost::asio::ip::tcp::acceptor;

    using opt_reuse_address = boost::asio::socket_base::reuse_address;

    using accept_handler_t  = std::function<void(const err_t&, tcp_socket*)>;

public:
    tcp_listener() = delete;

    explicit tcp_listener(io_t& io)
        : io_{io}
        , accept_handler_{}
    {
    }
    tcp_listener(io_t& io, const accept_handler_t&& fn)
        : io_{io}
        , accept_handler_{std::move(fn)}
    {
    }
    virtual ~tcp_listener()
    {
        close();
    }

    inline bool is_closed() { return closed_.load(); }
    inline void loop_start() { io_work_t work{io_}; io_.run(); }
    inline void loop_end() { io_.stop(); }

    template<typename T>
    inline bool set_option(T opt)
    {
        if (acceptor_ == nullptr || !acceptor_->is_open())
            return false;

        err_t err;
        acceptor_->set_option(opt, err);
        if (!err.failed())
            return true;

        assert(false);
        std::cerr << err << std::endl;
        return false;
    }

    tcp_socket* accept(uint16_t port)
    {
        endpoint_t ep{boost::asio::ip::tcp::v4(), port};
        return accept(ep);
    }

    tcp_socket* accept(const char* ip, uint16_t port)
    {
        endpoint_t ep{address_t::from_string(ip), port};
        return accept(ep);
    }

    tcp_socket* accept(const endpoint_t& ep)
    {
        if (binded_endpoint_ == ep) 
            return accept();

        if (acceptor_ != nullptr) 
        {
            acceptor_->close();
            delete acceptor_;
            acceptor_ = nullptr;
        }

        acceptor_ = new acceptor_t(io_, ep);
        acceptor_->set_option(opt_reuse_address(true));
        binded_endpoint_ = ep;
        return accept();
    }

    tcp_socket* accept()
    {
        assert(acceptor_ != nullptr);

        err_t err;
        auto sock = new sock_t(io_);
        try {
            acceptor_->accept(*sock, err);
        } catch (...) {
            assert(false);
        }

        if (err.failed()) 
        {
            delete sock;
            sock = nullptr;
            return nullptr;
        }

        auto ret = new tcp_socket(io_, sock);
        ret->set_conn_status(true);
        return ret;
    }

    void async_accept(uint16_t port)
    {
        endpoint_t ep{boost::asio::ip::tcp::v4(), port};
        accept_handler_t fn = accept_handler_;
        async_accept(ep, std::move(fn));
    }

    void async_accept(uint16_t port, accept_handler_t&& fn)
    {
        endpoint_t ep{boost::asio::ip::tcp::v4(), port};
        async_accept(ep, std::move(fn));
    }

    void async_accept(const char* ip, uint16_t port)
    {
        endpoint_t ep{address_t::from_string(ip), port};
        accept_handler_t fn = accept_handler_;
        async_accept(ep, std::move(fn));
    }

    void async_accept(const char* ip, uint16_t port, accept_handler_t&& fn)
    {
        endpoint_t ep{address_t::from_string(ip), port};
        async_accept(ep, std::move(fn));
    }

    void async_accept(endpoint_t ep)
    {
        accept_handler_t fn = accept_handler_;
        async_accept(ep, std::move(fn));
    }

    void async_accept(endpoint_t ep, accept_handler_t&& fn)
    {
        if (binded_endpoint_ == ep) 
        {
            async_accept(std::move(fn));
            return;
        }

        if (acceptor_ != nullptr) 
            acceptor_->close();

        acceptor_ = new acceptor_t(io_, ep);
        acceptor_->set_option(opt_reuse_address(true));
        binded_endpoint_ = ep;
        async_accept(std::move(fn));
    }

    void async_accept(accept_handler_t&& fn)
    {
        assert(acceptor_ != nullptr);

        sock_t* base = new sock_t(io_);
        tcp_socket* sock = new tcp_socket(io_, base);
        try {
            acceptor_->async_accept(*base, [fn, sock](const err_t& err) {
                if (err.failed()) 
                {
                    delete sock;
                    fn(err, nullptr);
                    return;
                }

                sock->set_conn_status(true);
                fn(err, sock);                
            });
        } catch (...) {
            assert(false);
            if (sock != nullptr)
                delete sock;
        }
    }

    void close()
    {
        if (closed_.load())
            return;

        closed_.store(true);
        if (acceptor_ != nullptr) 
        {
            acceptor_->close();
            delete acceptor_;
            acceptor_ = nullptr;
        }
        binded_endpoint_ = endpoint_t();
    }

private:
    io_t&                io_;
    acceptor_t*          acceptor_ = nullptr;
    endpoint_t           binded_endpoint_;
    std::atomic<bool>    closed_{false};

    accept_handler_t     accept_handler_;
};

}

#endif