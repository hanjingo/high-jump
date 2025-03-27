#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <libcpp/net/proto/message.hpp>
#include <libcpp/net/tcp/tcp_conn.hpp>

namespace libcpp
{

class tcp_client
{
public:
    using io_t                 = libcpp::tcp_conn::io_t;
    using io_work_t            = libcpp::tcp_conn::io_work_t;
    using conn_ptr_t           = libcpp::tcp_conn*;
    using msg_ptr_t            = libcpp::message*;

    bool req(const msg_ptr_t msg)
    {

    }

    bool async_req(const msg_ptr_t msg)
    {
        
    }
};

}

#endif