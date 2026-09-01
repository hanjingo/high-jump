/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025-2026 hanjingo <hehehunanchina@live.com>
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

#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

#ifdef HJ_ENABLE_HTTPS
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#endif

#include <hj/net/http/http_header.hpp>
#include <httplib.h>
#include <functional>
#include <string>
#include <string_view>

namespace hj::http
{

class http_server
{
  public:
    http_server()  = default;
    ~http_server() = default;

    http_server(const http_server &)            = delete;
    http_server &operator=(const http_server &) = delete;

    http_server(http_server &&) noexcept            = default;
    http_server &operator=(http_server &&) noexcept = default;

    http_server &
    route(http_method m, std::string_view pattern, http_handler handler)
    {
        auto        adapter = _make_adapter(std::move(handler));
        std::string pattern_str(pattern);

        switch(m)
        {
            case http_method::get:
                _server.Get(pattern_str, adapter);
                break;
            case http_method::post:
                _server.Post(pattern_str, adapter);
                break;
            case http_method::put:
                _server.Put(pattern_str, adapter);
                break;
            case http_method::patch:
                _server.Patch(pattern_str, adapter);
                break;
            case http_method::del:
                _server.Delete(pattern_str, adapter);
                break;
            case http_method::head:
                _server.Get(pattern_str, adapter);
                break;
            case http_method::options:
                _server.Options(pattern_str, adapter);
                break;
        }
        return *this;
    }

    http_server &get(std::string_view pattern, http_handler handler)
    {
        return route(http_method::get, pattern, std::move(handler));
    }

    http_server &post(std::string_view pattern, http_handler handler)
    {
        return route(http_method::post, pattern, std::move(handler));
    }

    http_server &put(std::string_view pattern, http_handler handler)
    {
        return route(http_method::put, pattern, std::move(handler));
    }

    http_server &patch(std::string_view pattern, http_handler handler)
    {
        return route(http_method::patch, pattern, std::move(handler));
    }

    http_server &del(std::string_view pattern, http_handler handler)
    {
        return route(http_method::del, pattern, std::move(handler));
    }

    http_server &head(std::string_view pattern, http_handler handler)
    {
        return route(http_method::head, pattern, std::move(handler));
    }

    http_server &options(std::string_view pattern, http_handler handler)
    {
        return route(http_method::options, pattern, std::move(handler));
    }

    bool listen(const std::string &host, int port)
    {
        return _server.listen(host, port);
    }

    void stop() { _server.stop(); }

    bool is_running() const { return _server.is_running(); }

    httplib::Server &native_handle() noexcept { return _server; }

    const httplib::Server &native_handle() const noexcept { return _server; }

  private:
    static httplib::Server::Handler _make_adapter(http_handler handler)
    {
        return [handler = std::move(handler)](const httplib::Request &raw_req,
                                              httplib::Response &raw_resp) {
            http_request  req = detail::parse_httplib_request(raw_req);
            http_response resp;
            resp.status_code = 200;
            handler(req, resp);
            detail::apply_response(resp, raw_resp);
        };
    }

    httplib::Server _server;
};

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
class http_ssl_server
{
  public:
    http_ssl_server(const char *cert_path, const char *private_key_path)
        : _server(cert_path, private_key_path)
    {
    }
    ~http_ssl_server() = default;

    http_ssl_server(const http_ssl_server &)                = delete;
    http_ssl_server &operator=(const http_ssl_server &)     = delete;
    http_ssl_server(http_ssl_server &&) noexcept            = default;
    http_ssl_server &operator=(http_ssl_server &&) noexcept = default;

    http_ssl_server &
    route(http_method m, std::string_view pattern, http_handler handler)
    {
        auto        adapter = _make_adapter(std::move(handler));
        std::string pattern_str(pattern);

        switch(m)
        {
            case http_method::get:
                _server.Get(pattern_str, adapter);
                break;
            case http_method::post:
                _server.Post(pattern_str, adapter);
                break;
            case http_method::put:
                _server.Put(pattern_str, adapter);
                break;
            case http_method::patch:
                _server.Patch(pattern_str, adapter);
                break;
            case http_method::del:
                _server.Delete(pattern_str, adapter);
                break;
            case http_method::head:
                _server.Get(pattern_str, adapter);
                break;
            case http_method::options:
                _server.Options(pattern_str, adapter);
                break;
        }
        return *this;
    }

    http_ssl_server &get(std::string_view pattern, http_handler handler)
    {
        return route(http_method::get, pattern, std::move(handler));
    }

    http_ssl_server &post(std::string_view pattern, http_handler handler)
    {
        return route(http_method::post, pattern, std::move(handler));
    }

    http_ssl_server &put(std::string_view pattern, http_handler handler)
    {
        return route(http_method::put, pattern, std::move(handler));
    }

    http_ssl_server &patch(std::string_view pattern, http_handler handler)
    {
        return route(http_method::patch, pattern, std::move(handler));
    }

    http_ssl_server &del(std::string_view pattern, http_handler handler)
    {
        return route(http_method::del, pattern, std::move(handler));
    }

    http_ssl_server &head(std::string_view pattern, http_handler handler)
    {
        return route(http_method::head, pattern, std::move(handler));
    }

    http_ssl_server &options(std::string_view pattern, http_handler handler)
    {
        return route(http_method::options, pattern, std::move(handler));
    }

    bool listen(const std::string &host, int port)
    {
        return _server.listen(host, port);
    }

    void stop() { _server.stop(); }

    bool is_running() const { return _server.is_running(); }

    httplib::SSLServer &native_handle() noexcept { return _server; }

  private:
    static httplib::Server::Handler _make_adapter(http_handler handler)
    {
        return [handler = std::move(handler)](const httplib::Request &raw_req,
                                              httplib::Response &raw_resp) {
            http_request  req = detail::parse_httplib_request(raw_req);
            http_response resp;
            resp.status_code = 200;

            handler(req, resp);

            detail::apply_response(resp, raw_resp);
        };
    }

    httplib::SSLServer _server;
};
#endif

} // namespace hj::http

#endif // HTTP_SERVER_HPP