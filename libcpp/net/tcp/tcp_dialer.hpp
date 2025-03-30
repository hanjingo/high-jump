#ifndef TCP_DIALER_HPP
#define TCP_DIALER_HPP

#include <libcpp/net/tcp/tcp_conn.hpp>

namespace libcpp
{

class tcp_dialer
{
public:
    using io_t       = libcpp::tcp_conn::io_t;
    using conn_t     = libcpp::tcp_conn;
    using conn_ptr_t = libcpp::tcp_conn*;
    using sock_t     = libcpp::tcp_conn::sock_t;
    using sock_ptr_t = libcpp::tcp_conn::sock_ptr_t;
    using err_t      = libcpp::tcp_conn::err_t;

    using conn_handler_t       = std::function<void(conn_ptr_t, err_t)>;

    tcp_dialer() {}
    virtual ~tcp_dialer() {}

    template<typename T>
    inline bool set_option(T opt)
    {
        // TODO
        return true;
    }

    conn_ptr_t dial(const char* ip, 
                    const std::uint16_t port, 
                    std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
                    int retry_times = 1)
    {
        return dial(io_, ip, port, timeout, retry_times);
    }
    
    conn_ptr_t dial(io_t& io,
                    const char* ip, 
                    const std::uint16_t port, 
                    std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
                    int retry_times = 1)
    {
        // TODO
    }

    void async_dail(const char* ip, 
                    uint16_t port, 
                    conn_handler_t&& fn)
    {
        async_dail(io_, ip, port, std::move(fn)); 
    }

    void async_dail(io_t& io,
                    const char* ip, 
                    uint16_t port, 
                    conn_handler_t&& fn)
    {
        // TODO
    }

    void close()
    {
        // TODO
    }

private:
    io_t io_;
};

}

#endif