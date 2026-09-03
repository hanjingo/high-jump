#include <gtest/gtest.h>
#include <hj/encoding/json.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

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

TEST(json_industrial, empty_and_constructors)
{
    hj::json js;
    EXPECT_TRUE(js.is_null());

    hj::json object = hj::json::object();
    hj::json array  = hj::json::array();
    EXPECT_TRUE(object.is_object());
    EXPECT_TRUE(array.is_array());
}

TEST(json_industrial, type_system_coverage)
{
    hj::json j_null  = nullptr;
    hj::json j_bool  = true;
    hj::json j_int   = -42;
    hj::json j_uint  = 42U;
    hj::json j_float = 3.14;
    hj::json j_str   = "text";
    hj::json j_arr   = hj::json::array();
    hj::json j_obj   = hj::json::object();

    EXPECT_TRUE(j_null.is_null());
    EXPECT_TRUE(j_bool.is_boolean());
    EXPECT_TRUE(j_int.is_number_integer());
    EXPECT_TRUE(j_uint.is_number_unsigned());
    EXPECT_TRUE(j_float.is_number_float());
    EXPECT_TRUE(j_str.is_string());
    EXPECT_TRUE(j_arr.is_array());
    EXPECT_TRUE(j_obj.is_object());
}

TEST(json_industrial, sixty_four_bit_integers)
{
    std::int64_t i64_val = 9223372036854775807LL;
    hj::json     js_i64  = i64_val;
    EXPECT_EQ(js_i64.get<std::int64_t>(), i64_val);
    EXPECT_TRUE(js_i64.is_number_integer());

    std::uint64_t u64_val = 18446744073709551615ULL;
    hj::json      js_u64  = u64_val;
    EXPECT_EQ(js_u64.get<std::uint64_t>(), u64_val);
    EXPECT_TRUE(js_u64.is_number_unsigned());
}

TEST(json_industrial, floating_point_edge_cases)
{
    std::vector<double> floats = {0.0, -0.0, -314.159, 1e-308, 1.79e308};
    for(double val : floats)
    {
        hj::json js   = val;
        auto     text = js.dump();
        auto     js2  = hj::json::parse(text);
        EXPECT_DOUBLE_EQ(js2.get<double>(), val);
    }
}

TEST(json_industrial, unicode_support)
{
    hj::json js = {{"中文", "你好"}, {"emoji", "😀"}, {"english", "hello"}};

    auto text = js.dump();
    auto js2  = hj::json::parse(text);

    EXPECT_EQ(js2["中文"].get<std::string>(), "你好");
    EXPECT_EQ(js2["emoji"].get<std::string>(), "😀");
    EXPECT_EQ(js2["english"].get<std::string>(), "hello");
}

TEST(json_industrial, dump_parse_closed_loop)
{
    hj::json js = {{"name", "harry"},
                   {"age", 30},
                   {"tags", {"magic", "hogwarts"}},
                   {"metadata", {{"score", 99.5}, {"active", true}}}};

    auto text = js.dump();
    auto js2  = hj::json::parse(text);

    // 验证复杂的嵌套对象经过序列化和反序列化后完全等价
    EXPECT_EQ(js2, js);
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