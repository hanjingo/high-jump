#include <gtest/gtest.h>
#include <hj/net/http/http_response.hpp>

TEST(http_response, construct)
{
    hj::http_response res;
    SUCCEED();
}

TEST(http_response, set_status_and_body)
{
    hj::http_response res;
    res.status = 404;
    res.body   = "Not Found";
    EXPECT_EQ(res.status, 404);
    EXPECT_EQ(res.body, "Not Found");
}

TEST(http_response, set_headers)
{
    hj::http_response res;
    res.headers.insert({"Content-Type", "application/json"});
    auto it = res.headers.find("Content-Type");
    ASSERT_NE(it, res.headers.end());
    EXPECT_EQ(it->second, "application/json");
}