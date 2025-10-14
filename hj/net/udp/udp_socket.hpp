#ifndef UDP_SOCKET_HPP
#define UDP_SOCKET_HPP

#include <iostream>
#include <memory>
#include <functional>
#include <boost/version.hpp>
#include <boost/asio.hpp>

namespace hj
{
namespace udp
{
namespace opt
{
using send_buf_sz = boost::asio::ip::udp::socket::send_buffer_size;
using recv_buf_sz = boost::asio::ip::udp::socket::receive_buffer_size;
using reuse_addr  = boost::asio::ip::udp::socket::reuse_address;
using keep_alive  = boost::asio::ip::udp::socket::keep_alive;
using broadcast   = boost::asio::ip::udp::socket::broadcast;
} // namespace opt

enum class error_code
{
    not_open,
    invalid_argument,
    other,
};

class error_category : public std::error_category
{
  public:
    const char *name() const noexcept override { return "udp"; }
    std::string message(int ev) const override
    {
        switch(static_cast<error_code>(ev))
        {
            case error_code::not_open:
                return "Socket not open";
            case error_code::invalid_argument:
                return "Invalid argument";
            case error_code::other:
                return "Other error";
            default:
                return "Unknown error";
        }
    }
};

static inline std::error_code make_err(error_code e)
{
    static error_category cat;
    return std::error_code(static_cast<int>(e), cat);
}

class socket : public std::enable_shared_from_this<hj::udp::socket>
{
  public:
    using executor_t     = boost::asio::any_io_executor;
    using err_t          = boost::system::error_code;
    using address_t      = boost::asio::ip::address;
    using const_buffer_t = boost::asio::const_buffer;
    using multi_buffer_t = boost::asio::mutable_buffer;
    using sock_t         = boost::asio::ip::udp::socket;
    using endpoint_t     = boost::asio::ip::udp::endpoint;

    using send_handler_t =
        std::function<void(const std::error_code &, std::size_t)>;
    using recv_handler_t =
        std::function<void(const std::error_code &, std::size_t)>;

    socket(executor_t exec, bool ipv6 = false)
        : _exec(exec)
        , _sock(std::make_unique<sock_t>(exec))
    {
        if(ipv6)
            _sock->open(boost::asio::ip::udp::v6());
        else
            _sock->open(boost::asio::ip::udp::v4());
    }
    socket(executor_t exec, endpoint_t &ep)
        : _exec(exec)
        , _sock(std::make_unique<sock_t>(exec, ep))
    {
    }
    socket(executor_t exec, const std::string &ip, const uint16_t port)
        : _exec(exec)
        , _sock(std::make_unique<sock_t>(exec, endpoint(ip, port)))
    {
    }
    socket(executor_t exec, std::unique_ptr<sock_t> sock) noexcept
        : _exec(exec)
        , _sock(std::move(sock))
    {
    }

    virtual ~socket() noexcept { close(); }

    socket(const socket &)            = delete;
    socket &operator=(const socket &) = delete;
    socket(socket &&)                 = default;
    socket &operator=(socket &&)      = default;

    inline bool is_open() const noexcept { return _sock && _sock->is_open(); }
    inline bool is_connected() const noexcept
    {
        return is_open() && _sock->remote_endpoint().address() != address_t();
    }

    template <typename Opt>
    inline void set_option(Opt opt, std::error_code &ec) noexcept
    {
        if(!is_open())
        {
            ec = make_err(error_code::not_open);
            return;
        }

        err_t err;
        _sock->set_option(opt, err);
        ec = err;
    }

    void bind(const endpoint_t &ep, std::error_code &ec) noexcept
    {
        if(!is_open())
        {
            ec = make_err(error_code::not_open);
            return;
        }

        if(ep.port() == 0)
        {
            ec = make_err(error_code::invalid_argument);
            return;
        }

        err_t err;
        _sock->bind(ep, err);
        ec = err;
    }

    void bind(const uint16_t port, std::error_code &ec) noexcept
    {
        bind(endpoint("0.0.0.0", port), ec);
    }

