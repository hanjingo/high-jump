#ifndef TCP_CONN
#define TCP_CONN

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <vector>

#include <hj/net/tcp/tcp_socket.hpp>

#include <concurrentqueue/blockingconcurrentqueue.h>
#include <boost/version.hpp>
#include <boost/asio.hpp>
#if BOOST_VERSION >= 108700
#include <boost/asio/post.hpp>
#include <boost/asio/ip/address.hpp>
#endif

#ifndef MTU
#define MTU 1500
#endif

namespace hj
{

class tcp_conn
{
  public:
    using flag_t = std::int64_t;
    using io_t   = hj::tcp_socket::io_t;
#if BOOST_VERSION < 108700
    using io_work_t = hj::tcp_socket::io_work_t;
#endif
    using err_t     = hj::tcp_socket::err_t;
    using msg_ptr_t = void *;
    using channel_t = moodycamel::BlockingConcurrentQueue<msg_ptr_t>;

#ifdef SMART_PTR_ENABLE
    using conn_ptr_t = std::shared_ptr<hj::tcp_conn>;
    using sock_ptr_t = std::shared_ptr<hj::tcp_socket>;
#else
    using conn_ptr_t = hj::tcp_conn *;
    using sock_ptr_t = hj::tcp_socket *;
#endif

    using conn_handler_t    = std::function<void(conn_ptr_t, const err_t &)>;
    using send_handler_t    = std::function<void(conn_ptr_t, msg_ptr_t)>;
    using recv_handler_t    = std::function<void(conn_ptr_t, msg_ptr_t)>;
    using disconn_handler_t = std::function<void(conn_ptr_t)>;
    using encode_handler_t  = std::function<std::size_t(
        unsigned char *, const std::size_t, msg_ptr_t)>;
    using decode_handler_t  = std::function<std::size_t(
        msg_ptr_t, const unsigned char *, const std::size_t)>;

    static const std::size_t block_sz =
        moodycamel::BlockingConcurrentQueue<msg_ptr_t>::BLOCK_SIZE;

  public:
    tcp_conn() = delete;

    tcp_conn(io_t &io, std::size_t rbuf_sz = 65535, std::size_t wbuf_sz = 65535)
        : _io{io}
        , _sock{new tcp_socket(_io)}
        , _r_buf{rbuf_sz}
        , _r_ch{rbuf_sz / MTU * block_sz}
        , _w_ch{wbuf_sz / MTU * block_sz}
    {
    }
    tcp_conn(io_t       &io,
             sock_ptr_t  sock,
             std::size_t rbuf_sz = 65535,
             std::size_t wbuf_sz = 65535)
        : _io{io}
        , _sock{sock}
        , _r_buf{rbuf_sz}
        , _r_ch{rbuf_sz / MTU * block_sz}
        , _w_ch{wbuf_sz / MTU * block_sz}
    {
    }
    ~tcp_conn() { close(); }

    inline void set_send_handler(const send_handler_t &&fn)
    {
        _send_handler = std::move(fn);
    }
    inline void set_recv_handler(const recv_handler_t &&fn)
    {
        _recv_handler = std::move(fn);
    }
    inline void set_disconn_handler(const disconn_handler_t &&fn)
    {
        _disconn_handler = std::move(fn);
    }
    inline void set_encode_handler(const encode_handler_t &&fn)
    {
        _encode_handler = std::move(fn);
    }
    inline void set_decode_handler(const decode_handler_t &&fn)
    {
        _decode_handler = std::move(fn);
    }
    inline flag_t get_flag() { return _flag.load(); }
    inline void   set_flag(const flag_t flag)
    {
        _flag.store(_flag.load() | flag);
    }
    inline void unset_flag(const flag_t flag)
    {
        _flag.store(_flag.load() & (~flag));
    }
    inline bool is_connected()
    {
        return _sock != nullptr && _sock->is_connected();
    }
    inline bool is_w_closed() { return _w_closed.load(); }
    inline bool is_r_closed() { return _r_closed.load(); }
    void        set_w_closed(bool is_closed)
    {
        _w_closed.store(is_closed);
        if(_w_closed.load() && _r_closed.load())
            disconnect();
    }
    void set_r_closed(bool is_closed)
    {
        _r_closed.store(is_closed);
        if(_w_closed.load() && _r_closed.load())
            disconnect();
    }

