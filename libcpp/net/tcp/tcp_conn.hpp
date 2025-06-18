#ifndef TCP_CONN
#define TCP_CONN

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <vector>

#include <libcpp/net/tcp/tcp_socket.hpp>
#include <libcpp/net/tcp/tcp_chan.hpp>
#include <libcpp/net/proto/message.hpp>

#include <libcpp/net/tool/buf_debug.hpp>

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

#ifdef SMART_PTR_ENABLE
    using msg_ptr_t        = std::shared_ptr<libcpp::message>;
    using conn_ptr_t       = std::shared_ptr<libcpp::tcp_conn>;
    using sock_ptr_t       = std::shared_ptr<libcpp::tcp_socket>;
#else
    using msg_ptr_t        = libcpp::message*;
    using conn_ptr_t       = libcpp::tcp_conn*;
    using sock_ptr_t       = libcpp::tcp_socket*;
#endif

    using conn_handler_t    = std::function<void(conn_ptr_t, const err_t&)>;
    using send_handler_t    = std::function<void(conn_ptr_t, msg_ptr_t)>;
    using recv_handler_t    = std::function<void(conn_ptr_t, msg_ptr_t)>;
    using disconn_handler_t = std::function<void(conn_ptr_t)>;

public:
    tcp_conn() = delete;

    tcp_conn(io_t& io, 
             disconn_handler_t&& disconn_handler = std::function<void(conn_ptr_t)>(),
             send_handler_t&& send_handler = std::function<void(conn_ptr_t, msg_ptr_t)>(),
             recv_handler_t&& recv_handler = std::function<void(conn_ptr_t, msg_ptr_t)>(),
             std::size_t rbuf_sz = 65535, 
             std::size_t wbuf_sz = 65535) 
        : io_{io}
        , sock_{new tcp_socket(io_)}
        , send_handler_{send_handler}
        , recv_handler_{recv_handler}
        , disconn_handler_{disconn_handler}
        , r_buf_{rbuf_sz}
        , r_ch_{rbuf_sz / MTU}
        , w_ch_{wbuf_sz / MTU}
    {
    }
    tcp_conn(io_t& io, 
             sock_ptr_t sock, 
             disconn_handler_t&& disconn_handler = std::function<void(conn_ptr_t)>(),
             send_handler_t&& send_handler = std::function<void(conn_ptr_t, msg_ptr_t)>(),
             recv_handler_t&& recv_handler = std::function<void(conn_ptr_t, msg_ptr_t)>(),
             std::size_t rbuf_sz = 65535, 
             std::size_t wbuf_sz = 65535) 
        : io_{io}
        , sock_{sock}
        , send_handler_{send_handler}
        , recv_handler_{recv_handler}
        , disconn_handler_{disconn_handler}
        , r_buf_{rbuf_sz}
        , r_ch_{rbuf_sz / MTU}
        , w_ch_{wbuf_sz / MTU}
    {
    }
    ~tcp_conn() 
    {
        close();
    }

    inline flag_t get_flag() { return flag_.load(); }
    inline void set_flag(const flag_t flag) { flag_.store(flag_.load() | flag); }
    inline void unset_flag(const flag_t flag) { flag_.store(flag_.load() & (~flag)); }
    inline bool is_connected() { return sock_ != nullptr && sock_->is_connected(); }
    inline bool is_w_closed() { return w_closed_.load(); }
    inline bool is_r_closed() { return r_closed_.load(); }
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

        // active send event
        io_.post(std::bind(&tcp_conn::_async_send, this, err_t(), 0));
        // active recv event
        io_.post(std::bind(&tcp_conn::_async_recv, this, err_t(), 0));
        return true;
    }

    bool async_connect(const char* ip, const std::uint16_t port, conn_handler_t&& fn)
    {
        if (is_connected())
            return false;

        sock_->async_connect(ip, port, [this, fn](const err_t& err) {
            if (!err.failed())
            {
                set_w_closed(false);
                set_r_closed(false);

                // active send event
                io_.post(std::bind(&tcp_conn::_async_send, this, err_t(), 0));
                // active recv event
                io_.post(std::bind(&tcp_conn::_async_recv, this, err_t(), 0));
            }

            fn(this, err);
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
        if (disconn_handler_)
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

    bool recv(msg_ptr_t msg)
    {
        if (!is_connected() || is_r_closed())
            return false;

        r_ch_ << msg;
        return _recv_all();
    }

    bool async_send(msg_ptr_t msg)
    {
        if (!is_connected() || is_w_closed())
            return false;

        w_ch_ << msg;
        io_.post(std::bind(&tcp_conn::_async_send, this, err_t(), 0));
        return true;
    }

    bool async_recv(msg_ptr_t msg)
    {
        if (!is_connected() || is_r_closed())
            return false;

        r_ch_ << msg;
        io_.post(std::bind(&tcp_conn::_async_recv, this, err_t(), 0));
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
        if (err.failed())
            set_w_closed(true);

        if (!is_connected() || is_w_closed())
            return;

        w_buf_.consume(sz);
        msg_ptr_t msg = nullptr;
        if (!w_ch_.try_dequeue(msg) || msg == nullptr)
            return;

        sz = msg->size();
        unsigned char* data = boost::asio::buffer_cast<unsigned char*>(w_buf_.prepare(sz));
        sz = msg->encode(data, sz);
        if (sz < 1)
        {
            set_w_closed(true);
            return;
        }
        w_buf_.commit(sz);
        BUF_PRINT(this->w_buf_);

        sock_->async_send(data, sz, std::bind(&tcp_conn::_async_send, this, std::placeholders::_1, std::placeholders::_2));
        if (send_handler_)
            send_handler_(this, msg);
    }

    void _async_recv(const err_t& err, std::size_t sz)
    {
        if (err.failed())
            set_r_closed(true);

        if (!is_connected() || is_r_closed()) 
            return;

        if (sz > 0)
        {
            this->r_buf_.commit(sz);
            is_recving_.store(false);
            BUF_PRINT(this->r_buf_);
        }

        msg_ptr_t msg = nullptr;
        do {
            msg = nullptr;
            if (!r_ch_.try_dequeue(msg) || msg == nullptr)
                return;

            auto data = boost::asio::buffer_cast<const unsigned char*>(r_buf_.data());
            sz = r_buf_.size();
            sz = msg->decode(data, sz);
            if (sz > 0) 
            {
                this->r_buf_.consume(sz);
                if (recv_handler_) { recv_handler_(this, msg); }
            }
            else
            {
                r_ch_ << msg; // put back
                break; // big msg, wait for next recv
            }
        } while (true);

        // recv again
        if (is_recving_.load())
            return; // already recving, exit
        auto buf = r_buf_.prepare(MTU);
        is_recving_.store(true);
        sock_->async_recv(buf, std::bind(&tcp_conn::_async_recv, this, std::placeholders::_1, std::placeholders::_2));
    }

    bool _send_all()
    {
        if (is_w_closed())
            return false;

        std::size_t sz = 0;
        std::size_t nsend = 0;
        msg_ptr_t msg = nullptr;
        do {
            msg = nullptr;
            if (sock_ == nullptr || !sock_->is_connected() || is_w_closed()) 
                return false;

            if (!w_ch_.try_dequeue(msg) || msg == nullptr)
                break;

            sz = msg->size();
            unsigned char* data = boost::asio::buffer_cast<unsigned char*>(w_buf_.prepare(sz));
            sz = msg->encode(data, sz);
            if (sz < 1)
            {
                set_w_closed(true);
                return false;
            }

            w_buf_.commit(sz);
            BUF_PRINT(this->w_buf_);

            nsend = sock_->send(w_buf_.data());
            if (nsend < sz) // send fail, exit
            {
                set_w_closed(true);
                return false;
            }

            w_buf_.consume(nsend);
            if (send_handler_)
                send_handler_(this, msg);
        } while (true);

        return true;
    }

    bool _recv_all()
    {
        if (is_r_closed())
            return false;

        std::size_t sz = 0;
        msg_ptr_t msg = nullptr;
        do {
            msg = nullptr;
            if (sock_ == nullptr || !sock_->is_connected() || is_r_closed()) 
                return false;

            if (!r_ch_.try_dequeue(msg) || msg == nullptr)
                break;
            
            auto data = boost::asio::buffer_cast<const unsigned char*>(r_buf_.data());
            sz = msg->decode(data, r_buf_.size());
            if (sz > 0) 
            { 
                r_buf_.consume(sz);
                BUF_PRINT(this->r_buf_);
                if (recv_handler_)
                    recv_handler_(this, msg);

                continue; 
            }

            // big msg
            r_ch_ << msg; // put back
            auto buf = r_buf_.prepare(MTU);
            sz = sock_->recv(buf);
            r_buf_.commit(sz);
        } while (true);
        return true;
    }

private:
    io_t&               io_;
    sock_ptr_t          sock_ = nullptr;
    std::atomic<flag_t> flag_{0};

    std::atomic<bool> w_closed_{false};
    std::atomic<bool> r_closed_{false};
    std::atomic<bool> is_recving_{false};

    // NOTE: maybe use boost::asio::strand is a better choice
    tcp_socket::streambuf_t  r_buf_;
    tcp_socket::streambuf_t  w_buf_;
    tcp_chan<msg_ptr_t>      r_ch_;
    tcp_chan<msg_ptr_t>      w_ch_;

    disconn_handler_t    disconn_handler_;
    recv_handler_t       recv_handler_;
    send_handler_t       send_handler_;
};

}

#endif