#ifndef UDP_SOCKET_HPP
#define UDP_SOCKET_HPP

#include <boost/asio.hpp>

namespace libcpp
{
    
class udp_socket
{
public:
    using io_t              = boost::asio::io_context;
    using io_work_t         = boost::asio::io_service::work;
    using err_t             = boost::system::error_code;
    using address_t         = boost::asio::ip::address;

    using const_buffer_t    = boost::asio::const_buffer;
    using multi_buffer_t    = boost::asio::mutable_buffer;

    using sock_t            = boost::asio::ip::udp::socket;
    using endpoint_t        = boost::asio::ip::udp::endpoint;

    using send_handler_t    = std::function<void(const err_t&, std::size_t)>;
    using recv_handler_t    = std::function<void(const err_t&, std::size_t)>;

public:
    udp_socket()
    {
    }
    
    udp_socket(const uint16_t port)
        : sock_{new sock_t(io_, endpoint_v4(port))}
    {
    }
    
    virtual ~udp_socket()
    {
        close();
    }

    inline void poll()
    {
        io_.run();
    }

    inline void loop_start()
    {
        io_work_t work{io_};
        io_.run();
    }

    inline void loop_end()
    {
        io_.stop();
    }

    void bind(const uint16_t port)
    {
        if (sock_ != nullptr)
        {
            sock_->close();
            sock_ = nullptr;
            return;
        }

        sock_ = new sock_t(io_, endpoint_v4(port));
    }

    size_t send(const const_buffer_t& buf, endpoint_t& ep)
    {
        return sock_->send_to(buf, ep);
    }

    size_t send(const char* data, size_t len, endpoint_t& ep)
    {
        auto buf = boost::asio::buffer(data, len);
        return send(buf, ep);
    }

    void async_send(const const_buffer_t& buf, endpoint_t& ep, send_handler_t&& fn)
    {
        sock_->async_send_to(buf, ep, std::move(fn));
    }

    void async_send(const char* data, size_t len, endpoint_t& ep, send_handler_t&& fn)
    {
        auto buf = boost::asio::buffer(data, len);
        async_send(buf, ep, std::move(fn));
    }

    size_t recv(multi_buffer_t& buf, endpoint_t& ep)
    {
        return sock_->receive_from(buf, ep);
    }

    size_t recv(char* data, size_t len, endpoint_t& ep)
    {
        multi_buffer_t buf{data, len};
        return recv(buf, ep);
    }

    void async_recv(multi_buffer_t& buf, endpoint_t& ep, recv_handler_t&& fn)
    {
        sock_->async_receive_from(buf, ep, std::move(fn));
    }

    void async_recv(char* data, size_t len, endpoint_t& ep, recv_handler_t&& fn)
    {
        multi_buffer_t buf{data, len};
        async_recv(buf, ep, std::move(fn));
    }

    void close()
    {
        if (sock_ == nullptr)
            return;

        sock_->close();
        delete sock_;
        sock_ = nullptr;

        loop_end();
    }

public:
    static endpoint_t end_point(const char* ip, const uint16_t port)
    {
        return endpoint_t(address_t::from_string(ip), port);
    }

    static endpoint_t endpoint_v4(const uint16_t port)
    {
        return endpoint_t(boost::asio::ip::udp::v4(), port);
    }

    static endpoint_t end_point_v6(const uint16_t port)
    {
        return endpoint_t(boost::asio::ip::udp::v6(), port);
    }

private:
    io_t       io_;
    sock_t*    sock_ = nullptr;
};

}

#endif