#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <atomic>
#include <unordered_map>
#include <libcpp/net/proto/message.hpp>
#include <libcpp/net/tcp/tcp_conn.hpp>

namespace libcpp
{

class tcp_client
{
public:
    using io_t                 = libcpp::tcp_conn::io_t;
    using conn_ptr_t           = libcpp::tcp_conn*;
    using msg_t                = libcpp::message;
    using msg_ptr_t            = libcpp::message*;
    using resp_handler_t       = std::function<void(msg_ptr_t, msg_ptr_t)>;
    using conn_flag_t          = libcpp::tcp_conn::flag_t;

    static const conn_flag_t flag_conn_binded_by_cli = 0x2;
    static const conn_flag_t flag_conn_detached_by_cli = ~(0x2);

public:
    tcp_client() = delete;
    tcp_client(const resp_handler_t&& fn)
        : fhandler_{std::move(fn)}
    {
    }

    bool bind(conn_ptr_t conn)
    {
        if (conn->get_flag() & flag_conn_binded_by_cli)
            return false; // conn already binded

        conn->set_flag(conn->get_flag() | flag_conn_detached_by_cli);
        return true;
    }

    void detach(conn_ptr_t conn)
    {
        conn->set_flag(conn->get_flag() & flag_conn_detached_by_cli);
    }

    bool req(conn_ptr_t conn, msg_ptr_t mreq, msg_ptr_t resp)
    {
        return conn->async_send(mreq, 
            [this, resp](conn_ptr_t conn, msg_ptr_t mreq){
                conn->async_recv(resp, 
                    [this, mreq](conn_ptr_t conn, msg_ptr_t resp){
                        this->fhandler_(mreq, resp);
                });
            });
    }

private:
    resp_handler_t fhandler_;
};

}

#endif