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

#ifndef TCP_SOCKET_HPP
#define TCP_SOCKET_HPP

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#endif

#include <chrono>
#include <iostream>
#include <functional>
#include <memory>
#include <csignal>
#include <initializer_list>

#include <boost/stacktrace.hpp>
#include <boost/version.hpp>
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>

#if BOOST_VERSION >= 108700
#include <boost/asio/ip/address.hpp>
#endif

namespace hj
{

class tcp_socket : public std::enable_shared_from_this<tcp_socket>
{
  public:
    using tcp_socket_ptr_t = std::shared_ptr<tcp_socket>;
    using io_t             = boost::asio::io_context;
#if BOOST_VERSION < 108700
    using io_work_t = boost::asio::io_service::work;
#endif
    using err_t = boost::system::error_code;

    using const_buffer_t = boost::asio::const_buffer;
    using multi_buffer_t = boost::asio::mutable_buffer;
    using streambuf_t    = boost::asio::streambuf;

    using sock_t = boost::asio::ip::tcp::socket;
#if BOOST_VERSION < 108700
    using address_t = boost::asio::ip::address;
#else
    using address_t = boost::asio::ip::address;
#endif
    using endpoint_t = boost::asio::ip::tcp::endpoint;

    using steady_timer_t   = boost::asio::steady_timer;
    using deadline_timer_t = boost::asio::deadline_timer;
    using ms_t             = std::chrono::milliseconds;

    using opt_no_delay    = boost::asio::ip::tcp::no_delay;
    using opt_send_buf_sz = boost::asio::ip::tcp::socket::send_buffer_size;
    using opt_recv_buf_sz = boost::asio::ip::tcp::socket::receive_buffer_size;
    using opt_reuse_addr  = boost::asio::ip::tcp::socket::reuse_address;
    using opt_keep_alive  = boost::asio::ip::tcp::socket::keep_alive;
    using opt_broadcast   = boost::asio::ip::tcp::socket::broadcast;

    using conn_handler_t = std::function<void(const err_t &)>;
    using send_handler_t = std::function<void(const err_t &, std::size_t)>;
    using recv_handler_t = std::function<void(const err_t &, std::size_t)>;

  public:
    explicit tcp_socket(io_t &io)
        : _io{io}
        , _sock{std::make_unique<sock_t>(io)}
    {
    }
    // NOTE: sock must be created by new operator
    explicit tcp_socket(io_t &io, sock_t *sock)
        : _io{io}
        , _sock{std::unique_ptr<sock_t>(sock)}
    {
    }
    virtual ~tcp_socket() { close(); }

    tcp_socket()                              = delete;
    tcp_socket(const tcp_socket &)            = delete;
    tcp_socket &operator=(const tcp_socket &) = delete;
    tcp_socket(tcp_socket &&)                 = default;
    tcp_socket &operator=(tcp_socket &&)      = default;

    inline io_t &io() { return _io; }

    template <typename T>
    inline bool set_option(T opt)
    {
        if(!_sock || !_sock->is_open())
            return false;

        err_t err;
        _sock->set_option(opt, err);
        if(!err.failed())
            return true;

        std::cerr << err << std::endl;
        return false;
    }

    inline bool is_connected() const noexcept
    {
        return _sock && _is_connected.load();
    }

    inline bool check_connected() noexcept
    {
        err_t err;
        return check_connected(err);
    }

    inline bool check_connected(err_t &err) noexcept
    {
        if(!_sock)
            return false;

        _sock->remote_endpoint(err);
        return !err.failed();
    }

    bool
    connect(const char               *ip,
            uint16_t                  port,
            std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
            int                       try_times = 1)
    {
#if BOOST_VERSION < 108700
        endpoint_t ep{address_t::from_string(ip), port};
#else
        endpoint_t ep{boost::asio::ip::make_address(ip), port};
#endif
        return connect(ep, timeout, try_times);
    }

    bool
    connect(endpoint_t                ep,
            std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
            int                       try_times = 1)
    {
        for(int i = 0; i < try_times; ++i)
        {
            set_conn_status(false);
            if(_sock && _sock->is_open())
                _sock->close();

            _sock = std::make_unique<sock_t>(_io);

            std::atomic_bool finished{false};
            std::atomic_bool success{false};

            steady_timer_t timer(_io);
            timer.expires_after(timeout);

            _sock->async_connect(
                ep,
                [this, &finished, &success, &timer](const err_t &err) {
                    if(finished.exchange(true))
                        return;

                    if(!err.failed())
                    {
                        this->set_conn_status(true);
                        success.exchange(true);
                    } else
                        this->set_conn_status(false);

                    timer.cancel();
                });

            timer.async_wait([this, &finished](const err_t &err) {
                (void) err; // ignore timer error
                if(finished.exchange(true))
                    return;

                if(this->_sock)
                    this->_sock->close();

                this->set_conn_status(false);
            });

            _io.restart();
            _io.run();
            if(success.load())
                return true;
        }
        return false;
    }

