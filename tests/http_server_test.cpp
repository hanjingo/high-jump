#include <gtest/gtest.h>
#include <hj/net/http/http_server.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <string>
#include <thread>
#include <vector>

namespace
{
void wait_for_server(const hj::http::http_server &server)
{
    int retries = 0;
    while(!server.is_running() && retries++ < 100)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_TRUE(server.is_running());
}
} // namespace

TEST(http_server, construct)
{
    EXPECT_NO_THROW({ hj::http::http_server server; });
}

TEST(http_server, route_convenience_methods)
{
    hj::http::http_server server;

    server
        .get("/get",
             [](const hj::http::http_request &, hj::http::http_response &res) {
                 res.status_code = 200;
             })
        .post("/post",
              [](const hj::http::http_request &, hj::http::http_response &res) {
                  res.status_code = 200;
              })
        .put("/put",
             [](const hj::http::http_request &, hj::http::http_response &res) {
                 res.status_code = 200;
             })
        .patch("/patch",
               [](const hj::http::http_request &,
                  hj::http::http_response &res) { res.status_code = 200; })
        .del("/del",
             [](const hj::http::http_request &, hj::http::http_response &res) {
                 res.status_code = 200;
             })
        .head("/head",
              [](const hj::http::http_request &, hj::http::http_response &res) {
                  res.status_code = 200;
              })
        .options("/options",
                 [](const hj::http::http_request &,
                    hj::http::http_response &res) { res.status_code = 200; });

    SUCCEED();
}

TEST(http_server, generic_route_method)
{
    hj::http::http_server server;

    server.route(
        hj::http::http_method::patch,
        "/patch_generic",
        [](const hj::http::http_request &req, hj::http::http_response &res) {
            EXPECT_EQ(req.method, hj::http::http_method::patch);
            res.status_code = 200;
            res.body        = "patched";
        });

    SUCCEED();
}

TEST(http_server, start_and_stop_integration)
{
    hj::http::http_server server;
    const std::string     host = "127.0.0.1";
    const int             port = 18080;

    server.patch(
        "/resource",
        [](const hj::http::http_request &req, hj::http::http_response &res) {
            EXPECT_EQ(req.method, hj::http::http_method::patch);
            res.status_code = 200;
            res.body        = "patched_ok";
        });

    std::thread server_thread(
        [&server, host, port]() { server.listen(host, port); });

    int retries = 0;
    while(!server.is_running() && retries++ < 50)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    ASSERT_TRUE(server.is_running());

    httplib::Client client(host, port);
    auto            res = client.Patch("/resource", "", "text/plain");

    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->body, "patched_ok");

    server.stop();
    if(server_thread.joinable())
    {
        server_thread.join();
    }
    EXPECT_FALSE(server.is_running());
}

TEST(http_server, handler_exception_handling)
{
    hj::http::http_server server;
    const std::string     host = "127.0.0.1";
    const int             port = 18081;

    server.get("/throw_std",
               [](const hj::http::http_request &, hj::http::http_response &) {
                   throw std::runtime_error("Database connection lost");
               });

    server.get("/throw_unknown",
               [](const hj::http::http_request &, hj::http::http_response &) {
                   throw 42; // unknown exception
               });

    std::thread server_thread(
        [&server, host, port]() { server.listen(host, port); });

    int retries = 0;
    while(!server.is_running() && retries++ < 50)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    ASSERT_TRUE(server.is_running());

    httplib::Client client(host, port);

    auto res1 = client.Get("/throw_std");
    ASSERT_NE(res1, nullptr);
    EXPECT_EQ(res1->status, 500);

    auto res2 = client.Get("/throw_unknown");
    ASSERT_NE(res2, nullptr);
    EXPECT_EQ(res2->status, 500);

    server.stop();
    if(server_thread.joinable())
    {
        server_thread.join();
    }
}

