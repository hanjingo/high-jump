#ifndef TCP_DIALER_HPP
#define TCP_DIALER_HPP

#include <libcpp/net/tcp/tcp_conn.hpp>

namespace libcpp
{

class tcp_dialer
{
public:
    using io_t = libcpp::tcp_conn::io_t;
    using conn_t = libcpp::tcp_conn;
    using conn_ptr_t = libcpp::tcp_conn*;
    using sock_t = libcpp::tcp_conn::sock_t;
    using sock_ptr_t = libcpp::tcp_conn::sock_ptr_t;
    
    conn_ptr_t dial(const char* ip, 
                    const std::uint16_t port, 
                    std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
                    int retry_times = 1)
    {
        conn_ptr_t conn = nullptr;
        sock_ptr_t sock = new sock_t(_io);
        sock->async_connect(ep, [&](const err_t& err) {
            if (err.failed())
                return;

            conn = new conn_t(_io, sock);
        });

        _io.restart();
        _io.run_for(timeout);
        if (!_io.stopped() || !is_connected()) {
            retry_times--;
            return dial(ctx, ip, port, timeout, retry_times);
        }

        return conn;
    }

private:
    io_t _io;
};

}

#endif