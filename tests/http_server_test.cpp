#include <gtest/gtest.h>
#include <hj/net/http/http_server.hpp>

#include <chrono>
#include <thread>

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