    void connect(const endpoint_t &ep, std::error_code &ec) noexcept
    {
        if(!is_open())
        {
            ec = make_err(error_code::not_open);
            return;
        }

        err_t err;
        _sock->connect(ep, err);
        ec = err;
    }

    void connect(const std::string &ip,
                 const uint16_t     port,
                 std::error_code   &ec) noexcept
    {
        return connect(endpoint(ip, port), ec);
    }

    void disconnect(std::error_code &ec) noexcept
    {
        return connect(endpoint_t(), ec);
    }

    size_t send(const const_buffer_t &buf,
                const endpoint_t     &ep,
                std::error_code      &ec) noexcept
    {
        if(!is_open())
        {
            ec = make_err(error_code::not_open);
            return 0;
        }

        err_t  err;
        size_t len = _sock->send_to(buf, ep, 0, err);
        ec         = err;
        return err ? 0 : len;
    }

    size_t send(const char       *data,
                size_t            len,
                const endpoint_t &ep,
                std::error_code  &ec) noexcept
    {
        if(!data || len == 0)
        {
            ec = make_err(error_code::invalid_argument);
            return 0;
        }
        return send(boost::asio::buffer(data, len), ep, ec);
    }

    size_t send(const std::vector<const_buffer_t> &bufs,
                const endpoint_t                  &ep,
                std::error_code                   &ec) noexcept
    {
        if(!is_open())
        {
            ec = make_err(error_code::not_open);
            return 0;
        }

        err_t  err;
        size_t len = _sock->send_to(bufs, ep, 0, err);
        ec         = err;
        return err ? 0 : len;
    }

    void async_send(const const_buffer_t &buf,
                    const endpoint_t     &ep,
                    send_handler_t      &&fn)
    {
        if(!is_open())
        {
            fn(make_err(error_code::not_open), 0);
            return;
        }
        _sock->async_send_to(buf, ep, std::move(fn));
    }

    void async_send(const char       *data,
                    size_t            len,
                    const endpoint_t &ep,
                    send_handler_t  &&fn)
    {
        if(!data || len == 0)
        {
            fn(make_err(error_code::invalid_argument), 0);
            return;
        }
        async_send(boost::asio::buffer(data, len), ep, std::move(fn));
    }

    size_t
    recv(multi_buffer_t &buf, endpoint_t &ep, std::error_code &ec) noexcept
    {
        if(!is_open())
        {
            ec = make_err(error_code::not_open);
            return 0;
        }
        err_t  err;
        size_t recvd = _sock->receive_from(buf, ep, 0, err);
        ec           = err;
        return err ? 0 : recvd;
    }

    size_t
    recv(char *data, size_t len, endpoint_t &ep, std::error_code &ec) noexcept
    {
        if(!data || len == 0)
        {
            ec = make_err(error_code::invalid_argument);
            return 0;
        }

        multi_buffer_t buf{data, len};
        return recv(buf, ep, ec);
    }

    size_t recv(std::vector<multi_buffer_t> &bufs,
                endpoint_t                  &ep,
                std::error_code             &ec) noexcept
    {
        if(!is_open())
        {
            ec = make_err(error_code::not_open);
            return 0;
        }

        err_t  err;
        size_t recvd = _sock->receive_from(bufs, ep, 0, err);
        ec           = err;
        return err ? 0 : recvd;
    }

    void async_recv(multi_buffer_t &buf, endpoint_t &ep, recv_handler_t &&fn)
    {
        if(!is_open())
        {
            fn(make_err(error_code::not_open), 0);
            return;
        }
        _sock->async_receive_from(buf, ep, std::move(fn));
    }

    void async_recv(char *data, size_t len, endpoint_t &ep, recv_handler_t &&fn)
    {
        if(!data || len == 0)
        {
            fn(make_err(error_code::invalid_argument), 0);
            return;
        }

        multi_buffer_t buf{data, len};
        async_recv(buf, ep, std::move(fn));
    }

    void close() noexcept
    {
        if(_sock && _sock->is_open())
        {
            err_t ec;
            _sock->close(ec);
        }
        _sock.reset();
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
    executor_t              _exec;
    std::unique_ptr<sock_t> _sock;
};

} // namespace udp
} // namespace hj

#endif