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

class tcp_socket : public std::enable_shared_from_this<tcp_socket>
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

    using raw_sock_t = boost::asio::ip::tcp::socket;
    using address_t  = boost::asio::ip::address;
    using endpoint_t = boost::asio::ip::tcp::endpoint;

    using ms_t = std::chrono::milliseconds;

    using opt_no_delay    = boost::asio::ip::tcp::no_delay;
    using opt_send_buf_sz = boost::asio::ip::tcp::socket::send_buffer_size;
    using opt_recv_buf_sz = boost::asio::ip::tcp::socket::receive_buffer_size;
    using opt_keep_alive  = boost::asio::ip::tcp::socket::keep_alive;

    using conn_handler_t  = std::function<void(const err_t &)>;
    using write_handler_t = std::function<void(const err_t &, std::size_t)>;
    using read_handler_t  = std::function<void(const err_t &, std::size_t)>;

  private:
    struct tcp_socket_key
    {
        explicit tcp_socket_key() = default;
    };

  public:
    tcp_socket()                              = delete;
    tcp_socket(const tcp_socket &)            = delete;
    tcp_socket &operator=(const tcp_socket &) = delete;
    tcp_socket(tcp_socket &&)                 = delete;
    tcp_socket &operator=(tcp_socket &&)      = delete;

    explicit tcp_socket(tcp_socket_key, io_t &io)
        : _io{io}
        , _sock{std::make_unique<raw_sock_t>(io)}
        , _state{state::closed}
    {
    }

    explicit tcp_socket(tcp_socket_key,
                        io_t                       &io,
                        std::unique_ptr<raw_sock_t> sock,
                        state initial_state = state::connected)
        : _io{io}
        , _sock{sock ? std::move(sock) : std::make_unique<raw_sock_t>(io)}
        , _state{initial_state}
    {
    }

    ~tcp_socket() noexcept { close(); }

    template <typename... Args>
    static std::shared_ptr<tcp_socket> make_shared(Args &&...args)
    {
        return std::make_shared<hj::tcp_socket>(tcp_socket_key{},
                                                std::forward<Args>(args)...);
    }

    inline io_t       &io() noexcept { return _io; }
    inline raw_sock_t &raw_socket() noexcept { return *_sock; }

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
            last_err = _status_chg(state::connecting);
            if(last_err.failed())
                return last_err;

            if(_sock && _sock->is_open())
            {
                err_t ec;
                _sock->close(ec);
                if(ec.failed())
                    continue;
            }

            _sock                = std::make_unique<raw_sock_t>(_io);
            raw_sock_t *raw_sock = _sock.get();

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
                return _status_chg(state::connected);
            } else
            {
                last_err = connect_ec;
            }

            _status_chg(state::closed);
        }

        return last_err;
    }

    err_t
    connect(const char               *ip,
            uint16_t                  port,
            std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
            int                       attempts = 1)
    {
        endpoint_t ep;
        try
        {
            std::string ip_str = ip ? ip : "";
#if BOOST_VERSION < 108700
            ep = endpoint_t(address_t::from_string(ip_str), port);
#else
            ep = endpoint_t(boost::asio::ip::make_address(ip_str), port);
#endif
        }
        catch(const std::exception &e)
        {
            return boost::system::errc::make_error_code(
                boost::system::errc::invalid_argument);
        }

        return connect(ep, timeout, attempts);
    }

    template <typename Handler = conn_handler_t>
    void async_connect(endpoint_t ep, Handler &&fn)
    {
        auto err = _status_chg(state::connecting);
        if(err.failed())
        {
            boost::asio::post(
                _io,
                [err, fn = std::forward<Handler>(fn)]() mutable { fn(err); });
            return;
        }

        if(_sock && _sock->is_open())
        {
            err_t ec;
            _sock->close(ec);
            if(ec.failed())
            {
                boost::asio::post(
                    _io,
                    [fn = std::forward<Handler>(fn), ec]() mutable { fn(ec); });
                return;
            }
        }

        _sock                               = std::make_unique<raw_sock_t>(_io);
        std::weak_ptr<tcp_socket> weak_self = weak_from_this();
        _sock->async_connect(
            ep,
            [weak_self,
             fn = std::forward<Handler>(fn)](const err_t &err) mutable {
                if(auto self = weak_self.lock())
                {
                    if(err.failed())
                        self->_status_chg(state::closed);
                    else
                        self->_status_chg(state::connected);

                    fn(err);
                } else
                {
                    fn(boost::asio::error::make_error_code(
                        boost::asio::error::operation_aborted));
                }
            });
    }

    template <typename Handler = conn_handler_t>
    void async_connect(const char *ip, const uint16_t port, Handler &&fn)
    {
        endpoint_t ep;
        try
        {
            std::string ip_str = ip ? ip : "";
#if BOOST_VERSION < 108700
            ep = endpoint_t(address_t::from_string(ip_str), port);
#else
            ep = endpoint_t(boost::asio::ip::make_address(ip_str), port);
#endif
        }
        catch(const std::exception &e)
        {
            boost::asio::post(_io, [fn = std::forward<Handler>(fn)]() mutable {
                fn(boost::system::errc::make_error_code(
                    boost::system::errc::invalid_argument));
            });
            return;
        }

        async_connect(ep, std::forward<Handler>(fn));
    }

    err_t shutdown()
    {
        if(_state.load() != state::connected)
            return boost::system::errc::make_error_code(
                boost::system::errc::not_connected);

        err_t err;
        if(_sock && _sock->is_open())
        {
            _sock->shutdown(raw_sock_t::shutdown_both, err);
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

    template <typename Handler = write_handler_t>
    void async_write(const_buffer_t buf, Handler &&fn)
    {
        if(_state.load() != state::connected)
        {
            boost::asio::post(_io, [fn = std::forward<Handler>(fn)]() mutable {
                fn(boost::system::errc::make_error_code(
                       boost::system::errc::not_connected),
                   0);
            });
            return;
        }

        std::weak_ptr<tcp_socket> weak_self = weak_from_this();
        boost::asio::async_write(
            *_sock,
            buf,
            [weak_self,
             fn = std::forward<Handler>(fn)](const err_t &err,
                                             size_t bytes_transferred) mutable {
                if(auto self = weak_self.lock())
                {
                    if(err.failed())
                        self->_status_chg(state::closed);

                    fn(err, bytes_transferred);
                } else
                {
                    fn(boost::asio::error::make_error_code(
                           boost::asio::error::operation_aborted),
                       0);
                }
            });
    }

    template <typename Handler = write_handler_t>
    void async_write(const unsigned char *data, size_t len, Handler &&fn)
    {
        return async_write(boost::asio::buffer(data, len),
                           std::forward<Handler>(fn));
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

    template <typename Handler = read_handler_t>
    void async_read(mutable_buffer_t buf, Handler &&fn)
    {
        if(_state.load() != state::connected)
        {
            boost::asio::post(_io, [fn = std::forward<Handler>(fn)]() mutable {
                fn(boost::system::errc::make_error_code(
                       boost::system::errc::not_connected),
                   0);
            });
            return;
        }

        std::weak_ptr<tcp_socket> weak_self = weak_from_this();
        _sock->async_read_some(
            buf,
            [weak_self,
             fn = std::forward<Handler>(fn)](const err_t &err,
                                             size_t bytes_transferred) mutable {
                if(auto self = weak_self.lock())
                {
                    if(err.failed())
                        self->_status_chg(state::closed);

                    fn(err, bytes_transferred);
                } else
                {
                    fn(boost::asio::error::make_error_code(
                           boost::asio::error::operation_aborted),
                       0);
                }
            });
    }

    template <typename Handler = read_handler_t>
    void async_read(unsigned char *data, size_t len, Handler &&fn)
    {
        mutable_buffer_t buf{data, len};
        async_read(buf, std::forward<Handler>(fn));
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

    err_t _status_chg(state to) noexcept
    {
        state from = _state.load();
        switch(from)
        {
            case state::closed: {
                if(to == state::connected)
                    return boost::system::errc::make_error_code(
                        boost::system::errc::operation_not_permitted);
                break;
            }
            case state::connecting: {
                if(to == state::connecting)
                    return boost::system::errc::make_error_code(
                        boost::system::errc::operation_in_progress);

                break;
            }
            case state::connected: {
                if(to == state::connected || to == state::connecting)
                    return boost::system::errc::make_error_code(
                        boost::system::errc::already_connected);

                break;
            }
        }
        return _state.compare_exchange_strong(from, to)
                   ? err_t{}
                   : boost::system::errc::make_error_code(
                         boost::system::errc::operation_not_permitted);
    }

  private:
    io_t                       &_io;
    std::unique_ptr<raw_sock_t> _sock;
    std::atomic<state>          _state{state::closed};
};

} // namespace hj

#endif // TCP_SOCKET_HPP