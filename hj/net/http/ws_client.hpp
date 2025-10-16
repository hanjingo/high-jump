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

#ifndef WS_CLIENT_HPP
#define WS_CLIENT_HPP

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <stdint.h>
#include <filesystem>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>

namespace hj
{

class ws_client : public std::enable_shared_from_this<ws_client>
{
  public:
    using io_t       = boost::asio::io_context;
    using addr_t     = boost::asio::ip::address;
    using endpoint_t = boost::asio::ip::tcp::endpoint;
    using resolver_t = boost::asio::ip::tcp::resolver;
    using ws_t = boost::beast::websocket::stream<boost::asio::ip::tcp::socket>;

    using buffer_t     = boost::beast::flat_buffer;
    using buffer_ptr_t = std::shared_ptr<buffer_t>;
    using err_t        = boost::system::error_code;

    using conn_handler_t  = std::function<void(const err_t &)>;
    using close_handler_t = std::function<void(const err_t &)>;
    using send_handler_t  = std::function<void(const err_t &, std::size_t)>;
    using recv_handler_t  = std::function<void(const err_t &, std::string)>;

  public:
    ws_client() = delete;
    ws_client(io_t &io)
        : _io{io}
        , _ws{std::make_unique<ws_t>(io)}
        , _connected{false}
        , _buffer{}
    {
    }
    ~ws_client() { close(); }

    static endpoint_t make_endpoint(const uint16_t port)
    {
        return endpoint_t(boost::asio::ip::tcp::v4(), port);
    }
    static endpoint_t make_endpoint(const char *host, const uint16_t port)
    {
#if BOOST_VERSION < 108700
        return endpoint_t(addr_t::from_string(host), port);
#else
        return endpoint_t(boost::asio::ip::make_address(host), port);
#endif
    }

    inline bool is_connected() const { return _connected; }

    inline std::string remote_ip() const
    {
        return _ws->next_layer().remote_endpoint().address().to_string();
    }
    inline uint16_t remote_port() const
    {
        return _ws->next_layer().remote_endpoint().port();
    }

    template <typename Opt>
    inline void set_option(const Opt &option)
    {
        _ws->set_option(option);
    }

    bool connect(const std::string &host,
                 const std::string &port,
                 const std::string &target = "/")
    {
        err_t      ec;
        resolver_t resolver(_io);
        auto const results = resolver.resolve(host, port, ec);
        if(ec)
            return false;

        boost::asio::connect(_ws->next_layer(),
                             results.begin(),
                             results.end(),
                             ec);
        if(ec)
            return false;

        _ws->handshake(host, target, ec);
        if(ec)
            return false;

        _connected = true;
        return true;
    }

    void async_connect(const std::string &host,
                       const std::string &port,
                       const std::string &target,
                       conn_handler_t     handler)
    {
        auto self     = shared_from_this();
        auto resolver = std::make_shared<resolver_t>(_io);
        resolver->async_resolve(
            host,
            port,
            [self, resolver, host, target, handler](
                const err_t             &ec,
                resolver_t::results_type results) {
                if(ec)
                {
                    handler(ec);
                    return;
                }

                boost::asio::async_connect(
                    self->_ws->next_layer(),
                    results.begin(),
                    results.end(),
                    [self, host, target, handler](const err_t &ec, auto) {
                        if(ec)
                        {
                            handler(ec);
                            return;
                        }
                        self->_ws->async_handshake(
                            host,
                            target,
                            [self, handler](const err_t &ec) {
                                if(!ec)
                                    self->_connected = true;
                                handler(ec);
                            });
                    });
            });
    }

    bool send(const std::string &msg)
    {
        if(!_connected)
            return false;

        err_t ec;
        _ws->write(boost::asio::buffer(msg), ec);
        return !ec;
    }

    void async_send(const std::string &msg, send_handler_t handler)
    {
        auto self = shared_from_this();
        if(!self->_connected)
        {
            handler(boost::asio::error::not_connected, 0);
            return;
        }
        auto msg_ptr = std::make_shared<std::string>(msg);
        self->_ws->async_write(
            boost::asio::buffer(*msg_ptr),
            [self, handler, msg_ptr](const err_t &ec, std::size_t sz) {
                handler(ec, sz);
            });
    }

