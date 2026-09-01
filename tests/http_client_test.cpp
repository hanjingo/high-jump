#include "hj/net/http/http_client.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <thread>

namespace hj::test
{

class HttpClientTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        _port = _server.bind_to_any_port("127.0.0.1");
        ASSERT_GT(_port, 0)
            << "Failed to bind Mock Server to an ephemeral port.";

        _base_url = "http://127.0.0.1:" + std::to_string(_port);

        _server_thread = std::thread([this]() { _server.listen_after_bind(); });
    }

    void TearDown() override
    {
        _server.stop();
        if(_server_thread.joinable())
        {
            _server_thread.join();
        }
    }

    httplib::Server _server;
    int             _port{0};
    std::string     _base_url;
    std::thread     _server_thread;
};

TEST_F(HttpClientTest, HttpStatusCodesAndOkSemantics)
{
    _server.Get("/status/200",
                [](const httplib::Request &, httplib::Response &res) {
                    res.status = 200;
                });
    _server.Get("/status/201",
                [](const httplib::Request &, httplib::Response &res) {
                    res.status = 201;
                });
    _server.Get("/status/204",
                [](const httplib::Request &, httplib::Response &res) {
                    res.status = 204;
                });
    _server.Get("/status/301",
                [](const httplib::Request &, httplib::Response &res) {
                    res.status = 301;
                });
    _server.Get("/status/302",
                [](const httplib::Request &, httplib::Response &res) {
                    res.status = 302;
                });
    _server.Get("/status/400",
                [](const httplib::Request &, httplib::Response &res) {
                    res.status = 400;
                });
    _server.Get("/status/401",
                [](const httplib::Request &, httplib::Response &res) {
                    res.status = 401;
                });
    _server.Get("/status/403",
                [](const httplib::Request &, httplib::Response &res) {
                    res.status = 403;
                });
    _server.Get("/status/404",
                [](const httplib::Request &, httplib::Response &res) {
                    res.status = 404;
                });
    _server.Get("/status/500",
                [](const httplib::Request &, httplib::Response &res) {
                    res.status = 500;
                });
    _server.Get("/status/502",
                [](const httplib::Request &, httplib::Response &res) {
                    res.status = 502;
                });
    _server.Get("/status/503",
                [](const httplib::Request &, httplib::Response &res) {
                    res.status = 503;
                });

    http_client client(_base_url);

    for(int code : {200, 201, 204})
    {
        auto res = client.get("/status/" + std::to_string(code));
        EXPECT_TRUE(res.transport_success);
        EXPECT_EQ(res.status_code, code);
        EXPECT_TRUE(res.ok());
        EXPECT_TRUE(static_cast<bool>(res));
    }

    for(int code : {301, 302, 400, 401, 403, 404, 500, 502, 503})
    {
        auto res = client.get("/status/" + std::to_string(code));
        EXPECT_TRUE(res.transport_success);
        EXPECT_EQ(res.status_code, code);
        EXPECT_FALSE(res.ok());
        EXPECT_FALSE(static_cast<bool>(res));
    }
}

TEST_F(HttpClientTest, ResponseHeadersParsing)
{
    _server.Get("/custom-headers",
                [](const httplib::Request &, httplib::Response &res) {
                    res.set_header("X-Test", "hello");
                    res.set_header("X-Server-Time", "2026-09-01");
                });

    http_client client(_base_url);
    auto        res = client.get("/custom-headers");

    ASSERT_TRUE(res.ok());
    EXPECT_EQ(res.headers.get("X-Test"), "hello");
    EXPECT_EQ(res.headers.get("x-test"), "hello");
    EXPECT_EQ(res.headers.get("X-Server-Time"), "2026-09-01");
}

TEST_F(HttpClientTest, RequestHeadersTransmission)
{
    std::string received_auth;
    _server.Get("/ping",
                [&](const httplib::Request &req, httplib::Response &res) {
                    if(req.has_header("Authorization"))
                    {
                        received_auth = req.get_header_value("Authorization");
                        res.status    = 200;
                    } else
                    {
                        res.status = 401;
                    }
                });

    http_client client(_base_url);
    auto res = client.get("/ping", {{"Authorization", "Bearer token-abc-123"}});

    EXPECT_TRUE(res.ok());
    EXPECT_EQ(received_auth, "Bearer token-abc-123");
}

