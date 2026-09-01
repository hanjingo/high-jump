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

#include <exception>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>

namespace hj::http
{

/**
 * @brief HTTP Server wrapper based on cpp-httplib.
 * 
 * @note Lifecycle & Thread Safety Policy:
 * Server configuration (registering routes via get(), post(), route(), 
 * and setting handlers like set_exception_handler(), set_metrics_handler()) 
 * is NOT thread-safe. All configurations MUST be completed prior to calling 
 * listen() or listen_async().
 * 
 * Once the server enters the RUNNING state, route mutation and configuration 
 * modification are strictly prohibited to prevent data races and undefined behavior.
 * 
 * Lifecycle State Machine:
 *   [ CONFIGURATION ] -> ( listen() / listen_async() ) -> [ RUNNING ]
 */
class http_server
{
  private:
    struct route_entry
    {
        http_handler get_handler;
        http_handler head_handler;
    };

    using route_ptr = std::shared_ptr<route_entry>;

  public:
    http_server()
        : _server(std::make_unique<httplib::Server>())
        , _exception_handler(_default_exception_handler)
    {
    }

    ~http_server() { stop(); }

    http_server(const http_server &)            = delete;
    http_server &operator=(const http_server &) = delete;

    http_server(http_server &&other)
    {
        if(other.is_running())
            throw std::logic_error(
                "Cannot move http_server while it is running");

        _server            = std::move(other._server);
        _worker_thread     = std::move(other._worker_thread);
        _exception_handler = std::move(other._exception_handler);
        _metrics_handler   = std::move(other._metrics_handler);
        _routes            = std::move(other._routes);
    }

    http_server &operator=(http_server &&other)
    {
        if(this != &other)
        {
            stop();
            if(other.is_running())
                throw std::logic_error(
                    "Cannot move http_server while source is running");

            _server            = std::move(other._server);
            _worker_thread     = std::move(other._worker_thread);
            _exception_handler = std::move(other._exception_handler);
            _metrics_handler   = std::move(other._metrics_handler);
            _routes            = std::move(other._routes);
        }

        return *this;
    }

    http_server &set_exception_handler(exception_handler handler)
    {
        if(is_running())
        {
            throw std::logic_error(
                "Cannot change exception_handler while server is running");
        }

        if(handler)
        {
            _exception_handler = std::move(handler);
        }

        return *this;
    }

    http_server &set_metrics_handler(server_metrics_handler handler)
    {
        if(handler)
        {
            _metrics_handler = std::move(handler);
        }

        return *this;
    }

