/*
 * This file is part of high-jump(hj).
 *
 * Copyright 2025 hanjingo <hehehunanchina@live.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HTTP_HEADER_HPP
#define HTTP_HEADER_HPP

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <functional>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <httplib.h>

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
#include <openssl/ssl.h>
#include <openssl/err.h>
#else
typedef struct ssl_ctx_st SSL_CTX;
#endif

namespace hj::http
{

enum class tls_version
{
    tls_1_2,
    tls_1_3
};

enum class method
{
    get,
    post,
    put,
    patch,
    del,
    head,
    options
};

enum class error
{
    none = 0,
    connection,
    tls,
    protocol,
    canceled,
    unsupported,
    unknown
};

enum class backoff_strategy
{
    fixed,
    exponential_jitter
};

struct case_insensitive_hash;
struct case_insensitive_equal;
struct timeout;
struct proxy_config;
struct tls_config;
struct ssl_config;
struct request_metrics;
struct server_metrics;
class headers;
struct response;
struct retry_policy;
struct request;

using raw_err      = httplib::Error;
using raw_result   = httplib::Result;
using raw_request  = httplib::Request;
using raw_response = httplib::Response;

using logger_callback = std::function<void(const request_metrics &)>;
using query_params    = std::vector<std::pair<std::string, std::string>>;
using handler         = std::function<void(const request &, response &)>;

using exception_handler =
    std::function<void(const request &, response &, std::exception_ptr)>;
using server_metrics_handler = std::function<void(const server_metrics &)>;

struct case_insensitive_hash
{
    using is_transparent = void;

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

    std::size_t operator()(const std::string &key) const noexcept
    {
        return operator()(std::string_view{key});
    }
};

struct case_insensitive_equal
{
    using is_transparent = void;

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

struct timeout
{
    std::chrono::milliseconds connect{5000};
    std::chrono::milliseconds read{5000};
    std::chrono::milliseconds write{5000};

    timeout(std::chrono::milliseconds timeout)
        : connect(timeout)
        , read(timeout)
        , write(timeout)
    {
    }

    timeout(std::chrono::milliseconds conn_ms,
            std::chrono::milliseconds read_ms,
            std::chrono::milliseconds write_ms)
        : connect(conn_ms)
        , read(read_ms)
        , write(write_ms)
    {
    }

    timeout() = default;
};

struct proxy_config
{
    std::string host;
    int         port{0};
    std::string username;
    std::string password;

    [[nodiscard]] bool valid() const noexcept
    {
        return !host.empty() && port > 0 && port <= 65535;
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

struct ssl_config
{
    std::string cert_path;
    std::string private_key_path;

    bool        client_auth{false};
    std::string ca_cert_path;
    std::string ca_cert_dir;

    tls_version min_version{tls_version::tls_1_2};

    std::string tls12_cipher_suites;
    std::string tls13_cipher_suites;

    bool enable_session_cache{true};
    long session_timeout_sec{7200};

    std::function<void(SSL_CTX *)> ssl_ctx_callback;
};

struct request_metrics
{
    method                    method{method::get};
    std::string               url;
    int                       status_code{0};
    std::chrono::microseconds latency{0};
    std::size_t               retry_count{0};
    error                     error{error::none};
    std::string               error_message;
    std::size_t               request_body_bytes{0};
    std::size_t               response_body_bytes{0};
};

struct server_metrics
{
    method                    method{method::get};
    std::string               path;
    int                       status_code{200};
    std::chrono::microseconds latency{0};
    std::size_t               request_body_bytes{0};
    std::size_t               response_body_bytes{0};
    std::string               client_ip;
};

class headers
{
  public:
    using container_type = std::unordered_multimap<std::string,
                                                   std::string,
                                                   case_insensitive_hash,
                                                   case_insensitive_equal>;

    headers() = default;

    headers(std::initializer_list<std::pair<std::string, std::string>> init)
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
        auto range = _headers.equal_range(std::string(key));
        _headers.erase(range.first, range.second);
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

struct response
{
    int         status_code{200};
    std::string body;
    headers     headers;

    bool        transport_success{false};
    error       error{error::none};
    std::string error_message;

    [[nodiscard]] bool ok() const noexcept
    {
        return transport_success && status_code >= 200 && status_code < 300;
    }

    [[nodiscard]] explicit operator bool() const noexcept { return ok(); }
};

struct retry_policy
{
    std::size_t               max_retries{0};
    std::chrono::milliseconds initial_delay{100};
    std::chrono::milliseconds max_delay{3000};
    backoff_strategy          backoff{backoff_strategy::exponential_jitter};

    bool retry_only_if_idempotent{true};
    bool respect_retry_after{true};

    std::function<bool(const response &)> should_retry_fn{
        [](const response &res) {
            if(!res.transport_success)
            {
                return res.error == error::connection
                       || res.error == error::protocol;
            }

            return res.status_code == 429 || res.status_code == 500
                   || res.status_code == 502 || res.status_code == 503
                   || res.status_code == 504;
        }};
};

struct request
{
    method      method{method::get};
    std::string path{"/"};
    headers     headers{};
    std::string body{};
    std::string content_type{};
    std::string client_ip{};

    query_params           query{};
    std::optional<timeout> timeout{std::nullopt};

    std::optional<retry_policy> retry{std::nullopt};
    std::optional<bool>         is_idempotent{std::nullopt};

    [[nodiscard]] bool idempotent() const noexcept
    {
        if(is_idempotent.has_value())
        {
            return is_idempotent.value();
        }

        if(headers.contains("Idempotency-Key")
           || headers.contains("X-Request-ID"))
        {
            return true;
        }

        return method == method::get || method == method::put
               || method == method::del || method == method::head
               || method == method::options;
    }
};

namespace detail
{
inline std::string url_encode(std::string_view value)
{
    static constexpr char hex_digits[] = "0123456789ABCDEF";
    std::string           escaped;
    escaped.reserve(value.size() * 3);

    for(char ch : value)
    {
        auto c = static_cast<unsigned char>(ch);
        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
           || (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_'
           || c == '~')
        {
            escaped += static_cast<char>(c);
        } else
        {
            escaped += '%';
            escaped += hex_digits[(c >> 4) & 0x0F];
            escaped += hex_digits[c & 0x0F];
        }
    }
    return escaped;
}

inline std::string build_full_path(std::string_view    path,
                                   const query_params &query,
                                   bool                sort_query = true)
{
    if(query.empty())
        return std::string(path);

    std::string full_path(path);
    full_path += "?";
    bool first = true;

    auto append_param = [&](const std::string &k, const std::string &v) {
        if(!first)
            full_path += "&";

        full_path += url_encode(k) + "=" + url_encode(v);
        first = false;
    };

    if(sort_query)
    {
        query_params sorted_query = query;
        std::sort(sorted_query.begin(),
                  sorted_query.end(),
                  [](const auto &a, const auto &b) {
                      if(a.first != b.first)
                          return a.first < b.first;

                      return a.second < b.second;
                  });
        for(const auto &[k, v] : sorted_query)
            append_param(k, v);
    } else
    {
        for(const auto &[k, v] : query)
            append_param(k, v);
    }

    return full_path;
}

[[nodiscard]] inline std::string_view to_string(method m) noexcept
{
    switch(m)
    {
        case method::get:
            return "GET";
        case method::post:
            return "POST";
        case method::put:
            return "PUT";
        case method::patch:
            return "PATCH";
        case method::del:
            return "DELETE";
        case method::head:
            return "HEAD";
        case method::options:
            return "OPTIONS";
    }
    return "UNKNOWN";
}

inline request parse_httplib_request(const raw_request &raw_req)
{
    request req;
    req.path      = raw_req.path;
    req.body      = raw_req.body;
    req.client_ip = raw_req.remote_addr;

    if(raw_req.method == "GET")
        req.method = method::get;
    else if(raw_req.method == "POST")
        req.method = method::post;
    else if(raw_req.method == "PUT")
        req.method = method::put;
    else if(raw_req.method == "PATCH")
        req.method = method::patch;
    else if(raw_req.method == "DELETE")
        req.method = method::del;
    else if(raw_req.method == "HEAD")
        req.method = method::head;
    else if(raw_req.method == "OPTIONS")
        req.method = method::options;

    for(const auto &[k, v] : raw_req.headers)
    {
        req.headers.append(k, v);
        if(case_insensitive_equal{}(k, "Content-Type"))
            req.content_type = v;
    }

    for(const auto &[k, v] : raw_req.params)
        req.query.emplace_back(k, v);

    return req;
}

static error to_error(raw_err err) noexcept
{
    switch(err)
    {
        case httplib::Error::Success:
            return error::none;

        case httplib::Error::Connection:
        case httplib::Error::BindIPAddress:
        case httplib::Error::ConnectionTimeout:
        case httplib::Error::ProxyConnection:
        case httplib::Error::ConnectionClosed:
        case httplib::Error::Timeout:
        case httplib::Error::UnsupportedAddressFamily:
            return error::connection;

        case httplib::Error::Read:
        case httplib::Error::Write:
        case httplib::Error::ExceedRedirectCount:
        case httplib::Error::Compression:
        case httplib::Error::ResourceExhaustion:
        case httplib::Error::TooManyFormDataFiles:
        case httplib::Error::ExceedMaxPayloadSize:
        case httplib::Error::ExceedUriMaxLength:
        case httplib::Error::ExceedMaxSocketDescriptorCount:
        case httplib::Error::InvalidRequestLine:
        case httplib::Error::InvalidHTTPMethod:
        case httplib::Error::InvalidHTTPVersion:
        case httplib::Error::InvalidHeaders:
        case httplib::Error::MultipartParsing:
        case httplib::Error::HTTPParsing:
        case httplib::Error::InvalidRangeHeader:
        case httplib::Error::UnsupportedMultipartBoundaryChars:
            return error::protocol;

        case httplib::Error::Canceled:
            return error::canceled;

        case httplib::Error::SSLConnection:
        case httplib::Error::SSLLoadingCerts:
        case httplib::Error::SSLServerVerification:
        case httplib::Error::SSLServerHostnameVerification:
        case httplib::Error::SSLPeerCouldBeClosed_:
            return error::tls;

        case httplib::Error::Unknown:
        default:
            return error::unknown;
    }
}

static response parse_response(const raw_result &res)
{
    response client_resp;
    if(res)
    {
        client_resp.transport_success = true;
        client_resp.status_code       = res->status;
        client_resp.body              = res->body;
        client_resp.error             = error::none;
        for(const auto &header : res->headers)
            client_resp.headers.append(header.first, header.second);
    } else
    {
        client_resp.transport_success = false;
        client_resp.status_code       = 0;
        client_resp.error             = to_error(res.error());
        client_resp.error_message     = httplib::to_string(res.error());
    }
    return client_resp;
}

inline void apply_response(const response &resp, raw_response &raw_resp)
{
    raw_resp.status = resp.status_code != 0 ? resp.status_code : 200;

    std::string content_type = resp.headers.get("Content-Type");
    if(content_type.empty())
        content_type = "text/plain";

    raw_resp.set_content(resp.body, content_type.c_str());
    for(const auto &[k, v] : resp.headers)
    {
        if(!case_insensitive_equal{}(k, "Content-Type"))
            raw_resp.set_header(k.c_str(), v.c_str());
    }
}

static httplib::Headers to_httplib_headers(const headers &headers)
{
    httplib::Headers h;
    for(const auto &[k, v] : headers)
        h.emplace(k, v);

    return h;
}

} // namespace detail
} // namespace hj::http

#endif // HTTP_HEADER_HPP