TEST(http_server, start_and_stop_async)
{
    hj::http::http_server server;
    server.get("/ping",
               [](const hj::http::http_request &,
                  hj::http::http_response &res) { res.body = "pong"; });

    auto fut = server.listen_async("127.0.0.1", 18080);

    while(!server.is_running())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    httplib::Client client("127.0.0.1", 18080);
    auto            res = client.Get("/ping");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->body, "pong");

    server.stop();
}

TEST(http_server_methods, all_http_methods)
{
    hj::http::http_server server;
    const std::string     host = "127.0.0.1";
    const int             port = 18090;

    server.get(
        "/test",
        [](const hj::http::http_request &req, hj::http::http_response &res) {
            EXPECT_EQ(req.method, hj::http::http_method::get);
            res.body = "GET_OK";
        });

    server.post(
        "/test",
        [](const hj::http::http_request &req, hj::http::http_response &res) {
            EXPECT_EQ(req.method, hj::http::http_method::post);
            res.body = "POST_OK";
        });

    server.put(
        "/test",
        [](const hj::http::http_request &req, hj::http::http_response &res) {
            EXPECT_EQ(req.method, hj::http::http_method::put);
            res.body = "PUT_OK";
        });

    server.patch(
        "/test",
        [](const hj::http::http_request &req, hj::http::http_response &res) {
            EXPECT_EQ(req.method, hj::http::http_method::patch);
            res.body = "PATCH_OK";
        });

    server.del(
        "/test",
        [](const hj::http::http_request &req, hj::http::http_response &res) {
            EXPECT_EQ(req.method, hj::http::http_method::del);
            res.body = "DELETE_OK";
        });

    server.head(
        "/test",
        [](const hj::http::http_request &req, hj::http::http_response &res) {
            EXPECT_EQ(req.method, hj::http::http_method::head);
            res.headers.set("X-Head-Status", "Pass");
        });

    server.options(
        "/test",
        [](const hj::http::http_request &req, hj::http::http_response &res) {
            EXPECT_EQ(req.method, hj::http::http_method::options);
            res.headers.set("Allow",
                            "GET, POST, PUT, PATCH, DELETE, HEAD, OPTIONS");
        });

    auto fut = server.listen_async(host, port);
    wait_for_server(server);

    httplib::Client client(host, port);

    // 1. GET
    auto res_get = client.Get("/test");
    ASSERT_NE(res_get, nullptr);
    EXPECT_EQ(res_get->status, 200);
    EXPECT_EQ(res_get->body, "GET_OK");

    // 2. POST
    auto res_post = client.Post("/test", "data", "text/plain");
    ASSERT_NE(res_post, nullptr);
    EXPECT_EQ(res_post->status, 200);
    EXPECT_EQ(res_post->body, "POST_OK");

    // 3. PUT
    auto res_put = client.Put("/test", "data", "text/plain");
    ASSERT_NE(res_put, nullptr);
    EXPECT_EQ(res_put->status, 200);
    EXPECT_EQ(res_put->body, "PUT_OK");

    // 4. PATCH
    auto res_patch = client.Patch("/test", "data", "text/plain");
    ASSERT_NE(res_patch, nullptr);
    EXPECT_EQ(res_patch->status, 200);
    EXPECT_EQ(res_patch->body, "PATCH_OK");

    // 5. DELETE
    auto res_del = client.Delete("/test");
    ASSERT_NE(res_del, nullptr);
    EXPECT_EQ(res_del->status, 200);
    EXPECT_EQ(res_del->body, "DELETE_OK");

    // 6. HEAD
    auto res_head = client.Head("/test");
    ASSERT_NE(res_head, nullptr);
    EXPECT_EQ(res_head->status, 200);
    EXPECT_TRUE(res_head->body.empty());
    EXPECT_EQ(res_head->get_header_value("X-Head-Status"), "Pass");

    // 7. OPTIONS
    auto res_opt = client.Options("/test");
    ASSERT_NE(res_opt, nullptr);
    EXPECT_EQ(res_opt->status, 200);
    EXPECT_EQ(res_opt->get_header_value("Allow"),
              "GET, POST, PUT, PATCH, DELETE, HEAD, OPTIONS");

    server.stop();
}

