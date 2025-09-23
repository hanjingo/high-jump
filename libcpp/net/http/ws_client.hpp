#ifndef WS_CLIENT_HPP
#define WS_CLIENT_HPP

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <stdint.h>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

namespace libcpp
{

class ws_client
{
public:
    using io_t         = boost::asio::io_context;
    using addr_t       = boost::asio::ip::address;
    using endpoint_t   = boost::asio::ip::tcp::endpoint;
    using resolver_t   = boost::asio::ip::tcp::resolver;
    using ws_stream_t  = boost::beast::websocket::stream<boost::asio::ip::tcp::socket>;
    using buffer_t     = boost::beast::flat_buffer;
    using buffer_ptr_t = std::shared_ptr<buffer_t>;
    using err_t        = boost::system::error_code;

    using conn_handler_t       = std::function<void(const err_t&)>;
    using close_handler_t      = std::function<void(const err_t&)>;
    using send_handler_t       = std::function<void(const err_t&, std::size_t)>;
    using recv_handler_t       = std::function<void(const err_t&, std::string)>;

public:
    ws_client() = delete;
    ws_client(io_t& io)
        : io_{io}
        , ws_{std::make_unique<ws_stream_t>(io)}
        , connected_{false}
        , buffer_{}
    {}
    ~ws_client()
    {
        //close();
    }

    static inline endpoint_t make_endpoint(const uint16_t port)
    {
        return endpoint_t(boost::asio::ip::tcp::v4(), port);
    }
    static inline endpoint_t make_endpoint(const char* host, const uint16_t port)
    {
#if BOOST_VERSION < 108700
        return endpoint_t(addr_t::from_string(host), port);
#else
        return endpoint_t(boost::asio::ip::make_address(host), port);
#endif
    }

    inline bool is_connected() const { return connected_; }
    inline std::string remote_ip() const { return ws_->next_layer().remote_endpoint().address().to_string(); }
    inline uint16_t remote_port() const { return ws_->next_layer().remote_endpoint().port(); }
    inline ws_stream_t& stream() { return *ws_; }
    
    template <typename Opt>
    inline void set_option(const Opt& option){  ws_->set_option(option); }

    // Connect to a WebSocket server (blocking)
    bool connect(const std::string& host, const std::string& port, const std::string& target = "/")
    {
        err_t ec;
        resolver_t resolver(io_);
        auto const results = resolver.resolve(host, port, ec);
        if (ec) 
            return false;

        boost::asio::connect(ws_->next_layer(), results.begin(), results.end(), ec);
        if (ec) 
            return false;

        ws_->handshake(host, target, ec);
        if (ec) 
            return false;

        connected_ = true;
        return true;
    }

    // Asynchronously connect to a WebSocket server
    void async_connect(const std::string& host, 
                       const std::string& port, 
                       const std::string& target,
                       conn_handler_t handler)
    {
        auto resolver = std::make_shared<resolver_t>(io_);
        resolver->async_resolve(host, port, [this, resolver, host, target, handler](const err_t& ec, resolver_t::results_type results) {
            if (ec) 
            { 
                handler(ec); 
                return; 
            }

            boost::asio::async_connect(ws_->next_layer(), results.begin(), results.end(), [this, host, target, handler](const err_t& ec, auto it) {
                if (ec) 
                { 
                    handler(ec); 
                    return; 
                }

                ws_->async_handshake(host, target, [this, handler](const err_t& ec) {
                    if (!ec) 
                        connected_ = true;

                    handler(ec);
                });
            });
        });
    }

    bool send(const std::string& msg)
    {
        if (!connected_ || !ws_) 
            return false;

        err_t ec;
        ws_->write(boost::asio::buffer(msg), ec);
        return !ec;
    }

    void async_send(const std::string& msg, send_handler_t handler)
    {
        if (!connected_ || !ws_) 
        {
            handler(boost::asio::error::not_connected, 0);
            return;
        }

        ws_->async_write(boost::asio::buffer(msg), handler);
    }

    bool recv(std::string& out)
    {
        if (!connected_ || !ws_) 
            return false;

        buffer_t buffer;
        err_t ec;
        ws_->read(buffer, ec);
        if (ec) 
            return false;

        out = boost::beast::buffers_to_string(buffer.data());
        return true;
    }

    void async_recv(recv_handler_t handler)
    {
        if (!connected_ || !ws_) 
        {
            handler(boost::asio::error::not_connected, "");
            return;
        }

        ws_->async_read(buffer_, [this, handler](const err_t& ec, std::size_t sz) {
            if (ec || sz == 0 || buffer_.size() == 0) 
            {
                handler(ec, "");
                return;
            }

            std::string msg = boost::beast::buffers_to_string(buffer_.data());
            handler(ec, msg);
        });
    }

    void close()
    {
        if (!connected_)
            return;

        connected_ = false;
        if (ws_ && ws_->is_open())
        {
            err_t ec;
            ws_->close(boost::beast::websocket::close_code::normal, ec);
        }
    }

    void async_close(close_handler_t handler)
    {
        if (!connected_)
        {
            handler(boost::asio::error::not_connected);
            return;
        }

        connected_ = false;
        if (ws_)
        {
            ws_->async_close(boost::beast::websocket::close_code::normal, [handler](const err_t& ec){
                handler(ec);
            });
        }
    }

private:
    io_t&                        io_;
    std::unique_ptr<ws_stream_t> ws_;
    std::atomic<bool>            connected_;
    buffer_t                     buffer_;
};

} // namespace libcpp

#endif // HTTP_WS_CLIENT_HPP