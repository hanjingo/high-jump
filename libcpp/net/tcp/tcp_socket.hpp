#ifndef TCP_SOCKET_HPP
#define TCP_SOCKET_HPP

#include <chrono>
#include <iostream>
#include <functional>
#include <memory>
#include <csignal>
#include <initializer_list>

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>

namespace libcpp
{

class tcp_socket
{
public:
    using io_t              = boost::asio::io_context;
    using io_work_t         = boost::asio::io_service::work;
    using err_t             = boost::system::error_code;
    using address_t         = boost::asio::ip::address;

    using const_buffer_t    = boost::asio::const_buffer;
    using multi_buffer_t    = boost::asio::mutable_buffer;
    using streambuf_t       = boost::asio::streambuf;

    using sock_t            = boost::asio::ip::tcp::socket;
    using endpoint_t        = boost::asio::ip::tcp::endpoint;

    using opt_no_delay      = boost::asio::ip::tcp::no_delay;
    using opt_send_buf_sz   = boost::asio::ip::tcp::socket::send_buffer_size;
    using opt_recv_buf_sz   = boost::asio::ip::tcp::socket::receive_buffer_size;
    using opt_reuse_addr    = boost::asio::ip::tcp::socket::reuse_address;
    using opt_keep_alive    = boost::asio::ip::tcp::socket::keep_alive;

    using conn_handler_t    = std::function<void(const err_t&, tcp_socket*)>;
    using send_handler_t    = std::function<void(const err_t&, std::size_t)>;
    using recv_handler_t    = std::function<void(const err_t&, std::size_t)>;

public:
    tcp_socket() = delete;
    explicit tcp_socket(io_t& io)
        : _io{io}
    {
    }
    explicit tcp_socket(io_t& io, sock_t* sock)
        : _io{io}
        , _sock{sock}
    {
    }
    virtual ~tcp_socket()
    {
        close();
    }

    template<typename T>
    inline void set_option(T opt)
    {
        if (_sock != nullptr && _sock->is_open())
            _sock->set_option(opt);
    }

    bool connect(const char* ip, 
                 uint16_t port, 
                 int retry_times = 1,
                 std::chrono::milliseconds timeout = std::chrono::milliseconds(2000))
    {
        endpoint_t ep{address_t::from_string(ip), port};
        return connect(ep, retry_times, timeout);
    }

    // WARNNING: boost.asio connect fail with default timeout duration = 2s
    // WARNNING: the feature of timeout is not recommended for use in a multi-connection context.
    // See Also: https://www.boost.org/doc/libs/1_80_0/doc/html/boost_asio/example/cpp03/timeouts/blocking_tcp_client.cpp
    bool connect(endpoint_t ep, 
                 int retry_times = 1, 
                 std::chrono::milliseconds timeout = std::chrono::milliseconds(2000))
    {
        if (is_connected() || retry_times < 1)
            return false;

        _sock = new sock_t(_io);
        err_t err;
        try {
            _sock->connect(ep, err);
        } catch (const boost::system::system_error& exception) {
            delete _sock;
            _sock = nullptr;
            return false;
        }

        if (err.failed()) 
        {
            retry_times--;
            disconnect();
            return connect(ep, retry_times);
        }

        set_conn_status(true);
        return true;
    }

    void async_connect(const char* ip, uint16_t port, conn_handler_t&& fn)
    {
        endpoint_t ep{address_t::from_string(ip), port};
        async_connect(ep, std::move(fn));
    }

    void async_connect(endpoint_t ep, conn_handler_t&& fn)
    {
        if (is_connected()) 
        {
            fn(boost::system::errc::make_error_code(boost::system::errc::already_connected), this);
            return;
        }

        if (this->_sock != nullptr)
        {
            this->_sock->close();
            delete this->_sock;
            this->_sock = nullptr;
        }
        _sock = new sock_t(_io);
        _sock->async_connect(ep, [this, fn](const err_t & err) {
            if (err.failed())
                set_conn_status(false);

            fn(err, this);
        });
    }

    void disconnect()
    {
        if (!is_connected()) 
            return;

        set_conn_status(false);
        if (_sock != nullptr)
        {
            _sock->close();
            delete _sock;
            _sock = nullptr;
        }
    }

