#ifndef TCP_DIALER_HPP
#define TCP_DIALER_HPP

#include <libcpp/net/tcp/tcp_conn.hpp>

namespace libcpp
{

class tcp_dialer
{
public:
    using io_t       = libcpp::tcp_conn::io_t;

#ifdef SMART_PTR_ENABLE
    using conn_ptr_t = std::shared_ptr<libcpp::tcp_conn>;
#else
    using conn_ptr_t = libcpp::tcp_conn*;
#endif

    using sock_ptr_t = libcpp::tcp_conn::sock_ptr_t;
    using err_t      = libcpp::tcp_conn::err_t;

    using dial_handler_t = libcpp::tcp_conn::conn_handler_t;

    tcp_dialer() = delete;
    virtual ~tcp_dialer() {}

    static conn_ptr_t dial(io_t& io,
                           const char* ip, 
                           const std::uint16_t port, 
                           std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
                           int try_times = 1)
    {
        tcp_conn* conn = new tcp_conn(io);
        if (conn->connect(ip, port, timeout, try_times))
            return conn;

        conn->close();
        delete conn;
        return nullptr;
    }

    static void async_dail(io_t& io,
                           const char* ip, 
                           uint16_t port, 
                           dial_handler_t&& fn)
    {
        tcp_conn* conn = new tcp_conn(io);
        conn->set_connect_handler(std::move(fn));
        conn->async_connect(ip, port);
    }
};

}

#endif