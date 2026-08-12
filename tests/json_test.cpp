#include <gtest/gtest.h>
#include <hj/encoding/json.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

struct json_obj
{
    std::string name;
    int         age;
    std::string email;
};

TEST(json, basic_types)
{
    hj::json js;
    js["null"]   = nullptr;
    js["bool"]   = true;
    js["int"]    = 42;
    js["double"] = 3.14;
    js["str"]    = "hello";

    EXPECT_TRUE(js["null"].is_null());
    EXPECT_TRUE(js["bool"].get<bool>());
    EXPECT_EQ(js["int"].get<int>(), 42);
    EXPECT_DOUBLE_EQ(js["double"].get<double>(), 3.14);
    EXPECT_EQ(js["str"].get<std::string>(), "hello");
}

TEST(json, nested_structure)
{
    hj::json js = {{"meta", {{"id", 1}, {"tags", {"cpp", "json"}}}},
                   {"data", {{"values", {10, 20, 30}}}}};

    EXPECT_EQ(js["meta"]["tags"][0].get<std::string>(), "cpp");
    EXPECT_EQ(js["data"]["values"].size(), 3);
    EXPECT_EQ(js["data"]["values"][2].get<int>(), 30);
}

TEST(json, exception_and_boundaries)
{
    hj::json js = {{"key", "value"}};

    EXPECT_THROW(js.at("not_exist"), hj::json::out_of_range);

    EXPECT_THROW(js["key"].get<int>(), hj::json::type_error);

    EXPECT_THROW(hj::json::parse("{invalid: json}"), hj::json::parse_error);
}

TEST(json, modification)
{
    hj::json js = {{"a", 1}, {"b", 2}};

    js.erase("a");
    EXPECT_FALSE(js.contains("a"));

    js["b"] = 3;
    EXPECT_EQ(js["b"].get<int>(), 3);
}

TEST(json, get_and_parse)
{
    json_obj obj;
    auto     js = hj::json{{"name", "harry potter"},
                           {"age", 30},
                           {"email", "hehehunanchina@live.com"}};
    js.at("name").get_to(obj.name);
    EXPECT_EQ(obj.name, "harry potter");

    auto js1 = hj::json::parse(R"({"pi": 3.141, "happy": true})");
    EXPECT_TRUE(js1["happy"].get<bool>());
}

TEST(json, make_array)
{
    hj::json arr = hj::json::array({1, 2, 3});
    arr.push_back("hello");

    EXPECT_EQ(arr.size(), 4);
    EXPECT_EQ(arr[3].get<std::string>(), "hello");
}