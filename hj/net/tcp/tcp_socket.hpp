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

#include <chrono>
#include <functional>
#include <memory>
#include <utility>
#include <atomic>

#include <boost/version.hpp>
#include <boost/asio.hpp>

namespace hj
{

class tcp_socket
{
  public:
    enum class state
    {
        closed,
        connecting,
        connected
    };

    using io_t  = boost::asio::io_context;
    using err_t = boost::system::error_code;

    using const_buffer_t   = boost::asio::const_buffer;
    using mutable_buffer_t = boost::asio::mutable_buffer;
    using streambuf_t      = boost::asio::streambuf;

    using sock_t     = boost::asio::ip::tcp::socket;
    using address_t  = boost::asio::ip::address;
    using endpoint_t = boost::asio::ip::tcp::endpoint;

    using ms_t = std::chrono::milliseconds;

    using opt_no_delay    = boost::asio::ip::tcp::no_delay;
    using opt_send_buf_sz = boost::asio::ip::tcp::socket::send_buffer_size;
    using opt_recv_buf_sz = boost::asio::ip::tcp::socket::receive_buffer_size;
    using opt_keep_alive  = boost::asio::ip::tcp::socket::keep_alive;

  public:
    explicit tcp_socket(io_t &io)
        : _io{io}
        , _sock{std::make_unique<sock_t>(io)}
        , _state{state::closed}
    {
    }

    explicit tcp_socket(io_t                   &io,
                        std::unique_ptr<sock_t> sock,
                        state initial_state = state::connected)
        : _io{io}
        , _sock{sock ? std::move(sock) : std::make_unique<sock_t>(io)}
        , _state{initial_state}
    {
    }

    ~tcp_socket() noexcept { close(); }

    tcp_socket()                              = delete;
    tcp_socket(const tcp_socket &)            = delete;
    tcp_socket &operator=(const tcp_socket &) = delete;
    tcp_socket(tcp_socket &&)                 = delete;
    tcp_socket &operator=(tcp_socket &&)      = delete;

    inline io_t &io() noexcept { return _io; }

