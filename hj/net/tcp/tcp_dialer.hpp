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

#ifndef TCP_DIALER_HPP
#define TCP_DIALER_HPP

#include <unordered_set>
#include <vector>
#include <mutex>
#include <functional>
#include <string>
#include <memory>
#include <system_error>
#include <utility>
#include <boost/asio/post.hpp>
#include <boost/version.hpp>
#include <hj/net/tcp/tcp_socket.hpp>

namespace hj
{

class tcp_dialer
{
  public:
    static constexpr std::size_t dial_max_size = 1024;

    enum class state
    {
        open,
        closing,
        closed
    };

    using io_t            = hj::tcp_socket::io_t;
    using err_t           = hj::tcp_socket::err_t;
    using endpoint_t      = boost::asio::ip::tcp::endpoint;
    using raw_sock_t      = boost::asio::ip::tcp::socket;
    using sock_ptr_t      = std::shared_ptr<hj::tcp_socket>;
    using dial_handler_t  = std::function<void(const err_t &, sock_ptr_t)>;
    using range_handler_t = std::function<bool(sock_ptr_t)>;

  public:
    explicit tcp_dialer(io_t &io, std::size_t max_size = dial_max_size)
        : _io{io}
        , _max_size{max_size}
    {
        if(_max_size == 0)
            throw std::invalid_argument("max_size must be greater than 0");
    }

    ~tcp_dialer() { close(); }

    tcp_dialer()                              = delete;
    tcp_dialer(const tcp_dialer &)            = delete;
    tcp_dialer &operator=(const tcp_dialer &) = delete;
    tcp_dialer(tcp_dialer &&)                 = delete;
    tcp_dialer &operator=(tcp_dialer &&)      = delete;

    [[nodiscard]] bool is_open() const noexcept
    {
        return _state.load(std::memory_order_relaxed) == state::open;
    }

    inline state status() const noexcept
    {
        return _state.load(std::memory_order_relaxed);
    }

    inline std::size_t size() const noexcept
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _socks.size();
    }

    inline bool is_exist(sock_ptr_t sock) const noexcept
    {
        if(!sock)
            return false;

        std::lock_guard<std::mutex> lock(_mu);
        return _socks.find(sock) != _socks.end();
    }

    sock_ptr_t
    dial(endpoint_t                ep,
         err_t                    &err,
         std::chrono::milliseconds timeout  = std::chrono::milliseconds(2000),
         int                       attempts = 1)
    {
        if(attempts <= 0)
            throw std::invalid_argument("attempts must be greater than 0");

        if(!is_open())
        {
            err = boost::system::errc::make_error_code(
                boost::system::errc::bad_file_descriptor);
            return nullptr;
        }

        {
            std::lock_guard<std::mutex> lock(_mu);
            if(_socks.size() >= _max_size)
            {
                err = boost::system::errc::make_error_code(
                    boost::system::errc::no_buffer_space);
                return nullptr;
            }
        }

        auto sock = std::make_shared<tcp_socket>(_io);
        err       = sock->connect(ep, timeout, attempts);
        if(!err.failed())
        {
            if(_add(sock))
                return sock;

            err = boost::system::errc::make_error_code(
                boost::system::errc::operation_not_permitted);
            sock->close();
        }

        return nullptr;
    }

    sock_ptr_t
    dial(const char               *ip,
         const std::uint16_t       port,
         err_t                    &err,
         std::chrono::milliseconds timeout  = std::chrono::milliseconds(2000),
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
            err = boost::system::errc::make_error_code(
                boost::system::errc::invalid_argument);
            return nullptr;
        }