    bool recv(std::string &out)
    {
        if(!_connected)
            return false;

        buffer_t buffer;
        err_t    ec;
        _ws->read(buffer, ec);
        if(ec)
            return false;

        out = boost::beast::buffers_to_string(buffer.data());
        return true;
    }

    void async_recv(recv_handler_t handler)
    {
        auto self = shared_from_this();
        if(!self->_connected)
        {
            handler(boost::asio::error::not_connected, "");
            return;
        }
        auto buffer = std::make_shared<buffer_t>();
        self->_ws->async_read(
            *buffer,
            [self, buffer, handler](const err_t &ec, std::size_t sz) {
                if(ec || sz == 0 || buffer->size() == 0)
                {
                    handler(ec, "");
                    return;
                }
                std::string msg =
                    boost::beast::buffers_to_string(buffer->data());
                handler(ec, msg);
            });
    }

    void close()
    {
        if(!_connected)
            return;

        _connected = false;
        if(!_ws)
            return;

        err_t ec;
        if(_ws->is_open())
            _ws->close(boost::beast::websocket::close_code::normal, ec);

        _ws.reset();
    }

    void async_close(close_handler_t handler)
    {
        auto self = shared_from_this();
        if(!self->_connected)
        {
            handler(boost::asio::error::not_connected);
            return;
        }
        self->_connected = false;
        if(!self->_ws)
            return;

        self->_ws->async_close(boost::beast::websocket::close_code::normal,
                               [self, handler](const err_t &ec) {
                                   self->_ws.reset();
                                   handler({});
                               });
    }

  private:
    io_t                 &_io;
    std::unique_ptr<ws_t> _ws;
    std::atomic<bool>     _connected;
    buffer_t              _buffer;
};

class ws_client_ssl : public std::enable_shared_from_this<ws_client_ssl>
{
  public:
    using io_t          = boost::asio::io_context;
    using ssl_ctx_t     = boost::asio::ssl::context;
    using ssl_ctx_ptr_t = std::shared_ptr<ssl_ctx_t>;
    using addr_t        = boost::asio::ip::address;
    using endpoint_t    = boost::asio::ip::tcp::endpoint;
    using resolver_t    = boost::asio::ip::tcp::resolver;
    using ws_t          = boost::beast::websocket::stream<
                 boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>;

    using buffer_t     = boost::beast::flat_buffer;
    using buffer_ptr_t = std::shared_ptr<buffer_t>;
    using err_t        = boost::system::error_code;

    using conn_handler_t  = std::function<void(const err_t &)>;
    using close_handler_t = std::function<void(const err_t &)>;
    using send_handler_t  = std::function<void(const err_t &, std::size_t)>;
    using recv_handler_t  = std::function<void(const err_t &, std::string)>;

  public:
    ws_client_ssl() = delete;
    ws_client_ssl(io_t &io, ssl_ctx_ptr_t ssl_ctx)
        : _io{io}
        , _ssl_ctx{ssl_ctx}
        , _ws{std::make_unique<ws_t>(io, *_ssl_ctx)}
        , _connected{false}
        , _buffer{}
    {
    }
    ~ws_client_ssl() { close(); }

    static endpoint_t make_endpoint(const uint16_t port)
    {
        return endpoint_t(boost::asio::ip::tcp::v4(), port);
    }
    static endpoint_t make_endpoint(const char *host, const uint16_t port)
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
        auto ctx = std::make_shared<ssl_ctx_t>(ssl_ctx_t::sslv23_client);
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

    inline bool is_connected() const { return _connected; }

    inline std::string remote_ip() const
    {
        return _ws->next_layer()
            .next_layer()
            .remote_endpoint()
            .address()
            .to_string();
    }
    inline uint16_t remote_port() const
    {
        return _ws->next_layer().next_layer().remote_endpoint().port();
    }

