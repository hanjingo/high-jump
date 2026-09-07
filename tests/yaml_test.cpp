#include <gtest/gtest.h>
#include <hj/encoding/yaml.hpp>
#include <cstdio>
#include <filesystem>
#include <fstream>
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

TEST(yaml, dump_to_buffer)
{
    hj::yaml y;
    y["x"]          = 100;
    char   buf[128] = {};
    size_t sz       = sizeof(buf);

    EXPECT_TRUE(y.dump(buf, sz));
    std::string s(buf);
    EXPECT_NE(s.find("x: 100"), std::string::npos);

    char   small[4] = {};
    size_t small_sz = sizeof(small);
    EXPECT_FALSE(y.dump(small, small_sz));
}

TEST(yaml, invalid_load)
{
    hj::yaml y = hj::yaml::load_from_string("{ [ invalid yaml }");

    EXPECT_FALSE(y.is_defined());
    EXPECT_TRUE(y.is_null() || !y);

    hj::yaml y2 = hj::yaml::load_from_file("not_exist.yaml");
    EXPECT_FALSE(y2.is_defined());
}