TEST(http_server_request, full_request_parsing)
{
    hj::http::http_server server;
    const std::string     host = "127.0.0.1";
    const int             port = 18091;

    server.post(
        "/users",
        [](const hj::http::http_request &req, hj::http::http_response &res) {
            EXPECT_EQ(req.path, "/users");
            EXPECT_EQ(req.method, hj::http::http_method::post);

            ASSERT_GE(req.query.size(), 2u);
            bool found_id   = false;
            bool found_page = false;
            for(const auto &[k, v] : req.query)
            {
                if(k == "id" && v == "100")
                    found_id = true;
                if(k == "page" && v == "2")
                    found_page = true;
            }
            EXPECT_TRUE(found_id);
            EXPECT_TRUE(found_page);

            EXPECT_EQ(req.content_type, "application/json");

            EXPECT_EQ(req.headers.get("X-Custom-Header"), "TestValue");

            EXPECT_EQ(req.body, R"({"name":"foo"})");

            res.status_code = 201;
            res.body        = R"({"status":"created"})";
        });

    auto fut = server.listen_async(host, port);
    wait_for_server(server);

    httplib::Client  client(host, port);
    httplib::Headers headers = {{"X-Custom-Header", "TestValue"}};

    auto res = client.Post("/users?id=100&page=2",
                           headers,
                           R"({"name":"foo"})",
                           "application/json");

    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 201);
    EXPECT_EQ(res->body, R"({"status":"created"})");

    server.stop();
}

TEST(http_server_response, status_codes)
{
    hj::http::http_server server;
    const std::string     host = "127.0.0.1";
    const int             port = 18092;

    const std::vector<int> status_codes =
        {200, 201, 204, 301, 400, 401, 403, 404, 409, 429, 500, 503};

    for(int code : status_codes)
    {
        std::string path = "/status/" + std::to_string(code);
        server.get(path,
                   [code](const hj::http::http_request &,
                          hj::http::http_response &res) {
                       res.status_code = code;
                       if(code != 204)
                       {
                           res.body = "status_" + std::to_string(code);
                       }
                   });
    }

    auto fut = server.listen_async(host, port);
    wait_for_server(server);

    httplib::Client client(host, port);

    for(int code : status_codes)
    {
        std::string path = "/status/" + std::to_string(code);
        auto        res  = client.Get(path);
        ASSERT_NE(res, nullptr);
        EXPECT_EQ(res->status, code);
        if(code != 204)
        {
            EXPECT_EQ(res->body, "status_" + std::to_string(code));
        }
    }

    server.stop();
}

TEST(http_server_headers, advanced_header_tests)
{
    hj::http::http_server server;
    const std::string     host = "127.0.0.1";
    const int             port = 18093;

    server.get(
        "/headers",
        [](const hj::http::http_request &req, hj::http::http_response &res) {
            EXPECT_EQ(req.headers.get("x-request-id"), "req-12345");
            EXPECT_EQ(req.headers.get("X-REQUEST-ID"), "req-12345");

            auto cookies = req.headers.get_all("Cookie");
            EXPECT_GE(cookies.size(), 2u);

            res.headers.set("Content-Type", "application/json; charset=utf-8");
            res.headers.set("X-Request-ID", "resp-98765");
            res.headers.append("Set-Cookie", "session=abc; Path=/");
            res.headers.append("Set-Cookie", "theme=dark; Path=/");
            res.body = R"({"ok":true})";
        });

    auto fut = server.listen_async(host, port);
    wait_for_server(server);

    httplib::Client  client(host, port);
    httplib::Headers headers = {{"X-Request-ID", "req-12345"},
                                {"Cookie", "session=abc"},
                                {"Cookie", "theme=dark"}};

    auto res = client.Get("/headers", headers);
    ASSERT_NE(res, nullptr);

    EXPECT_EQ(res->get_header_value("Content-Type"),
              "application/json; charset=utf-8");
    EXPECT_EQ(res->get_header_value("X-Request-ID"), "resp-98765");

    std::vector<std::string> set_cookies;
    auto                     count = res->get_header_value_count("Set-Cookie");
    for(size_t i = 0; i < count; ++i)
    {
        set_cookies.push_back(res->get_header_value("Set-Cookie", "", i));
    }
    EXPECT_EQ(set_cookies.size(), 2u);

    server.stop();
}

