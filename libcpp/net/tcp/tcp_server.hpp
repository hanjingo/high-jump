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
    using req_handler_t        = std::function<msg_ptr_t(msg_ptr_t)>;
    using next_handler_t       = std::function<msg_ptr_t(msg_ptr_t)>;
    using conn_flag_t          = libcpp::tcp_conn::flag_t;

    static const conn_flag_t flag_conn_binded_by_srv = 0x1;
    static const conn_flag_t flag_conn_unbinded_by_srv = ~(0x1);

public:
    tcp_server() = delete;
    tcp_server(const req_handler_t&& fn, const next_handler_t&& fnext)
        : _fhandler{std::move(fn)}
        , _fnext{std::move(fnext)}
    {
    }

    bool bind(conn_ptr_t conn)
    {
        if (conn->get_flag() & flag_conn_binded_by_srv)
            return false; // conn already binded

        conn->set_flag(conn->get_flag() | flag_conn_binded_by_srv);
        _next_req(conn, nullptr);
        return true;
    }

    void unbind(conn_ptr_t conn)
    {
        conn->set_flag(conn->get_flag() & flag_conn_unbinded_by_srv);
    }

private:
    void _on_req(conn_ptr_t conn, msg_ptr_t req)
    {
        std::cout << "fuck4" << std::endl;
        msg_ptr_t resp = _fhandler(req);
        conn->async_send(resp, [this, req](conn_ptr_t sender, msg_ptr_t resp) {
            std::cout << "fuck5" << std::endl;
            this->_next_req(sender, req);
            std::cout << "fuck6" << std::endl;
        });
    }

    void _next_req(conn_ptr_t conn, msg_ptr_t pre_req)
    {
        std::cout << "fuck7" << std::endl;
        if (!(conn->get_flag() & flag_conn_binded_by_srv))
                return;

        std::cout << "fuck8" << std::endl;
        auto req = _fnext(pre_req);
        conn->async_recv(req, std::bind(&tcp_server::_on_req, this, std::placeholders::_1, std::placeholders::_2));
        std::cout << "fuck9" << std::endl;
    }

private:
    req_handler_t _fhandler;
    next_handler_t _fnext;
};

}

#endif