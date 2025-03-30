#ifndef TCP_CONN
#define TCP_CONN

#include <atomic>
#include <chrono>
#include <functional>

#include <libcpp/net/tcp/tcp_socket.hpp>
#include <libcpp/net/tcp/tcp_chan.hpp>
#include <libcpp/net/proto/message.hpp>

#ifndef MTU
#define MTU 1500
#endif

namespace libcpp
{

class tcp_conn
{
public:
    using flag_t           = std::int64_t;
    using io_t             = libcpp::tcp_socket::io_t;
    using io_work_t        = libcpp::tcp_socket::io_work_t;
    using err_t            = libcpp::tcp_socket::err_t;
    using msg_ptr_t        = libcpp::message*;
    using conn_ptr_t       = libcpp::tcp_conn*;
    using sock_ptr_t       = libcpp::tcp_socket*;

    using conn_handler_t       = std::function<void(conn_ptr_t, err_t)>;
    using send_handler_t       = std::function<void(conn_ptr_t, msg_ptr_t)>;
    using recv_handler_t       = std::function<msg_ptr_t(conn_ptr_t, msg_ptr_t)>;
    using disconnect_handler_t = std::function<void(conn_ptr_t)>;

public:
    tcp_conn() = delete;

    tcp_conn(io_t& io, std::size_t rbuf_sz = 65535, std::size_t wbuf_sz = 65535) 
        : io_{io}
        , sock_{new tcp_socket(io_)}
        , r_buf_{rbuf_sz}
        , r_ch_{rbuf_sz / MTU}
        , w_ch_{wbuf_sz / MTU}
    {
    }
    tcp_conn(io_t& io, sock_ptr_t sock, std::size_t rbuf_sz = 65535, std::size_t wbuf_sz = 65535) 
        : io_{io}
        , sock_{sock}
        , r_buf_{rbuf_sz}
        , r_ch_{rbuf_sz / MTU}
        , w_ch_{wbuf_sz / MTU}
    {
    }
    ~tcp_conn() 
    {
        close();
    }

    inline void set_disconnect_handler(disconnect_handler_t&& fn) { disconnect_handler_ = std::move(fn); }
    inline void set_recv_handler(recv_handler_t&& fn) { recv_handler_ = std::move(fn); }
    inline void set_send_handler(send_handler_t&& fn) { send_handler_ = std::move(fn); }
    inline flag_t get_flag() { return flag_.load(); }
    inline void set_flag(const flag_t flag) { flag_.store(flag); }

    bool connect(const char* ip, 
                 const std::uint16_t port, 
                 int retry_times = 1)
    {
        if (is_connected())
            return false;

        if (!sock_->connect(ip, port, retry_times))
            return false;

        w_closed_.store(false);
        r_closed_.store(false);

        _async_send(err_t(), 0);
        _async_recv(err_t(), 0);
        return true;
    }

    bool async_connect(const char* ip, const std::uint16_t port, conn_handler_t&& fn)
    {
        if (is_connected())
            return false;

        sock_->async_connect(ip, port, [this, fn](const err_t& err, sock_ptr_t sock) {
            if (!err.failed())
            {
                this->w_closed_.store(false);
                this->r_closed_.store(false);
                this->_async_send(err_t(), 0);
                this->_async_recv(err_t(), 0);
            }
            fn(this, err);
        });
        return true;
    }

    inline bool is_connected()
    {
        return !w_closed_.load() && !r_closed_.load() && sock_ != nullptr && sock_->is_connected();
    }

    bool disconnect()
    {
        if (!is_connected())
            return false;

        while (!w_closed_.load())
            w_ch_ << nullptr;

        while (!r_closed_.load())
            r_ch_ << nullptr;

        sock_->disconnect();
        disconnect_handler_(this);
        return true;
    }

    bool send(msg_ptr_t msg)
    {
        if (sock_ == nullptr || !sock_->is_connected() || w_closed_.load())
            return false;

        w_ch_ << msg;
        return true;
    }

    bool async_send(msg_ptr_t msg, send_handler_t&& fn)
    {
        if (sock_ == nullptr || !sock_->is_connected() || w_closed_.load())
            return false;

        w_ch_ << msg;
        return true;
    }

    bool recv(msg_ptr_t msg)
    {
        if (sock_ == nullptr || !sock_->is_connected() || r_closed_.load())
            return false;

        r_ch_ << msg;
        return true;
    }

    bool async_recv(msg_ptr_t msg)
    {
        if (sock_ == nullptr || !sock_->is_connected() || r_closed_.load())
            return false;

        r_ch_ << msg;
        return true;
    }

    bool async_recv(msg_ptr_t msg, recv_handler_t&& fn)
    {
        if (sock_ == nullptr || !sock_->is_connected() || r_closed_.load())
            return false;

        r_ch_ << msg;
        recv_handler_ = std::move(fn);
        return true;
    }

    void close()
    {
        w_closed_.store(true);
        r_closed_.store(true);
        if (sock_ == nullptr)
            return;

        sock_->close();
        delete sock_;
        sock_ = nullptr;
    }

private:
    void _async_send(const err_t& err, std::size_t sz)
    {
        if (err.failed())
            w_closed_.store(true);
        if (sock_ == nullptr || !sock_->is_connected() || w_closed_.load()) 
            return;

        msg_ptr_t msg = nullptr;
        w_ch_ >> msg;
        if (msg == nullptr)
        {
            w_closed_.store(true);
            return;
        }

        sz = msg->size();
        unsigned char buf[sz];
        sz = msg->encode(buf, sz);
        if (sz == 0)
        {
            w_closed_.store(true);
            return;
        }

        send_handler_(this, msg);
        sock_->async_send(buf, sz, std::bind(&tcp_conn::_async_send, this, std::placeholders::_1, std::placeholders::_2));
    }

    void _async_recv(const err_t& err, std::size_t sz)
    {
        if (err.failed())
            r_closed_.store(true);
        if (sock_ == nullptr || !sock_->is_connected() || r_closed_.load()) 
            return;

        this->r_buf_.commit(sz);
        msg_ptr_t msg = nullptr;
        r_ch_ >> msg;
        if (msg == nullptr)
        {
            w_closed_.store(true);
            return;
        }
        
        auto data = boost::asio::buffer_cast<const unsigned char*>(r_buf_.data());
        sz = msg->decode(data, r_buf_.size());
        if (sz > 0)
        {
            this->r_buf_.consume(sz);
            msg = recv_handler_(this, msg);
            if (msg != nullptr)
                r_ch_ << msg;
        }

        auto buf = r_buf_.prepare(MTU);
        sock_->async_recv(buf, std::bind(&tcp_conn::_async_send, this, std::placeholders::_1, std::placeholders::_2));
    }

private:
    io_t&               io_;
    sock_ptr_t          sock_ = nullptr;
    std::atomic<flag_t> flag_{0};

    std::atomic<bool> w_closed_{false};
    std::atomic<bool> r_closed_{false};

    // NOTE: maybe use boost::asio::strand is a better choice
    tcp_socket::streambuf_t  r_buf_;
    tcp_chan<msg_ptr_t>      r_ch_;
    tcp_chan<msg_ptr_t>      w_ch_;

    disconnect_handler_t disconnect_handler_;
    recv_handler_t       recv_handler_;
    send_handler_t       send_handler_;
};

}

#endif