#ifndef TCP_SOCKET_HPP
#define TCP_SOCKET_HPP

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#endif

#include <chrono>
#include <iostream>
#include <functional>
#include <memory>
#include <csignal>
#include <initializer_list>

#include <boost/stacktrace.hpp>
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>

namespace libcpp
{

class tcp_socket
{
public:
#ifdef SMART_PTR_ENABLE
    using tcp_socket_ptr_t  = std::shared_ptr<tcp_socket>;
#else
    using tcp_socket_ptr_t  = tcp_socket*;
#endif

    using io_t              = boost::asio::io_context;
    using io_work_t         = boost::asio::io_service::work;
    using err_t             = boost::system::error_code;
    
    using const_buffer_t    = boost::asio::const_buffer;
    using multi_buffer_t    = boost::asio::mutable_buffer;
    using streambuf_t       = boost::asio::streambuf;

    using sock_t            = boost::asio::ip::tcp::socket;
    using address_t         = boost::asio::ip::address;
    using endpoint_t        = boost::asio::ip::tcp::endpoint;

    using steady_timer_t    = boost::asio::steady_timer;
    using deadline_timer_t  = boost::asio::deadline_timer;
    using ms_t              = std::chrono::milliseconds;

    using opt_no_delay      = boost::asio::ip::tcp::no_delay;
    using opt_send_buf_sz   = boost::asio::ip::tcp::socket::send_buffer_size;
    using opt_recv_buf_sz   = boost::asio::ip::tcp::socket::receive_buffer_size;
    using opt_reuse_addr    = boost::asio::ip::tcp::socket::reuse_address;
    using opt_keep_alive    = boost::asio::ip::tcp::socket::keep_alive;
    using opt_broadcast     = boost::asio::ip::tcp::socket::broadcast;

    using conn_handler_t    = std::function<void(const err_t&)>;
    using send_handler_t    = std::function<void(const err_t&, std::size_t)>;
    using recv_handler_t    = std::function<void(const err_t&, std::size_t)>;

public:
    tcp_socket() = delete;
    explicit tcp_socket(io_t& io)
        : io_{io}
    {
    }
    explicit tcp_socket(io_t& io, sock_t* sock)
        : io_{io}
        , sock_{sock}
    {
    }
    virtual ~tcp_socket()
    {
        close();
    }

    inline io_t& io() { return io_; }

    template<typename T>
    inline bool set_option(T opt)
    {
        if (sock_ == nullptr || !sock_->is_open())
            return false;

        err_t err;
        sock_->set_option(opt, err);
        if (!err.failed())
            return true;

        assert(false);
        std::cerr << err << std::endl;
        return false;
    }

    inline bool is_connected() { return sock_ != nullptr && is_connected_.load(); }
    
    bool check_connected()
    {
        err_t err;
        return check_connected(err);
    }

    bool check_connected(err_t& err)
    {
        if (sock_ == nullptr)
            return false;

        sock_->remote_endpoint(err);
        return !err.failed();
    }

    bool connect(const char* ip, 
                 uint16_t port, 
                 std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
                 int try_times = 1)
    {
        endpoint_t ep{address_t::from_string(ip), port};
        return connect(ep, timeout, try_times);
    }

    bool connect(endpoint_t ep, 
                 std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
                 int try_times = 1)
    {
        for (int i = 0; i < try_times; ++i)
        {
            set_conn_status(false);

            if (sock_ != nullptr)
            {
                sock_->close();
                delete sock_;
                sock_ = nullptr;
            }
            sock_ = new sock_t(io_);

            std::atomic_bool finished{false};
            std::atomic_bool success{false};

            steady_timer_t timer(io_);
            timer.expires_after(timeout);

            sock_->async_connect(ep, [&](const err_t& err) {
                if (finished.exchange(true)) 
                    return;

                if (!err.failed())
                {
                    set_conn_status(true);
                    success.exchange(true);
                }
                else
                    set_conn_status(false);
                timer.cancel();
            });

            timer.async_wait([&](const err_t& err) {
                (void)err; // ignore timer error
                if (finished.exchange(true)) 
                    return;

                if (sock_)
                    sock_->close();

                set_conn_status(false);
            });

            io_.restart();
            io_.run();

            if (success.load())
                return true;
        }
        return false;
    }

    void async_connect(const char* ip, const uint16_t port, conn_handler_t&& fn)
    {
        endpoint_t ep{address_t::from_string(ip), port};
        async_connect(ep, std::move(fn));
    }

    void async_connect(endpoint_t ep, conn_handler_t&& fn)
    {
        if (is_connected()) 
        {
            fn(boost::system::errc::make_error_code(boost::system::errc::already_connected));
            return;
        }

        if (this->sock_ != nullptr)
        {
            this->sock_->close();
            delete this->sock_;
            this->sock_ = nullptr;
        }
        sock_ = new sock_t(io_);
        sock_->async_connect(ep, [this, fn](const err_t & err) {
            if (!err.failed())
                set_conn_status(true);
            else
                set_conn_status(false);

            fn(err);
        });
    }

    void disconnect()
    {
        if (!is_connected())
            return;

        _disconnect_anyway();
    }

    size_t send(const const_buffer_t& buf)
    {
        if (!is_connected())
            return 0;

        try{
            return sock_->send(buf);
        } catch (...) {
            return 0;
        }
    }

    size_t send(const unsigned char* data, size_t len)
    {
        return send(boost::asio::buffer(data, len));
    }

    void async_send(const const_buffer_t& buf, send_handler_t&& fn)
    {
        if (!is_connected()) 
        {
            fn(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
            return;
        }

        try {
            sock_->async_send(buf, std::move(fn));
        } catch (const boost::system::error_code& ec) {
            assert(false);
            std::cerr << ec << std::endl;
            return;
        }
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

        err_t err;
        auto sz = sock_->read_some(buf, err);
        if (err.failed())
            assert(false);

        return sz;
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

        err_t err;
        auto sz = boost::asio::read(*sock_, buf, boost::asio::transfer_at_least(least), err);
        if (err.failed())
            assert(false);

        return sz;
    }

    void async_recv(multi_buffer_t& buf, recv_handler_t&& fn)
    {
        auto cb = std::move(fn);
        if (!is_connected()) 
        {
            cb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
            return;
        }

        sock_->async_read_some(buf, cb);
    }

    void async_recv(unsigned char* data, size_t len, recv_handler_t&& fn)
    {
        multi_buffer_t buf{data, len};
        async_recv(buf, std::move(fn));
    }

    bool set_conn_status(bool is_connected)
    {
        bool old = is_connected_.load();
        return is_connected_.compare_exchange_strong(old, is_connected);
    }

    void close()
    {
        _disconnect_anyway();
    }

private:
    void _disconnect_anyway()
    {
        set_conn_status(false);
        if (sock_ != nullptr)
        {
            sock_->close();
            delete sock_;
            sock_ = nullptr;
        }
    }

private:
    io_t&                 io_;
    sock_t*               sock_ = nullptr;
    std::atomic_bool      is_connected_{false};
};

}

#endif