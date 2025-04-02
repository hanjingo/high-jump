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

    using conn_handler_t    = std::function<void(conn_ptr_t, err_t)>;
    using send_handler_t    = std::function<void(conn_ptr_t, msg_ptr_t)>;
    using recv_handler_t    = std::function<void(conn_ptr_t, msg_ptr_t)>;
    using disconn_handler_t = std::function<void(conn_ptr_t)>;

public:
    tcp_conn() = delete;

    tcp_conn(io_t& io, std::size_t rbuf_sz = 65535, std::size_t wbuf_sz = 65535) 
        : io_{io}
        , sock_{new tcp_socket(io_)}
        , r_buf_{rbuf_sz}
        , w_buf_{wbuf_sz}
        , r_ch_{rbuf_sz / MTU}
        , w_ch_{wbuf_sz / MTU}
    {
    }
    tcp_conn(io_t& io, sock_ptr_t sock, std::size_t rbuf_sz = 65535, std::size_t wbuf_sz = 65535) 
        : io_{io}
        , sock_{sock}
        , r_buf_{rbuf_sz}
        , w_buf_{wbuf_sz}
        , r_ch_{rbuf_sz / MTU}
        , w_ch_{wbuf_sz / MTU}
    {
    }
    ~tcp_conn() 
    {
        close();
    }

    inline void set_connect_handler(conn_handler_t&& fn) { conn_handler_ = std::move(fn); }
    inline void set_disconnect_handler(disconn_handler_t&& fn) { disconn_handler_ = std::move(fn); }
    inline void set_recv_handler(recv_handler_t&& fn) { recv_handler_ = std::move(fn); }
    inline void set_send_handler(send_handler_t&& fn) { send_handler_ = std::move(fn); }
    inline flag_t get_flag() { return flag_.load(); }
    inline void set_flag(const flag_t flag) { flag_.store(flag_.load() | flag); }
    inline void unset_flag(const flag_t flag) { flag_.store(flag_.load() & (~flag)); }
    inline bool is_connected() { return sock_ != nullptr && sock_->is_connected(); }
    inline bool is_w_closed() { return w_closed_.load(); }
    inline bool is_r_closed() { return w_closed_.load(); }
    void set_w_closed(bool is_closed) 
    { 
        w_closed_.store(is_closed);
        if (w_closed_.load() && r_closed_.load())
            disconnect();
    }
    void set_r_closed(bool is_closed) 
    { 
        r_closed_.store(is_closed);
        if (w_closed_.load() && r_closed_.load())
            disconnect();
    }

    bool connect(const char* ip, 
                 const std::uint16_t port,
                 std::chrono::milliseconds timeout = std::chrono::milliseconds(2000), 
                 int try_times = 1)
    {
        if (is_connected())
            return false;

        if (!sock_->connect(ip, port, timeout, try_times))
            return false;

        set_w_closed(false);
        set_r_closed(false);
        return true;
    }

    bool async_connect(const char* ip, const std::uint16_t port)
    {
        if (is_connected())
            return false;

        sock_->async_connect(ip, port, [this](const err_t& err, sock_ptr_t sock) {
            if (!err.failed())
            {
                set_w_closed(false);
                set_r_closed(false);

                io_.post(std::bind(&tcp_conn::_async_send, this, err_t(), 1));
                io_.post(std::bind(&tcp_conn::_async_recv, this, err_t(), 1));
            }

            this->conn_handler_(this, err);
        });
        return true;
    }

    bool disconnect()
    {
        if (!is_connected())
            return false;

        _send_all();
        _recv_all();

        sock_->disconnect();
        disconn_handler_(this);
        return true;
    }

    bool send(msg_ptr_t msg)
    {
        if (!is_connected() || is_w_closed())
            return false;

        w_ch_ << msg;
        return _send_all();
    }

    template<typename T>
    bool send(T arg)
    {
        return send(static_cast<msg_ptr_t>(arg));
    }

    bool recv(msg_ptr_t msg)
    {
        if (!is_connected() || is_r_closed())
            return false;

        r_ch_ << msg;
        return true;
    }

    template<typename T>
    bool recv(T arg)
    {
        return recv(static_cast<msg_ptr_t>(arg));
    }

    bool async_send(msg_ptr_t msg)
    {
        if (!is_connected() || is_w_closed())
            return false;

        w_ch_ << msg;
        return true;
    }

    bool async_recv(msg_ptr_t msg)
    {
        if (!is_connected() || is_r_closed())
            return false;

        r_ch_ << msg;
        return true;
    }

    void close()
    {
        set_w_closed(true);
        set_r_closed(true);

        if (sock_ == nullptr)
            return;
        sock_->close();
        delete sock_;
        sock_ = nullptr;
    }

