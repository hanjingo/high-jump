#include <gtest/gtest.h>
#include <hj/encoding/yaml.hpp>
#include <cstdio>
#include <string>

using namespace hj;

TEST(YamlTest, ParseFromString)
{
    const char* text = "name: test\nvalue: 42\n";
    yaml y = yaml::load(text);
    auto n1 = y["name"];
    auto n2 = y["value"];
    EXPECT_TRUE(n1);
    EXPECT_EQ(n1.as<std::string>(), "test");
    EXPECT_EQ(n2.as<int>(), 42);
}

TEST(YamlTest, ReadWriteFile)
{
    std::ofstream fout("test.yaml", std::ios::binary);
    yaml y;
    y["foo"] = "bar";
    y["num"] = 123;
    y.dump(fout);
    fout.close();

    std::ifstream fin("test.yaml", std::ios::binary);
    yaml y2;
    EXPECT_TRUE(y2.load(fin));
    auto nfoo = y2["foo"];
    auto nnum = y2["num"];
    EXPECT_EQ(nfoo.as<std::string>(), "bar");
    EXPECT_EQ(nnum.as<int>(), 123);
    fin.close();
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