TEST(http_server_lifecycle, edge_cases)
{
    const std::string host = "127.0.0.1";
    const int         port = 18094;

    // 1. stop before listen
    {
        hj::http::http_server server;
        EXPECT_NO_THROW(server.stop());
        EXPECT_FALSE(server.is_running());
    }

    // 2. stop twice
    {
        hj::http::http_server server;
        auto                  fut = server.listen_async(host, port);
        wait_for_server(server);
        EXPECT_TRUE(server.is_running());

        server.stop();
        EXPECT_FALSE(server.is_running());
        EXPECT_NO_THROW(server.stop());
    }

    // 3. destructor while running
    {
        auto server = std::make_unique<hj::http::http_server>();
        auto fut    = server->listen_async(host, port);
        wait_for_server(*server);
        EXPECT_TRUE(server->is_running());
        EXPECT_NO_THROW(server.reset());
    }

    // 4. listen bind failure
    {
        hj::http::http_server server1;
        auto                  fut1 = server1.listen_async(host, port);
        wait_for_server(server1);

        hj::http::http_server server2;
        auto fut2 = std::async(std::launch::async, [&server2, host, port]() {
            return server2.listen(host, port);
        });

        if(fut2.wait_for(std::chrono::milliseconds(500))
           != std::future_status::ready)
        {
            server2.stop();
        }

        bool bind_ok = fut2.get();
        EXPECT_FALSE(server2.is_running() && bind_ok);

        server1.stop();
    }

    // 5. start after stop
    {
        std::atomic<int>      call_count{0};
        hj::http::http_server server;

        server.get(
            "/ping",
            [&](const hj::http::http_request &, hj::http::http_response &res) {
                if(call_count.load() == 0)
                {
                    res.body = "pong1";
                } else
                {
                    res.body = "pong2";
                }
            });

        auto fut1 = server.listen_async(host, port);
        wait_for_server(server);

        httplib::Client client(host, port);
        auto            res1 = client.Get("/ping");
        ASSERT_NE(res1, nullptr);
        EXPECT_EQ(res1->body, "pong1");

        server.stop();
        EXPECT_FALSE(server.is_running());

        call_count.store(1);
        auto fut2 = server.listen_async(host, port);
        wait_for_server(server);

        auto res2 = client.Get("/ping");
        ASSERT_NE(res2, nullptr);
        EXPECT_EQ(res2->body, "pong2");

        server.stop();
    }
}

