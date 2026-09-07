#include <gtest/gtest.h>
#include <hj/encoding/yaml.hpp>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

TEST(yaml, parse_from_string)
{
    const char *text = "name: test\nvalue: 42\n";
    hj::yaml    y    = hj::yaml::load_from_string(text);

    auto n1 = y["name"];
    auto n2 = y["value"];

    EXPECT_TRUE(n1.is_defined());
    EXPECT_EQ(n1.as<std::string>(), "test");
    EXPECT_EQ(n2.as<int>(), 42);

    EXPECT_EQ(n2.value_or(0), 42);

    auto n3 = y["not_exist"];
    EXPECT_FALSE(n3.is_defined());
    EXPECT_FALSE(static_cast<bool>(n3));
    EXPECT_EQ(n3.value_or(100), 100);
}

TEST(yaml, read_write_file)
{
    std::filesystem::path file_path = "test_config.yaml";

    {
        std::ofstream fout(file_path, std::ios::binary);
        hj::yaml      y;
        y["foo"] = "bar";
        y["num"] = 123;
        ASSERT_TRUE(y.dump(fout));
    }

    hj::yaml y2 = hj::yaml::load_from_file(file_path);
    EXPECT_TRUE(y2.is_defined());
    EXPECT_EQ(y2["foo"].as<std::string>(), "bar");
    EXPECT_EQ(y2["num"].as<int>(), 123);

    std::filesystem::remove(file_path);
}

TEST(yaml, stream_dump_and_string_str)
{
    hj::yaml y;
    y["x"] = 100;

    std::string str_val = y.str();
    EXPECT_NE(str_val.find("x: 100"), std::string::npos);

    std::ostringstream ss;
    EXPECT_TRUE(y.dump(ss));
    EXPECT_NE(ss.str().find("x: 100"), std::string::npos);
}

TEST(yaml, iterator_and_type_checks)
{
    const char *text =
        "arr:\n  - 10\n  - 20\n  - 30\nmap:\n  k1: v1\n  k2: v2\n";
    hj::yaml y = hj::yaml::load_from_string(text);

    auto arr = y["arr"];
    auto map = y["map"];

    EXPECT_TRUE(arr.is_sequence());
    EXPECT_TRUE(map.is_map());

    int sum = 0;
    for(auto it = arr.begin(); it != arr.end(); ++it)
    {
        sum += it->as<int>();
    }
    EXPECT_EQ(sum, 60);

    int count = 0;
    for(auto it = map.begin(); it != map.end(); ++it)
    {
        EXPECT_TRUE(it.key().is_scalar());
        EXPECT_TRUE(it.value().is_scalar());
        ++count;
    }
    EXPECT_EQ(count, 2);
}

TEST(yaml, deep_copy_and_shallow_copy)
{
    hj::yaml y1;
    y1["a"] = 10;

    hj::yaml y2 = y1;
    y2["a"]     = 20;
    EXPECT_EQ(y1["a"].as<int>(), 20);

    hj::yaml y3 = y1.clone();
    y3["a"]     = 99;
    EXPECT_EQ(y1["a"].as<int>(), 20);
    EXPECT_EQ(y3["a"].as<int>(), 99);
}

TEST(yaml, invalid_load_throwing)
{
    EXPECT_THROW(hj::yaml::load_from_string("{ [ invalid yaml }"),
                 YAML::Exception);
    EXPECT_THROW(hj::yaml::load_from_file("not_exist.yaml"),
                 std::filesystem::filesystem_error);
}

TEST(yaml, invalid_load_try_optional)
{
    auto opt1 = hj::yaml::try_load_from_string("{ [ invalid yaml }");
    EXPECT_FALSE(opt1.has_value());

    auto opt2 = hj::yaml::try_load_from_file("not_exist.yaml");
    EXPECT_FALSE(opt2.has_value());
}

TEST(yaml, nullptr_handling)
{
    EXPECT_THROW(hj::yaml::load(nullptr), std::invalid_argument);

    auto opt = hj::yaml::try_load(nullptr);
    EXPECT_FALSE(opt.has_value());
}