    template <typename T>
    inline err_t set_option(T opt) noexcept
    {
        err_t err;
        if(_sock)
            _sock->set_option(opt, err);
        else
            err = boost::system::errc::make_error_code(
                boost::system::errc::not_connected);
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
    inline state status() const noexcept { return _state.load(); }
    bool         status_chg(state to) noexcept
    {
        state from = _state.load();
        switch(from)
        {
            case state::closed: {
                if(to == state::connected)
                    return false;

                break;
            }
            case state::connecting: {
                break;
            }
            case state::connected: {
                if(to == state::connected || to == state::connecting)
                    return false;

                break;
            }
        }
        return _state.compare_exchange_strong(from, to);
    }

    err_t
    connect(endpoint_t                ep,
            std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
            int                       attempts = 1)
    {
        if(attempts <= 0)
        {
            return boost::system::errc::make_error_code(
                boost::system::errc::invalid_argument);
        }

        err_t last_err;

        for(int i = 0; i < attempts; ++i)
        {
            if(!status_chg(state::connecting))
            {
                state current = _state.load();
                if(current == state::connected)
                    return boost::system::errc::make_error_code(
                        boost::system::errc::already_connected);
                if(current == state::connecting && i == 0)
                    return boost::system::errc::make_error_code(
                        boost::system::errc::operation_in_progress);
            }

            if(_sock && _sock->is_open())
            {
                err_t ec;
                _sock->close(ec);
            }

            _sock            = std::make_unique<sock_t>(_io);
            sock_t *raw_sock = _sock.get();

            boost::asio::steady_timer timer(_io);
            timer.expires_after(timeout);

            err_t connect_ec;
            bool  timed_out = false;

            timer.async_wait([raw_sock, &timed_out](const err_t &ec) {
                if(!ec)
                {
                    timed_out = true;
                    err_t close_ec;
                    if(raw_sock)
                        raw_sock->close(close_ec);
                }
            });

            _sock->async_connect(ep, [&timer, &connect_ec](const err_t &ec) {
                timer.cancel();
                connect_ec = ec;
            });

            _io.restart();
            _io.run();

            if(timed_out)
            {
                last_err = boost::system::errc::make_error_code(
                    boost::system::errc::timed_out);
            } else if(!connect_ec)
            {
                status_chg(state::connected);
                return {};
            } else
            {
                last_err = connect_ec;
            }

            status_chg(state::closed);
        }

        return last_err;
    }

    err_t
    connect(const char               *ip,
            uint16_t                  port,
            std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
            int                       attempts = 1)
    {
#if BOOST_VERSION < 108700
        endpoint_t ep{address_t::from_string(ip), port};
#else
        endpoint_t ep{boost::asio::ip::make_address(ip), port};
#endif
        return connect(ep, timeout, attempts);
    }

    err_t shutdown()
    {
        if(_state.load() != state::connected)
            return boost::system::errc::make_error_code(
                boost::system::errc::not_connected);

        err_t err;
        if(_sock && _sock->is_open())
        {
            _sock->shutdown(sock_t::shutdown_both, err);
        }
        return err;
    }

    err_t close()
    {
        _state.store(state::closed);

        if(!_sock || !_sock->is_open())
            return {};

        err_t err;
        _sock->close(err);
        return err;
    }

    size_t write(const const_buffer_t &buf, err_t &err)
    {
        if(_state.load() != state::connected)
        {
            err = boost::system::errc::make_error_code(
                boost::system::errc::not_connected);
            return 0;
        }

        auto sz = boost::asio::write(*_sock, buf, err);
        _on_io_error(err);
        return sz;
    }

    size_t write(const unsigned char *data, size_t len, err_t &err)
    {
        return write(boost::asio::buffer(data, len), err);
    }

    size_t read(mutable_buffer_t &buf, err_t &err)
    {
        if(_state.load() != state::connected)
        {
            err = boost::system::errc::make_error_code(
                boost::system::errc::not_connected);
            return 0;
        }

        size_t bytes = _sock->read_some(buf, err);
        _on_io_error(err);
        return bytes;
    }

    size_t read(unsigned char *data, size_t len, err_t &err)
    {
        mutable_buffer_t buf{data, len};
        return read(buf, err);
    }

    size_t read_at_least(streambuf_t &buf, size_t least, err_t &err)
    {
        if(_state.load() != state::connected)
        {
            err = boost::system::errc::make_error_code(
                boost::system::errc::not_connected);
            return 0;
        }

        auto sz = boost::asio::read(*_sock,
                                    buf,
                                    boost::asio::transfer_at_least(least),
                                    err);
        _on_io_error(err);
        return sz;
    }

    size_t read_exactly(streambuf_t &buf, size_t exact, err_t &err)
    {
        if(_state.load() != state::connected)
        {
            err = boost::system::errc::make_error_code(
                boost::system::errc::not_connected);
            return 0;
        }

        auto sz = boost::asio::read(*_sock,
                                    buf,
                                    boost::asio::transfer_exactly(exact),
                                    err);
        _on_io_error(err);
        return sz;
    }

  private:
    void _on_io_error(const err_t &err) noexcept
    {
        if(!err.failed())
            return;

        if(err == boost::asio::error::connection_reset
           || err == boost::asio::error::broken_pipe
           || err == boost::system::errc::connection_reset
           || err == boost::system::errc::broken_pipe
           || err == boost::system::errc::not_connected
           || err == boost::asio::error::eof)
        {
            close();
        }
    }

  private:
    io_t                   &_io;
    std::unique_ptr<sock_t> _sock;
    std::atomic<state>      _state{state::closed};
};

} // namespace hj

#endif // TCP_SOCKET_HPP