    http_server &
    route(http_method m, std::string_view pattern, http_handler handler)
    {
        if(is_running())
        {
            throw std::logic_error(
                "Cannot mutate routes while server is running");
        }

        if(pattern.empty() || !handler)
        {
            return *this;
        }

        const std::string pattern_str(pattern);

        switch(m)
        {
            case http_method::get:
                _register_get_handler(pattern_str, std::move(handler));
                break;

            case http_method::head:
                _register_head_handler(pattern_str, std::move(handler));
                break;

            case http_method::post:
                _server->Post(
                    pattern_str,
                    _make_adapter(http_method::post, std::move(handler)));
                break;

            case http_method::put:
                _server->Put(
                    pattern_str,
                    _make_adapter(http_method::put, std::move(handler)));
                break;

            case http_method::patch:
                _server->Patch(
                    pattern_str,
                    _make_adapter(http_method::patch, std::move(handler)));
                break;

            case http_method::del:
                _server->Delete(
                    pattern_str,
                    _make_adapter(http_method::del, std::move(handler)));
                break;

            case http_method::options:
                _server->Options(
                    pattern_str,
                    _make_adapter(http_method::options, std::move(handler)));
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
        return _server ? _server->listen(host, port) : false;
    }

    std::future<bool> listen_async(const std::string &host, int port)
    {
        stop();

        auto promise = std::make_shared<std::promise<bool>>();
        auto fut     = promise->get_future();

        _worker_thread = std::thread([this, host, port, promise]() {
            bool ret = _server ? _server->listen(host, port) : false;

            try
            {
                promise->set_value(ret);
            }
            catch(const std::future_error &)
            {
                // ignore
            }
        });

        return fut;
    }

    void stop()
    {
        if(_server)
        {
            _server->stop();
        }

        if(_worker_thread.joinable())
        {
            _worker_thread.join();
        }
    }

    bool is_running() const { return _server ? _server->is_running() : false; }

    httplib::Server &native_handle() noexcept { return *_server; }

    const httplib::Server &native_handle() const noexcept { return *_server; }

  private:
    static void _default_exception_handler(const http_request &,
                                           http_response     &resp,
                                           std::exception_ptr ep)
    {
        resp.status_code = 500;
        resp.headers.set("Content-Type", "application/json");

        resp.body = R"({"error": "Internal Server Error"})";
    }

    route_ptr _get_or_create_route(const std::string &pattern)
    {
        auto it = _routes.find(pattern);

        if(it != _routes.end())
        {
            return it->second;
        }

        auto entry = std::make_shared<route_entry>();
        _routes.emplace(pattern, entry);

        return entry;
    }

    void _register_get_handler(const std::string &pattern, http_handler handler)
    {
        auto entry = _get_or_create_route(pattern);

        const bool first_registration =
            !entry->get_handler && !entry->head_handler;

        entry->get_handler = std::move(handler);

        if(first_registration)
        {
            _server->Get(pattern, _make_get_head_adapter(entry));
        }
    }

    void _register_head_handler(const std::string &pattern,
                                http_handler       handler)
    {
        auto entry = _get_or_create_route(pattern);

        const bool first_registration =
            !entry->get_handler && !entry->head_handler;

        entry->head_handler = std::move(handler);

        if(first_registration)
        {
            _server->Get(pattern, _make_get_head_adapter(entry));
        }
    }

    auto _make_get_head_adapter(const route_ptr &entry)
    {
        return [this, entry](const httplib::Request &raw_req,
                             httplib::Response      &raw_resp) {
            http_request req = detail::parse_httplib_request(raw_req);

            http_handler *handler = nullptr;

            if(req.method == http_method::head)
            {
                if(entry->head_handler)
                    handler = &entry->head_handler;
                else if(entry->get_handler)
                    handler = &entry->get_handler;
            } else if(req.method == http_method::get)
            {
                if(entry->get_handler)
                    handler = &entry->get_handler;
            }

            if(!handler || !(*handler))
            {
                raw_resp.status = 405;
                raw_resp.set_header("Allow", "GET, HEAD");
                return;
            }

            _invoke_handler(req, raw_resp, *handler);
        };
    }

    auto _make_adapter(http_method expected_method, http_handler handler)
    {
        return [this, expected_method, handler = std::move(handler)](
                   const httplib::Request &raw_req,
                   httplib::Response      &raw_resp) {
            http_request req = detail::parse_httplib_request(raw_req);

            if(req.method != expected_method)
            {
                raw_resp.status = 405;
                raw_resp.set_header(
                    "Allow",
                    std::string(detail::to_string(expected_method)));
                return;
            }

            _invoke_handler(req, raw_resp, handler);
        };
    }

    void _invoke_handler(const http_request &req,
                         httplib::Response  &raw_resp,
                         const http_handler &handler)
    {
        auto          start_time = std::chrono::steady_clock::now();
        http_response resp;
        resp.status_code = 200;
        try
        {
            handler(req, resp);
        }
        catch(...)
        {
            resp.status_code = 500;
            if(_exception_handler)
            {
                try
                {
                    _exception_handler(req, resp, std::current_exception());
                }
                catch(...)
                {
                    _default_exception_handler(req,
                                               resp,
                                               std::current_exception());
                }
            } else
            {
                _default_exception_handler(req, resp, std::current_exception());
            }
        }

        detail::apply_response(resp, raw_resp);

        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time);

        if(_metrics_handler)
        {
            http_server_metrics metrics;
            metrics.method = req.method;
            metrics.path   = req.path;
            metrics.status_code =
                resp.status_code != 0 ? resp.status_code : 200;
            metrics.latency             = duration;
            metrics.request_body_bytes  = req.body.size();
            metrics.response_body_bytes = resp.body.size();
            metrics.client_ip           = req.client_ip;

            try
            {
                _metrics_handler(metrics);
            }
            catch(...)
            {
            }
        }
    }

  private:
    std::unique_ptr<httplib::Server> _server;

    std::thread _worker_thread;

    exception_handler      _exception_handler;
    server_metrics_handler _metrics_handler;

