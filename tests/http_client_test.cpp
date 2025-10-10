#include <gtest/gtest.h>
#include <hj/net/http/http_client.hpp>

TEST(http_client, construct)
{
    hj::http_client client("http://localhost:8080");
    SUCCEED();
}

TEST(http_client, get_request)
{
    hj::http_client client("http://httpbin.org");
    auto            res = client.Get("/get");
    if(res)
    {
        EXPECT_EQ(res->status, 200);
        EXPECT_NE(res->body.find("httpbin.org"), std::string::npos);
    } else
    {
        GTEST_SKIP() << "No network or httpbin.org not reachable.";
    }
}