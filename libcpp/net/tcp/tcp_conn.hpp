#ifndef TCP_CONN
#define TCP_CONN

#include <atomic>
#include <functional>

#include <libcpp/net/tcp/tcp_socket.hpp>
#include <libcpp/net/proto/message.hpp>
#include <boost/circular_buffer.hpp>

#ifndef TCP_CONN_RBUF_SZ
#define TCP_CONN_RBUF_SZ 1024
#endif

#ifndef TCP_CONN_WBUF_SZ
#define TCP_CONN_WBUF_SZ 1024
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
        : _conn_cb{[](tcp_conn*){}}
        , _send_cb{[](tcp_conn*, message*){}}
        , _recv_cb{[](tcp_conn*, message*){}}
        , _disconnect_cb{[](tcp_conn*){}}
    {}
    ~tcp_conn() {}

    inline void set_conn_cb(conn_handler_t&& fn) {_conn_cb = std::move(fn);}
    inline void set_send_cb(send_handler_t&& fn) {_send_cb = std::move(fn);}
    inline void set_recv_cb(recv_handler_t&& fn) {_recv_cb = std::move(fn);}
    inline void set_disconnect_cb(disconnect_handler_t&& fn) {_disconnect_cb = std::move(fn);}

    inline bool connect(const char* ip, 
                        const std::uint16_t port, 
                        std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
                        int retry_times = 1)
    {
        return _sock.connect(ip, port, timeout, retry_times);
    }

    inline bool async_connect(const char* ip, 
                              const std::uint16_t port)
    {
        _sock.async_connect(ip, port, [this](const tcp_socket::err_t& err, tcp_socket* sock){
            this->_conn_cb(this);
        });
        return true;
    }

    bool disconnect()
    {
        if (_w_closed.load() || _r_closed.load())
            return false;

        _sock.disconnect();
        _disconnect_cb(this);
        return true;
    }

    bool send(message* msg)
    {
        unsigned char buf[msg->size()];
        std::size_t sz = msg->encode(buf, sizeof(buf));
        std::size_t nsend = _sock.send(buf, sz);
        return nsend == sz;
    }

    bool async_send(message* msg)
    {
        unsigned char buf[msg->size()];
        std::size_t sz = msg->encode(buf, sizeof(buf));
        if (sz != msg->size())
            return false;

        _sock.async_send(buf, sz, [this, msg](const tcp_socket::err_t& err, std::size_t sz){
            this->_send_cb(this, msg);
        });
        return true;
    }

    bool recv(message* msg)
    {
        do {
            if (_sock.recv(_r_buf) == 0)
                break;
        } while(!msg->decode((unsigned char*)(_r_buf.data()), _r_buf.size()));
    }

    bool async_recv(message* msg)
    {
        _sock.async_recv(_r_buf, [this, msg](const tcp_socket::err_t& err, std::size_t sz){
            this->_recv_cb(this, msg);
        });
    }

    void close()
    {
        _w_closed.store(true);
        _r_closed.store(true);
        _sock.disconnect();
        _sock.loop_end();
    }

    void poll()
    {
        _sock.poll();
    }

private:
    tcp_socket _sock;

    std::atomic<bool> _w_closed{true};
    std::atomic<bool> _r_closed{true};

    tcp_socket::multi_buffer_t _r_buf{};

    conn_handler_t _conn_cb;
    send_handler_t _send_cb;
    recv_handler_t _recv_cb;
    disconnect_handler_t _disconnect_cb;
};

}

#endif