TEST(yaml, iterator_const_conversion_and_zero_alloc)
{
    hj::yaml y   = hj::yaml::load_from_string("arr:\n  - 1\n  - 2");
    auto     arr = y["arr"];

    hj::yaml::iterator       it  = arr.begin();
    hj::yaml::const_iterator cit = it;
    EXPECT_EQ(cit->as<int>(), 1);

    EXPECT_TRUE(it == cit);
    EXPECT_FALSE(it != cit);

    int sum = 0;
    for(auto iter = arr.cbegin(); iter != arr.cend(); ++iter)
    {
        sum += iter->as<int>();
    }
    EXPECT_EQ(sum, 3);
}

TEST(yaml, strict_conversion_vs_missing)
{
    const char *text   = R"(
server:
  port: invalid_port_number
)";
    hj::yaml    config = hj::yaml::load_from_string(text);

    auto missing_node = config["server"]["missing_field"];
    EXPECT_FALSE(missing_node.as_optional<int>().has_value());
    EXPECT_EQ(missing_node.value_or(8080), 8080);

    auto invalid_node = config["server"]["port"];
    EXPECT_THROW(invalid_node.as_optional<int>(), YAML::BadConversion);
    EXPECT_THROW(invalid_node.value_or(8080), YAML::BadConversion);

    auto res1 = missing_node.try_as<int>();
    ASSERT_TRUE(std::holds_alternative<hj::convert_error>(res1));
    EXPECT_EQ(std::get<hj::convert_error>(res1),
              hj::convert_error::not_defined);

    auto res2 = invalid_node.try_as<int>();
    ASSERT_TRUE(std::holds_alternative<hj::convert_error>(res2));
    EXPECT_EQ(std::get<hj::convert_error>(res2),
              hj::convert_error::bad_conversion);
}

TEST(yaml, scalar_safety_checks)
{
    const char *text   = R"(
scalar_node: "hello world"
map_node:
  a: 1
)";
    hj::yaml    config = hj::yaml::load_from_string(text);

    auto scalar_node = config["scalar_node"];
    EXPECT_EQ(scalar_node.scalar(), "hello world");
    ASSERT_TRUE(scalar_node.scalar_optional().has_value());
    EXPECT_EQ(scalar_node.scalar_optional().value(), "hello world");

    auto map_node = config["map_node"];
    EXPECT_FALSE(map_node.scalar_optional().has_value());

    EXPECT_THROW(map_node.scalar(), YAML::BadConversion);
}

TEST(yaml, insert_and_assign_abstractions)
{
    hj::yaml y;

    // 1. insert_or_assign 实现覆盖更新
    y.insert_or_assign("key1", "val1");
    EXPECT_EQ(y["key1"].as<std::string>(), "val1");

    y.insert_or_assign("key1", "val2");
    EXPECT_EQ(y["key1"].as<std::string>(), "val2");

    // 2. insert 插入新键值对
    y.insert("key2", 100);
    EXPECT_EQ(y["key2"].as<int>(), 100);
}

TEST(yaml, null_value)
{
    hj::yaml y = hj::yaml::load_from_string("key: null\nkey2: ~\n");

    EXPECT_TRUE(y["key"].is_defined());
    EXPECT_TRUE(y["key"].is_null());
    EXPECT_FALSE(static_cast<bool>(y["key"]));

    EXPECT_TRUE(y["key2"].is_null());
    EXPECT_FALSE(y["key"].as_optional<std::string>().has_value());
}

TEST(yaml, empty_document)
{
    hj::yaml y = hj::yaml::load_from_string("");
    EXPECT_TRUE(y.is_null() || !y.is_defined());
    EXPECT_FALSE(static_cast<bool>(y));
}

TEST(yaml, empty_map)
{
    hj::yaml y    = hj::yaml::load_from_string("empty_map: {}");
    auto     node = y["empty_map"];

    EXPECT_TRUE(node.is_defined());
    EXPECT_TRUE(node.is_map());
    EXPECT_EQ(node.begin(), node.end());
}

TEST(yaml, empty_sequence)
{
    hj::yaml y    = hj::yaml::load_from_string("empty_seq: []");
    auto     node = y["empty_seq"];

    EXPECT_TRUE(node.is_defined());
    EXPECT_TRUE(node.is_sequence());
    EXPECT_EQ(node.begin(), node.end());
}

TEST(yaml, nested_structures)
{
    const char *text = R"(
server:
  host: localhost
  ports:
    - 80
    - 443
)";
    hj::yaml    y    = hj::yaml::load_from_string(text);

    EXPECT_EQ(y["server"]["host"].as<std::string>(), "localhost");
    EXPECT_TRUE(y["server"]["ports"].is_sequence());
    EXPECT_EQ(y["server"]["ports"][0].as<int>(), 80);
    EXPECT_EQ(y["server"]["ports"][1].as<int>(), 443);
}

