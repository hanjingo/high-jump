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
    using conn_flag_t          = libcpp::tcp_conn::flag_t;

    using req_handler_t        = std::function<msg_ptr_t(tcp_server*, conn_ptr_t, msg_ptr_t)>;
    using tag_t                = std::uint64_t;

    static const conn_flag_t flag_conn_binded_by_srv = 0x1;
    static const conn_flag_t flag_conn_detached_by_srv = ~(0x1);

public:
    tcp_server() = delete;
    tcp_server(const req_handler_t&& fn)
        : fn_{std::move(fn)}
    {
    }

    bool bind(conn_ptr_t conn)
    {
        if (conn->get_flag() & flag_conn_binded_by_srv)
            return false; // conn already binded

        conn->set_flag(conn->get_flag() | flag_conn_binded_by_srv);
        conn->set_recv_handler(
            [this](libcpp::tcp_conn::conn_ptr_t conn, libcpp::tcp_conn::msg_ptr_t req)->libcpp::tcp_conn::msg_ptr_t{
            this->tag_inc();
            return this->fn_(this, conn, req);
        });
        return true;
    }

    void detach()
    {
        conn_->set_flag(conn_->get_flag() & flag_conn_detached_by_srv);
        conn_->set_recv_handler();
        conn_ = nullptr;
    }

    tag_t tag_inc() 
    { 
        tag_t tag = tag_.load(); 
        tag_.compare_exchange_strong(tag, tag + 1); 
        return tag_.load();
    }

    tag_t tag_get() { return tag_.load(); }

private:
    std::atomic<tag_t> tag_{0};
    req_handler_t fn_;
    libcpp::tcp_conn* conn_;
};

}

#endif