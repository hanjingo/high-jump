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

namespace hj
{

class ws_server
{
  public:
    using io_t       = boost::asio::io_context;
    using addr_t     = boost::asio::ip::address;
    using sock_t     = boost::asio::ip::tcp::socket;
    using endpoint_t = boost::asio::ip::tcp::endpoint;
    using acceptor_t = boost::asio::ip::tcp::acceptor;
    using ws_stream_t =
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket>;
    using ws_stream_ptr_t = std::shared_ptr<
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket> >;
    using err_t = boost::system::error_code;

    using opt_reuse_address = boost::asio::socket_base::reuse_address;

    using accept_handler_t =
        std::function<void(const err_t &, ws_stream_ptr_t)>;
    using recv_handler_t =
        std::function<void(const err_t &, ws_stream_ptr_t, std::string)>;
    using send_handler_t =
        std::function<void(const err_t &, ws_stream_ptr_t, std::size_t)>;

  public:
    ws_server() = delete;
    explicit ws_server(io_t &io)
        : _io{io}
        , _binded_endpoint{}
        , _closed{false}
        , _acceptor{nullptr}
    {
    }
    explicit ws_server(io_t &io, endpoint_t ep, accept_handler_t &&handler)
        : _io{io}
        , _binded_endpoint{}
        , _closed{false}
        , _acceptor{nullptr}
    {
        async_accept(ep, std::move(handler));
    }
    virtual ~ws_server() { close(); }

    inline bool              is_closed() { return _closed.load(); }
    inline endpoint_t       &binded_endpoint() { return _binded_endpoint; }
    static inline endpoint_t make_endpoint(const uint16_t port)
    {
        return endpoint_t(boost::asio::ip::tcp::v4(), port);
    }
    static inline endpoint_t make_endpoint(const char    *host,
                                           const uint16_t port)
    {
#if BOOST_VERSION < 108700
        return endpoint_t(addr_t::from_string(host), port);
#else
        return endpoint_t(boost::asio::ip::make_address(host), port);
#endif
    }

    ws_stream_ptr_t accept(const uint16_t port, err_t &err)
    {
        endpoint_t ep{boost::asio::ip::tcp::v4(), port};
        return accept(ep, err);
    }

    ws_stream_ptr_t accept(const endpoint_t &ep, err_t &err)
    {
        if(_binded_endpoint == ep)
            return accept(err);

        if(_acceptor != nullptr)
        {
            _acceptor->close();
            delete _acceptor;
            _acceptor = nullptr;
        }

        _acceptor = new acceptor_t(_io, ep);
        _acceptor->set_option(opt_reuse_address(true));
        _binded_endpoint = ep;
        return accept(err);
    }

    ws_stream_ptr_t accept(err_t &err)
    {
        if(_closed.load() || _acceptor == nullptr || !_acceptor->is_open())
        {
            err = boost::asio::error::not_connected;
            return nullptr;
        }

        auto sock = std::make_shared<sock_t>(_io);
        _acceptor->accept(*sock);
        auto ws = std::make_shared<ws_stream_t>(std::move(*sock));

        // handshake
        ws->accept(err);
        if(err)
            return nullptr;

        return ws;
    }

    void async_accept(endpoint_t ep, accept_handler_t &&handler)
    {
        if(_closed.load())
        {
            if(handler)
                handler(make_error_code(boost::system::errc::not_connected),
                        nullptr);
            return;
        }

        if(_binded_endpoint == ep)
        {
            async_accept(std::move(handler));
            return;
        }

        if(_acceptor != nullptr)
            _acceptor->close();

        _acceptor = new acceptor_t(_io, ep);
        _acceptor->set_option(opt_reuse_address(true));
        _binded_endpoint = ep;
        async_accept(std::move(handler));
    }

    void async_accept(accept_handler_t &&handler)
    {
        if(_closed.load() || _acceptor == nullptr || !_acceptor->is_open())
        {
            if(handler)
                handler(make_error_code(boost::system::errc::not_connected),
                        nullptr);
            return;
        }

        auto sock = std::make_shared<sock_t>(_io);
        _acceptor->async_accept(*sock, [this, sock, handler](const err_t &ec) {
            if(ec)
            {
                handler(ec, nullptr);
                return;
            }

            auto ws = std::make_shared<ws_stream_t>(std::move(*sock));
            ws->async_accept(
                [this, ws, handler](const err_t &ec2) { handler(ec2, ws); });
        });
    }

    std::string recv(std::shared_ptr<ws_stream_t> ws, err_t &err)
    {
        if(_closed.load() || !ws)
        {
            err = boost::asio::error::not_connected;
            return std::string();
        }

        boost::beast::flat_buffer buffer(1024);
        auto                      sz = ws->read(buffer, err);
        if(err)
            return std::string();

        return boost::beast::buffers_to_string(buffer.data());
    }

    void async_recv(std::shared_ptr<ws_stream_t> ws, recv_handler_t handler)
    {
        if(_closed.load() || !ws)
        {
            if(handler)
                handler(make_error_code(boost::system::errc::not_connected),
                        ws,
                        "");

            return;
        }

        auto buffer = std::make_shared<boost::beast::flat_buffer>();
        ws->async_read(*buffer,
                       [buffer, handler, ws](const err_t &ec, std::size_t) {
                           std::string msg;
                           if(!ec)
                               msg = boost::beast::buffers_to_string(
                                   buffer->data());

                           handler(ec, ws, msg);
                       });
    }

    size_t
    send(std::shared_ptr<ws_stream_t> ws, err_t &err, const std::string &msg)
    {
        if(_closed.load() || !ws)
        {
            err = boost::asio::error::not_connected;
            return 0;
        }

        return ws->write(boost::asio::buffer(msg), err);
    }

    void async_send(std::shared_ptr<ws_stream_t> ws,
                    const std::string           &msg,
                    send_handler_t               handler)
    {
        if(_closed.load() || !ws)
        {
            handler(boost::asio::error::not_connected, ws, 0);
            return;
        }

        ws->async_write(boost::asio::buffer(msg),
                        [handler, ws](const err_t &ec, std::size_t bytes) {
                            handler(ec, ws, bytes);
                        });
    }

    void close()
    {
        if(_closed.load())
            return;

        _closed.store(true);
        if(_acceptor)
        {
            _acceptor->close();
            delete _acceptor;
            _acceptor = nullptr;
        }
        _binded_endpoint = endpoint_t();
    }

  private:
    io_t             &_io;
    acceptor_t       *_acceptor;
    endpoint_t        _binded_endpoint;
    std::atomic<bool> _closed;
};

} // namespace hj

#endif // HTTP_WS_SERVER_HPP