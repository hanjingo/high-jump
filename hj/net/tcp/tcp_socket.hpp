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
#if defined(_WIN32) && !defined(NOMINMAX)
#define NOMINMAX
#endif
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
    enum class state
    {
        closed,
        connecting,
        connected
    };

    using tcp_socket_ptr_t = std::shared_ptr<tcp_socket>;
    using io_t             = boost::asio::io_context;
#if BOOST_VERSION < 108700
    using io_work_t = boost::asio::io_service::work;
#endif
    using err_t = boost::system::error_code;

    using const_buffer_t   = boost::asio::const_buffer;
    using mutable_buffer_t = boost::asio::mutable_buffer;
    using streambuf_t      = boost::asio::streambuf;

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
    inline err_t set_option(T opt) noexcept
    {
        err_t err;
        _sock->set_option(opt, err);
        return err;
    }

    inline err_t non_blocking(bool mode) noexcept
    {
        err_t err;
        if(_sock)
            _sock->non_blocking(mode, err);
        else
            err = boost::system::errc::make_error_code(
                boost::system::errc::not_connected);
        return err;
    }

    inline bool  is_open() const noexcept { return _sock && _sock->is_open(); }
    inline state stat() const noexcept { return _state.load(); }

    bool set_conn_status(state stat) noexcept
    {
        auto old = _state.load();
        return _state.compare_exchange_strong(old, stat);
    }

    err_t set_read_timeout(std::chrono::milliseconds ms) noexcept
    {
        err_t err;
        if(!_sock)
            return boost::system::errc::make_error_code(
                boost::system::errc::not_connected);

#ifdef _WIN32
        DWORD timeout = static_cast<DWORD>(ms.count());
        ::setsockopt(_sock->native_handle(),
                     SOL_SOCKET,
                     SO_RCVTIMEO,
                     reinterpret_cast<const char *>(&timeout),
                     sizeof(timeout));
#else
        struct timeval tv;
        tv.tv_sec  = static_cast<long>(ms.count() / 1000);
        tv.tv_usec = static_cast<long>((ms.count() % 1000) * 1000);
        ::setsockopt(_sock->native_handle(),
                     SOL_SOCKET,
                     SO_RCVTIMEO,
                     &tv,
                     sizeof(tv));
#endif
        return err;
    }

    err_t set_write_timeout(std::chrono::milliseconds ms) noexcept
    {
        err_t err;
        if(!_sock)
            return boost::system::errc::make_error_code(
                boost::system::errc::not_connected);

#ifdef _WIN32
        DWORD timeout = static_cast<DWORD>(ms.count());
        ::setsockopt(_sock->native_handle(),
                     SOL_SOCKET,
                     SO_SNDTIMEO,
                     reinterpret_cast<const char *>(&timeout),
                     sizeof(timeout));
#else
        struct timeval tv;
        tv.tv_sec  = static_cast<long>(ms.count() / 1000);
        tv.tv_usec = static_cast<long>((ms.count() % 1000) * 1000);
        ::setsockopt(_sock->native_handle(),
                     SOL_SOCKET,
                     SO_SNDTIMEO,
                     &tv,
                     sizeof(tv));
#endif
        return err;
    }

    err_t
    connect(endpoint_t                ep,
            std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
            int                       try_times = 1)
    {
        for(int i = 0; i < try_times; ++i)
        {
            set_conn_status(state::connecting);
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
                        this->set_conn_status(state::connected);
                        success.exchange(true);
                    } else
                        this->set_conn_status(state::closed);

                    timer.cancel();
                });

            timer.async_wait([this, &finished](const err_t &tm_err) {
                (void) tm_err; // ignore timer error
                if(finished.exchange(true))
                    return;

                if(this->_sock)
                    this->_sock->close();

                this->set_conn_status(state::closed);
            });

            _io.restart();
            _io.run();
            if(success.load())
                return {};
        }
        return boost::system::errc::make_error_code(
            boost::system::errc::connection_refused);
    }

    err_t
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
        if(_state.load() == state::connected)
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
                this->set_conn_status(state::connected);
            else
                this->set_conn_status(state::closed);

            if(fn)
                fn(err);
        });
    }

    err_t close()
    {
        if(_state.load() != state::connected)
        {
            return boost::system::errc::make_error_code(
                boost::system::errc::not_connected);
        }

        err_t err;
        set_conn_status(state::closed);
        if(_sock && _sock->is_open())
        {
            boost::asio::socket_base::linger opt;
            _sock->get_option(opt, err);
            if(!err && (!opt.enabled() || opt.timeout() > 0))
                _sock->shutdown(sock_t::shutdown_both, err);

            _sock->close(err);
            _sock.reset();
        }

        return err;
    }

    size_t write(const const_buffer_t &buf, err_t &err) noexcept
    {
        if(_state.load() != state::connected)
        {
            err = boost::system::errc::make_error_code(
                boost::system::errc::not_connected);
            return 0;
        }

        return boost::asio::write(*_sock, buf, err);
    }

    size_t write(const unsigned char *data, size_t len, err_t &err) noexcept
    {
        return write(boost::asio::buffer(data, len), err);
    }

    void async_write(
        const const_buffer_t     &buf,
        send_handler_t          &&fn,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(3000))
    {
        auto cb = std::move(fn);
        if(_state.load() != state::connected)
        {
            if(cb)
                cb(boost::system::errc::make_error_code(
                       boost::system::errc::not_connected),
                   0);
            return;
        }

        auto timer = std::make_shared<steady_timer_t>(_io);
        timer->expires_after(timeout);

        auto done = std::make_shared<std::atomic_bool>(false);

        boost::asio::async_write(
            *_sock,
            buf,
            [this, timer, done, cb](const err_t &err, std::size_t bytes) {
                if(done->exchange(true))
                    return;

                timer->cancel();
                if(cb)
                    cb(err, bytes);
            });

        timer->async_wait([this, done, cb](const err_t &timer_err) {
            if(timer_err == boost::asio::error::operation_aborted)
                return;

            if(done->exchange(true))
                return;

            err_t ec;
            if(this->_sock)
                this->_sock->cancel(ec);

            if(cb)
                cb(boost::system::errc::make_error_code(
                       boost::system::errc::timed_out),
                   0);
        });
    }

    void async_write(
        const unsigned char      *data,
        size_t                    len,
        send_handler_t          &&fn,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(3000))
    {
        return async_write(boost::asio::buffer(data, len),
                           std::move(fn),
                           timeout);
    }

    size_t read(mutable_buffer_t &buf, err_t &err) noexcept
    {
        if(_state.load() != state::connected)
        {
            err = boost::system::errc::make_error_code(
                boost::system::errc::not_connected);
            return 0;
        }

        return _sock->read_some(buf, err);
    }

    size_t read(unsigned char *data, size_t len, err_t &err) noexcept
    {
        mutable_buffer_t buf{data, len};
        return read(buf, err);
    }

    size_t read_until(streambuf_t &buf, size_t least, err_t &err) noexcept
    {
        if(_state.load() != state::connected)
        {
            err = boost::system::errc::make_error_code(
                boost::system::errc::not_connected);
            return 0;
        }

        return boost::asio::read(*_sock,
                                 buf,
                                 boost::asio::transfer_at_least(least),
                                 err);
    }

    void async_read(
        mutable_buffer_t         &buf,
        recv_handler_t          &&fn,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(3000))
    {
        auto cb = std::move(fn);
        if(_state.load() != state::connected)
        {
            if(cb)
                cb(boost::system::errc::make_error_code(
                       boost::system::errc::not_connected),
                   0);
            return;
        }

        auto timer = std::make_shared<steady_timer_t>(_io);
        timer->expires_after(timeout);
        auto done = std::make_shared<std::atomic_bool>(false);
        _sock->async_read_some(
            buf,
            [this, timer, done, cb](const err_t &err, std::size_t bytes) {
                if(done->exchange(true))
                    return;

                timer->cancel();
                if(cb)
                    cb(err, bytes);
            });

        timer->async_wait([this, done, cb](const err_t &timer_err) {
            if(timer_err == boost::asio::error::operation_aborted)
                return;

            if(done->exchange(true))
                return;

            err_t ec;
            if(this->_sock)
                this->_sock->cancel(ec);

            if(cb)
                cb(boost::system::errc::make_error_code(
                       boost::system::errc::timed_out),
                   0);
        });
    }

    void async_read(
        unsigned char            *data,
        size_t                    len,
        recv_handler_t          &&fn,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(3000))
    {
        mutable_buffer_t buf{data, len};
        async_read(buf, std::move(fn), timeout);
    }

  private:
    io_t                   &_io;
    std::unique_ptr<sock_t> _sock;
    std::atomic<state>      _state{state::closed};
};

}

#endif