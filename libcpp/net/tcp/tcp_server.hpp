#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <atomic>
#include <unordered_map>
#include <libcpp/net/proto/message.hpp>
#include <libcpp/net/tcp/tcp_conn.hpp>

namespace libcpp
{

class tcp_server
{
public:
    using conn_ptr_t           = libcpp::tcp_conn*;
    using msg_t                = libcpp::message;
    using msg_ptr_t            = libcpp::message*;
    using serial_num_t         = std::uint64_t;
    using req_handler_t        = std::function<msg_ptr_t(tcp_server*, conn_ptr_t, msg_ptr_t)>;

    enum flag : std::int64_t 
    {
        conn_binded_by_srv = 0x1,
        conn_detached_by_srv = ~(0x1),
    };

public:
    tcp_server() = delete;
    tcp_server(const req_handler_t&& fn)
        : fn_{std::move(fn)}
    {
    }

    bool bind(conn_ptr_t conn)
    {

    }

    void detach()
    {

    }

    serial_num_t inc() 
    { 

    }

private:
    std::atomic<serial_num_t> num_{0};
    req_handler_t fn_;
    libcpp::tcp_conn* conn_;
};

}

#endif