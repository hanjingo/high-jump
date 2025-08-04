#ifndef TCP_DIALER_HPP
#define TCP_DIALER_HPP

#include <functional>
#include <mutex>
#include <unordered_set>

#include <libcpp/net/tcp/tcp_socket.hpp>

namespace libcpp {

class tcp_dialer
{
  public:
    using io_t = libcpp::tcp_socket::io_t;
    using err_t = libcpp::tcp_socket::err_t;

#ifdef SMART_PTR_ENABLE
    using sock_ptr_t = std::shared_ptr<libcpp::tcp_socket>;
#else
    using sock_ptr_t = libcpp::tcp_socket*;
#endif

    using dial_handler_t = std::function<void(const err_t&, sock_ptr_t)>;
    using range_handler_t = std::function<bool(sock_ptr_t)>;

  public:
    tcp_dialer() = delete;
    explicit tcp_dialer(io_t& io) : io_{ io } {}

    virtual ~tcp_dialer() { close(); }

    inline std::size_t size()
    {
        std::lock_guard<std::mutex> lock(mu_);
        return socks_.size();
    }

    inline bool is_exist(sock_ptr_t sock)
    {
        std::lock_guard<std::mutex> lock(mu_);
        return socks_.find(sock) != socks_.end();
    }

    sock_ptr_t dial(
        const char* ip,
        const std::uint16_t port,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
        int try_times = 1)
    {
        sock_ptr_t sock = new tcp_socket(io_);
        if (sock->connect(ip, port, timeout, try_times))
        {
            return add_(sock) ? sock : nullptr;
        }

        delete sock;
        sock = nullptr;
        return nullptr;
    }

    void async_dial(const char* ip, const uint16_t port, dial_handler_t&& fn)
    {
        sock_ptr_t sock = new tcp_socket(io_);
        sock->async_connect(
            ip,
            port,
            [this, sock, fn = std::move(fn)](const err_t& err) mutable {
                this->add_(sock);
                fn(err, sock);
            });
    }

    void range(range_handler_t&& handler)
    {
        std::lock_guard<std::mutex> lock(mu_);
        for (auto sock : socks_)
        {
            if (!handler(sock))
                return;
        }
    }

    void remove(sock_ptr_t sock)
    {
        if (sock == nullptr || !is_exist(sock))
            return;

        if (sock->is_connected())
            sock->close();
        del_(sock);
        delete sock;
        sock = nullptr;
    }

    void close()
    {
        std::lock_guard<std::mutex> lock(mu_);
        for (auto itr = socks_.begin(); itr != socks_.end(); ++itr)
        {
            if (*itr == nullptr)
                continue;

            sock_ptr_t sock = (*itr);
            sock->close();
            delete sock;
        }
        socks_.clear();
    }

  private:
    bool add_(sock_ptr_t sock)
    {
        std::lock_guard<std::mutex> lock(mu_);
        return socks_.insert(sock).second;
    }

    bool del_(sock_ptr_t sock)
    {
        std::lock_guard<std::mutex> lock(mu_);
        return socks_.erase(sock) > 0;
    }

  private:
    io_t& io_;
    std::unordered_set<sock_ptr_t> socks_;
    std::mutex mu_;
};

}  // namespace libcpp

#endif