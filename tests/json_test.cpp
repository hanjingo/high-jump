#include <gtest/gtest.h>
#include <hj/encoding/json.hpp>
#include <filesystem>

struct json_obj
{
    std::string name;
    int         age;
    std::string email;
};

TEST(json, get)
{
    json_obj obj;
    auto     js = hj::json{{"name", "harry potter"},
                           {"age", 30},
                           {"email", "hehehunanchina@live.com"}};
    js.at("name").get_to(obj.name);
    ASSERT_EQ(obj.name, std::string("harry potter"));
    ASSERT_EQ(js.at("name").get<std::string>(), std::string("harry potter"));

    js.at("age").get_to(obj.age);
    ASSERT_EQ(obj.age, 30);
    ASSERT_EQ(js.at("age").get<int>(), 30);

    ASSERT_STREQ(js.at("email").get<std::string>().c_str(),
                 "hehehunanchina@live.com");
}

TEST(json, parse)
{
    // from string
    auto js1 = hj::json::parse(R"(
        {
            "pi": 3.141,
            "happy": true
        }
    )");

    ASSERT_EQ(js1.contains("pi"), true);
    ASSERT_EQ(js1["pi"].get<double>(), 3.141);
    ASSERT_EQ(js1.contains("happy"), true);
    ASSERT_EQ(js1["happy"].get<bool>(), true);

    // from ifstream
    std::string str_src = "./json_test.json";
    if(!std::filesystem::exists(str_src))
    {
        GTEST_SKIP() << "skip test json parse not exist: " << str_src;
    }
    std::ifstream f(str_src);
    auto          js2 = hj::json::parse(f);

    ASSERT_EQ(js2.contains("pi"), true);
    ASSERT_EQ(js2["pi"].get<double>(), 3.141);
    ASSERT_EQ(js2.contains("happy"), true);
    ASSERT_EQ(js2["happy"].get<bool>(), true);
}

TEST(json, make_object)
{
    hj::json js;
    js["id"]     = 1001;
    js["name"]   = "test_user";
    js["active"] = true;
    js["score"]  = 99.5;

    ASSERT_EQ(js["id"], 1001);
    ASSERT_EQ(js["name"], "test_user");
    ASSERT_EQ(js["active"], true);
    ASSERT_DOUBLE_EQ(js["score"].get<double>(), 99.5);

    std::string dumped = js.dump();
    hj::json    js2    = hj::json::parse(dumped);
    ASSERT_EQ(js2["id"], 1001);
    ASSERT_EQ(js2["name"], "test_user");
    ASSERT_EQ(js2["active"], true);
    ASSERT_DOUBLE_EQ(js2["score"].get<double>(), 99.5);
}

TEST(json, make_array)
{
    hj::json arr = hj::json::array();
    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);
    arr.push_back("hello");

    ASSERT_EQ(arr.size(), 4);
    ASSERT_EQ(arr[0], 1);
    ASSERT_EQ(arr[1], 2);
    ASSERT_EQ(arr[2], 3);
    ASSERT_EQ(arr[3], "hello");

    std::string dumped = arr.dump();
    hj::json    arr2   = hj::json::parse(dumped);
    ASSERT_EQ(arr2.size(), 4);
    ASSERT_EQ(arr2[0], 1);
    ASSERT_EQ(arr2[1], 2);
    ASSERT_EQ(arr2[2], 3);
    ASSERT_EQ(arr2[3], "hello");
}