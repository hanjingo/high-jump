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

namespace hj
{

/**
 * @brief High-performance thin wrapper around boost::asio::ip::udp::socket.
 * 
 * @note Thread Safety Contract:
 * Thread safety follows Boost.Asio socket semantics. Concurrent access to the
 * same socket object must obey Asio's documented thread-safety requirements.
 * Specifically:
 *  - Distinct objects: Safe to access concurrently from different threads.
 *  - Shared objects: It is unsafe to invoke non-const member functions (or start
 *    concurrent operations of the same type, e.g., multiple concurrent sends or closes)
 *    on the same socket object simultaneously without external synchronization.
 *  - One concurrent read and one concurrent write operation is supported as long as
 *    the underlying Boost.Asio socket permits it.
 */
class udp_socket final : public std::enable_shared_from_this<udp_socket>
{
  private:
    struct udp_socket_key
    {
        explicit udp_socket_key() = default;
    };

  public:
    using executor_t         = boost::asio::any_io_executor;
    using err_t              = boost::system::error_code;
    using address_t          = boost::asio::ip::address;
    using raw_sock_t         = boost::asio::ip::udp::socket;
    using endpoint_t         = boost::asio::ip::udp::endpoint;
    using const_buffer_t     = boost::asio::const_buffer;
    using mutable_buffer_t   = boost::asio::mutable_buffer;
    using native_handle_type = raw_sock_t::native_handle_type;

    using opt_send_buf_sz   = boost::asio::ip::udp::socket::send_buffer_size;
    using opt_recv_buf_sz   = boost::asio::ip::udp::socket::receive_buffer_size;
    using opt_reuse_addr    = boost::asio::ip::udp::socket::reuse_address;
    using opt_broadcast     = boost::asio::ip::udp::socket::broadcast;
    using opt_join_group    = boost::asio::ip::multicast::join_group;
    using opt_leave_group   = boost::asio::ip::multicast::leave_group;
    using opt_multicast_ttl = boost::asio::ip::multicast::hops;
    using opt_multicast_loop = boost::asio::ip::multicast::enable_loopback;
    using opt_outbound_interface =
        boost::asio::ip::multicast::outbound_interface;

    explicit udp_socket(udp_socket_key, executor_t exec, bool ipv6 = false)
        : _sock(exec,
                ipv6 ? boost::asio::ip::udp::v6() : boost::asio::ip::udp::v4())
        , _is_ipv6(ipv6)
    {
    }

    template <typename ExecutionContext,
              typename = std::enable_if_t<
                  std::is_convertible_v<ExecutionContext &,
                                        boost::asio::execution_context &>>>
    explicit udp_socket(udp_socket_key,
                        ExecutionContext &context,
                        bool              ipv6 = false)
        : _sock(context,
                ipv6 ? boost::asio::ip::udp::v6() : boost::asio::ip::udp::v4())
        , _is_ipv6(ipv6)
    {
    }

    udp_socket(udp_socket_key, executor_t exec, const endpoint_t &ep)
        : _sock(exec, ep)
        , _is_ipv6(ep.address().is_v6())
    {
    }

    explicit udp_socket(udp_socket_key, raw_sock_t &&raw_sock) noexcept
        : _sock(std::move(raw_sock))
        , _is_ipv6(false)
    {
        if(!_sock.is_open())
            return;

        err_t ec;
        auto  ep = _sock.local_endpoint(ec);
        if(!ec)
        {
            _is_ipv6 = ep.address().is_v6();
        } else
        {
            boost::asio::ip::udp::socket::reuse_address opt;
            _sock.get_option(opt, ec);
        }
    }

    ~udp_socket() noexcept = default;

    udp_socket(const udp_socket &)            = delete;
    udp_socket &operator=(const udp_socket &) = delete;
    udp_socket(udp_socket &&) noexcept        = default;
    udp_socket &operator=(udp_socket &&)      = default;

    template <typename... Args>
    static std::shared_ptr<udp_socket> make_shared(Args &&...args)
    {
        return std::make_shared<hj::udp_socket>(udp_socket_key{},
                                                std::forward<Args>(args)...);
    }

    [[nodiscard]] bool is_open() const noexcept { return _sock.is_open(); }

    [[nodiscard]] bool has_remote_endpoint() const noexcept
    {
        err_t ec;
        _sock.remote_endpoint(ec);
        return !ec;
    }

