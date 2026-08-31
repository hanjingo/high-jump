/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025-2026 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in me hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HJ_NET_HTTP_CLIENT_HPP
#define HJ_NET_HTTP_CLIENT_HPP

#ifdef HJ_ENABLE_HTTPS
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#endif

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

#include <httplib.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

namespace hj
{

struct case_insensitive_hash
{
    std::size_t operator()(std::string_view key) const noexcept
    {
        std::size_t hash = 0;
        for(char c : key)
        {
            hash = hash * 31
                   + static_cast<std::size_t>(
                       std::tolower(static_cast<unsigned char>(c)));
        }
        return hash;
    }
};

struct case_insensitive_equal
{
    bool operator()(std::string_view lhs, std::string_view rhs) const noexcept
    {
        return lhs.size() == rhs.size()
               && std::equal(
                   lhs.begin(),
                   lhs.end(),
                   rhs.begin(),
                   [](char a, char b) {
                       return std::tolower(static_cast<unsigned char>(a))
                              == std::tolower(static_cast<unsigned char>(b));
                   });
    }
};

class http_headers
{
  public:
    using container_type = std::unordered_multimap<std::string,
                                                   std::string,
                                                   case_insensitive_hash,
                                                   case_insensitive_equal>;

    http_headers() = default;

    http_headers(
        std::initializer_list<std::pair<std::string, std::string>> init)
    {
        for(const auto &[k, v] : init)
        {
            _headers.emplace(k, v);
        }
    }

    void append(std::string key, std::string value)
    {
        _headers.emplace(std::move(key), std::move(value));
    }

    void set(std::string key, std::string value)
    {
        _headers.erase(key);
        _headers.emplace(std::move(key), std::move(value));
    }

    [[nodiscard]] std::string get(std::string_view key) const
    {
        auto it = _headers.find(std::string(key));
        if(it != _headers.end())
        {
            return it->second;
        }
        return {};
    }

    [[nodiscard]] std::vector<std::string> get_all(std::string_view key) const
    {
        std::vector<std::string> result;
        auto [range_begin, range_end] = _headers.equal_range(std::string(key));
        for(auto it = range_begin; it != range_end; ++it)
        {
            result.push_back(it->second);
        }
        return result;
    }

    [[nodiscard]] bool contains(std::string_view key) const
    {
        return _headers.find(std::string(key)) != _headers.end();
    }

    [[nodiscard]] bool empty() const noexcept { return _headers.empty(); }
    [[nodiscard]] std::size_t size() const noexcept { return _headers.size(); }

    auto begin() noexcept { return _headers.begin(); }
    auto end() noexcept { return _headers.end(); }
    auto begin() const noexcept { return _headers.begin(); }
    auto end() const noexcept { return _headers.end(); }

  private:
    container_type _headers;
};

enum class http_method
{
    get,
    post,
    put,
    patch,
    del,
    head,
    options
};

[[nodiscard]] inline std::string_view to_string(http_method m) noexcept
{
    switch(m)
    {
        case http_method::get:
            return "GET";
        case http_method::post:
            return "POST";
        case http_method::put:
            return "PUT";
        case http_method::patch:
            return "PATCH";
        case http_method::del:
            return "DELETE";
        case http_method::head:
            return "HEAD";
        case http_method::options:
            return "OPTIONS";
    }
    return "UNKNOWN";
}

struct http_timeout
{
    std::chrono::milliseconds connect{5000};
    std::chrono::milliseconds read{5000};
    std::chrono::milliseconds write{5000};

    /* implicit */ http_timeout(std::chrono::milliseconds timeout)
        : connect(timeout)
        , read(timeout)
        , write(timeout)
    {
    }

    http_timeout(std::chrono::milliseconds conn_ms,
                 std::chrono::milliseconds read_ms,
                 std::chrono::milliseconds write_ms)
        : connect(conn_ms)
        , read(read_ms)
        , write(write_ms)
    {
    }

    http_timeout() = default;
};

struct proxy_config
{
    std::string host;
    int         port{0};
    std::string username;
    std::string password;

    [[nodiscard]] bool valid() const noexcept
    {
        return !host.empty() && port > 0;
    }
};

struct tls_config
{
    bool verify_server_certificate{true};
    bool verify_hostname{true};

