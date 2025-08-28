#ifndef HTTP_WS_SERVER_HPP
#define HTTP_WS_SERVER_HPP

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

namespace libcpp
{

class ws_server
{
public:
    using io_t              = boost::asio::io_context;
    using addr_t            = boost::asio::ip::address;
    using sock_t            = boost::asio::ip::tcp::socket;
    using endpoint_t        = boost::asio::ip::tcp::endpoint;
    using acceptor_t        = boost::asio::ip::tcp::acceptor;
    using ws_stream_t       = boost::beast::websocket::stream<boost::asio::ip::tcp::socket>;
    using ws_stream_ptr_t   = std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>;
    using err_t             = boost::system::error_code;

    using opt_reuse_address = boost::asio::socket_base::reuse_address;

    using accept_handler_t  = std::function<void(const err_t&, ws_stream_ptr_t)>;
    using recv_handler_t    = std::function<void(const err_t&, ws_stream_ptr_t, std::string)>;
    using send_handler_t    = std::function<void(const err_t&, ws_stream_ptr_t, std::size_t)>;

public:
    ws_server() = delete;
    explicit ws_server(io_t& io)
        : io_{io}
        , binded_endpoint_{}
        , closed_{false}
        , acceptor_{nullptr}
    {}
    explicit ws_server(io_t& io, endpoint_t ep, accept_handler_t&& handler)
        : io_{io}
        , binded_endpoint_{}
        , closed_{false}
        , acceptor_{nullptr}
    {
        async_accept(ep, std::move(handler));
    }
    virtual ~ws_server()
    {
        close();
    }

    inline bool is_closed() { return closed_.load(); }
    inline endpoint_t& binded_endpoint() { return binded_endpoint_; }
    static inline endpoint_t make_endpoint(const uint16_t port)
    {
        return endpoint_t(boost::asio::ip::tcp::v4(), port);
    }
    static inline endpoint_t make_endpoint(const char* host, const uint16_t port)
    {
        return endpoint_t(addr_t::from_string(host), port);
    }

    ws_stream_ptr_t accept(const uint16_t port, err_t& err)
    {
        endpoint_t ep{boost::asio::ip::tcp::v4(), port};
        return accept(ep, err);
    }

    ws_stream_ptr_t accept(const endpoint_t& ep, err_t& err)
    {
        if (binded_endpoint_ == ep) 
            return accept(err);

        if (acceptor_ != nullptr) 
        {
            acceptor_->close();
            delete acceptor_;
            acceptor_ = nullptr;
        }

        acceptor_ = new acceptor_t(io_, ep);
        acceptor_->set_option(opt_reuse_address(true));
        binded_endpoint_ = ep;
        return accept(err);
    }

    ws_stream_ptr_t accept(err_t& err)
    {
        if (closed_.load() || acceptor_ == nullptr || !acceptor_->is_open())
        {
            err = boost::asio::error::not_connected;
            return nullptr;
        }

        auto sock = std::make_shared<sock_t>(io_);
        acceptor_->accept(*sock);
        auto ws = std::make_shared<ws_stream_t>(std::move(*sock));

        // handshake
        ws->accept(err);
        if (err)
            return nullptr;

        return ws;
    }

    void async_accept(endpoint_t ep, accept_handler_t&& handler)
    {
        if (closed_.load()) 
        {
            if (handler) 
                handler(make_error_code(boost::system::errc::not_connected), nullptr);
            return;
        }

        if (binded_endpoint_ == ep) 
        {
            async_accept(std::move(handler));
            return;
        }

        if (acceptor_ != nullptr) 
            acceptor_->close();

        acceptor_ = new acceptor_t(io_, ep);
        acceptor_->set_option(opt_reuse_address(true));
        binded_endpoint_ = ep;
        async_accept(std::move(handler));
    }

    void async_accept(accept_handler_t&& handler)
    {
        if (closed_.load() || acceptor_ == nullptr || !acceptor_->is_open())
        {
            if (handler) 
                handler(make_error_code(boost::system::errc::not_connected), nullptr);
            return;
        }

        auto sock = std::make_shared<sock_t>(io_);
        acceptor_->async_accept(*sock, [this, sock, handler](const err_t& ec) {
            if (ec) 
            {
                handler(ec, nullptr);
                return;
            }

            auto ws = std::make_shared<ws_stream_t>(std::move(*sock));
            ws->async_accept([this, ws, handler](const err_t& ec2) {
                handler(ec2, ws);
            });
        });
    }

    std::string recv(std::shared_ptr<ws_stream_t> ws, err_t& err)
    {
        if (closed_.load() || !ws)
        {
            err = boost::asio::error::not_connected;
            return std::string();
        }

        boost::beast::flat_buffer buffer(1024);
        auto sz = ws->read(buffer, err);
        if (err) 
            return std::string();

        return boost::beast::buffers_to_string(buffer.data());
    }

    void async_recv(std::shared_ptr<ws_stream_t> ws, recv_handler_t handler)
    {
        if (closed_.load() || !ws)
        {
            if (handler) 
                handler(make_error_code(boost::system::errc::not_connected), ws, "");

            return;
        }

        auto buffer = std::make_shared<boost::beast::flat_buffer>();
        ws->async_read(*buffer, [buffer, handler, ws](const err_t& ec, std::size_t) {
            std::string msg;
            if (!ec)
                msg = boost::beast::buffers_to_string(buffer->data());

            handler(ec, ws, msg);
        });
    }

    size_t send(std::shared_ptr<ws_stream_t> ws, err_t& err, const std::string& msg)
    {
        if (closed_.load() || !ws)
        {
            err = boost::asio::error::not_connected;
            return 0;
        }

        return ws->write(boost::asio::buffer(msg), err);
    }

    void async_send(std::shared_ptr<ws_stream_t> ws, const std::string& msg, send_handler_t handler)
    {
        if (closed_.load() || !ws) 
        {
            handler(boost::asio::error::not_connected, ws, 0);
            return;
        }

        ws->async_write(boost::asio::buffer(msg), [handler, ws](const err_t& ec, std::size_t bytes) {
            handler(ec, ws, bytes);
        });
    }

    void close()
    {
        if (closed_.load())
            return;

        closed_.store(true);
        if (acceptor_) 
        {
            acceptor_->close();
            delete acceptor_;
            acceptor_ = nullptr;
        }
        binded_endpoint_ = endpoint_t();
    }

private:
    io_t&             io_;
    acceptor_t*       acceptor_;
    endpoint_t        binded_endpoint_;
    std::atomic<bool> closed_;
};

} // namespace libcpp

#endif // HTTP_WS_SERVER_HPP