    std::unordered_map<std::string, route_ptr> _routes;
};


#ifdef CPPHTTPLIB_OPENSSL_SUPPORT

/**
 * @brief HTTP Server wrapper based on cpp-httplib.
 * 
 * @note Lifecycle & Thread Safety Policy:
 * Server configuration (registering routes via get(), post(), route(), 
 * and setting handlers like set_exception_handler(), set_metrics_handler()) 
 * is NOT thread-safe. All configurations MUST be completed prior to calling 
 * listen() or listen_async().
 * 
 * Once the server enters the RUNNING state, route mutation and configuration 
 * modification are strictly prohibited to prevent data races and undefined behavior.
 * 
 * Lifecycle State Machine:
 *   [ CONFIGURATION ] -> ( listen() / listen_async() ) -> [ RUNNING ]
 */
class http_ssl_server
{
  private:
    struct route_entry
    {
        http_handler get_handler;
        http_handler head_handler;
    };

    using route_ptr = std::shared_ptr<route_entry>;

  public:
    http_ssl_server(const char *cert_path, const char *private_key_path)
        : _server(
              std::make_unique<httplib::SSLServer>(cert_path, private_key_path))
        , _exception_handler(_default_exception_handler)
    {
        if(!_server || !_server->is_valid())
        {
            throw std::runtime_error("Failed to initialize SSLServer: invalid "
                                     "certificate or private key ("
                                     + std::string(cert_path) + ", "
                                     + std::string(private_key_path) + ")");
        }
    }

    explicit http_ssl_server(const ssl_config &cfg)
        : _server(std::make_unique<httplib::SSLServer>(
              cfg.cert_path.c_str(), cfg.private_key_path.c_str()))
        , _exception_handler(_default_exception_handler)
    {
        if(!_server || !_server->is_valid())
        {
            throw std::runtime_error("Failed to initialize SSLServer: invalid "
                                     "cert_path or private_key_path");
        }

        _apply_ssl_config(cfg);
    }

    ~http_ssl_server() { stop(); }

    http_ssl_server(const http_ssl_server &)            = delete;
    http_ssl_server &operator=(const http_ssl_server &) = delete;

    http_ssl_server(http_ssl_server &&other)
    {
        if(other.is_running())
            throw std::logic_error(
                "Cannot move http_ssl_server while it is running");

        _server            = std::move(other._server);
        _worker_thread     = std::move(other._worker_thread);
        _exception_handler = std::move(other._exception_handler);
        _metrics_handler   = std::move(other._metrics_handler);
        _routes            = std::move(other._routes);
    }

    http_ssl_server &operator=(http_ssl_server &&other)
    {
        if(this != &other)
        {
            stop();

            if(other.is_running())
                throw std::logic_error(
                    "Cannot move http_ssl_server while source is running");

            _server            = std::move(other._server);
            _worker_thread     = std::move(other._worker_thread);
            _exception_handler = std::move(other._exception_handler);
            _metrics_handler   = std::move(other._metrics_handler);
            _routes            = std::move(other._routes);
        }

        return *this;
    }

    http_ssl_server &set_exception_handler(exception_handler handler)
    {
        if(is_running())
            throw std::logic_error(
                "Cannot change exception_handler while server is running");

        if(handler)
            _exception_handler = std::move(handler);

        return *this;
    }

    http_ssl_server &set_metrics_handler(server_metrics_handler handler)
    {
        if(handler)
            _metrics_handler = std::move(handler);

        return *this;
    }

