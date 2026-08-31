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

#ifndef UDP_SOCKET_HPP
#define UDP_SOCKET_HPP

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <boost/asio.hpp>
#include <boost/version.hpp>

namespace hj::udp
{

namespace opt
{
using send_buf_sz = boost::asio::ip::udp::socket::send_buffer_size;
using recv_buf_sz = boost::asio::ip::udp::socket::receive_buffer_size;
using reuse_addr  = boost::asio::ip::udp::socket::reuse_address;
using keep_alive  = boost::asio::ip::udp::socket::keep_alive;
using broadcast   = boost::asio::ip::udp::socket::broadcast;
} // namespace opt

class socket final : public std::enable_shared_from_this<socket>
{
  public:
    using executor_t     = boost::asio::any_io_executor;
    using err_t          = boost::system::error_code;
    using address_t      = boost::asio::ip::address;
    using sock_t         = boost::asio::ip::udp::socket;
    using endpoint_t     = boost::asio::ip::udp::endpoint;
    using const_buffer_t = boost::asio::const_buffer;
    using multi_buffer_t = boost::asio::mutable_buffer;

    template <typename... Args>
    static std::shared_ptr<socket> create(Args &&...args)
    {
        return std::shared_ptr<socket>(new socket(std::forward<Args>(args)...));
    }

    explicit socket(executor_t exec, bool ipv6 = false)
        : _sock(exec,
                ipv6 ? boost::asio::ip::udp::v6() : boost::asio::ip::udp::v4())
    {
    }

    template <typename ExecutionContext,
              typename = std::enable_if_t<
                  std::is_convertible_v<ExecutionContext &,
                                        boost::asio::execution_context &>>>
    explicit socket(ExecutionContext &context, bool ipv6 = false)
        : _sock(context,
                ipv6 ? boost::asio::ip::udp::v6() : boost::asio::ip::udp::v4())
    {
    }

    socket(executor_t exec, const endpoint_t &ep)
        : _sock(exec, ep)
    {
    }

    socket(executor_t exec, const std::string &ip, uint16_t port)
        : _sock(exec, endpoint(ip, port))
    {
    }

    explicit socket(sock_t &&raw_sock) noexcept
        : _sock(std::move(raw_sock))
    {
    }

    ~socket() noexcept = default;

    socket(const socket &)            = delete;
    socket &operator=(const socket &) = delete;
    socket(socket &&) noexcept        = default;
    socket &operator=(socket &&)      = default;

    [[nodiscard]] bool is_open() const noexcept { return _sock.is_open(); }

    [[nodiscard]] bool is_connected() const noexcept
    {
        err_t ec;
        _sock.remote_endpoint(ec);
        return !ec;
    }

    [[nodiscard]] sock_t       &native_handle() noexcept { return _sock; }
    [[nodiscard]] const sock_t &native_handle() const noexcept { return _sock; }
    [[nodiscard]] executor_t    get_executor() noexcept
    {
        return _sock.get_executor();
    }

    template <typename Option>
    void set_option(const Option &option, err_t &ec) noexcept
    {
        _sock.set_option(option, ec);
    }

    void bind(const endpoint_t &ep, err_t &ec) noexcept { _sock.bind(ep, ec); }

    void bind(uint16_t port, err_t &ec) noexcept
    {
        bind(endpoint("0.0.0.0", port), ec);
    }

    void connect(const endpoint_t &ep, err_t &ec) noexcept
    {
        _sock.connect(ep, ec);
    }

    void connect(const std::string &ip, uint16_t port, err_t &ec) noexcept
    {
        connect(endpoint(ip, port), ec);
    }

    void disconnect(err_t &ec) noexcept { _sock.close(ec); }

    template <typename ConstBufferSequence>
    size_t send_to(const ConstBufferSequence &buffers,
                   const endpoint_t          &ep,
                   err_t                     &ec) noexcept
    {
        return _sock.send_to(buffers, ep, 0, ec);
    }

    size_t send_to(const char       *data,
                   size_t            len,
                   const endpoint_t &ep,
                   err_t            &ec) noexcept
    {
        return send_to(boost::asio::buffer(data, len), ep, ec);
    }

    template <typename ConstBufferSequence>
    size_t send(const ConstBufferSequence &buffers, err_t &ec) noexcept
    {
        return _sock.send(buffers, 0, ec);
    }

    template <typename MutableBufferSequence>
    size_t receive_from(const MutableBufferSequence &buffers,
                        endpoint_t                  &ep,
                        err_t                       &ec) noexcept
    {
        return _sock.receive_from(buffers, ep, 0, ec);
    }

    size_t
    receive_from(char *data, size_t len, endpoint_t &ep, err_t &ec) noexcept
    {
        return receive_from(boost::asio::buffer(data, len), ep, ec);
    }

    template <typename MutableBufferSequence>
    size_t receive(const MutableBufferSequence &buffers, err_t &ec) noexcept
    {
        return _sock.receive(buffers, 0, ec);
    }

    template <typename ConstBufferSequence, typename WriteHandler>
    void async_send_to(const ConstBufferSequence &buffers,
                       const endpoint_t          &ep,
                       WriteHandler             &&handler)
    {
        auto self = shared_from_this();
        _sock.async_send_to(buffers,
                            ep,
                            [self, fn = std::forward<WriteHandler>(handler)](
                                const err_t &ec,
                                std::size_t  bytes_transferred) mutable {
                                fn(ec, bytes_transferred);
                            });
    }

    template <typename MutableBufferSequence, typename ReadHandler>
    void async_receive_from(const MutableBufferSequence &buffers,
                            endpoint_t                  &ep,
                            ReadHandler                &&handler)
    {
        auto self = shared_from_this();
        _sock.async_receive_from(
            buffers,
            ep,
            [self, fn = std::forward<ReadHandler>(handler)](
                const err_t &ec,
                std::size_t  bytes_transferred) mutable {
                fn(ec, bytes_transferred);
            });
    }

    void close(err_t &ec) noexcept { _sock.close(ec); }

    void close() noexcept
    {
        err_t ec;
        _sock.close(ec);
    }

    static address_t address(const std::string &ip)
    {
#if BOOST_VERSION < 108700
        return address_t::from_string(ip);
#else
        return boost::asio::ip::make_address(ip);
#endif
    }

    static endpoint_t endpoint(const std::string &ip, uint16_t port)
    {
        return endpoint_t(address(ip), port);
    }

  private:
    sock_t _sock;
};

} // namespace hj::udp

#endif // UDP_SOCKET_HPP