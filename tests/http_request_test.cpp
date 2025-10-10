#include <gtest/gtest.h>
#include <hj/net/http/http_request.hpp>

TEST(http_request, construct)
{
    hj::http_request req;
    SUCCEED();
}

TEST(http_request, set_method_and_path)
{
    hj::http_request req;
    req.method = "POST";
    req.path   = "/api/test";
    EXPECT_EQ(req.method, "POST");
    EXPECT_EQ(req.path, "/api/test");
}

TEST(http_request, set_headers)
{
    hj::http_request req;
    req.headers.insert({"Content-Type", "application/json"});
    auto it = req.headers.find("Content-Type");
    ASSERT_NE(it, req.headers.end());
    EXPECT_EQ(it->second, "application/json");
}

TEST(http_request, set_params)
{
    hj::http_params params;
    params.insert({"key1", "value1"});
    params.insert({"key2", "value2"});
    auto it1 = params.find("key1");
    auto it2 = params.find("key2");
    ASSERT_NE(it1, params.end());
    ASSERT_NE(it2, params.end());
    EXPECT_EQ(it1->second, "value1");
    EXPECT_EQ(it2->second, "value2");
}

TEST(http_request, multipart_form_data)
{
    hj::http_multipart_form_data_items items = {
        {"field1", "value1", "filename1.txt", "text/plain"},
        {"field2", "value2", "filename2.txt", "text/plain"}};
    EXPECT_EQ(items.size(), 2);
    EXPECT_EQ(items[0].name, "field1");
    EXPECT_EQ(items[1].filename, "filename2.txt");
}