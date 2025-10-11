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
    SUCCEED();
}