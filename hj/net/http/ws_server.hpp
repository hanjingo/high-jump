/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include <filesystem>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <boost/beast/websocket.hpp>
#include <boost/asio/ssl/stream.hpp>

// support for boost.beast websocket ssl teardown
namespace boost
{
namespace beast
{

template <class Executor>
void teardown(
    boost::beast::role_type,
    boost::asio::ssl::stream<boost::asio::basic_stream_socket<Executor>>
                              &stream,
    boost::system::error_code &ec)
{
    stream.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                                 ec);
    stream.next_layer().close(ec);
}

} // namespace beast
} // namespace boost

// ws_server / ws_server_ssl
namespace hj
{

class ws_server : public std::enable_shared_from_this<ws_server>
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
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>;
    using err_t = boost::system::error_code;

    using opt_reuse_address = boost::asio::socket_base::reuse_address;

    using accept_handler_t =
        std::function<void(const err_t &, ws_stream_ptr_t)>;
    using recv_handler_t =
        std::function<void(const err_t &, ws_stream_ptr_t, std::string)>;
    using send_handler_t =
        std::function<void(const err_t &, ws_stream_ptr_t, std::size_t)>;
    using close_handler_t = std::function<void(const err_t &)>;

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
        _mu.lock();
        if(_binded_endpoint == ep)
        {
            _mu.unlock();
            return accept(err);
        }

        if(_acceptor)
        {
            _acceptor->close();
            _acceptor.reset();
        }

