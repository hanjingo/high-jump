#ifndef TCP_LISTENER_HPP
#define TCP_LISTENER_HPP

#include <memory>
#include <functional>
#include <atomic>
#include <iostream>
#include <utility>
#include <cstdint>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/version.hpp>

#include <hj/net/tcp/tcp_socket.hpp>

namespace hj
{

class tcp_listener : public std::enable_shared_from_this<tcp_listener>
{
  public:
#if BOOST_VERSION < 108700
    using address_t = tcp_socket::address_t;
#else
    using address_t = boost::asio::ip::address;
#endif

    using io_t       = tcp_socket::io_t;
    using err_t      = tcp_socket::err_t;
    using sock_t     = tcp_socket::sock_t;
    using endpoint_t = tcp_socket::endpoint_t;
    using protocol_t = boost::asio::ip::tcp;
    using acceptor_t = boost::asio::ip::tcp::acceptor;

    using opt_reuse_addr = boost::asio::socket_base::reuse_address;

    using accept_handler_t =
        std::function<void(const err_t &, std::shared_ptr<tcp_socket>)>;

    enum class state
    {
        init,
        opened,
        listening,
        closed
    };

  public:
    template <typename... Args>
    static std::shared_ptr<tcp_listener> create(Args &&...args)
    {
        return std::shared_ptr<tcp_listener>(
            new tcp_listener(std::forward<Args>(args)...));
    }

    explicit tcp_listener(io_t &io) noexcept
        : _io{io}
    {
    }

    tcp_listener(io_t &io, accept_handler_t fn)
        : _io{io}
        , _accept_handler{std::move(fn)}
    {
    }

    ~tcp_listener() noexcept { close(); }

    tcp_listener(const tcp_listener &)            = delete;
    tcp_listener &operator=(const tcp_listener &) = delete;
    tcp_listener(tcp_listener &&)                 = delete;
    tcp_listener &operator=(tcp_listener &&)      = delete;

    [[nodiscard]] bool is_closed() const noexcept
    {
        return _state.load(std::memory_order_relaxed) == state::closed;
    }

    [[nodiscard]] bool is_listening() const noexcept
    {
        return _state.load(std::memory_order_relaxed) == state::listening;
    }

    [[nodiscard]] state current_state() const noexcept
    {
        return _state.load(std::memory_order_relaxed);
    }

    err_t open(const protocol_t &protocol = protocol_t::v4())
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);

        err_t ec;
        if(_acceptor && _acceptor->is_open())
            _close_unlocked();