private:
    void _async_send(const err_t& err, std::size_t sz)
    {
        if (err.failed() || sz == 0)
            set_w_closed(true);

        if (!is_connected() || is_w_closed()) 
            return;

        msg_ptr_t msg = nullptr;
        w_ch_ >> msg;
        if (msg == nullptr)
        {
            io_.post(std::bind(&tcp_conn::_async_send, this, err_t(), 1));
            return;
        }

        sz = msg->size();
        unsigned char buf[sz];
        sz = msg->encode(buf, sz);
        if (sz == 0)
        {
            set_w_closed(true);
            return;
        }

        send_handler_(this, msg);
        sock_->async_send(buf, sz, std::bind(&tcp_conn::_async_send, this, std::placeholders::_1, std::placeholders::_2));
    }

    void _async_recv(const err_t& err, std::size_t sz)
    {
        if (err.failed() || sz == 0)
            set_r_closed(true);

        if (!is_connected() || is_r_closed()) 
            return;

        this->r_buf_.commit(sz);
        msg_ptr_t msg = nullptr;
        r_ch_ >> msg;
        if (msg != nullptr)
        {
            auto data = boost::asio::buffer_cast<const unsigned char*>(r_buf_.data());
            sz = msg->decode(data, r_buf_.size());
            if (sz > 0)
                recv_handler_(this, msg);

            this->r_buf_.consume(sz);
        }

        auto buf = r_buf_.prepare(MTU);
        sock_->async_recv(buf, std::bind(&tcp_conn::_async_send, this, std::placeholders::_1, std::placeholders::_2));
    }

    bool _send_all()
    {
        std::size_t sz = 0;
        msg_ptr_t msg = nullptr;
        do {
            w_ch_ >> msg;
            if (msg == nullptr)
                break;

            if (sock_ == nullptr || !sock_->is_connected()) 
                return false;

            sz = msg->size();
            unsigned char buf[sz];
            sz = msg->encode(buf, sz);
            if (sz < 1) // decode fail, exit
            {
                set_w_closed(true);
                return false;
            }

            if (sock_->send(buf, sz) < sz) // send fail, exit
                return false;
        } while (sz > 0);
        return true;
    }

    bool _recv_all()
    {
        std::size_t sz = 0;
        msg_ptr_t msg = nullptr;
        r_ch_ >> msg;
        do {
            if (sock_ == nullptr || !sock_->is_connected()) 
                return false;

            if (msg == nullptr)
                break;
            
            auto data = boost::asio::buffer_cast<const unsigned char*>(r_buf_.data());
            sz = msg->decode(data, r_buf_.size());
            if (sz > 0)
            {
                r_ch_ >> msg;
                continue;
            }

            // big msg
            auto buf = r_buf_.prepare(MTU);
            sz = sock_->recv(buf);
            r_buf_.commit(sz);
        } while (sz > 0);
        return true;
    }

private:
    io_t&               io_;
    sock_ptr_t          sock_ = nullptr;
    std::atomic<flag_t> flag_{0};

    std::atomic<bool> w_closed_{false};
    std::atomic<bool> r_closed_{false};

    // NOTE: maybe use boost::asio::strand is a better choice
    tcp_socket::streambuf_t  r_buf_;
    tcp_socket::streambuf_t  w_buf_;
    tcp_chan<msg_ptr_t>      r_ch_;
    tcp_chan<msg_ptr_t>      w_ch_;

    conn_handler_t    conn_handler_ = [](conn_ptr_t, err_t){};
    disconn_handler_t disconn_handler_ = [](conn_ptr_t){};
    recv_handler_t       recv_handler_ = [](conn_ptr_t, msg_ptr_t){};
    send_handler_t       send_handler_ = [](conn_ptr_t, msg_ptr_t){};
};

}

#endif