    std::string ca_cert_path;
    std::string ca_cert_dir;
    std::string client_cert_path;
    std::string client_key_path;
};

enum class http_error
{
    none = 0,
    connection,
    tls,
    protocol,
    canceled,
    unsupported,
    unknown
};

struct http_response
{
    int          status_code{0};
    std::string  body;
    http_headers headers;

    bool        transport_success{false};
    http_error  error{http_error::none};
    std::string error_message;

    [[nodiscard]] bool ok() const noexcept
    {
        return transport_success && status_code >= 200 && status_code < 300;
    }

    [[nodiscard]] explicit operator bool() const noexcept { return ok(); }
};

enum class backoff_strategy
{
    fixed,
    exponential_jitter
};

struct retry_policy
{
    std::size_t               max_retries{0};
    std::chrono::milliseconds initial_delay{100};
    std::chrono::milliseconds max_delay{3000};
    backoff_strategy          backoff{backoff_strategy::exponential_jitter};

    bool retry_only_if_idempotent{true};

    std::function<bool(const http_response &)> should_retry_fn{
        [](const http_response &res) {
            if(!res.transport_success)
            {
                return true;
            }
            return res.status_code == 500 || res.status_code == 502
                   || res.status_code == 503 || res.status_code == 504;
        }};
};

struct http_request_metrics
{
    http_method               method{http_method::get};
    std::string               url;
    int                       status_code{0};
    std::chrono::microseconds latency{0};
    std::size_t               retry_count{0};
    http_error                error{http_error::none};
    std::string               error_message;
    std::size_t               request_bytes{0};
    std::size_t               response_bytes{0};
};

using logger_callback = std::function<void(const http_request_metrics &)>;

struct http_client_options
{
    http_timeout    timeout{};
    retry_policy    retry{};
    logger_callback logger{nullptr};
};

using query_params = std::unordered_map<std::string, std::string>;

struct http_request
{
    http_method      method{http_method::get};
    std::string_view path{"/"};
    http_headers     headers{};
    std::string_view body{};
    std::string_view content_type{"application/json"};

    query_params                query{};
    std::optional<http_timeout> timeout{std::nullopt};

    std::optional<retry_policy> retry{std::nullopt};
    std::optional<bool>         is_idempotent{std::nullopt};

    [[nodiscard]] bool idempotent() const noexcept
    {
        if(is_idempotent.has_value())
        {
            return is_idempotent.value();
        }
        return method == http_method::get || method == http_method::put
               || method == http_method::del || method == http_method::head
               || method == http_method::options;
    }
};

class http_client
{
  public:
    explicit http_client(const std::string  &base_url,
                         http_client_options options = {})
        : _base_url(base_url)
        , _client(std::make_unique<httplib::Client>(base_url))
        , _options(std::move(options))
    {
        set_client_timeout(_client.get(), _options.timeout);
    }

    explicit http_client(const std::string &base_url,
                         http_timeout       timeout,
                         retry_policy       retry = {})
        : http_client(base_url,
                      http_client_options{timeout, std::move(retry), nullptr})
    {
    }

    ~http_client() = default;

    http_client(const http_client &)            = delete;
    http_client &operator=(const http_client &) = delete;

    http_client(http_client &&) noexcept            = default;
    http_client &operator=(http_client &&) noexcept = default;

    void configure_tls([[maybe_unused]] const tls_config &config)
    {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        _client->enable_server_certificate_verification(
            config.verify_server_certificate);
        _client->enable_server_hostname_verification(config.verify_hostname);

        if(!config.ca_cert_path.empty() || !config.ca_cert_dir.empty())
        {
            const char *ca_path = config.ca_cert_path.empty()
                                      ? nullptr
                                      : config.ca_cert_path.c_str();
            const char *ca_dir  = config.ca_cert_dir.empty()
                                      ? nullptr
                                      : config.ca_cert_dir.c_str();
            _client->set_ca_cert_path(ca_path, ca_dir);
        }

        if(!config.client_cert_path.empty() && !config.client_key_path.empty())
        {
            _client->set_client_cert_path(config.client_cert_path.c_str(),
                                          config.client_key_path.c_str());
        }
#endif
    }

    void configure_proxy(const proxy_config &proxy)
    {
        if(!proxy.valid())
        {
            return;
        }

        _client->set_proxy(proxy.host.c_str(), proxy.port);
        if(!proxy.username.empty())
        {
            _client->set_proxy_basic_auth(proxy.username.c_str(),
                                          proxy.password.c_str());
        }
    }