    bool
    connect(const char               *ip,
            const std::uint16_t       port,
            std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
            int                       try_times = 1)
    {
        if(is_connected())
            return false;

        if(!_sock->connect(ip, port, timeout, try_times))
            return false;

        set_w_closed(false);
        set_r_closed(false);

#if BOOST_VERSION < 108700
        _io.post(std::bind(&tcp_conn::_async_send, this, err_t(), 0));
        _io.post(std::bind(&tcp_conn::_async_recv, this, err_t(), 0));
#else
        boost::asio::post(_io,
                          std::bind(&tcp_conn::_async_send, this, err_t(), 0));
        boost::asio::post(_io,
                          std::bind(&tcp_conn::_async_recv, this, err_t(), 0));
#endif
        return true;
    }

    bool
    async_connect(const char *ip, const std::uint16_t port, conn_handler_t &&fn)
    {
        if(is_connected())
            return false;

        _sock->async_connect(ip, port, [this, fn](const err_t &err) {
            if(!err.failed())
            {
                set_w_closed(false);
                set_r_closed(false);

#if BOOST_VERSION < 108700
                _io.post(std::bind(&tcp_conn::_async_send, this, err_t(), 0));
                _io.post(std::bind(&tcp_conn::_async_recv, this, err_t(), 0));
#else
                boost::asio::post(_io, std::bind(&tcp_conn::_async_send, this, err_t(), 0));
                boost::asio::post(_io, std::bind(&tcp_conn::_async_recv, this, err_t(), 0));
#endif
            }

            fn(this, err);
        });
        return true;
    }

    bool disconnect()
    {
        if(!is_connected())
            return false;

        _send_all();
        _recv_all();

        _sock->disconnect();
        if(_disconn_handler)
            _disconn_handler(this);
        return true;
    }

    bool send(msg_ptr_t msg)
    {
        if(!is_connected() || is_w_closed())
            return false;

        _w_ch.enqueue(msg);
        return _send_all();
    }

    bool recv(msg_ptr_t msg)
    {
        if(!is_connected() || is_r_closed())
            return false;

        _r_ch.enqueue(msg);
        return _recv_all();
    }

    bool async_send(msg_ptr_t msg)
    {
        if(!is_connected() || is_w_closed())
            return false;

        _w_ch.enqueue(msg);
#if BOOST_VERSION < 108700
        _io.post(std::bind(&tcp_conn::_async_send, this, err_t(), 0));
#else
        boost::asio::post(_io,
                          std::bind(&tcp_conn::_async_send, this, err_t(), 0));
#endif
        return true;
    }

    bool async_recv(msg_ptr_t msg)
    {
        if(!is_connected() || is_r_closed())
            return false;

        _r_ch.enqueue(msg);
#if BOOST_VERSION < 108700
        _io.post(std::bind(&tcp_conn::_async_recv, this, err_t(), 0));
#else
        boost::asio::post(_io,
                          std::bind(&tcp_conn::_async_recv, this, err_t(), 0));
#endif
        return true;
    }

    void close()
    {
        set_w_closed(true);
        set_r_closed(true);

        if(_sock == nullptr)
            return;
        _sock->close();
        delete _sock;
        _sock = nullptr;
    }

  private:
    void _async_send(const err_t &err, std::size_t sz)
    {
        if(err.failed())
            set_w_closed(true);

        if(!is_connected() || is_w_closed())
            return;

        _w_buf.consume(sz);
        msg_ptr_t msg = nullptr;
        if(!_w_ch.try_dequeue(msg) || msg == nullptr || !_encode_handler)
            return;

        auto buf = _w_buf.prepare(MTU);
        sz       = buf.size();
#if BOOST_VERSION < 108700
        unsigned char *data = boost::asio::buffer_cast<unsigned char *>(buf);
#else
        unsigned char *data = static_cast<unsigned char *>(buf.data());
#endif
        sz = _encode_handler(data, sz, msg);
        if(sz < 1)
        {
            set_w_closed(true);
            return;
        }
        _w_buf.commit(sz);

        _sock->async_send(data,
                          sz,
                          std::bind(&tcp_conn::_async_send,
                                    this,
                                    std::placeholders::_1,
                                    std::placeholders::_2));
        if(_send_handler)
            _send_handler(this, msg);
    }

