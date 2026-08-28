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

#ifndef TCP_CONN_HPP
#define TCP_CONN_HPP

#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <boost/version.hpp>

#include <hj/net/tcp/tcp_socket.hpp>

namespace hj
{

class tcp_conn : public std::enable_shared_from_this<tcp_conn>
{
  public:
    static constexpr std::size_t max_pack_sz = 65535;

    using io_t  = boost::asio::io_context;
    using err_t = boost::system::error_code;

    using strand_t   = boost::asio::strand<io_t::executor_type>;
    using conn_ptr_t = std::shared_ptr<tcp_conn>;
    using sock_ptr_t = std::shared_ptr<hj::tcp_socket>;

    using msg_buffer_t = std::vector<std::uint8_t>;

    using write_handler_t =
        std::function<void(conn_ptr_t, msg_buffer_t &&, const err_t &)>;
    using read_handler_t = std::function<void(conn_ptr_t, msg_buffer_t &&)>;
    using err_handler_t  = std::function<void(conn_ptr_t, const err_t &)>;
    using conn_handler_t = std::function<void(conn_ptr_t, const err_t &)>;

  private:
    struct tcp_conn_key
    {
        explicit tcp_conn_key() = default;
    };

  public:
    explicit tcp_conn(tcp_conn_key, io_t &io)
        : _io(io)
        , _strand{boost::asio::make_strand(io)}
        , _sock{nullptr}
    {
    }

    explicit tcp_conn(tcp_conn_key, io_t &io, sock_ptr_t sock)
        : _io(io)
        , _strand{boost::asio::make_strand(io)}
        , _sock{std::move(sock)}
    {
    }

    ~tcp_conn() noexcept
    {
        _closed.store(true, std::memory_order_release);
        if(_sock)
        {
            _sock->close();
        }
        _send_queue.clear();
    }

    tcp_conn(const tcp_conn &)            = delete;
    tcp_conn &operator=(const tcp_conn &) = delete;
    tcp_conn(tcp_conn &&)                 = delete;
    tcp_conn &operator=(tcp_conn &&)      = delete;

    template <typename... Args>
    static conn_ptr_t make_shared(Args &&...args)
    {
        auto conn = std::make_shared<tcp_conn>(tcp_conn_key{},
                                               std::forward<Args>(args)...);
        conn->_init();
        return conn;
    }

    bool is_connected() const noexcept
    {
        return _sock != nullptr
               && _sock->status() == hj::tcp_socket::state::connected;
    }

    bool is_closed() const noexcept
    {
        return _closed.load(std::memory_order_acquire);
    }

    void set_read_callback(read_handler_t cb) { _read_cb = std::move(cb); }
    void set_error_callback(err_handler_t cb) { _err_cb = std::move(cb); }

    void run()
    {
        auto self = shared_from_this();
        boost::asio::dispatch(_strand, [this, self]() {
            if(!_closed && is_connected())
            {
                _read();
            }
        });
    }

    void
    async_connect(const char *ip, std::uint16_t port, conn_handler_t handler)
    {
        auto self = shared_from_this();
        boost::asio::dispatch(
            _strand,
            [this,
             self,
             ip_str = std::string(ip),
             port,
             handler = std::move(handler)]() mutable {
                if(_closed.load())
                {
                    if(handler)
                        handler(self,
                                boost::system::errc::make_error_code(
                                    boost::system::errc::bad_file_descriptor));
                    return;
                }

                if(!_sock)
                    _sock = tcp_socket::make_shared(_io);

                _sock->async_connect(ip_str.c_str(),
                                     port,
                                     [this, self, handler = std::move(handler)](
                                         const err_t &ec) {
                                         boost::asio::dispatch(
                                             _strand,
                                             [this, self, handler, ec]() {
                                                 if(!ec)
                                                     _read();

                                                 if(handler)
                                                     handler(self, ec);
                                             });
                                     });
            });
    }

    void send(msg_buffer_t data)
    {
        if(data.empty())
            return;

        auto self = shared_from_this();
        boost::asio::dispatch(_strand,
                              [this, self, data = std::move(data)]() mutable {
                                  if(_closed || !is_connected())
                                      return;

                                  bool write_in_progress = !_send_queue.empty();
                                  _send_queue.push_back(std::move(data));

                                  if(!write_in_progress)
                                      _write();
                              });
    }

    void send(const std::uint8_t *data, std::size_t len)
    {
        if(!data || len == 0)
            return;
        msg_buffer_t buf(data, data + len);
        send(std::move(buf));
    }

    void close()
    {
        bool expected = false;
        if(!_closed.compare_exchange_strong(expected, true))
        {
            return;
        }

        try
        {
            auto self = shared_from_this();
            boost::asio::dispatch(_strand, [this, self]() {
                _send_queue.clear();
                if(_sock)
                {
                    _sock->close();
                }
            });
        }
        catch(const std::bad_weak_ptr &)
        {
            _send_queue.clear();
            if(_sock)
            {
                _sock->close();
            }
        }
    }

  private:
    void _init()
    {
        if(!_sock)
        {
            _sock = tcp_socket::make_shared(_io);
        }
    }

    void _read()
    {
        auto self = shared_from_this();
        _recv_buf.resize(max_pack_sz);

        _sock->async_read(
            boost::asio::buffer(_recv_buf),
            boost::asio::bind_executor(
                _strand,
                [this, self](const err_t &ec, std::size_t bytes_transferred) {
                    if(_closed)
                        return;

                    if(!ec && bytes_transferred > 0)
                    {
                        _recv_buf.resize(bytes_transferred);
                        if(_read_cb)
                            _read_cb(self, std::move(_recv_buf));

                        _read();
                    } else
                    {
                        _handle_error(ec);
                    }
                }));
    }

    void _write()
    {
        auto        self        = shared_from_this();
        const auto &current_msg = _send_queue.front();

        _sock->async_write(
            boost::asio::buffer(current_msg),
            boost::asio::bind_executor(
                _strand,
                [this, self](const err_t &ec, std::size_t bytes_transferred) {
                    if(_closed)
                        return;

                    if(!ec)
                    {
                        _send_queue.pop_front();
                        if(!_send_queue.empty())
                        {
                            _write();
                        }
                    } else
                    {
                        _handle_error(ec);
                    }
                }));
    }

    void _handle_error(const err_t &ec)
    {
        if(_closed.exchange(true))
            return;

        if(_sock)
        {
            _sock->close();
        }

        _send_queue.clear();

        if(_err_cb)
        {
            _err_cb(shared_from_this(), ec);
        }
    }

  private:
    io_t             &_io;
    strand_t          _strand;
    sock_ptr_t        _sock;
    std::atomic<bool> _closed{false};

    std::deque<msg_buffer_t> _send_queue;
    msg_buffer_t             _recv_buf;

    read_handler_t _read_cb;
    err_handler_t  _err_cb;
};

} // namespace hj

#endif // TCP_CONN_HPP