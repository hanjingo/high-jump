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

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <hj/net/http/http_header.hpp>
#include <httplib.h>

namespace hj::http
{

struct http_client_options
{
    http_timeout    timeout{};
    retry_policy    retry{};
    logger_callback logger{nullptr};
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
        _set_client_timeout(_client.get(), _options.timeout);
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

    bool configure_tls([[maybe_unused]] const tls_config &config)
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
        return true;
#else
        return false;
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

    http_client_response request(const http_request &req)
    {
        const auto start_time = std::chrono::steady_clock::now();
        const auto policy     = req.retry.value_or(_options.retry);

        std::string full_path = detail::build_full_path(req.path, req.query);
        std::size_t attempt   = 0;
        http_client_response res;

        while(true)
        {
            attempt++;
            res = _execute(req, full_path);

            if(res.ok() || policy.max_retries == 0
               || attempt > policy.max_retries)
                break;

            if(policy.retry_only_if_idempotent && !req.idempotent())
                break;

            if(policy.should_retry_fn && !policy.should_retry_fn(res))
                break;

            _sleep_backoff(attempt, policy, res);
        }

        const auto end_time = std::chrono::steady_clock::now();
        const auto latency =
            std::chrono::duration_cast<std::chrono::microseconds>(end_time
                                                                  - start_time);
        if(_options.logger)
        {
            http_request_metrics metrics;
            metrics.method              = req.method;
            metrics.url                 = _base_url + full_path;
            metrics.status_code         = res.response.status_code;
            metrics.latency             = latency;
            metrics.retry_count         = attempt - 1;
            metrics.error               = res.error;
            metrics.error_message       = res.error_message;
            metrics.request_body_bytes  = req.body.size();
            metrics.response_body_bytes = res.response.body.size();

            try
            {
                _options.logger(metrics);
            }
            catch(...)
            {
                // Observability best-effort Exception Safety
            }
        }

        return res;
    }

    http_client_response get(std::string_view    path,
                             const http_headers &headers = {})
    {
        http_request req;
        req.method  = http_method::get;
        req.path    = std::string(path);
        req.headers = headers;
        return request(req);
    }

    http_client_response post(std::string_view    path,
                              std::string_view    body,
                              std::string_view    content_type = "text/plain",
                              const http_headers &headers      = {})
    {
        http_request req;
        req.method       = http_method::post;
        req.path         = std::string(path);
        req.headers      = headers;
        req.body         = std::string(body);
        req.content_type = std::string(content_type);
        return request(req);
    }

    http_client_response put(std::string_view    path,
                             std::string_view    body,
                             std::string_view    content_type = {},
                             const http_headers &headers      = {})
    {
        http_request req;
        req.method       = http_method::put;
        req.path         = std::string(path);
        req.headers      = headers;
        req.body         = std::string(body);
        req.content_type = std::string(content_type);
        return request(req);
    }

    http_client_response patch(std::string_view    path,
                               std::string_view    body,
                               std::string_view    content_type = {},
                               const http_headers &headers      = {})
    {
        http_request req;
        req.method       = http_method::patch;
        req.path         = std::string(path);
        req.headers      = headers;
        req.body         = std::string(body);
        req.content_type = std::string(content_type);
        return request(req);
    }

    http_client_response del(std::string_view    path,
                             const http_headers &headers = {})
    {
        http_request req;
        req.method  = http_method::del;
        req.path    = std::string(path);
        req.headers = headers;
        return request(req);
    }

    http_client_response head(std::string_view    path,
                              const http_headers &headers = {})
    {
        http_request req;
        req.method  = http_method::head;
        req.path    = std::string(path);
        req.headers = headers;
        return request(req);
    }

    http_client_response options(std::string_view    path,
                                 const http_headers &headers = {})
    {
        http_request req;
        req.method  = http_method::options;
        req.path    = std::string(path);
        req.headers = headers;
        return request(req);
    }

    http_client_response post_json(std::string_view    path,
                                   std::string_view    json_body,
                                   const http_headers &headers = {})
    {
        return post(path, json_body, "application/json", headers);
    }

    http_client_response post_text(std::string_view    path,
                                   std::string_view    text_body,
                                   const http_headers &headers = {})
    {
        return post(path, text_body, "text/plain", headers);
    }

    http_client_response
    post_binary(std::string_view    path,
                std::string_view    binary_body,
                std::string_view    content_type = "application/octet-stream",
                const http_headers &headers      = {})
    {
        return post(path, binary_body, content_type, headers);
    }

    http_client_response put_json(std::string_view    path,
                                  std::string_view    json_body,
                                  const http_headers &headers = {})
    {
        return put(path, json_body, "application/json", headers);
    }

  private:
    http_client_response _execute(const http_request &req,
                                  const std::string  &full_path)
    {
        auto req_headers = detail::to_httplib_headers(req.headers);

        const char *c_type =
            req.content_type.empty() ? "text/plain" : req.content_type.c_str();

        auto timeout =
            req.timeout.has_value() ? req.timeout.value() : _options.timeout;
        _set_client_timeout(_client.get(), timeout);

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
                                    c_type);
                break;
            case http_method::put:
                res = _client->Put(full_path.c_str(),
                                   req_headers,
                                   req.body.data(),
                                   req.body.size(),
                                   c_type);
                break;
            case http_method::patch:
                res = _client->Patch(full_path.c_str(),
                                     req_headers,
                                     req.body.data(),
                                     req.body.size(),
                                     c_type);
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
                                          c_type);
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

        if(req.timeout.has_value())
            _set_client_timeout(_client.get(), _options.timeout);

        return detail::parse_response(res);
    }

    static std::optional<std::chrono::milliseconds>
    _parse_retry_after(const http_headers &headers)
    {
        std::string retry_after_str = headers.get("Retry-After");
        if(retry_after_str.empty())
        {
            return std::nullopt;
        }

        try
        {
            std::size_t pos     = 0;
            long long   seconds = std::stoll(retry_after_str, &pos);
            if(pos == retry_after_str.size() && seconds >= 0)
            {
                return std::chrono::milliseconds(seconds * 1000);
            }
        }
        catch(...)
        {
        }

        return std::nullopt;
    }

    static void _sleep_backoff(std::size_t                 attempt,
                               const retry_policy         &policy,
                               const http_client_response &res)
    {
        if(policy.respect_retry_after && res.transport_success)
        {
            auto retry_after_ms = _parse_retry_after(res.response.headers);
            if(retry_after_ms.has_value())
            {
                auto delay = std::min(retry_after_ms.value(), policy.max_delay);
                std::this_thread::sleep_for(delay);
                return;
            }
        }

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

    static void _set_client_timeout(httplib::Client    *client,
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

    std::string                      _base_url;
    std::unique_ptr<httplib::Client> _client;
    http_client_options              _options;
};

} // namespace hj::http

#endif // HJ_NET_HTTP_CLIENT_HPP