    void async_connect(const char *ip, const uint16_t port, conn_handler_t &&fn)
    {
#if BOOST_VERSION < 108700
        endpoint_t ep{address_t::from_string(ip), port};
#else
        endpoint_t ep{boost::asio::ip::make_address(ip), port};
#endif
        async_connect(ep, std::move(fn));
    }

    void async_connect(endpoint_t ep, conn_handler_t &&fn)
    {
        if(is_connected())
        {
            fn(boost::system::errc::make_error_code(
                boost::system::errc::already_connected));
            return;
        }

        if(_sock && _sock->is_open())
            _sock->close();

        _sock = std::make_unique<sock_t>(_io);
        _sock->async_connect(ep, [this, fn](const err_t &err) {
            if(!err.failed())
                this->set_conn_status(true);
            else
                this->set_conn_status(false);

            if(fn)
                fn(err);
        });
    }

    void disconnect()
    {
        if(!is_connected())
            return;

        _disconnect_anyway();
    }

    size_t send(const const_buffer_t &buf)
    {
        if(!is_connected())
            return 0;

        try
        {
            return _sock->send(buf);
        }
        catch(const std::exception &e)
        {
            std::cerr << "send exception: " << e.what() << std::endl;
            return 0;
        }
        catch(...)
        {
            std::cerr << "send unknown exception" << std::endl;
            return 0;
        }
    }

    size_t send(const unsigned char *data, size_t len)
    {
        return send(boost::asio::buffer(data, len));
    }

    void async_send(const const_buffer_t &buf, send_handler_t &&fn)
    {
        if(!is_connected())
        {
            fn(boost::system::errc::make_error_code(
                   boost::system::errc::not_connected),
               0);
            return;
        }

        try
        {
            _sock->async_send(buf, std::move(fn));
        }
        catch(const std::exception &e)
        {
            std::cerr << "async_send exception: " << e.what() << std::endl;
            return;
        }
        catch(...)
        {
            std::cerr << "async_send unknown exception" << std::endl;
            return;
        }
    }

    void async_send(const unsigned char *data, size_t len, send_handler_t &&fn)
    {
        if(!is_connected())
        {
            fn(boost::system::errc::make_error_code(
                   boost::system::errc::not_connected),
               0);
            return;
        }

        return async_send(boost::asio::buffer(data, len), std::move(fn));
    }

    size_t recv(multi_buffer_t &buf)
    {
        if(!is_connected())
            return 0;

        err_t err;
        auto  sz = _sock->read_some(buf, err);
        if(err.failed())
            assert(false);

        return sz;
    }

    size_t recv(unsigned char *data, size_t len)
    {
        multi_buffer_t buf{data, len};
        return recv(buf);
    }

    size_t recv_until(streambuf_t &buf, size_t least)
    {
        if(!is_connected())
            return 0;

        err_t err;
        auto  sz = boost::asio::read(*_sock,
                                    buf,
                                    boost::asio::transfer_at_least(least),
                                    err);
        if(err.failed())
            assert(false);

        return sz;
    }

    void async_recv(multi_buffer_t &buf, recv_handler_t &&fn)
    {
        auto cb = std::move(fn);
        if(!is_connected())
        {
            cb(boost::system::errc::make_error_code(
                   boost::system::errc::not_connected),
               0);
            return;
        }

        _sock->async_read_some(buf, cb);
    }

    void async_recv(unsigned char *data, size_t len, recv_handler_t &&fn)
    {
        multi_buffer_t buf{data, len};
        async_recv(buf, std::move(fn));
    }

    bool set_conn_status(bool is_connected) noexcept
    {
        bool old = _is_connected.load();
        return _is_connected.compare_exchange_strong(old, is_connected);
    }

    void close()
    {
        if(!is_connected())
            return;

        _disconnect_anyway();
    }

  private:
    void _disconnect_anyway()
    {
        set_conn_status(false);
        if(_sock && _sock->is_open())
            _sock->close();

        if(_sock)
            _sock.reset();
    }

  private:
    io_t                   &_io;
    std::unique_ptr<sock_t> _sock;
    std::atomic_bool        _is_connected{false};
};

}

#endif