        _acceptor = std::make_unique<acceptor_t>(_io);
        _acceptor->open(protocol, ec);
        if(!ec)
        {
            _state.store(state::opened, std::memory_order_release);
        }
        return ec;
    }

    template <typename Option>
    err_t set_option(const Option &opt)
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);

        err_t err;
        if(!_acceptor || !_acceptor->is_open() || is_closed())
        {
            err = boost::system::errc::make_error_code(
                boost::system::errc::bad_file_descriptor);
            return err;
        }

        _acceptor->set_option(opt, err);
        return err;
    }

    err_t listen(const endpoint_t &ep,
                 int backlog = boost::asio::socket_base::max_listen_connections)
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);

        err_t ec;
        if(!_acceptor || !_acceptor->is_open())
        {
            ec = open(ep.protocol());
            if(ec)
                return ec;
        }

        _acceptor->bind(ep, ec);
        if(ec)
            return ec;

        _acceptor->listen(backlog, ec);
        if(ec)
            return ec;

        _binded_endpoint = ep;
        _state.store(state::listening, std::memory_order_release);
        return ec;
    }

    err_t listen(uint16_t port,
                 int backlog = boost::asio::socket_base::max_listen_connections)
    {
        endpoint_t ep{protocol_t::v4(), port};
        return listen(ep, backlog);
    }

    err_t listen(const char *ip,
                 uint16_t    port,
                 int backlog = boost::asio::socket_base::max_listen_connections)
    {
        err_t ec;
        auto  addr = _parse_address(ip, ec);
        if(ec)
            return ec;

        return listen(endpoint_t{addr, port}, backlog);
    }

    std::shared_ptr<tcp_socket> accept(err_t &err)
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);

        if(!is_listening() || !_acceptor)
        {
            err = boost::system::errc::make_error_code(
                boost::system::errc::not_connected);
            return nullptr;
        }

        auto sock = std::make_unique<sock_t>(_io);
        _acceptor->accept(*sock, err);
        if(err.failed())
            return nullptr;

        hj::tcp_socket::state stat = hj::tcp_socket::state::closed;
        if(sock && sock->is_open())
            stat = hj::tcp_socket::state::connected;

        return std::make_shared<tcp_socket>(_io, std::move(sock), stat);
    }

    std::shared_ptr<tcp_socket> accept(const endpoint_t &ep, err_t &err)
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);

        if(_binded_endpoint != ep || !is_listening())
        {
            err = listen(ep);
            if(err.failed())
                return nullptr;
        }
        return accept(err);
    }

    std::shared_ptr<tcp_socket> accept(uint16_t port, err_t &err)
    {
        endpoint_t ep{protocol_t::v4(), port};
        return accept(ep, err);
    }

    std::shared_ptr<tcp_socket>
    accept(const char *ip, uint16_t port, err_t &err)
    {
        err_t ec;
        auto  addr = _parse_address(ip, ec);
        if(ec.failed())
        {
            err = ec;
            return nullptr;
        }

        return accept(endpoint_t{addr, port}, err);
    }

    void async_accept(accept_handler_t fn)
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);

        if(!is_listening() || !_acceptor)
        {
            if(fn)
            {
                err_t err = boost::asio::error::make_error_code(
                    boost::asio::error::bad_descriptor);
                fn(err, nullptr);
            }
            return;
        }

        auto  sock_base = std::make_unique<sock_t>(_io);
        auto *raw_sock  = sock_base.get();
        auto &io_ref    = _io;

        _acceptor->async_accept(
            *raw_sock,
            [&io_ref, fn = std::move(fn), base = std::move(sock_base)](
                const err_t &err) mutable {
                if(err.failed())
                {
                    if(fn)
                        fn(err, nullptr);
                    return;
                }

                auto sock = std::make_shared<tcp_socket>(
                    io_ref,
                    std::move(base),
                    hj::tcp_socket::state::connected);

                if(fn)
                    fn(err, std::move(sock));
            });
    }

    void async_accept(uint16_t port) { async_accept(port, _accept_handler); }

    void async_accept(uint16_t port, accept_handler_t fn)
    {
        endpoint_t ep{protocol_t::v4(), port};
        async_accept(ep, std::move(fn));
    }

    void async_accept(const char *ip, uint16_t port)
    {
        async_accept(ip, port, _accept_handler);
    }

    void async_accept(const char *ip, uint16_t port, accept_handler_t fn)
    {
        err_t ec;
        auto  addr = _parse_address(ip, ec);
        if(ec)
        {
            if(fn)
                fn(ec, nullptr);
            return;
        }
        async_accept(endpoint_t{addr, port}, std::move(fn));
    }

    void async_accept(endpoint_t ep) { async_accept(ep, _accept_handler); }

    void async_accept(endpoint_t ep, accept_handler_t fn)
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);

        if(_binded_endpoint != ep || !is_listening())
        {
            err_t err = listen(ep);
            if(err.failed())
            {
                if(fn)
                    fn(err, nullptr);
                return;
            }
        }

        async_accept(std::move(fn));
    }

    void cancel()
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        if(_acceptor && _acceptor->is_open())
        {
            err_t ec;
            _acceptor->cancel(ec);
        }
    }

    void close()
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        _close_unlocked();
    }

  private:
    void _close_unlocked()
    {
        auto expected = _state.load(std::memory_order_relaxed);
        if(expected == state::closed)
            return;

        _state.store(state::closed, std::memory_order_release);

        if(_acceptor)
        {
            err_t ec;
            _acceptor->cancel(ec);
            _acceptor->close(ec);
            _acceptor.reset();
        }
        _binded_endpoint = endpoint_t();
    }

    static address_t _parse_address(const char *ip, err_t &ec) noexcept
    {
#if BOOST_VERSION < 108700
        return address_t::from_string(ip, ec);
#else
        return boost::asio::ip::make_address(ip, ec);
#endif
    }

  private:
    io_t                        &_io;
    mutable std::recursive_mutex _mtx;
    std::unique_ptr<acceptor_t>  _acceptor;
    endpoint_t                   _binded_endpoint;
    std::atomic<state>           _state{state::init};
    accept_handler_t             _accept_handler;
};

} // namespace hj

#endif // TCP_LISTENER_HPP