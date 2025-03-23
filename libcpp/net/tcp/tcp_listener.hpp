#ifndef TCP_LISTENER_HPP
#define TCP_LISTENER_HPP

#include <thread>
#include <chrono>
#include <iostream>
#include <functional>

#include <boost/asio.hpp>

#include <libcpp/net/tcp/tcp_socket.hpp>

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

    using accept_handler_t  = std::function<void(const err_t&, tcp_socket*)>;
    using signal_handler_t  = std::function<void(const err_t&, signal_t)>;

public:
    tcp_listener()
        : _sigs{_io}
    {
    }
    virtual ~tcp_listener()
    {
        close();
    }

    inline void poll()
    {
        _io.run();
    }

    inline void loop_start()
    {
        io_work_t work{_io};
        _io.run();
    }

    inline void loop_end()
    {
        _io.stop();
    }

    template<typename T>
    inline void set_option(T opt)
    {
        if (_acceptor != nullptr && _acceptor->is_open()) {
            _acceptor->set_option(opt);
        }
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
        _acceptor->accept(*sock, err);
        if (err.failed()) {
            delete sock;
            return nullptr;
        }

        auto net = new tcp_socket(sock);
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
            auto net = new tcp_socket(sock);

            if (!err.failed()) {
                fn(err, net);
            }
        });
    }

    inline void add_signal(signal_t sig, signal_handler_t&& fn)
    {
        _sigs.add(sig);
        _sigs.async_wait(fn);
    }

    void remove_signal(signal_t sig)
    {
        _sigs.remove(sig);
    }

    void close()
    {
        if (_acceptor != nullptr) {
            _acceptor->close();
            delete _acceptor;
            _acceptor = nullptr;
        }

        loop_end();
    }

private:
    io_t                 _io;
    signal_set_t         _sigs;

    acceptor_t*          _acceptor = nullptr;
    endpoint_t           _binded_endpoint;
};

}

#endif