    http_ssl_server &
    route(http_method m, std::string_view pattern, http_handler handler)
    {
        if(is_running())
            throw std::logic_error(
                "Cannot mutate routes while server is running");

        if(pattern.empty() || !handler)
            return *this;

        const std::string pattern_str(pattern);
        switch(m)
        {
            case http_method::get:
                _register_get_handler(pattern_str, std::move(handler));
                break;

            case http_method::head:
                _register_head_handler(pattern_str, std::move(handler));
                break;

            case http_method::post:
                _server->Post(
                    pattern_str,
                    _make_adapter(http_method::post, std::move(handler)));
                break;

            case http_method::put:
                _server->Put(
                    pattern_str,
                    _make_adapter(http_method::put, std::move(handler)));
                break;

            case http_method::patch:
                _server->Patch(
                    pattern_str,
                    _make_adapter(http_method::patch, std::move(handler)));
                break;

            case http_method::del:
                _server->Delete(
                    pattern_str,
                    _make_adapter(http_method::del, std::move(handler)));
                break;

            case http_method::options:
                _server->Options(
                    pattern_str,
                    _make_adapter(http_method::options, std::move(handler)));
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
        return _server ? _server->listen(host, port) : false;
    }

    std::future<bool> listen_async(const std::string &host, int port)
    {
        stop();

        auto promise = std::make_shared<std::promise<bool>>();
        auto fut     = promise->get_future();

        _worker_thread = std::thread([this, host, port, promise]() {
            bool ret = _server ? _server->listen(host, port) : false;

            try
            {
                promise->set_value(ret);
            }
            catch(const std::future_error &)
            {
            }
        });

        return fut;
    }

    void stop()
    {
        if(_server)
            _server->stop();

        if(_worker_thread.joinable())
            _worker_thread.join();
    }

    bool is_running() const { return _server ? _server->is_running() : false; }

    httplib::SSLServer &native_handle() noexcept { return *_server; }

    const httplib::SSLServer &native_handle() const noexcept
    {
        return *_server;
    }

    SSL_CTX *ssl_context() noexcept
    {
        return _server ? _server->ssl_context() : nullptr;
    }

  private:
    static void _default_exception_handler(const http_request &,
                                           http_response     &resp,
                                           std::exception_ptr ep)
    {
        resp.status_code = 500;
        resp.headers.set("Content-Type", "application/json");
        resp.body = R"({"error": "Internal Server Error"})";
    }

    route_ptr _get_or_create_route(const std::string &pattern)
    {
        auto it = _routes.find(pattern);

        if(it != _routes.end())
            return it->second;

        auto entry = std::make_shared<route_entry>();
        _routes.emplace(pattern, entry);
        return entry;
    }

    void _register_get_handler(const std::string &pattern, http_handler handler)
    {
        auto       entry = _get_or_create_route(pattern);
        const bool first_registration =
            !entry->get_handler && !entry->head_handler;
        entry->get_handler = std::move(handler);
        if(first_registration)
            _server->Get(pattern, _make_get_head_adapter(entry));
    }

    void _register_head_handler(const std::string &pattern,
                                http_handler       handler)
    {
        auto entry = _get_or_create_route(pattern);

        const bool first_registration =
            !entry->get_handler && !entry->head_handler;

        entry->head_handler = std::move(handler);

        if(first_registration)
            _server->Get(pattern, _make_get_head_adapter(entry));
    }

    auto _make_get_head_adapter(const route_ptr &entry)
    {
        return [this, entry](const httplib::Request &raw_req,
                             httplib::Response      &raw_resp) {
            http_request req = detail::parse_httplib_request(raw_req);

            http_handler *handler = nullptr;

            if(req.method == http_method::head)
            {
                if(entry->head_handler)
                    handler = &entry->head_handler;
                else if(entry->get_handler)
                    handler = &entry->get_handler;
            } else if(req.method == http_method::get)
            {
                if(entry->get_handler)
                    handler = &entry->get_handler;
            }

            if(!handler || !(*handler))
            {
                raw_resp.status = 405;
                raw_resp.set_header("Allow", "GET, HEAD");
                return;
            }

            _invoke_handler(req, raw_resp, *handler);
        };
    }

    httplib::SSLServer::Handler _make_adapter(http_method  expected_method,
                                              http_handler handler)
    {
        return [this, expected_method, handler = std::move(handler)](
                   const httplib::Request &raw_req,
                   httplib::Response      &raw_resp) {
            http_request req = detail::parse_httplib_request(raw_req);

            if(req.method != expected_method)
            {
                raw_resp.status = 405;
                raw_resp.set_header(
                    "Allow",
                    std::string(detail::to_string(expected_method)));
                return;
            }

            _invoke_handler(req, raw_resp, handler);
        };
    }

    void _invoke_handler(const http_request &req,
                         httplib::Response  &raw_resp,
                         const http_handler &handler)
    {
        auto          start_time = std::chrono::steady_clock::now();
        http_response resp;
        resp.status_code = 200;
        try
        {
            handler(req, resp);
        }
        catch(...)
        {
            resp.status_code = 500;
            if(_exception_handler)
            {
                try
                {
                    _exception_handler(req, resp, std::current_exception());
                }
                catch(...)
                {
                    _default_exception_handler(req,
                                               resp,
                                               std::current_exception());
                }
            } else
            {
                _default_exception_handler(req, resp, std::current_exception());
            }
        }

        detail::apply_response(resp, raw_resp);

        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time);

        if(_metrics_handler)
        {
            http_server_metrics metrics;
            metrics.method = req.method;
            metrics.path   = req.path;
            metrics.status_code =
                resp.status_code != 0 ? resp.status_code : 200;
            metrics.latency             = duration;
            metrics.request_body_bytes  = req.body.size();
            metrics.response_body_bytes = resp.body.size();
            metrics.client_ip           = req.client_ip;
            try
            {
                _metrics_handler(metrics);
            }
            catch(...)
            {
            }
        }
    }