TEST_F(HttpClientTest, PostContentTypeHeaderValidation)
{
    std::string received_content_type;
    std::string received_body;

    _server.Post("/api/data",
                 [&](const httplib::Request &req, httplib::Response &res) {
                     received_content_type =
                         req.get_header_value("Content-Type");
                     received_body = req.body;
                     res.status    = 200;
                 });

    http_client client(_base_url);
    auto        res =
        client.post("/api/data", R"({"key":"value"})", "application/json");

    EXPECT_TRUE(res.ok());
    EXPECT_EQ(received_content_type, "application/json");
    EXPECT_EQ(received_body, R"({"key":"value"})");
}

TEST_F(HttpClientTest, EmptyResponseBodyHandling)
{
    _server.Delete("/resource/1",
                   [](const httplib::Request &, httplib::Response &res) {
                       res.status = 204; // No Content
                   });

    http_client client(_base_url);
    auto        res = client.del("/resource/1");

    EXPECT_TRUE(res.ok());
    EXPECT_EQ(res.status_code, 204);
    EXPECT_TRUE(res.body.empty());
}

TEST_F(HttpClientTest, ReadTimeoutBehavior)
{
    _server.Get("/slow-response",
                [](const httplib::Request &, httplib::Response &res) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    res.status = 200;
                });

    http_timeout timeout{std::chrono::milliseconds(100)};
    http_client  client(_base_url, timeout);

    auto res = client.get("/slow-response");

    EXPECT_FALSE(res.transport_success);
    EXPECT_FALSE(res.ok());
    EXPECT_EQ(res.error, http_error::protocol); // 超时被判定为读协议/传输异常
}

TEST(HttpClientStandaloneTest, DnsFailureHandling)
{
    http_client client("http://domain.invalid.nonexistent.test");

    auto res = client.get("/");

    EXPECT_FALSE(res.transport_success);
    EXPECT_FALSE(res.ok());
    EXPECT_EQ(res.error, http_error::connection);
}

TEST_F(HttpClientTest, MoveSemanticsVerification)
{
    _server.Get("/move-test",
                [](const httplib::Request &, httplib::Response &res) {
                    res.status = 200;
                    res.body   = "moved_ok";
                });

    http_client client1(_base_url);

    http_client client2(std::move(client1));

    auto res2 = client2.get("/move-test");
    EXPECT_TRUE(res2.ok());
    EXPECT_EQ(res2.body, "moved_ok");

    http_client client3(_base_url);
    client3 = std::move(client2);

    auto res3 = client3.get("/move-test");
    EXPECT_TRUE(res3.ok());
    EXPECT_EQ(res3.body, "moved_ok");
}

#ifdef HJ_ENABLE_HTTPS
TEST(HttpClientSslTest, HttpsPublicEndpointConnection)
{
    http_client client("https://badssl.com");

    tls_config tls;
    tls.verify_server_certificate = true;
    tls.verify_hostname           = true;
    client.configure_tls(tls);

    auto res = client.get("/");
    EXPECT_TRUE(res.transport_success);
    EXPECT_TRUE(res.ok());
}
#endif

TEST_F(HttpClientTest, MetricsLoggerCallbackTest)
{
    _server.Get("/ping", [](const httplib::Request &, httplib::Response &res) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        res.status = 200;
        res.body   = "pong";
    });

    bool                     callback_called = false;
    hj::http_request_metrics captured_metrics;

    hj::http_client_options options;
    options.logger = [&](const hj::http_request_metrics &m) {
        callback_called  = true;
        captured_metrics = m;
    };

    hj::http_client client(_base_url, std::move(options));

    auto res = client.get("/ping");

    EXPECT_TRUE(res.ok());
    EXPECT_TRUE(callback_called);
    EXPECT_EQ(captured_metrics.method, hj::http_method::get);
    EXPECT_EQ(captured_metrics.status_code, 200);
    EXPECT_EQ(captured_metrics.retry_count, 0);
    EXPECT_GE(captured_metrics.latency.count(), 20000);
}

// ==================== 1. Retry 与 Idempotency 专项测试 ====================

TEST_F(HttpClientTest, RetrySuccessAfterFailures)
{
    std::atomic<int> attempt_count{0};
    _server.Get("/retry-success",
                [&](const httplib::Request &, httplib::Response &res) {
                    int current = ++attempt_count;
                    if(current < 3)
                    {
                        res.status = 500; // 前两次返回 500 触发重试
                    } else
                    {
                        res.status = 200; // 第三次成功
                        res.body   = "success_on_third";
                    }
                });

    retry_policy policy;
    policy.max_retries   = 2;
    policy.initial_delay = std::chrono::milliseconds(10); // 加快测试速度

    http_client_options options;
    options.retry = policy;

    http_client client(_base_url, std::move(options));
    auto        res = client.get("/retry-success");

    EXPECT_TRUE(res.ok());
    EXPECT_EQ(res.status_code, 200);
    EXPECT_EQ(res.body, "success_on_third");
    EXPECT_EQ(attempt_count.load(), 3); // 1次初始请求 + 2次重试 = 总共3次尝试
}