    void set_retry_policy(retry_policy policy)
    {
        _options.retry = std::move(policy);
    }

    void set_logger_callback(logger_callback cb)
    {
        _options.logger = std::move(cb);
    }

    http_response request(const http_request &req)
    {
        const auto start_time = std::chrono::steady_clock::now();
        const auto policy     = req.retry.value_or(_options.retry);

        std::string   full_path = build_full_path(req.path, req.query);
        std::size_t   attempt   = 0;
        http_response res;

        while(true)
        {
            attempt++;
            res = execute_single_request(req, full_path);

            if(res.ok() || policy.max_retries == 0
               || attempt > policy.max_retries)
            {
                break;
            }

            if(policy.retry_only_if_idempotent && !req.idempotent())
            {
                break;
            }

            if(policy.should_retry_fn && !policy.should_retry_fn(res))
            {
                break;
            }

            sleep_backoff(attempt, policy);
        }

        const auto end_time = std::chrono::steady_clock::now();
        const auto latency =
            std::chrono::duration_cast<std::chrono::microseconds>(end_time
                                                                  - start_time);
        if(_options.logger)
        {
            http_request_metrics metrics;
            metrics.method         = req.method;
            metrics.url            = _base_url + full_path;
            metrics.status_code    = res.status_code;
            metrics.latency        = latency;
            metrics.retry_count    = attempt - 1;
            metrics.error          = res.error;
            metrics.error_message  = res.error_message;
            metrics.request_bytes  = req.body.size();
            metrics.response_bytes = res.body.size();

            _options.logger(metrics);
        }

        return res;
    }

    // Convenience APIs
    http_response get(std::string_view path, const http_headers &headers = {})
    {
        http_request req;
        req.method  = http_method::get;
        req.path    = path;
        req.headers = headers;
        return request(req);
    }

    http_response post(std::string_view    path,
                       std::string_view    body,
                       std::string_view    content_type = "application/json",
                       const http_headers &headers      = {})
    {
        http_request req;
        req.method       = http_method::post;
        req.path         = path;
        req.headers      = headers;
        req.body         = body;
        req.content_type = content_type;
        return request(req);
    }

    http_response put(std::string_view    path,
                      std::string_view    body,
                      std::string_view    content_type = "application/json",
                      const http_headers &headers      = {})
    {
        http_request req;
        req.method       = http_method::put;
        req.path         = path;
        req.headers      = headers;
        req.body         = body;
        req.content_type = content_type;
        return request(req);
    }

    http_response patch(std::string_view    path,
                        std::string_view    body,
                        std::string_view    content_type = "application/json",
                        const http_headers &headers      = {})
    {
        http_request req;
        req.method       = http_method::patch;
        req.path         = path;
        req.headers      = headers;
        req.body         = body;
        req.content_type = content_type;
        return request(req);
    }

    http_response del(std::string_view path, const http_headers &headers = {})
    {
        http_request req;
        req.method  = http_method::del;
        req.path    = path;
        req.headers = headers;
        return request(req);
    }

    http_response head(std::string_view path, const http_headers &headers = {})
    {
        http_request req;
        req.method  = http_method::head;
        req.path    = path;
        req.headers = headers;
        return request(req);
    }

    http_response options(std::string_view    path,
                          const http_headers &headers = {})
    {
        http_request req;
        req.method  = http_method::options;
        req.path    = path;
        req.headers = headers;
        return request(req);
    }