        return dial(ep, err, timeout, attempts);
    }

    void async_dial(endpoint_t ep, dial_handler_t &&fn)
    {
        if(!fn)
            return;

        if(!is_open())
        {
            if(fn)
                fn(boost::system::errc::make_error_code(
                       boost::system::errc::bad_file_descriptor),
                   nullptr);
            return;
        }

        {
            std::lock_guard<std::mutex> lock(_mu);
            if(_socks.size() >= _max_size)
            {
                boost::asio::post(_io, [fn = std::move(fn)]() {
                    fn(boost::system::errc::make_error_code(
                           boost::system::errc::no_buffer_space),
                       nullptr);
                });
                return;
            }
        }

        auto  sock_base = std::make_unique<raw_sock_t>(_io);
        auto *raw_sock  = sock_base.get();
        auto &io_ref    = _io;
        auto  sock =
            std::make_shared<tcp_socket>(io_ref,
                                         std::move(sock_base),
                                         hj::tcp_socket::state::connecting);

        if(!_add(sock))
        {
            boost::asio::post(_io, [fn = std::move(fn)]() {
                fn(boost::system::errc::make_error_code(
                       boost::system::errc::operation_not_permitted),
                   nullptr);
            });
            return;
        }

        raw_sock->async_connect(
            ep,
            [&io_ref, fn = std::move(fn), sock = std::move(sock)](
                const err_t &err) mutable {
                if(err.failed())
                {
                    if(fn)
                        fn(err, nullptr);

                    return;
                }

                if(fn)
                    fn(err, std::move(sock));
            });
    }

    void async_dial(const char *ip, const uint16_t port, dial_handler_t &&fn)
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
            boost::asio::post(_io, [fn = std::move(fn)]() {
                fn(boost::system::errc::make_error_code(
                       boost::system::errc::invalid_argument),
                   nullptr);
            });
            return;
        }

        async_dial(ep, std::move(fn));
    }

    err_t range(range_handler_t &&handler)
    {
        if(!handler)
            return boost::system::errc::make_error_code(
                boost::system::errc::invalid_argument);

        std::vector<sock_ptr_t> snapshot;
        {
            std::lock_guard<std::mutex> lock(_mu);
            snapshot.reserve(_socks.size());
            snapshot.assign(_socks.begin(), _socks.end());
        }

        for(const auto &sock : snapshot)
        {
            if(!is_open())
                return boost::system::errc::make_error_code(
                    boost::system::errc::operation_canceled);

            try
            {
                if(!handler(sock))
                    return boost::system::errc::make_error_code(
                        boost::system::errc::operation_canceled);
            }
            catch(const err_t &e)
            {
                return e;
            }
            catch(const std::exception &)
            {
                return boost::system::errc::make_error_code(
                    boost::system::errc::operation_canceled);
            }
            catch(...)
            {
                return boost::system::errc::make_error_code(
                    boost::system::errc::operation_canceled);
            }
        }
        return err_t{};
    }

    bool remove(sock_ptr_t sock) noexcept
    {
        if(!sock)
            return false;

        if(_del(sock))
        {
            sock->close();
            return true;
        }
        return false;
    }

    void close()
    {
        state expected = state::open;
        if(!_state.compare_exchange_strong(expected,
                                           state::closing,
                                           std::memory_order_acq_rel))
            return;

        std::unordered_set<sock_ptr_t> socks_to_close;
        {
            std::lock_guard<std::mutex> lock(_mu);
            socks_to_close.swap(_socks);
        }

        for(auto &sock : socks_to_close)
        {
            if(sock)
                sock->close();
        }
        _state.store(state::closed, std::memory_order_release);
    }

  private:
    bool _add(sock_ptr_t sock)
    {
        std::lock_guard<std::mutex> lock(_mu);
        if(!is_open())
            return false;

        if(_socks.size() >= _max_size)
            return false;

        return _socks.insert(sock).second;
    }

    bool _del(sock_ptr_t sock)
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _socks.erase(sock) > 0;
    }


  private:
    io_t                          &_io;
    std::size_t                    _max_size;
    mutable std::mutex             _mu;
    std::atomic<state>             _state{state::open};
    std::unordered_set<sock_ptr_t> _socks;
};

}

#endif