TEST(http_server_concurrency, multi_client_high_pressure)
{
    hj::http::http_server server;
    const std::string     host = "127.0.0.1";
    const int             port = 18095;

    std::atomic<uint64_t> total_requests{0};
    std::atomic<uint64_t> total_errors{0};

    server.get(
        "/benchmark",
        [&](const hj::http::http_request &req, hj::http::http_response &res) {
            total_requests.fetch_add(1, std::memory_order_relaxed);
            std::string client_id = req.headers.get("X-Client-ID");
            res.status_code       = 200;
            res.body              = "echo:" + client_id;
        });

    auto fut = server.listen_async(host, port);
    wait_for_server(server);

    constexpr int NUM_THREADS         = 10;
    constexpr int REQUESTS_PER_THREAD = 50;

    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    for(int t = 0; t < NUM_THREADS; ++t)
    {
        threads.emplace_back(
            [t, host, port, &total_errors, REQUESTS_PER_THREAD]() {
                httplib::Client client(host, port);
                client.set_keep_alive(true);

                std::string      client_id = "thread_" + std::to_string(t);
                httplib::Headers headers   = {{"X-Client-ID", client_id}};

                for(int i = 0; i < REQUESTS_PER_THREAD; ++i)
                {
                    auto res = client.Get("/benchmark", headers);
                    if(!res || res->status != 200
                       || res->body != ("echo:" + client_id))
                    {
                        total_errors.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            });
    }

    for(auto &th : threads)
    {
        if(th.joinable())
            th.join();
    }

    EXPECT_EQ(total_errors.load(), 0u);
    EXPECT_EQ(total_requests.load(),
              static_cast<uint64_t>(NUM_THREADS * REQUESTS_PER_THREAD));

    server.stop();
}

TEST(http_server, custom_exception_handler)
{
    hj::http::http_server server;
    const std::string     host = "127.0.0.1";
    const int             port = 18082;

    struct invalid_param_error : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    server.set_exception_handler([](const hj::http::http_request &req,
                                    hj::http::http_response      &resp,
                                    std::exception_ptr            ep) {
        try
        {
            if(ep)
                std::rethrow_exception(ep);
        }
        catch(const invalid_param_error &e)
        {
            resp.status_code = 400;
            resp.headers.set("Content-Type", "application/json");
            resp.body =
                R"({"code": 40001, "msg": ")" + std::string(e.what()) + R"("})";
        }
        catch(const std::exception &e)
        {
            resp.status_code = 500;
            resp.headers.set("Content-Type", "application/json");
            resp.body = R"({"code": 50000, "msg": "Custom Server Error"})";
        }
    });

    server.get("/bad_param",
               [](const hj::http::http_request &, hj::http::http_response &) {
                   throw invalid_param_error("field 'age' is required");
               });

    server.get("/crash",
               [](const hj::http::http_request &, hj::http::http_response &) {
                   throw std::runtime_error("db crash");
               });

    auto fut = server.listen_async(host, port);
    wait_for_server(server);

    httplib::Client client(host, port);

    auto res1 = client.Get("/bad_param");
    ASSERT_NE(res1, nullptr);
    EXPECT_EQ(res1->status, 400);
    EXPECT_EQ(res1->body,
              R"({"code": 40001, "msg": "field 'age' is required"})");

    auto res2 = client.Get("/crash");
    ASSERT_NE(res2, nullptr);
    EXPECT_EQ(res2->status, 500);
    EXPECT_EQ(res2->body, R"({"code": 50000, "msg": "Custom Server Error"})");

    server.stop();
}

TEST(http_server_metrics, basic_metrics_collection)
{
    hj::http::http_server server;
    const std::string     host = "127.0.0.1";
    const int             port = 18096;

    std::atomic<bool>             metric_collected{false};
    hj::http::http_server_metrics recorded_metrics;

    server.set_metrics_handler([&](const hj::http::http_server_metrics &m) {
        recorded_metrics = m;
        metric_collected.store(true);
    });

    server.post(
        "/data",
        [](const hj::http::http_request &req, hj::http::http_response &res) {
            res.status_code = 201;
            res.body        = "created";
        });

    auto fut = server.listen_async(host, port);
    wait_for_server(server);

    httplib::Client client(host, port);
    auto            res = client.Post("/data", "hello server", "text/plain");

    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 201);

    EXPECT_TRUE(metric_collected.load());
    EXPECT_EQ(recorded_metrics.method, hj::http::http_method::post);
    EXPECT_EQ(recorded_metrics.path, "/data");
    EXPECT_EQ(recorded_metrics.status_code, 201);
    EXPECT_EQ(recorded_metrics.request_bytes,
              std::string("hello server").size());
    EXPECT_EQ(recorded_metrics.response_bytes, std::string("created").size());
    EXPECT_GT(recorded_metrics.latency.count(), 0);

    server.stop();
}