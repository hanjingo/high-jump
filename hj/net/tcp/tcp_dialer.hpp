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
#include <mutex>
#include <functional>
#include <boost/version.hpp>
#include <hj/net/tcp/tcp_socket.hpp>

#ifndef TCP_DIALER_MAX_SIZE
#define TCP_DIALER_MAX_SIZE 1024
#endif

namespace hj
{

class tcp_dialer : public std::enable_shared_from_this<tcp_dialer>
{
  public:
    using io_t            = hj::tcp_socket::io_t;
    using err_t           = hj::tcp_socket::err_t;
    using sock_ptr_t      = std::shared_ptr<hj::tcp_socket>;
    using dial_handler_t  = std::function<void(const err_t &, sock_ptr_t)>;
    using range_handler_t = std::function<bool(sock_ptr_t)>;

  public:
    explicit tcp_dialer(io_t &io, std::size_t max_size = TCP_DIALER_MAX_SIZE)
        : _io{io}
        , _max_size{max_size}
    {
    }

    virtual ~tcp_dialer() { close(); }

    tcp_dialer()                              = delete;
    tcp_dialer(const tcp_dialer &)            = delete;
    tcp_dialer &operator=(const tcp_dialer &) = delete;
    tcp_dialer(tcp_dialer &&)                 = default;
    tcp_dialer &operator=(tcp_dialer &&)      = default;

    inline std::size_t size() const noexcept
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _socks.size();
    }

    inline bool is_exist(sock_ptr_t sock) const noexcept
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _socks.find(sock) != _socks.end();
    }

    sock_ptr_t
    dial(const char               *ip,
         const std::uint16_t       port,
         std::chrono::milliseconds timeout   = std::chrono::milliseconds(2000),
         int                       try_times = 1)
    {
        if(size() > _max_size)
            return nullptr;

        auto sock = std::make_shared<tcp_socket>(_io);
        try
        {
            if(sock->connect(ip, port, timeout, try_times))
            {
                return _add(sock) ? sock : nullptr;
            }
        }
        catch(const std::exception &e)
        {
            std::cerr << "dial exception: " << e.what() << std::endl;
        }

        return nullptr;
    }

    void async_dial(const char *ip, const uint16_t port, dial_handler_t &&fn)
    {
        if(size() > _max_size)
        {
            fn(make_error_code(std::errc::no_buffer_space), nullptr);
            return;
        }

        auto sock = std::make_shared<tcp_socket>(_io);
        sock->async_connect(
            ip,
            port,
            [this, sock, fn = std::move(fn)](const err_t &err) mutable {
                if(!err)
                {
                    this->_add(sock);
                    if(fn)
                        fn(err, sock);
                } else
                {
                    if(fn)
                        fn(err, nullptr);
                }
            });
    }

    void range(range_handler_t &&handler)
    {
        if(!handler)
            return;

        std::lock_guard<std::mutex> lock(_mu);
        for(auto sock : _socks)
        {
            try
            {
                if(!handler(sock))
                    return;
            }
            catch(...)
            {
            }
        }
    }

    void remove(sock_ptr_t sock)
    {
        if(!sock || !is_exist(sock))
            return;

        if(sock->is_connected())
            sock->close();

        _del(sock);
    }

    void close()
    {
        std::lock_guard<std::mutex> lock(_mu);
        for(auto &sock : _socks)
        {
            if(sock)
                sock->close();
        }
        _socks.clear();
    }

  private:
    bool _add(sock_ptr_t sock)
    {
        std::lock_guard<std::mutex> lock(_mu);
        if(_socks.size() > _max_size)
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
    std::unordered_set<sock_ptr_t> _socks;
    mutable std::mutex             _mu;
    std::size_t                    _max_size;
};

}

#endif