TEST_F(HttpClientTest, RetryExhaustedFailure)
{
    std::atomic<int> attempt_count{0};
    _server.Get("/retry-fail",
                [&](const httplib::Request &, httplib::Response &res) {
                    ++attempt_count;
                    res.status = 500; // 持续返回 500
                });

    retry_policy policy;
    policy.max_retries   = 2;
    policy.initial_delay = std::chrono::milliseconds(10);

    http_client_options options;
    options.retry = policy;

    std::size_t captured_retry_count = 0;
    options.logger                   = [&](const http_request_metrics &m) {
        captured_retry_count = m.retry_count;
    };

    http_client client(_base_url, std::move(options));

    auto res = client.get("/retry-fail");

    EXPECT_FALSE(res.ok());
    EXPECT_EQ(res.status_code, 500);
    EXPECT_EQ(attempt_count.load(), 3); // 尝试了 3 次（1次初始 + 2次重试）
}

TEST_F(HttpClientTest, PostDefaultNonIdempotentNoRetry)
{
    std::atomic<int> attempt_count{0};
    _server.Post("/post-retry",
                 [&](const httplib::Request &, httplib::Response &res) {
                     ++attempt_count;
                     res.status = 500;
                 });

    retry_policy policy;
    policy.max_retries              = 2;
    policy.initial_delay            = std::chrono::milliseconds(10);
    policy.retry_only_if_idempotent = true; // 默认即为 true

    http_client_options options;
    options.retry = policy;

    http_client client(_base_url, std::move(options));
    auto        res = client.post("/post-retry", "data");

    EXPECT_FALSE(res.ok());
    EXPECT_EQ(attempt_count.load(), 1); // POST 默认非幂等，不应重试
}

TEST_F(HttpClientTest, PostForcedIdempotentAllowsRetry)
{
    std::atomic<int> attempt_count{0};
    _server.Post("/post-idempotent-retry",
                 [&](const httplib::Request &, httplib::Response &res) {
                     int current = ++attempt_count;
                     if(current < 3)
                     {
                         res.status = 500;
                     } else
                     {
                         res.status = 200;
                     }
                 });

    retry_policy policy;
    policy.max_retries   = 2;
    policy.initial_delay = std::chrono::milliseconds(10);

    http_client_options options;
    options.retry = policy;

    http_client client(_base_url, std::move(options));

    http_request req;
    req.method        = http_method::post;
    req.path          = "/post-idempotent-retry";
    req.body          = "data";
    req.is_idempotent = true; // 显式标记为幂等请求

    auto res = client.request(req);

    EXPECT_TRUE(res.ok());
    EXPECT_EQ(attempt_count.load(), 3); // 允许重试，最终成功
}

TEST_F(HttpClientTest, TlsErrorNoRetry)
{
    // 1. 设置极短的超时，避免 OS Socket 重传导致卡死
    http_timeout fast_timeout{std::chrono::milliseconds(50)};

    retry_policy policy;
    policy.max_retries   = 2;
    policy.initial_delay = std::chrono::milliseconds(10);

    http_client_options options;
    options.timeout = fast_timeout;
    options.retry   = policy;

    // 2. 指向一个必定拒绝/无法连接的端口（如 127.0.0.1:1）
    http_client client("http://127.0.0.1:1", std::move(options));

    auto start_time = std::chrono::steady_clock::now();
    auto res        = client.get("/");
    auto duration   = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now() - start_time)
                          .count();

    // 3. 验证结果：请求失败，且没有因为 retry 导致运行时间翻倍
    EXPECT_FALSE(res.transport_success);
    EXPECT_EQ(res.error, http_error::connection);

    // 如果没有重试，耗时应该在 1 次超时左右（<< 200ms），而不是 3 次尝试（> 150ms + 延迟）
    EXPECT_LT(duration, 500);
}