    void _async_recv(const err_t &err, std::size_t sz)
    {
        if(err.failed())
            set_r_closed(true);

        if(!is_connected() || is_r_closed())
            return;

        if(sz > 0)
        {
            this->_r_buf.commit(sz);
            _is_recving.store(false);
        }

        msg_ptr_t msg = nullptr;
        do
        {
            msg = nullptr;
            if(!_r_ch.try_dequeue(msg) || msg == nullptr || !_decode_handler)
                return;

#if BOOST_VERSION < 108700
            auto data =
                boost::asio::buffer_cast<const unsigned char *>(_r_buf.data());
#else
            auto data =
                static_cast<const unsigned char *>(_r_buf.data().data());
#endif
            sz = _r_buf.size();
            sz = _decode_handler(msg, data, sz);
            if(sz > 0)
            {
                this->_r_buf.consume(sz);
                if(_recv_handler)
                {
                    _recv_handler(this, msg);
                }
            } else
            {
                _r_ch.enqueue(msg); // put back
                break;
            }
        } while(true);

        if(_is_recving.load())
            return;
        auto buf = _r_buf.prepare(MTU);
        _is_recving.store(true);
        _sock->async_recv(buf,
                          std::bind(&tcp_conn::_async_recv,
                                    this,
                                    std::placeholders::_1,
                                    std::placeholders::_2));
    }

    bool _send_all()
    {
        if(is_w_closed())
            return false;

        std::size_t sz    = 0;
        std::size_t nsend = 0;
        msg_ptr_t   msg   = nullptr;
        do
        {
            msg = nullptr;
            if(_sock == nullptr || !_sock->is_connected() || is_w_closed())
                return false;

            if(!_w_ch.try_dequeue(msg) || msg == nullptr || !_encode_handler)
                break;

            auto buf = _w_buf.prepare(MTU);
            sz       = buf.size();
#if BOOST_VERSION < 108700
            unsigned char *data =
                boost::asio::buffer_cast<unsigned char *>(buf);
#else
            unsigned char *data = static_cast<unsigned char *>(buf.data());
#endif
            sz = _encode_handler(data, sz, msg);
            if(sz < 1)
            {
                set_w_closed(true);
                return false;
            }

            _w_buf.commit(sz);

            nsend = _sock->send(_w_buf.data());
            if(nsend < sz)
            {
                set_w_closed(true);
                return false;
            }

            _w_buf.consume(nsend);
            if(_send_handler)
                _send_handler(this, msg);
        } while(true);

        return true;
    }

    bool _recv_all()
    {
        if(is_r_closed())
            return false;

        std::size_t sz  = 0;
        msg_ptr_t   msg = nullptr;
        do
        {
            msg = nullptr;
            if(_sock == nullptr || !_sock->is_connected() || is_r_closed())
                return false;

            if(!_r_ch.try_dequeue(msg) || msg == nullptr || !_decode_handler)
                break;

#if BOOST_VERSION < 108700
            auto data =
                boost::asio::buffer_cast<const unsigned char *>(_r_buf.data());
#else
            auto data =
                static_cast<const unsigned char *>(_r_buf.data().data());
#endif
            sz = _r_buf.size();
            sz = _decode_handler(msg, data, sz);
            if(sz > 0)
            {
                _r_buf.consume(sz);
                if(_recv_handler)
                    _recv_handler(this, msg);

                continue;
            }

            _r_ch.enqueue(msg);
            auto buf = _r_buf.prepare(MTU);
            sz       = _sock->recv(buf);
            _r_buf.commit(sz);
        } while(true);
        return true;
    }

  private:
    io_t               &_io;
    sock_ptr_t          _sock = nullptr;
    std::atomic<flag_t> _flag{0};

    std::atomic<bool> _w_closed{false};
    std::atomic<bool> _r_closed{false};
    std::atomic<bool> _is_recving{false};

    // NOTE: maybe use boost::asio::strand is a better choice
    tcp_socket::streambuf_t _r_buf;
    tcp_socket::streambuf_t _w_buf;
    channel_t               _r_ch;
    channel_t               _w_ch;

    disconn_handler_t _disconn_handler;
    recv_handler_t    _recv_handler;
    send_handler_t    _send_handler;
    encode_handler_t  _encode_handler;
    decode_handler_t  _decode_handler;
};

}

#endif