    [[nodiscard]] raw_sock_t       &raw_socket() noexcept { return _sock; }
    [[nodiscard]] const raw_sock_t &raw_socket() const noexcept
    {
        return _sock;
    }
    [[nodiscard]] native_handle_type native_handle() noexcept
    {
        return _sock.native_handle();
    }
    [[nodiscard]] executor_t get_executor() noexcept
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
        if(!_sock.is_open())
        {
            _sock.open(_is_ipv6 ? boost::asio::ip::udp::v6()
                                : boost::asio::ip::udp::v4(),
                       ec);
            if(ec)
                return;
        }

        if(_is_ipv6)
        {
            bind(endpoint_t(boost::asio::ip::address_v6::any(), port), ec);
        } else
        {
            bind(endpoint_t(boost::asio::ip::address_v4::any(), port), ec);
        }
    }

    void bind(const std::string &ip, uint16_t port, err_t &ec) noexcept
    {
        bind(endpoint(ip, port), ec);
    }

    void connect(const endpoint_t &ep, err_t &ec) noexcept
    {
        _sock.connect(ep, ec);
    }

    void connect(const std::string &ip, uint16_t port, err_t &ec) noexcept
    {
        connect(endpoint(ip, port), ec);
    }

    void join_multicast_group(const address_t &multicast_addr,
                              err_t           &ec) noexcept
    {
        set_option(opt_join_group(multicast_addr), ec);
    }

    void join_multicast_group(const std::string &multicast_ip,
                              err_t             &ec) noexcept
    {
        join_multicast_group(address(multicast_ip), ec);
    }

    void join_multicast_group(const std::string &multicast_ip,
                              const std::string &interface_ip,
                              err_t             &ec) noexcept
    {
        auto group_addr = address(multicast_ip);
        auto if_addr    = address(interface_ip);

        if(group_addr.is_v4() && if_addr.is_v4())
        {
            set_option(
                boost::asio::ip::multicast::join_group(group_addr.to_v4(),
                                                       if_addr.to_v4()),
                ec);
        } else
        {
            set_option(opt_join_group(group_addr), ec);
        }
    }

    void leave_multicast_group(const address_t &multicast_addr,
                               err_t           &ec) noexcept
    {
        set_option(opt_leave_group(multicast_addr), ec);
    }

    void leave_multicast_group(const std::string &multicast_ip,
                               err_t             &ec) noexcept
    {
        leave_multicast_group(address(multicast_ip), ec);
    }

    void set_multicast_ttl(int ttl, err_t &ec) noexcept
    {
        set_option(opt_multicast_ttl(ttl), ec);
    }

    void set_multicast_loop(bool enable, err_t &ec) noexcept
    {
        set_option(opt_multicast_loop(enable), ec);
    }

    void set_outbound_interface(const std::string &interface_ip,
                                err_t             &ec) noexcept
    {
        auto if_addr = address(interface_ip);
        if(if_addr.is_v4())
        {
            set_option(opt_outbound_interface(if_addr.to_v4()), ec);
        }
    }

    template <typename ConstBufferSequence>
    size_t send_to_group(const ConstBufferSequence &buffers,
                         const std::string         &multicast_ip,
                         uint16_t                   port,
                         err_t                     &ec) noexcept
    {
        return send_to(buffers, endpoint(multicast_ip, port), ec);
    }

    size_t send_to_group(const char        *data,
                         size_t             len,
                         const std::string &multicast_ip,
                         uint16_t           port,
                         err_t             &ec) noexcept
    {
        return send_to(boost::asio::buffer(data, len),
                       endpoint(multicast_ip, port),
                       ec);
    }

    template <typename ConstBufferSequence, typename WriteHandler>
    void async_send_to_group(const ConstBufferSequence &buffers,
                             const std::string         &multicast_ip,
                             uint16_t                   port,
                             WriteHandler             &&handler)
    {
        async_send_to(buffers,
                      endpoint(multicast_ip, port),
                      std::forward<WriteHandler>(handler));
    }

    template <typename WriteHandler>
    void async_send_to_group(std::string        data,
                             const std::string &multicast_ip,
                             uint16_t           port,
                             WriteHandler     &&handler)
    {
        async_send_to(std::move(data),
                      endpoint(multicast_ip, port),
                      std::forward<WriteHandler>(handler));
    }

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

    template <typename WriteHandler>
    void async_send_to(std::string       data,
                       const endpoint_t &ep,
                       WriteHandler    &&handler)
    {
        auto self    = shared_from_this();
        auto payload = std::make_shared<std::string>(std::move(data));

        _sock.async_send_to(
            boost::asio::buffer(*payload),
            ep,
            [self, payload, fn = std::forward<WriteHandler>(handler)](
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
    raw_sock_t _sock;
    bool       _is_ipv6{false};
};

} // namespace hj

#endif // UDP_SOCKET_HPP