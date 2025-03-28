#ifndef TCP_CONN
#define TCP_CONN

#include <atomic>
#include <chrono>
#include <functional>

#include <libcpp/net/tcp/tcp_socket.hpp>
#include <libcpp/net/tcp/tcp_chan.hpp>
#include <libcpp/net/proto/message.hpp>

#ifndef TCP_CONN_RBUF_SZ
#define TCP_CONN_RBUF_SZ 65535
#endif

#ifndef TCP_CONN_WBUF_SZ
#define TCP_CONN_WBUF_SZ 65535
#endif

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
    using recv_handler_t       = std::function<void(conn_ptr_t, msg_ptr_t)>;
    using disconnect_handler_t = std::function<void(conn_ptr_t)>;

public:
    tcp_conn() = delete;

    explicit tcp_conn(io_t& io) 
        : _io{io}
        , _sock{new tcp_socket(_io)}
    {
    }
    explicit tcp_conn(io_t& io, sock_ptr_t sock) 
        : _io{io}
        , _sock{sock}
    {
    }
    ~tcp_conn() 
    {
        close();
    }

    inline void set_disconnect_handler(disconnect_handler_t&& fn) { _disconnect_handler = std::move(fn); }
    inline flag_t get_flag() { return _flag.load(); }
    inline void set_flag(const flag_t flag) { _flag.store(flag); }

    bool connect(const char* ip, const std::uint16_t port, int retry_times = 1)
    {
        if (_sock == nullptr || _sock->is_connected())
            return false;

        if (!_sock->connect(ip, port, retry_times))
            return false;

        _w_closed.store(false);
        _r_closed.store(false);
        return true;
    }

    bool async_connect(const char* ip, 
                       const std::uint16_t port,
                       conn_handler_t&& fn)
    {
        if (_sock == nullptr || _sock->is_connected())
            return false;

        _sock->async_connect(ip, port, [this, fn](const err_t& err, sock_ptr_t sock) {
            this->_w_closed.store(err.failed());
            this->_r_closed.store(err.failed());
            fn(this, err);
        });
        return true;
    }

    inline bool is_connected()
    {
        return !_w_closed.load() && !_r_closed.load() && _sock != nullptr && _sock->is_connected();
    }

    bool disconnect()
    {
        if (!is_connected())
            return false;

        if (!_w_closed.load())
        {
            _w_closed.store(true);
            write_all();
        }
        if (!_r_closed.load())
        {
            _r_closed.store(true);
            read_all();
        }
        _sock->disconnect();
        _disconnect_handler(this);
        return true;
    }

    bool send(msg_ptr_t msg)
    {
        if (_sock == nullptr || !_sock->is_connected() || _w_closed.load())
            return false;

        _w_ch << msg;
        write_all();
        return true;
    }

    bool async_send(msg_ptr_t msg, send_handler_t&& fn)
    {
        if (_sock == nullptr || !_sock->is_connected() || _w_closed.load())
            return false;

        _w_ch << msg;
        _w_ch >> msg;
        if (msg == nullptr) // already consumed by other thread
            return true;

        std::size_t sz = msg->size();
        unsigned char buf[sz];
        sz = msg->encode(buf, sz);
        if (sz < 1)
            return false;

        _sock->async_send(buf, sz, [this, msg, fn](const err_t& err, std::size_t sz){
            if (err.failed())
            {
                _w_closed.store(true);
                return;
            }

            fn(this, msg);
        });
        return true;
    }

    bool recv(msg_ptr_t msg)
    {
        if (_sock == nullptr || !_sock->is_connected() || _r_closed.load())
            return false;

        _r_ch << msg;
        read_all();
        return true;
    }

    bool async_recv(msg_ptr_t msg, recv_handler_t&& fn)
    {
        if (_sock == nullptr || !_sock->is_connected() || _r_closed.load())
            return false;

        _r_ch << msg; // push
        _r_ch >> msg; // pop
        _async_recv(msg, std::move(fn));
        return true;
    }

    void close()
    {
        if (_sock == nullptr)
            return;

        _w_closed.store(true);
        _r_closed.store(true);
        _sock->close();
        delete _sock;
        _sock = nullptr;
    }
    
    void read_all()
    {
        msg_ptr_t msg = nullptr;
        std::size_t sz = 0;
        while (_sock != nullptr && _sock->is_connected() && !_r_closed.load()) 
        {
            // read
            _r_ch >> msg;
            if (msg == nullptr)
                break;
           
            do {
                sz = msg->decode(boost::asio::buffer_cast<const unsigned char*>(_r_buf.data()), _r_buf.size());
                if (sz > 0)
                    break;

                auto buf = _r_buf.prepare(MTU);
                sz = _sock->recv(buf);
                _r_buf.commit(sz);
                if (sz == 0)
                {
                    _r_closed.store(true);
                    return;
                }
            } while(sz > 0);

            _r_buf.consume(sz);
        }
    }

    void write_all()
    {
        msg_ptr_t msg = nullptr;
        std::size_t sz;
        while (_sock != nullptr && _sock->is_connected() && !_w_closed.load()) 
        {
            // write
            _w_ch >> msg;
            if (msg == nullptr)
                break;
            
            unsigned char buf[msg->size()];
            sz = msg->encode(buf, sizeof(buf));
            if (sz == 0)
                break;

            if (_sock->send(buf, sz) < sz) // send fail
                _w_closed.store(true);
        }
    }

private:
    inline int64_t _time_since_epoch_ms()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    bool _async_recv(msg_ptr_t msg, recv_handler_t fn)
    {
        if (msg != nullptr)
        {
            auto data = boost::asio::buffer_cast<const unsigned char*>(_r_buf.data());
            std::size_t sz = msg->decode(data, _r_buf.size());
            if (sz > 0)
            {
                this->_r_buf.consume(sz);
                fn(this, msg);

                // next msg
                _r_ch >> msg;
            }
        } 
        else 
        {
            // pop msg if null
            _r_ch >> msg;
        }

        auto buf = _r_buf.prepare(MTU);
        _sock->async_recv(buf, [this, msg, fn](const err_t& err, std::size_t sz){
            if (err.failed())
            {
                this->_r_closed.store(true);
                return;
            }
    
            this->_r_buf.commit(sz);
            
            // msg too big do it again
            this->_async_recv(msg, fn);
        });
        return true;
    }

private:
    io_t&               _io;
    sock_ptr_t          _sock = nullptr;
    std::atomic<flag_t> _flag{0};

    std::atomic<bool> _w_closed{false};
    std::atomic<bool> _r_closed{false};

    // NOTE: maybe use boost::asio::strand is a better choice
    tcp_socket::streambuf_t  _r_buf{TCP_CONN_RBUF_SZ};
    tcp_chan<msg_ptr_t>      _r_ch{TCP_CONN_RBUF_SZ / MTU};
    tcp_chan<msg_ptr_t>      _w_ch{TCP_CONN_WBUF_SZ / MTU};

    disconnect_handler_t _disconnect_handler = [](conn_ptr_t conn){};
};

}

#endif