TEST(yaml, boolean_types)
{
    hj::yaml y = hj::yaml::load_from_string("enabled: true\ndisabled: false");

    EXPECT_TRUE(y["enabled"].as<bool>());
    EXPECT_FALSE(y["disabled"].as<bool>());
}

TEST(yaml, floating_point)
{
    hj::yaml y = hj::yaml::load_from_string("ratio: 0.123");

    EXPECT_DOUBLE_EQ(y["ratio"].as<double>(), 0.123);
    EXPECT_FLOAT_EQ(y["ratio"].as<float>(), 0.123f);
}

TEST(yaml, unicode_support)
{
    hj::yaml y = hj::yaml::load_from_string("name: 中文测试_🚀");

    EXPECT_EQ(y["name"].as<std::string>(), "中文测试_🚀");
}

TEST(yaml, multiline_string)
{
    const char *text = R"(
description: |
  line 1
  line 2
)";
    hj::yaml    y    = hj::yaml::load_from_string(text);

    EXPECT_EQ(y["description"].as<std::string>(), "line 1\nline 2\n");
}

TEST(yaml, anchors_and_aliases)
{
    const char *text = R"(
defaults: &defaults
  timeout: 30

server:
  port: 8080
)";
    hj::yaml    y    = hj::yaml::load_from_string(text);

    EXPECT_EQ(y["defaults"]["timeout"].as<int>(), 30);
    EXPECT_EQ(y["server"]["port"].as<int>(), 8080);
}

TEST(yaml, malformed_syntax_errors)
{
    // 语法错误：序列/映射括号混用与未闭合语法冲突
    EXPECT_THROW(hj::yaml::load_from_string("key: [unclosed"), YAML::Exception);
    EXPECT_THROW(hj::yaml::load_from_string("a: [1, 2}\n"), YAML::Exception);

    EXPECT_FALSE(hj::yaml::try_load_from_string("key: [unclosed").has_value());
}

TEST(yaml, invalid_type_conversion)
{
    hj::yaml y = hj::yaml::load_from_string("port: not_a_number");

    EXPECT_THROW(y["port"].as<int>(), YAML::BadConversion);
    EXPECT_THROW(y["port"].as_optional<int>(), YAML::BadConversion);

    auto res = y["port"].try_as<int>();
    ASSERT_TRUE(std::holds_alternative<hj::convert_error>(res));
    EXPECT_EQ(std::get<hj::convert_error>(res),
              hj::convert_error::bad_conversion);
}

TEST(yaml, stream_failure)
{
    std::istringstream bad_ss;
    bad_ss.setstate(std::ios::failbit);

    EXPECT_THROW(hj::yaml::load_from_stream(bad_ss), std::runtime_error);

    auto opt = hj::yaml::try_load_from_stream(bad_ss);
    EXPECT_FALSE(opt.has_value());
}

TEST(yaml, insufficient_buffer)
{
    hj::yaml y;
    y["key"] = "a_very_long_string_value_for_testing";

    std::string expected_str = y.str();
    size_t      needed_len   = expected_str.size();

    char   buf[10] = {};
    size_t buf_sz  = sizeof(buf);

    EXPECT_FALSE(y.dump(buf, buf_sz));
    EXPECT_EQ(buf_sz, needed_len);

    std::vector<char> dynamic_buf(needed_len + 1);
    size_t            valid_sz = dynamic_buf.size();
    EXPECT_TRUE(y.dump(dynamic_buf.data(), valid_sz));
    EXPECT_STREQ(dynamic_buf.data(), expected_str.c_str());
}

TEST(yaml, string_view_boundary)
{
    std::string      source = "name: abcXXX";
    std::string_view view(source.data(), 9);

    hj::yaml y = hj::yaml::load_from_string(view);

    ASSERT_TRUE(y.is_defined());
    EXPECT_TRUE(y["name"].is_defined());
    EXPECT_EQ(y["name"].as<std::string>(), "abc");

    auto opt = hj::yaml::try_load_from_string(view);
    ASSERT_TRUE(opt.has_value());
    EXPECT_EQ((*opt)["name"].as<std::string>(), "abc");
}