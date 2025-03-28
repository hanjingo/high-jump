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
    using sock_ptr_t        = tcp_socket::sock_t*;
    using endpoint_t        = tcp_socket::endpoint_t;
    using acceptor_t        = boost::asio::ip::tcp::acceptor;

    using accept_handler_t  = std::function<void(const err_t&, tcp_socket*)>;

public:
    tcp_listener() = delete;

    explicit tcp_listener(io_t& io)
        : _io{io}
    {
    }
    virtual ~tcp_listener()
    {
        close();
    }

    template<typename T>
    inline void set_option(T opt)
    {
        if (_acceptor != nullptr && _acceptor->is_open())
            _acceptor->set_option(opt);
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
        if (_binded_endpoint == ep) {
            return accept();
        }
        if (_acceptor != nullptr) {
            _acceptor->close();
        }

        _acceptor        = new acceptor_t(_io, ep);
        _binded_endpoint = ep;
        return accept();
    }

    tcp_socket* accept()
    {
        assert(_acceptor != nullptr);

        err_t err;
        auto sock = new sock_t(_io);

        try {
            _acceptor->accept(*sock, err);
        } catch (const boost::system::system_error& exception) {
            delete sock;
            sock = nullptr;
            return nullptr;
        }

        if (err.failed()) {
            delete sock;
            return nullptr;
        }

        auto ret = new tcp_socket(_io, sock);
        ret->set_conn_status(true);
        return ret;
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
        if (_binded_endpoint == ep) {
            async_accept(std::move(fn));
            return;
        }
        if (_acceptor != nullptr) {
            _acceptor->close();
        }

        _acceptor        = new acceptor_t(_io, ep);
        _binded_endpoint = ep;
        async_accept(std::move(fn));
    }

    void async_accept(accept_handler_t&& fn)
    {
        assert(_acceptor != nullptr);

        auto sock = new sock_t(_io);
        _acceptor->async_accept(*sock, [&](const err_t & err) {
            auto net = new tcp_socket(_io, sock);

            if (!err.failed()) {
                fn(err, net);
            }
        });
    }

    void close()
    {
        if (_closed.load())
            return;

        _closed.store(true);
        if (_acceptor != nullptr) {
            _acceptor->close();
            delete _acceptor;
            _acceptor = nullptr;
        }
    }

private:
    io_t&                _io;
    acceptor_t*          _acceptor = nullptr;
    endpoint_t           _binded_endpoint;
    std::atomic<bool>    _closed{false};
};

}

#endif