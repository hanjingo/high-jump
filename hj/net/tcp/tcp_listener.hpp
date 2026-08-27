#ifndef TCP_LISTENER_HPP
#define TCP_LISTENER_HPP

#include <memory>
#include <functional>
#include <atomic>
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

    using io_t       = boost::asio::io_context;
    using err_t      = boost::system::error_code;
    using raw_sock_t = boost::asio::ip::tcp::socket;
    using endpoint_t = boost::asio::ip::tcp::endpoint;
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
    static std::shared_ptr<tcp_listener> make_shared(Args &&...args)
    {
        return std::make_shared<tcp_listener>(std::forward<Args>(args)...);
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

    [[nodiscard]] state status() const noexcept
    {
        return _state.load(std::memory_order_relaxed);
    }

    err_t open(const protocol_t &protocol = protocol_t::v4())
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _open(protocol);
    }

    template <typename Option>
    err_t set_option(const Option &opt)
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _set_option(opt);
    }

    err_t listen(const endpoint_t &ep,
                 int backlog = boost::asio::socket_base::max_listen_connections)
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _listen(ep, backlog);
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
        std::shared_ptr<acceptor_t> safe_acceptor;

        {
            std::lock_guard<std::mutex> lock(_mu);
            if(!is_listening() || !_acceptor || !_acceptor->is_open())
            {
                err = boost::system::errc::make_error_code(
                    boost::system::errc::not_connected);
                return nullptr;
            }
            safe_acceptor = _acceptor;
        }

        auto sock = std::make_unique<raw_sock_t>(_io);
        safe_acceptor->accept(*sock, err);

        if(err.failed())
            return nullptr;

        if(is_closed())
        {
            err = boost::asio::error::operation_aborted;
            return nullptr;
        }

        hj::tcp_socket::state stat = (sock && sock->is_open())
                                         ? hj::tcp_socket::state::connected
                                         : hj::tcp_socket::state::closed;

        return tcp_socket::make_shared(_io, std::move(sock), stat);
    }

    std::shared_ptr<tcp_socket> accept(const endpoint_t &ep, err_t &err)
    {
        std::shared_ptr<acceptor_t> safe_acceptor;

        {
            std::lock_guard<std::mutex> lock(_mu);

            if(is_listening() && _binded_endpoint != ep)
                _close();

            if(!is_listening())
            {
                err = _listen(ep);
                if(err.failed())
                    return nullptr;
            }

            safe_acceptor = _acceptor;
        }

        auto sock = std::make_unique<raw_sock_t>(_io);
        safe_acceptor->accept(*sock, err);

        if(err.failed())
            return nullptr;

        if(is_closed())
        {
            err = boost::asio::error::operation_aborted;
            return nullptr;
        }

        hj::tcp_socket::state stat = (sock && sock->is_open())
                                         ? hj::tcp_socket::state::connected
                                         : hj::tcp_socket::state::closed;

        return tcp_socket::make_shared(_io, std::move(sock), stat);
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
        std::lock_guard<std::mutex> lock(_mu);

        _async_accept(std::move(fn));
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
            {
                boost::asio::post(_io, [fn = std::move(fn), ec]() {
                    fn(ec, nullptr);
                });
            }
            return;
        }
        async_accept(endpoint_t{addr, port}, std::move(fn));
    }

    void async_accept(endpoint_t ep) { async_accept(ep, _accept_handler); }

    void async_accept(endpoint_t ep, accept_handler_t fn)
    {
        std::lock_guard<std::mutex> lock(_mu);

        if(is_listening() && _binded_endpoint != ep)
        {
            _close();
        }

        if(!is_listening())
        {
            err_t err = _listen(ep);
            if(err.failed())
            {
                if(fn)
                {
                    boost::asio::post(_io, [fn = std::move(fn), err]() {
                        fn(err, nullptr);
                    });
                }
                return;
            }
        }

        _async_accept(std::move(fn));
    }

    err_t cancel()
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _cancel();
    }

    void close()
    {
        std::lock_guard<std::mutex> lock(_mu);
        _close();
    }

  private:
    void _close()
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

    err_t _open(const protocol_t &protocol)
    {
        if(_acceptor && _acceptor->is_open())
            return boost::asio::error::already_open;

        err_t ec;
        _acceptor = std::make_shared<acceptor_t>(_io);
        _acceptor->open(protocol, ec);
        if(!ec)
            _state.store(state::opened, std::memory_order_release);
        else
            _acceptor.reset();

        return ec;
    }

    template <typename Option>
    err_t _set_option(const Option &opt)
    {
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

    err_t
    _listen(const endpoint_t &ep,
            int backlog = boost::asio::socket_base::max_listen_connections)
    {
        if(is_listening())
            return boost::asio::error::already_started;

        err_t ec;
        if(!_acceptor || !_acceptor->is_open())
        {
            ec = _open(ep.protocol());
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

    void _async_accept(accept_handler_t fn)
    {
        if(!is_listening() || !_acceptor)
        {
            if(fn)
            {
                err_t err = boost::asio::error::make_error_code(
                    boost::asio::error::bad_descriptor);

                boost::asio::post(_io, [fn = std::move(fn), err]() {
                    fn(err, nullptr);
                });
            }
            return;
        }

        auto  sock_base = std::make_unique<raw_sock_t>(_io);
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

                auto sock =
                    tcp_socket::make_shared(io_ref,
                                            std::move(base),
                                            hj::tcp_socket::state::connected);

                if(fn)
                    fn(err, std::move(sock));
            });
    }

    err_t _cancel()
    {
        if(!_acceptor || !_acceptor->is_open() || is_closed())
        {
            return boost::system::errc::make_error_code(
                boost::system::errc::bad_file_descriptor);
        }

        err_t ec;
        _acceptor->cancel(ec);
        return ec;
    }

  private:
    io_t                       &_io;
    mutable std::mutex          _mu;
    std::shared_ptr<acceptor_t> _acceptor;
    endpoint_t                  _binded_endpoint;
    std::atomic<state>          _state{state::init};
    accept_handler_t            _accept_handler;
};

} // namespace hj

#endif // TCP_LISTENER_HPP