    static std::string _get_openssl_error_string()
    {
        unsigned long err = ERR_get_error();
        if(err == 0)
            return "Unknown OpenSSL error";
        char buf[256];
        ERR_error_string_n(err, buf, sizeof(buf));
        return std::string(buf);
    }

    void _apply_ssl_config(const ssl_config &cfg)
    {
        if(!_server)
            throw std::runtime_error("SSLServer instance is null");

        SSL_CTX *ctx = _server->ssl_context();
        if(!ctx)
            throw std::runtime_error(
                "Failed to acquire SSL_CTX from SSLServer");

        int min_ver = (cfg.min_version == tls_version::tls_1_3)
                          ? TLS1_3_VERSION
                          : TLS1_2_VERSION;
        if(SSL_CTX_set_min_proto_version(ctx, min_ver) != 1)
            throw std::runtime_error("Failed to set minimum TLS version: "
                                     + _get_openssl_error_string());

        if(!cfg.tls12_cipher_suites.empty())
        {
            if(SSL_CTX_set_cipher_list(ctx, cfg.tls12_cipher_suites.c_str())
               != 1)
                throw std::runtime_error("Failed to set TLS 1.2 cipher list '"
                                         + cfg.tls12_cipher_suites
                                         + "': " + _get_openssl_error_string());
        }

        if(!cfg.tls13_cipher_suites.empty())
        {
            if(SSL_CTX_set_ciphersuites(ctx, cfg.tls13_cipher_suites.c_str())
               != 1)
                throw std::runtime_error("Failed to set TLS 1.3 ciphersuites '"
                                         + cfg.tls13_cipher_suites
                                         + "': " + _get_openssl_error_string());
        }

        if(cfg.client_auth)
        {
            SSL_CTX_set_verify(ctx,
                               SSL_VERIFY_PEER
                                   | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                               nullptr);

            if(!cfg.ca_cert_path.empty() || !cfg.ca_cert_dir.empty())
            {
                const char *ca_file = cfg.ca_cert_path.empty()
                                          ? nullptr
                                          : cfg.ca_cert_path.c_str();
                const char *ca_dir =
                    cfg.ca_cert_dir.empty() ? nullptr : cfg.ca_cert_dir.c_str();

                if(SSL_CTX_load_verify_locations(ctx, ca_file, ca_dir) != 1)
                {
                    throw std::runtime_error(
                        "Failed to load CA verify locations (file: "
                        + cfg.ca_cert_path + ", dir: " + cfg.ca_cert_dir
                        + "): " + _get_openssl_error_string());
                }
            } else
            {
                throw std::runtime_error(
                    "Client authentication is enabled (client_auth=true), "
                    "but neither ca_cert_path nor ca_cert_dir is specified");
            }
        }

        if(cfg.enable_session_cache)
        {
            SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_SERVER);
            SSL_CTX_set_timeout(ctx, cfg.session_timeout_sec);
        }

        if(cfg.ssl_ctx_callback)
            cfg.ssl_ctx_callback(ctx);
    }

  private:
    std::unique_ptr<httplib::SSLServer> _server;

    std::thread _worker_thread;

    exception_handler      _exception_handler;
    server_metrics_handler _metrics_handler;

    std::unordered_map<std::string, route_ptr> _routes;
};

#endif // CPPHTTPLIB_OPENSSL_SUPPORT

} // namespace hj::http

#endif // HTTP_SERVER_HPP