    size_t send(const const_buffer_t& buf)
    {
        if (!is_connected())
            return 0;

        return _sock->send(buf);
    }

    size_t send(const char* data, size_t len)
    {
        if (!is_connected()) 
            return 0;

        return _sock->send(boost::asio::buffer(data, len));
    }

    size_t send(const unsigned char* data, size_t len)
    {
        if (!is_connected()) 
            return 0;

        try {
            return _sock->send(boost::asio::buffer(data, len));
        } catch (const boost::system::system_error& err) {
            return 0;
        }
    }

    void async_send(const const_buffer_t& buf, send_handler_t&& fn)
    {
        if (!is_connected()) 
        {
            fn(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
            return;
        }

        try {
            _sock->async_send(buf, std::move(fn));
        } catch (const boost::system::system_error& err) {
            return;
        }
    }

    void async_send(const char* data, size_t len, send_handler_t&& fn)
    {
        if (!is_connected()) 
        {
            fn(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
            return;
        }

        return async_send(boost::asio::buffer(data, len), std::move(fn));
    }

    void async_send(const unsigned char* data, size_t len, send_handler_t&& fn)
    {
        if (!is_connected()) 
        {
            fn(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
            return;
        }

        return async_send(boost::asio::buffer(data, len), std::move(fn));
    }

    size_t recv(multi_buffer_t& buf)
    {
        if (!is_connected()) 
            return 0;

        try {
            return _sock->read_some(buf);
        } catch (const boost::system::system_error& err) {
            return 0;
        }
    }

    size_t recv(char* data, size_t len)
    {
        multi_buffer_t buf{data, len};
        return recv(buf);
    }

    size_t recv(unsigned char* data, size_t len)
    {
        multi_buffer_t buf{data, len};
        return recv(buf);
    }

    size_t recv_until(streambuf_t& buf, size_t least)
    {
        if (!is_connected())
            return 0;

        try {
            return boost::asio::read(*_sock, buf, boost::asio::transfer_at_least(least));
        } catch (const boost::system::system_error& err) {
            return 0;
        }
    }

    void async_recv(multi_buffer_t& buf, recv_handler_t&& fn)
    {
        if (!is_connected()) {
            fn(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
            return;
        }

        try {
            _sock->async_read_some(buf, std::move(fn));
        } catch (const boost::system::system_error& err) {
            return;
        }
    }

    void async_recv(char* data, size_t len, recv_handler_t&& fn)
    {
        multi_buffer_t buf{data, len};
        async_recv(buf, std::move(fn));
    }

    void async_recv(unsigned char* data, size_t len, recv_handler_t&& fn)
    {
        multi_buffer_t buf{data, len};
        async_recv(buf, std::move(fn));
    }

    bool set_conn_status(bool is_connected)
    {
        bool old = _is_connected.load();
        return _is_connected.compare_exchange_strong(old, is_connected);
    }

    bool is_connected()
    {
        return _is_connected.load();
    }

    void close()
    {
        disconnect();
    }

    // WARNNING: this function will be blocked while timeout = 0ms
    // WARNNING: the feature of timeout is not recommended for use in a multi-thread context.
    // See Also: https://www.boost.org/doc/libs/1_80_0/doc/html/boost_asio/example/cpp03/timeouts/blocking_tcp_client.cpp
    size_t recv(multi_buffer_t& buf, std::chrono::milliseconds timeout)
    {
        if (!is_connected())
            return 0;

        std::size_t nrecvd = 0;
        std::future<void> f = std::async(std::launch::async, [&]() {
            nrecvd = _sock->read_some(buf);
        });
        f.wait_for(timeout);
        return nrecvd;
    }

    size_t recv(char* data, size_t len, std::chrono::milliseconds timeout)
    {
        multi_buffer_t buf{data, len};
        return recv(buf, timeout);
    }

    size_t recv(unsigned char* data, size_t len, std::chrono::milliseconds timeout)
    {
        multi_buffer_t buf{data, len};
        return recv(buf, timeout);
    }

private:
    io_t&                 _io;
    sock_t*               _sock = nullptr;
    std::atomic_bool      _is_connected{false};
};

}

#endif