  private:
    http_response execute_single_request(const http_request &req,
                                         const std::string  &full_path)
    {
        auto req_headers = to_httplib_headers(req.headers);

        if(req.timeout.has_value())
        {
            set_client_timeout(_client.get(), req.timeout.value());
        }

        httplib::Result res;
        switch(req.method)
        {
            case http_method::get:
                res = _client->Get(full_path.c_str(), req_headers);
                break;
            case http_method::post:
                res = _client->Post(full_path.c_str(),
                                    req_headers,
                                    req.body.data(),
                                    req.body.size(),
                                    req.content_type.data());
                break;
            case http_method::put:
                res = _client->Put(full_path.c_str(),
                                   req_headers,
                                   req.body.data(),
                                   req.body.size(),
                                   req.content_type.data());
                break;
            case http_method::patch:
                res = _client->Patch(full_path.c_str(),
                                     req_headers,
                                     req.body.data(),
                                     req.body.size(),
                                     req.content_type.data());
                break;
            case http_method::del:
                if(req.body.empty())
                {
                    res = _client->Delete(full_path.c_str(), req_headers);
                } else
                {
                    res = _client->Delete(full_path.c_str(),
                                          req_headers,
                                          req.body.data(),
                                          req.body.size(),
                                          req.content_type.data());
                }
                break;
            case http_method::head:
                res = _client->Head(full_path.c_str(), req_headers);
                break;
            case http_method::options:
                res = _client->Options(full_path.c_str(), req_headers);
                break;
            default:
                break;
        }

        return parse_response(res);
    }

    static void sleep_backoff(std::size_t attempt, const retry_policy &policy)
    {
        if(policy.backoff == backoff_strategy::fixed)
        {
            std::this_thread::sleep_for(policy.initial_delay);
            return;
        }

        double temp = policy.initial_delay.count() * std::pow(2.0, attempt - 1);
        auto   max_calculated =
            std::min(static_cast<double>(policy.max_delay.count()), temp);

        thread_local std::random_device  rd;
        thread_local std::mt19937        gen(rd());
        std::uniform_real_distribution<> dis(0.0, max_calculated);

        auto sleep_ms = static_cast<uint64_t>(dis(gen));
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    static std::string build_full_path(std::string_view    path,
                                       const query_params &query)
    {
        if(query.empty())
        {
            return std::string(path);
        }

        std::string full_path(path);
        full_path += "?";
        bool first = true;
        for(const auto &[k, v] : query)
        {
            if(!first)
            {
                full_path += "&";
            }
            full_path += k + "=" + v;
            first = false;
        }
        return full_path;
    }

    static void set_client_timeout(httplib::Client    *client,
                                   const http_timeout &timeout)
    {
        auto apply_timeout = [](std::chrono::milliseconds           ms,
                                std::function<void(time_t, time_t)> setter) {
            auto sec = std::chrono::duration_cast<std::chrono::seconds>(ms);
            auto usec =
                std::chrono::duration_cast<std::chrono::microseconds>(ms - sec);
            setter(static_cast<time_t>(sec.count()),
                   static_cast<time_t>(usec.count()));
        };

        apply_timeout(timeout.connect, [client](time_t s, time_t us) {
            client->set_connection_timeout(s, us);
        });
        apply_timeout(timeout.read, [client](time_t s, time_t us) {
            client->set_read_timeout(s, us);
        });
        apply_timeout(timeout.write, [client](time_t s, time_t us) {
            client->set_write_timeout(s, us);
        });
    }

    static httplib::Headers to_httplib_headers(const http_headers &headers)
    {
        httplib::Headers h;
        for(const auto &[k, v] : headers)
        {
            h.emplace(k, v);
        }
        return h;
    }

    static http_error to_http_error(httplib::Error err) noexcept
    {
        switch(err)
        {
            case httplib::Error::Success:
                return http_error::none;
            case httplib::Error::Connection:
            case httplib::Error::BindIPAddress:
                return http_error::connection;
            case httplib::Error::Read:
            case httplib::Error::Write:
            case httplib::Error::ExceedRedirectCount:
                return http_error::protocol;
            case httplib::Error::Canceled:
                return http_error::canceled;
            case httplib::Error::SSLConnection:
            case httplib::Error::SSLServerVerification:
                return http_error::tls;
            case httplib::Error::Unknown:
            default:
                return http_error::unknown;
        }
    }

    static http_response parse_response(const httplib::Result &res)
    {
        http_response response;
        if(res)
        {
            response.transport_success = true;
            response.status_code       = res->status;
            response.body              = res->body;
            response.error             = http_error::none;
            for(const auto &header : res->headers)
            {
                response.headers.append(header.first, header.second);
            }
        } else
        {
            response.transport_success = false;
            response.status_code       = 0;
            response.error             = to_http_error(res.error());
            response.error_message     = httplib::to_string(res.error());
        }
        return response;
    }

    std::string                      _base_url;
    std::unique_ptr<httplib::Client> _client;
    http_client_options              _options;
};

} // namespace hj

#endif // HJ_NET_HTTP_CLIENT_HPP