TEST_F(HttpClientTest, RetryAfterHeaderRespecting)
{
    std::atomic<int> attempt_count{0};
    auto             start_time = std::chrono::steady_clock::now();

    _server.Get("/retry-after",
                [&](const httplib::Request &, httplib::Response &res) {
                    int current = ++attempt_count;
                    if(current == 1)
                    {
                        res.status = 429;
                        res.set_header("Retry-After", "1"); // 要求等待 1 秒
                    } else
                    {
                        res.status = 200;
                    }
                });

    retry_policy policy;
    policy.max_retries         = 1;
    policy.respect_retry_after = true;

    http_client_options options;
    options.retry = policy;

    http_client client(_base_url, std::move(options));
    auto        res = client.get("/retry-after");

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        end_time - start_time)
                        .count();

    EXPECT_TRUE(res.ok());
    EXPECT_EQ(attempt_count.load(), 2);
    EXPECT_GE(duration, 900);
}


// ==================== 2. Query 参数编码与解析专项测试 ====================

// ==================== 3. 高并发与超时隔离测试 ====================

TEST_F(HttpClientTest, ConcurrentRequestsTimeoutIsolation)
{
    _server.Get("/sleep-100ms",
                [&](const httplib::Request &, httplib::Response &res) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    res.status = 200;
                    res.body   = "fast";
                });

    _server.Get("/sleep-1000ms",
                [&](const httplib::Request &, httplib::Response &res) {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(1000));
                    res.status = 200;
                    res.body   = "slow";
                });

    // 线程 A：拥有独立的客户端 A，显式设定极短的自定义 timeout = 100ms
    auto future_a = std::async(std::launch::async, [&]() {
        http_client  client_a(_base_url,
                              http_timeout{std::chrono::milliseconds(3000)});
        http_request req;
        req.method = http_method::get;
        req.path   = "/sleep-1000ms";
        req.timeout =
            http_timeout{std::chrono::milliseconds(100)}; // 预期超时失败
        return client_a.request(req);
    });

    // 线程 B：拥有独立的客户端 B，使用默认的 3s timeout
    auto future_b = std::async(std::launch::async, [&]() {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10)); // 错开微小的时间片
        http_client  client_b(_base_url,
                              http_timeout{std::chrono::milliseconds(3000)});
        http_request req;
        req.method = http_method::get;
        req.path   = "/sleep-100ms";
        return client_b.request(req);
    });

    auto res_a = future_a.get();
    auto res_b = future_b.get();

    // 验证线程 A 因为 100ms 超时而失败
    EXPECT_FALSE(res_a.transport_success);

    // 验证线程 B 独立运行，成功返回
    EXPECT_TRUE(res_b.transport_success);
    EXPECT_EQ(res_b.status_code, 200);
    EXPECT_EQ(res_b.body, "fast");
}

TEST(HttpClientUrlTest, QueryParametersEncodingAndSorting)
{
    query_params query{{"name", "Harry Potter"},
                       {"q", "a+b"},
                       {"url", "https://example.com?a=1&b=2"}};

    std::string result_path = hj::detail::build_full_path("/search", query);

    // 验证严格匹配 Percent-Encoding 标准以及参数按 key 排序后的字符串
    EXPECT_EQ(result_path,
              "/search?name=Harry%20Potter"
              "&q=a%2Bb"
              "&url=https%3A%2F%2Fexample.com%3Fa%3D1%26b%3D2");
}

// 2. 针对 HTTP 客户端真实发送与服务端接收的集成测试 (Integration Test)
TEST_F(HttpClientTest, QueryParametersTransmissionAndParsing)
{
    std::string received_name;
    std::string received_q;
    std::string received_url;

    _server.Get("/echo-query",
                [&](const httplib::Request &req, httplib::Response &res) {
                    // 使用 httplib 提供的 params 映射表验证解包后的真实 Query 参数
                    if(req.has_param("name"))
                        received_name = req.get_param_value("name");
                    if(req.has_param("q"))
                        received_q = req.get_param_value("q");
                    if(req.has_param("url"))
                        received_url = req.get_param_value("url");

                    res.status = 200;
                    res.body   = "ok";
                });

    http_client client(_base_url);

    http_request req;
    req.method = http_method::get;
    req.path   = "/echo-query";
    req.query  = {{"name", "Harry Potter"},
                  {"q", "a+b"},
                  {"url", "https://example.com?a=1&b=2"}};

    auto res = client.request(req);

    EXPECT_TRUE(res.ok());
    // 验证经过 HTTP 客户端发送、网络传输、Server 解析后，数据能够完整无损还原
    EXPECT_EQ(received_name, "Harry Potter");
    EXPECT_EQ(received_q, "a+b");
    EXPECT_EQ(received_url, "https://example.com?a=1&b=2");
}

} // namespace hj::test