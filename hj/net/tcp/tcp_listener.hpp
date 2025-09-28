#ifndef TCP_LISTENER_HPP
#define TCP_LISTENER_HPP

#include <thread>
#include <chrono>
#include <iostream>
#include <functional>
#include <atomic>
#include <memory>

#include <boost/asio.hpp>
#include <boost/version.hpp>

#include <hj/net/tcp/tcp_socket.hpp>

namespace hj
{

class tcp_listener
{
  public:
#if BOOST_VERSION < 108700
    using io_work_t = tcp_socket::io_work_t;
    using address_t = tcp_socket::address_t;
#else
    using address_t = boost::asio::ip::address;
#endif

    using io_t       = tcp_socket::io_t;
    using err_t      = tcp_socket::err_t;
    using sock_t     = tcp_socket::sock_t;
    using endpoint_t = tcp_socket::endpoint_t;
    using acceptor_t = boost::asio::ip::tcp::acceptor;

    using opt_reuse_address = boost::asio::socket_base::reuse_address;

    using accept_handler_t = std::function<void(const err_t &, tcp_socket *)>;

  public:
    tcp_listener() = delete;

    explicit tcp_listener(io_t &io)
        : _io{io}
        , _accept_handler{}
    {
    }
    tcp_listener(io_t &io, const accept_handler_t &&fn)
        : _io{io}
        , _accept_handler{std::move(fn)}
    {
    }
    virtual ~tcp_listener() { close(); }

    inline bool is_closed() { return _closed.load(); }

    template <typename T>
    inline bool set_option(T opt)
    {
        if(_acceptor == nullptr || !_acceptor->is_open())
            return false;

        err_t err;
        _acceptor->set_option(opt, err);
        if(!err.failed())
            return true;

        assert(false);
        std::cerr << err << std::endl;
        return false;
    }

    tcp_socket *accept(uint16_t port)
    {
        endpoint_t ep{boost::asio::ip::tcp::v4(), port};
        return accept(ep);
    }

    tcp_socket *accept(const char *ip, uint16_t port)
    {
#if BOOST_VERSION < 108700
        endpoint_t ep{address_t::from_string(ip), port};
#else
        endpoint_t ep{boost::asio::ip::make_address(ip), port};
#endif
        return accept(ep);
    }

    tcp_socket *accept(const endpoint_t &ep)
    {
        if(_binded_endpoint == ep)
            return accept();

        if(_acceptor != nullptr)
        {
            _acceptor->close();
            delete _acceptor;
            _acceptor = nullptr;
        }

        _acceptor = new acceptor_t(_io, ep);
        _acceptor->set_option(opt_reuse_address(true));
        _binded_endpoint = ep;
        return accept();
    }

    tcp_socket *accept()
    {
        assert(_acceptor != nullptr);

        err_t err;
        auto  sock = new sock_t(_io);
        try
        {
            _acceptor->accept(*sock, err);
        }
        catch(...)
        {
            assert(false);
        }

        if(err.failed())
        {
            delete sock;
            sock = nullptr;
            return nullptr;
        }

        auto ret = new tcp_socket(_io, sock);
        ret->set_conn_status(true);
        return ret;
    }

    void async_accept(uint16_t port)
    {
        endpoint_t       ep{boost::asio::ip::tcp::v4(), port};
        accept_handler_t fn = _accept_handler;
        async_accept(ep, std::move(fn));
    }

    void async_accept(uint16_t port, accept_handler_t &&fn)
    {
        endpoint_t ep{boost::asio::ip::tcp::v4(), port};
        async_accept(ep, std::move(fn));
    }

    void async_accept(const char *ip, uint16_t port)
    {
#if BOOST_VERSION < 108700
        endpoint_t ep{address_t::from_string(ip), port};
#else
        endpoint_t ep{boost::asio::ip::make_address(ip), port};
#endif
        accept_handler_t fn = _accept_handler;
        async_accept(ep, std::move(fn));
    }

    void async_accept(const char *ip, uint16_t port, accept_handler_t &&fn)
    {
#if BOOST_VERSION < 108700
        endpoint_t ep{address_t::from_string(ip), port};
#else
        endpoint_t ep{boost::asio::ip::make_address(ip), port};
#endif
        async_accept(ep, std::move(fn));
    }

    void async_accept(endpoint_t ep)
    {
        accept_handler_t fn = _accept_handler;
        async_accept(ep, std::move(fn));
    }

    void async_accept(endpoint_t ep, accept_handler_t &&fn)
    {
        if(_binded_endpoint == ep)
        {
            async_accept(std::move(fn));
            return;
        }

        if(_acceptor != nullptr)
            _acceptor->close();

        _acceptor = new acceptor_t(_io, ep);
        _acceptor->set_option(opt_reuse_address(true));
        _binded_endpoint = ep;
        async_accept(std::move(fn));
    }

    void async_accept(accept_handler_t &&fn)
    {
        assert(_acceptor != nullptr);

        sock_t     *base = new sock_t(_io);
        tcp_socket *sock = new tcp_socket(_io, base);
        try
        {
            _acceptor->async_accept(*base, [fn, sock](const err_t &err) {
                if(err.failed())
                {
                    delete sock;
                    fn(err, nullptr);
                    return;
                }

                sock->set_conn_status(true);
                fn(err, sock);
            });
        }
        catch(...)
        {
            assert(false);
            if(sock != nullptr)
                delete sock;
        }
    }

    void close()
    {
        if(_closed.load())
            return;

        _closed.store(true);
        if(_acceptor != nullptr)
        {
            _acceptor->close();
            delete _acceptor;
            _acceptor = nullptr;
        }
        _binded_endpoint = endpoint_t();
    }

  private:
    io_t             &_io;
    acceptor_t       *_acceptor = nullptr;
    endpoint_t        _binded_endpoint;
    std::atomic<bool> _closed{false};

    accept_handler_t _accept_handler;
};

}

#endif