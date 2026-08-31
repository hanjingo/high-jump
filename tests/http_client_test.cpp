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

} // namespace hj::test