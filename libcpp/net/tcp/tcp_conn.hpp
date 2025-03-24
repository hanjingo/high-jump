#ifndef TCP_CONN
#define TCP_CONN

#include <atomic>
#include <chrono>
#include <functional>

#include <libcpp/net/tcp/tcp_socket.hpp>
#include <libcpp/net/tcp/tcp_chan.hpp>
#include <libcpp/net/proto/message.hpp>
#include <boost/circular_buffer.hpp>

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
    template<typename T>
    using ring_buffer_t = boost::circular_buffer<T>;

    using conn_handler_t       = std::function<void(tcp_conn*)>;
    using send_handler_t       = std::function<void(tcp_conn*, message*)>;
    using recv_handler_t       = std::function<void(tcp_conn*, message*)>;
    using disconnect_handler_t = std::function<void(tcp_conn*)>;

public:
    tcp_conn() 
        : _sock{new tcp_socket()}
        , _conn_cb{std::bind(&tcp_conn::on_conn, this, std::placeholders::_1)}
        , _send_cb{std::bind(&tcp_conn::on_send, this, std::placeholders::_1, std::placeholders::_2)}
        , _recv_cb{std::bind(&tcp_conn::on_recv, this, std::placeholders::_1, std::placeholders::_2)}
        , _disconnect_cb{std::bind(&tcp_conn::on_disconnect, this, std::placeholders::_1)}
    {
    }
    tcp_conn(tcp_socket* sock) 
        : _sock{sock}
        , _conn_cb{std::bind(&tcp_conn::on_conn, this, std::placeholders::_1)}
        , _send_cb{std::bind(&tcp_conn::on_send, this, std::placeholders::_1, std::placeholders::_2)}
        , _recv_cb{std::bind(&tcp_conn::on_recv, this, std::placeholders::_1, std::placeholders::_2)}
        , _disconnect_cb{std::bind(&tcp_conn::on_disconnect, this, std::placeholders::_1)}
    {
    }
    ~tcp_conn() 
    {
        close();
    }

    inline void set_conn_cb(conn_handler_t&& fn) {_conn_cb = std::move(fn);}
    inline void set_send_cb(send_handler_t&& fn) {_send_cb = std::move(fn);}
    inline void set_recv_cb(recv_handler_t&& fn) {_recv_cb = std::move(fn);}
    inline void set_disconnect_cb(disconnect_handler_t&& fn) {_disconnect_cb = std::move(fn);}

    bool connect(const char* ip, 
                 const std::uint16_t port, 
                 std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
                 int retry_times = 1)
    {
        if (_sock != nullptr || !_sock->connect(ip, port, timeout, retry_times))
            return false;

        _conn_cb(this);
        return true;
    }

    bool async_connect(const char* ip, 
                       const std::uint16_t port)
    {
        _sock->async_connect(ip, port, [this](const tcp_socket::err_t& err, tcp_socket* sock){
            this->_conn_cb(this);
        });
        return true;
    }

    bool disconnect()
    {
        if (_sock == nullptr || _w_closed.load() || _r_closed.load())
            return false;

        poll();
        _sock->disconnect();
        _disconnect_cb(this);
        return true;
    }

    bool send(message* msg)
    {
        if (_sock == nullptr || _w_closed.load())
            return false;

        _w_ch << msg;
        return true;
    }

    bool recv(message* msg)
    {
        if (_sock == nullptr || _r_closed.load())
            return false;

        _r_ch << msg;
        return true;
    }

    void close()
    {
        _w_closed.store(true);
        _r_closed.store(true);
        _sock->close();
        delete _sock;
        _sock = nullptr;
        _disconnect_cb(this);
    }
    
    void read_all()
    {
        message* msg = nullptr;
        std::size_t sz = 0;
        do {
            // read
            _r_ch >> msg;
            if (msg == nullptr)
                break;

            do {
                sz = msg->decode(
                    boost::asio::buffer_cast<const unsigned char*>(_r_buf.data()), 
                    _r_buf.size());
                if (sz > 0)
                    break;

                auto buf = _r_buf.prepare(MTU);
                sz = _sock->recv(buf);
                if (sz == 0)
                    throw("tcp socket read fail");
                _r_buf.commit(sz);
            } while(sz > 0);

            _r_buf.consume(sz);
            _recv_cb(this, msg);
        } while (_sock != nullptr);
    }

    void write_all()
    {
        message* msg = nullptr;
        std::size_t sz;
        while (_sock != nullptr) 
        {
            // write
            _w_ch >> msg;
            if (msg == nullptr)
                break;
            
            unsigned char buf[msg->size()];
            sz = msg->encode(buf, sizeof(buf));
            if (sz == 0)
                break;

            if (_sock->send(buf, sz) < sz)
                break;

            _send_cb(this, msg);
        }
    }

    void poll()
    {
        _sock->poll();
        read_all();
        write_all();
    }

public:
    virtual void on_conn(tcp_conn* conn)
    {
        _w_closed.store(false);
        _r_closed.store(false);
    }

    virtual void on_send(tcp_conn* conn, message* msg)
    {
        // do something
    }

    virtual void on_recv(tcp_conn* conn, message* msg)
    {
        // do something
    }

    virtual void on_disconnect(tcp_conn* conn)
    {
        _w_closed.store(true);
        _r_closed.store(true);
    }

private:
    tcp_socket* _sock;

    std::atomic<bool> _w_closed{true};
    std::atomic<bool> _r_closed{true};

    tcp_socket::streambuf_t _r_buf{TCP_CONN_RBUF_SZ};
    tcp_chan<message*>      _r_ch{TCP_CONN_RBUF_SZ / MTU};
    tcp_chan<message*>      _w_ch{TCP_CONN_WBUF_SZ / MTU};

    conn_handler_t _conn_cb;
    send_handler_t _send_cb;
    recv_handler_t _recv_cb;
    disconnect_handler_t _disconnect_cb;
};

}

#endif