    template <typename Opt>
    inline void set_option(const Opt &option)
    {
        _ws->set_option(option);
    }

    bool connect(const std::string &host,
                 const std::string &port,
                 const std::string &target = "/")
    {
        err_t      ec;
        resolver_t resolver(_io);
        auto const results = resolver.resolve(host, port, ec);
        if(ec)
            return false;

        boost::asio::connect(_ws->next_layer().next_layer(),
                             results.begin(),
                             results.end(),
                             ec);
        if(ec)
            return false;

        _ws->next_layer().handshake(boost::asio::ssl::stream_base::client, ec);
        if(ec)
            return false;

        _ws->handshake(host, target, ec);
        if(ec)
            return false;

        _connected = true;
        return true;
    }

    void async_connect(const std::string &host,
                       const std::string &port,
                       const std::string &target,
                       conn_handler_t     handler)
    {
        auto resolver = std::make_shared<resolver_t>(_io);
        resolver->async_resolve(
            host,
            port,
            [this, resolver, host, target, handler](
                const err_t             &ec,
                resolver_t::results_type results) {
                if(ec)
                {
                    handler(ec);
                    return;
                }

                boost::asio::async_connect(
                    _ws->next_layer().next_layer(),
                    results.begin(),
                    results.end(),
                    [this, host, target, handler](const err_t &ec, auto) {
                        if(ec)
                        {
                            handler(ec);
                            return;
                        }
                        _ws->next_layer().async_handshake(
                            boost::asio::ssl::stream_base::client,
                            [this, host, target, handler](const err_t &ec) {
                                if(ec)
                                {
                                    handler(ec);
                                    return;
                                }
                                _ws->async_handshake(
                                    host,
                                    target,
                                    [this, handler](const err_t &ec) {
                                        if(!ec)
                                            _connected = true;
                                        handler(ec);
                                    });
                            });
                    });
            });
    }

    bool send(const std::string &msg)
    {
        if(!_connected)
            return false;

        err_t ec;
        _ws->write(boost::asio::buffer(msg), ec);
        return !ec;
    }

    void async_send(const std::string &msg, send_handler_t handler)
    {
        if(!_connected)
        {
            handler(boost::asio::error::not_connected, 0);
            return;
        }
        _ws->async_write(boost::asio::buffer(msg), handler);
    }

    bool recv(std::string &out)
    {
        if(!_connected)
            return false;

        buffer_t buffer;
        err_t    ec;
        _ws->read(buffer, ec);
        if(ec)
            return false;

        out = boost::beast::buffers_to_string(buffer.data());
        return true;
    }

    void async_recv(recv_handler_t handler)
    {
        if(!_connected)
        {
            handler(boost::asio::error::not_connected, "");
            return;
        }

        _ws->async_read(_buffer,
                        [this, handler](const err_t &ec, std::size_t sz) {
                            if(ec || sz == 0 || _buffer.size() == 0)
                            {
                                handler(ec, "");
                                return;
                            }
                            std::string msg =
                                boost::beast::buffers_to_string(_buffer.data());
                            handler(ec, msg);
                        });
    }

    void close()
    {
        if(!_connected)
            return;

        _connected = false;
        if(!_ws)
            return;

        err_t ec;
        if(_ws->is_open())
            _ws->close(boost::beast::websocket::close_code::normal, ec);

        _ws.reset();
    }

    void async_close(close_handler_t handler)
    {
        if(!_connected)
        {
            handler(boost::asio::error::not_connected);
            return;
        }
        _connected = false;
        if(!_ws)
            return;

        _ws->async_close(boost::beast::websocket::close_code::normal,
                         [this, handler](const err_t &ec) {
                             _ws.reset();
                             handler({});
                         });
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

    io_t                 &_io;
    ssl_ctx_ptr_t         _ssl_ctx;
    std::unique_ptr<ws_t> _ws;
    std::atomic<bool>     _connected;
    buffer_t              _buffer;
};

} // namespace hj

#endif // WS_CLIENT_HPP
