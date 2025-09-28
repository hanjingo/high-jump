#ifndef TCP_DIALER_HPP
#define TCP_DIALER_HPP

#include <unordered_set>
#include <mutex>
#include <functional>
#include <boost/version.hpp>
#include <hj/net/tcp/tcp_socket.hpp>

namespace hj
{

class tcp_dialer
{
  public:
    using io_t  = hj::tcp_socket::io_t;
    using err_t = hj::tcp_socket::err_t;

#ifdef SMART_PTR_ENABLE
    using sock_ptr_t = std::shared_ptr<hj::tcp_socket>;
#else
    using sock_ptr_t = hj::tcp_socket *;
#endif

    using dial_handler_t  = std::function<void(const err_t &, sock_ptr_t)>;
    using range_handler_t = std::function<bool(sock_ptr_t)>;

  public:
    tcp_dialer() = delete;
    explicit tcp_dialer(io_t &io)
        : _io{io}
    {
    }

    virtual ~tcp_dialer() { close(); }

    inline std::size_t size()
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _socks.size();
    }

    inline bool is_exist(sock_ptr_t sock)
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _socks.find(sock) != _socks.end();
    }

    sock_ptr_t
    dial(const char               *ip,
         const std::uint16_t       port,
         std::chrono::milliseconds timeout   = std::chrono::milliseconds(2000),
         int                       try_times = 1)
    {
        sock_ptr_t sock = new tcp_socket(_io);
        if(sock->connect(ip, port, timeout, try_times))
        {
            return add_(sock) ? sock : nullptr;
        }

        delete sock;
        sock = nullptr;
        return nullptr;
    }

    void async_dial(const char *ip, const uint16_t port, dial_handler_t &&fn)
    {
        sock_ptr_t sock = new tcp_socket(_io);
        sock->async_connect(
            ip,
            port,
            [this, sock, fn = std::move(fn)](const err_t &err) mutable {
                this->add_(sock);
                fn(err, sock);
            });
    }

    void range(range_handler_t &&handler)
    {
        std::lock_guard<std::mutex> lock(_mu);
        for(auto sock : _socks)
        {
            if(!handler(sock))
                return;
        }
    }

    void remove(sock_ptr_t sock)
    {
        if(sock == nullptr || !is_exist(sock))
            return;

        if(sock->is_connected())
            sock->close();
        del_(sock);
        delete sock;
        sock = nullptr;
    }

    void close()
    {
        std::lock_guard<std::mutex> lock(_mu);
        for(auto itr = _socks.begin(); itr != _socks.end(); ++itr)
        {
            if(*itr == nullptr)
                continue;

            sock_ptr_t sock = (*itr);
            sock->close();
            delete sock;
        }
        _socks.clear();
    }

  private:
    bool add_(sock_ptr_t sock)
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _socks.insert(sock).second;
    }

    bool del_(sock_ptr_t sock)
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _socks.erase(sock) > 0;
    }

  private:
    io_t                          &_io;
    std::unordered_set<sock_ptr_t> _socks;
    std::mutex                     _mu;
};

}

#endif