#include <gtest/gtest.h>
#include <hj/net/http/http_server.hpp>

TEST(http_server, construct)
{
    hj::http_server server;
    SUCCEED();
}

TEST(http_server, set_route)
{
    hj::http_server server;
    server.Get("/hello", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello, World!", "text/plain");
    });
    SUCCEED();
}

TEST(http_server, start_and_stop)
{
    hj::http_server server;

    GTEST_SKIP()
        << "Network listen test skipped (would block or require free port).";
    // server.listen("localhost", 8080);
    // server.stop();
}