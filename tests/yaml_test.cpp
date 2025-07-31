#include <gtest/gtest.h>
#include <libcpp/encoding/yaml.hpp>
#include <cstdio>
#include <string>

using namespace libcpp;

TEST(YamlTest, ParseFromString)
{
    const char* text = "name: test\nvalue: 42\n";
    yaml y = yaml::parse(text);
    EXPECT_TRUE(y["name"]);
    EXPECT_EQ(y["name"].as<std::string>(), "test");
    EXPECT_EQ(y["value"].as<int>(), 42);
}

TEST(YamlTest, ReadWriteFile)
{
    const char* filename = "test.yaml";
    yaml y;
    y["foo"] = "bar";
    y["num"] = 123;
    y.write_file(filename);

    yaml y2;
    EXPECT_TRUE(y2.read_file(filename));
    EXPECT_EQ(y2["foo"].as<std::string>(), "bar");
    EXPECT_EQ(y2["num"].as<int>(), 123);

    std::remove(filename);
}

TEST(YamlTest, SerializeToString)
{
    yaml y;
    y["a"] = 1;
    y["b"] = 2;
    std::string s = y.str();
    EXPECT_NE(s.find("a: 1"), std::string::npos);
    EXPECT_NE(s.find("b: 2"), std::string::npos);
}