        _acceptor = std::make_unique<acceptor_t>(_io, ep);
        _acceptor->set_option(opt_reuse_address(true));
        _binded_endpoint = ep;
        _mu.unlock();
        return accept(err);
    }

    ws_stream_ptr_t accept(err_t &err)
    {
        std::lock_guard<std::mutex> lock(_mu);
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

        if(_acceptor)
            _acceptor->close();

        _acceptor = std::make_unique<acceptor_t>(_io, ep);
        _acceptor->set_option(opt_reuse_address(true));
        _binded_endpoint = ep;
        async_accept(std::move(handler));
    }

    void async_accept(accept_handler_t &&handler)
    {
        auto self = shared_from_this();
        if(self->_closed.load() || !self->_acceptor
           || !self->_acceptor->is_open())
        {
            if(handler)
                handler(make_error_code(boost::system::errc::not_connected),
                        nullptr);
            return;
        }

        auto sock = std::make_shared<sock_t>(self->_io);
        self->_acceptor->async_accept(
            *sock,
            [self, sock, handler](const err_t &ec) {
                if(ec)
                {
                    handler(ec, nullptr);
                    return;
                }

                auto ws = std::make_shared<ws_stream_t>(std::move(*sock));
                ws->async_accept([self, ws, handler](const err_t &ec2) {
                    handler(ec2, ws);
                });
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
        auto self = shared_from_this();
        if(self->_closed.load() || !ws)
        {
            if(handler)
                handler(make_error_code(boost::system::errc::not_connected),
                        ws,
                        std::string());
            return;
        }

        auto buffer = std::make_shared<boost::beast::flat_buffer>();
        ws->async_read(
            *buffer,
            [self, buffer, handler, ws](const err_t &ec, std::size_t) {
                std::string msg;
                if(!ec)
                    msg = boost::beast::buffers_to_string(buffer->data());
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
        auto self = shared_from_this();
        if(self->_closed.load() || !ws)
        {
            handler(boost::asio::error::not_connected, ws, 0);
            return;
        }

        auto msg_ptr = std::make_shared<std::string>(msg);
        ws->async_write(
            boost::asio::buffer(*msg_ptr),
            [self, handler, ws, msg_ptr](const err_t &ec, std::size_t bytes) {
                handler(ec, ws, bytes);
            });
    }

    void close()
    {
        if(_closed.load())
            return;

        _closed.store(true);
        std::lock_guard<std::mutex> lock(_mu);
        if(_acceptor)
        {
            _acceptor->close();
            _acceptor.reset();
        }
        _binded_endpoint = endpoint_t();
    }

    void async_close(close_handler_t              handler,
                     std::vector<ws_stream_ptr_t> conns = {})
    {
        if(_closed.load())
        {
            if(handler)
                handler(boost::asio::error::not_connected);

            return;
        }
        _closed.store(true);

        std::lock_guard<std::mutex> lock(_mu);
        if(_acceptor)
        {
            _acceptor->close();
            _acceptor.reset();
        }

        _binded_endpoint = endpoint_t();
        // wait all ws closed
        auto left = std::make_shared<size_t>(conns.size());
        for(auto &ws : conns)
        {
            if(!ws || !ws->is_open())
                continue;

            ws->async_close(boost::beast::websocket::close_code::normal,
                            [left, handler](const err_t &e) {
                                if(--(*left) > 0 || !handler)
                                    return;

                                handler({});
                            });
            return;
        }

        if(handler)
            handler({});
    }

  private:
    io_t                       &_io;
    std::mutex                  _mu;
    std::unique_ptr<acceptor_t> _acceptor;
    endpoint_t                  _binded_endpoint;
    std::atomic<bool>           _closed;
};

class ws_server_ssl : public std::enable_shared_from_this<ws_server_ssl>
{
  public:
    using io_t              = boost::asio::io_context;
    using ssl_ctx_t         = boost::asio::ssl::context;
    using ssl_ctx_ptr_t     = std::shared_ptr<ssl_ctx_t>;
    using addr_t            = boost::asio::ip::address;
    using sock_t            = boost::asio::ip::tcp::socket;
    using endpoint_t        = boost::asio::ip::tcp::endpoint;
    using acceptor_t        = boost::asio::ip::tcp::acceptor;
    using ssl_stream_t      = boost::asio::ssl::stream<sock_t>;
    using ws_stream_t       = boost::beast::websocket::stream<ssl_stream_t>;
    using ws_stream_ptr_t   = std::shared_ptr<ws_stream_t>;
    using err_t             = boost::system::error_code;
    using opt_reuse_address = boost::asio::socket_base::reuse_address;

    using accept_handler_t =
        std::function<void(const err_t &, ws_stream_ptr_t)>;
    using recv_handler_t =
        std::function<void(const err_t &, ws_stream_ptr_t, std::string)>;
    using send_handler_t =
        std::function<void(const err_t &, ws_stream_ptr_t, std::size_t)>;
    using close_handler_t = std::function<void(const err_t &)>;

  public:
    ws_server_ssl() = delete;
    ws_server_ssl(io_t &io, ssl_ctx_ptr_t ssl_ctx)
        : _io{io}
        , _ssl_ctx{ssl_ctx}
        , _binded_endpoint{}
        , _closed{false}
        , _acceptor{nullptr}
    {
    }
    virtual ~ws_server_ssl() { close(); }

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

    static ssl_ctx_ptr_t make_ctx(const std::string &cert_file,
                                  const std::string &key_file,
                                  const std::string &ca_file = "")
    {
        auto ctx = std::make_shared<ssl_ctx_t>(ssl_ctx_t::sslv23);
        ctx->set_options(boost::asio::ssl::context::default_workarounds
                         | boost::asio::ssl::context::no_sslv2);
        ctx->use_certificate_chain_file(_resolve_path(cert_file));
        ctx->use_private_key_file(_resolve_path(key_file), boost::asio::ssl::context::pem);
        if(!ca_file.empty())
        {
            ctx->load_verify_file(_resolve_path(ca_file));
            ctx->set_verify_mode(boost::asio::ssl::verify_peer);
        } else
        {
            ctx->set_verify_mode(boost::asio::ssl::verify_none);
        }
        return ctx;
    }

    ws_stream_ptr_t accept(const uint16_t port, err_t &err)
    {
        endpoint_t ep{boost::asio::ip::tcp::v4(), port};
        return accept(ep, err);
    }

    ws_stream_ptr_t accept(const endpoint_t &ep, err_t &err)
    {
        _mu.lock();
        if(_binded_endpoint == ep)
        {
            _mu.unlock();
            return accept(err);
        }

        if(_acceptor)
        {
            _acceptor->close();
            _acceptor.reset();
        }

        _acceptor = std::make_unique<acceptor_t>(_io, ep);
        _acceptor->set_option(opt_reuse_address(true));
        _binded_endpoint = ep;
        _mu.unlock();
        return accept(err);
    }

    ws_stream_ptr_t accept(err_t &err)
    {
        std::lock_guard<std::mutex> lock(_mu);
        if(_closed.load() || _acceptor == nullptr || !_acceptor->is_open())
        {
            err = boost::asio::error::not_connected;
            return nullptr;
        }

        auto sock = std::make_shared<sock_t>(_io);
        _acceptor->accept(*sock);
        auto ssl_stream =
            std::make_shared<ssl_stream_t>(std::move(*sock), *_ssl_ctx);
        ssl_stream->handshake(boost::asio::ssl::stream_base::server, err);
        if(err)
            return nullptr;

        auto ws = std::make_shared<ws_stream_t>(std::move(*ssl_stream));
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

        if(_acceptor)
            _acceptor->close();

        _acceptor = std::make_unique<acceptor_t>(_io, ep);
        _acceptor->set_option(opt_reuse_address(true));
        _binded_endpoint = ep;
        async_accept(std::move(handler));
    }

    void async_accept(accept_handler_t &&handler)
    {
        auto self = shared_from_this();
        if(self->_closed.load() || !self->_acceptor
           || !self->_acceptor->is_open())
        {
            if(handler)
                handler(make_error_code(boost::system::errc::not_connected),
                        nullptr);
            return;
        }

        auto sock = std::make_shared<sock_t>(self->_io);
        self->_acceptor->async_accept(
            *sock,
            [self, sock, handler](const err_t &ec) {
                if(ec)
                {
                    handler(ec, nullptr);
                    return;
                }
                auto ssl_stream =
                    std::make_shared<ssl_stream_t>(std::move(*sock),
                                                   *self->_ssl_ctx);
                ssl_stream->async_handshake(
                    boost::asio::ssl::stream_base::server,
                    [self, ssl_stream, handler](const err_t &ec2) {
                        if(ec2)
                        {
                            handler(ec2, nullptr);
                            return;
                        }
                        auto ws = std::make_shared<ws_stream_t>(
                            std::move(*ssl_stream));
                        ws->async_accept([self, ws, handler](const err_t &ec3) {
                            handler(ec3, ws);
                        });
                    });
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
        auto self = shared_from_this();
        if(self->_closed.load() || !ws)
        {
            if(handler)
                handler(make_error_code(boost::system::errc::not_connected),
                        ws,
                        std::string());
            return;
        }
        auto buffer = std::make_shared<boost::beast::flat_buffer>();
        ws->async_read(
            *buffer,
            [self, buffer, handler, ws](const err_t &ec, std::size_t) {
                std::string msg;
                if(!ec)
                    msg = boost::beast::buffers_to_string(buffer->data());
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
        auto self = shared_from_this();
        if(self->_closed.load() || !ws)
        {
            handler(boost::asio::error::not_connected, ws, 0);
            return;
        }
        auto msg_ptr = std::make_shared<std::string>(msg);
        ws->async_write(
            boost::asio::buffer(*msg_ptr),
            [self, handler, ws, msg_ptr](const err_t &ec, std::size_t bytes) {
                handler(ec, ws, bytes);
            });
    }

    void close()
    {
        if(_closed.load())
            return;

        _closed.store(true);
        std::lock_guard<std::mutex> lock(_mu);
        if(_acceptor)
        {
            _acceptor->close();
            _acceptor.reset();
        }
        _binded_endpoint = endpoint_t();
    }

    void async_close(close_handler_t              handler,
                     std::vector<ws_stream_ptr_t> conns = {})
    {
        if(_closed.load())
        {
            if(handler)
                handler(boost::asio::error::not_connected);

            return;
        }

        _closed.store(true);
        std::lock_guard<std::mutex> lock(_mu);
        if(_acceptor)
        {
            _acceptor->close();
            _acceptor.reset();
        }

        _binded_endpoint = endpoint_t();
        // wait all ws closed
        auto left = std::make_shared<size_t>(conns.size());
        for(auto &ws : conns)
        {
            if(!ws || !ws->is_open())
                continue;

            ws->async_close(boost::beast::websocket::close_code::normal,
                            [left, handler](const err_t &e) {
                                if(--(*left) > 0 || !handler)
                                    return;

                                handler({});
                            });
        }

        if(handler)
            handler({});
    }

  private:
    static std::string _resolve_path(const std::string &path)
    {
        if (path.empty()) return path;
        
        namespace fs = std::filesystem;
        fs::path file_path(path);
        
        // If already absolute, return as-is
        if (file_path.is_absolute()) {
            return path;
        }
        
        // For relative paths, resolve against current working directory
        return fs::absolute(file_path).string();
    }

    io_t                       &_io;
    ssl_ctx_ptr_t               _ssl_ctx;
    std::mutex                  _mu;
    std::unique_ptr<acceptor_t> _acceptor;
    endpoint_t                  _binded_endpoint;
    std::atomic<bool>           _closed;
};

} // namespace hj

#endif // HTTP_WS_SERVER_HPP