#ifndef TCP_CONN
#define TCP_CONN

#include <atomic>
#include <chrono>
#include <functional>

#include <libcpp/net/tcp/tcp_socket.hpp>
#include <libcpp/net/tcp/tcp_chan.hpp>
#include <libcpp/net/proto/message.hpp>
#include <boost/circular_buffer.hpp>

#ifndef TCP_CONN_RCH_SZ
#define TCP_CONN_RCH_SZ 10
#endif

#ifndef TCP_CONN_WCH_SZ
#define TCP_CONN_WCH_SZ 10
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
    {
        init();
    }
    tcp_conn(tcp_socket* sock) 
        : _sock{sock}
    {
        init();
    }
    ~tcp_conn() 
    {
        close();
    }

    inline void set_conn_cb(conn_handler_t fn) {_conn_cb = fn;}
    inline void set_send_cb(send_handler_t fn) {_send_cb = fn;}
    inline void set_recv_cb(recv_handler_t fn) {_recv_cb = fn;}
    inline void set_disconnect_cb(disconnect_handler_t fn) {_disconnect_cb = fn;}
    inline tcp_socket* socket() {return _sock;}

    void init()
    {
        set_conn_cb(std::bind(this, &tcp_conn::on_conn, std::placeholders::_1));
        set_send_cb(std::bind(this, &tcp_conn::on_send, std::placeholders::_1, std::placeholders::_2));
        set_recv_cb(std::bind(this, &tcp_conn::on_recv, std::placeholders::_1, std::placeholders::_2));
        set_disconnect_cb(std::bind(this, &tcp_conn::on_disconnect, std::placeholders::_1));
    }

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

    bool send(message* msg, bool block = true, int timeout_ms = -1)
    {
        if (_sock == nullptr || _w_closed.load())
            return false;

        _w_ch << msg;
        if (block)
            write_flush(timeout_ms);
        return true;
    }

    bool recv(message* msg, bool block = true, int timeout_ms = -1)
    {
        if (_sock == nullptr || _r_closed.load())
            return false;

        _r_ch << msg;
        if (block)
            read_flush(timeout_ms);
        return true;
    }

    void close()
    {
        _w_closed.store(true);
        _r_closed.store(true);
        _sock->close();
        delete _sock;
        _sock == nullptr
        _disconnect_cb(this);
    }

    void read_flush(int timeout_ms = -1)
    {
        _sock->poll();

        auto start = std::chrono::system_clock::now();
        auto tm_passed_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
        message* msg = nullptr;
        std::size_t sz;
        while (_sock != nullptr) 
        {
            tm_passed_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
            if (timeout_ms != -1 && (timeout_ms - tm_passed_ms.count()) < 0)
                break;

            // read
            _r_ch >> msg;
            if (msg == nullptr)
                break;
            
            tcp_socket::multi_buffer_t buf;
            if (_sock->recv(buf) == 0)
                break;

            _r_buf
            sz = msg->decode(_r_buf.data(), _r_buf.size());
            if (sz == 0)
                continue;

            _r_buf.consume(sz);
            _recv_cb(this, msg);
        }
    }

    void write_flush(int timeout_ms = -1)
    {
        _sock->poll();

        auto start = std::chrono::system_clock::now();
        auto tm_passed_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
        message* msg = nullptr;
        std::size_t sz;
        while (_sock != nullptr) 
        {
            tm_passed_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
            if (timeout_ms != -1 && (timeout_ms - tm_passed_ms.count()) < 0)
                break;

            // write
            _w_ch >> msg;
            if (msg == nullptr)
                break;
            
            sz = msg->encode(_w_buf.data(), _w_buf.size());
            if (sz == 0)
                break;

            if (_sock->send(_w_buf.data(), sz) != sz)
                break;

            _w_buf.consume(sz);
            _send_cb(this, msg);
        }
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

    tcp_socket::streambuf_t _r_buf{};
    tcp_socket::streambuf_t _w_buf{};
    tcp_chan<message*>      _r_ch{TCP_CONN_RCH_SZ};
    tcp_chan<message*>      _w_ch{TCP_CONN_WCH_SZ};

    conn_handler_t _conn_cb = [](tcp_conn*){};
    send_handler_t _send_cb = [](tcp_conn*, message*){};
    recv_handler_t _recv_cb = [](tcp_conn*, message*){};
    disconnect_handler_t _disconnect_cb = [](tcp_conn*){};
};

}

#endif