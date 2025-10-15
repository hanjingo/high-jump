/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

class tcp_listener : public std::enable_shared_from_this<tcp_listener>
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

    using accept_handler_t =
        std::function<void(const err_t &, std::shared_ptr<tcp_socket>)>;

  public:
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

    tcp_listener()                                = delete;
    tcp_listener(const tcp_listener &)            = delete;
    tcp_listener &operator=(const tcp_listener &) = delete;
    tcp_listener(tcp_listener &&)                 = default;
    tcp_listener &operator=(tcp_listener &&)      = default;

    inline bool is_closed() const noexcept { return _closed.load(); }

    template <typename T>
    inline bool set_option(T opt)
    {
        if(!_acceptor || !_acceptor->is_open())
            return false;

        err_t err;
        _acceptor->set_option(opt, err);
        if(!err.failed())
            return true;

        assert(false);
        std::cerr << err << std::endl;
        return false;
    }

    std::shared_ptr<tcp_socket> accept(uint16_t port)
    {
        endpoint_t ep{boost::asio::ip::tcp::v4(), port};
        return accept(ep);
    }

    std::shared_ptr<tcp_socket> accept(const char *ip, uint16_t port)
    {
#if BOOST_VERSION < 108700
        endpoint_t ep{address_t::from_string(ip), port};
#else
        endpoint_t ep{boost::asio::ip::make_address(ip), port};
#endif
        return accept(ep);
    }

    std::shared_ptr<tcp_socket> accept(const endpoint_t &ep)
    {
        if(_binded_endpoint == ep)
            return accept();

        if(_acceptor)
            _acceptor->close();

        _acceptor = std::make_unique<acceptor_t>(_io, ep);
        _acceptor->set_option(opt_reuse_address(true));
        _binded_endpoint = ep;
        return accept();
    }

    std::shared_ptr<tcp_socket> accept()
    {
        if(!_acceptor)
            return nullptr;

        err_t err;
        auto  sock = new sock_t(_io);
        try
        {
            _acceptor->accept(*sock, err);
        }
        catch(const std::exception &e)
        {
            std::cerr << "accept exception: " << e.what() << std::endl;
            return nullptr;
        }

        if(err.failed())
            return nullptr;

        auto ret = std::make_shared<tcp_socket>(_io, sock);
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

        _acceptor = std::make_unique<acceptor_t>(_io, ep);
        _acceptor->set_option(opt_reuse_address(true));
        _binded_endpoint = ep;
        async_accept(std::move(fn));
    }

    void async_accept(accept_handler_t &&fn)
    {
        if(!_acceptor)
            return;

        auto base = new sock_t(_io);
        auto sock = std::make_shared<tcp_socket>(_io, base);
        try
        {
            _acceptor->async_accept(*base, [fn, sock](const err_t &err) {
                if(err.failed())
                {
                    if(fn)
                        fn(err, nullptr);

                    return;
                }

                sock->set_conn_status(true);
                if(fn)
                    fn(err, sock);
            });
        }
        catch(const std::exception &e)
        {
            std::cerr << "async_accept exception: " << e.what() << std::endl;
        }
    }

    void close()
    {
        if(_closed.load())
            return;

        _closed.store(true);
        if(_acceptor)
        {
            _acceptor->close();
            _acceptor.reset();
        }
        _binded_endpoint = endpoint_t();
    }

  private:
    io_t                       &_io;
    std::unique_ptr<acceptor_t> _acceptor;
    endpoint_t                  _binded_endpoint;
    std::atomic<bool>           _closed{false};
    accept_handler_t            _accept_handler;
};

}

#endif