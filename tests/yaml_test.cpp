
#include <gtest/gtest.h>
#include <hj/encoding/yaml.hpp>
#include <cstdio>
#include <string>
#include <fstream>

TEST(yaml, parse_from_string)
{
    const char *text = "name: test\nvalue: 42\n";
    hj::yaml    y    = hj::yaml::load(text);
    auto        n1   = y["name"];
    auto        n2   = y["value"];
    EXPECT_TRUE(n1.is_defined());
    EXPECT_EQ(n1.as<std::string>(), "test");
    EXPECT_EQ(n2.as<int>(), 42);

    auto n3 = y["not_exist"];
    EXPECT_FALSE(n3.is_defined());
    EXPECT_TRUE(n3.is_null() || !n3);
}

TEST(yaml, read_write_file)
{
    std::ofstream fout("test.yaml", std::ios::binary);
    hj::yaml      y;
    y["foo"] = "bar";
    y["num"] = 123;
    ASSERT_TRUE(y.dump(fout));
    fout.close();

    std::ifstream fin("test.yaml", std::ios::binary);
    hj::yaml      y2;
    y2.load(fin);
    auto nfoo = y2["foo"];
    auto nnum = y2["num"];
    EXPECT_TRUE(nfoo.is_defined());
    EXPECT_EQ(nfoo.as<std::string>(), "bar");
    EXPECT_EQ(nnum.as<int>(), 123);
    fin.close();

    std::remove("test.yaml");
}

TEST(yaml, serialize_to_string)
{
    hj::yaml y;
    y["a"]        = 1;
    y["b"]        = 2;
    std::string s = y.str();
    EXPECT_NE(s.find("a: 1"), std::string::npos);
    EXPECT_NE(s.find("b: 2"), std::string::npos);
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

TEST(yaml, iterator_and_type_checks)
{
    const char *text = "arr:\n  - 1\n  - 2\n  - 3\nmap:\n  k1: v1\n  k2: v2\n";
    hj::yaml    y    = hj::yaml::load(text);
    auto        arr  = y["arr"];
    auto        map  = y["map"];
    EXPECT_TRUE(arr.is_sequence());
    EXPECT_TRUE(map.is_map());
    int sum = 0;
    for(auto it = arr.begin(); it != arr.end(); ++it)
    {
        sum += it->as<int>();
    }
    EXPECT_EQ(sum, 6);
    int count = 0;
    for(auto it = map.begin(); it != map.end(); ++it)
    {
        ++count;
    }
    EXPECT_EQ(count, 2);
}

TEST(yaml, assignment_and_copy)
{
    hj::yaml y1;
    y1["a"]     = 10;
    hj::yaml y2 = y1;
    EXPECT_EQ(y2["a"].as<int>(), 10);
    y2["a"] = 20;
    EXPECT_EQ(y2["a"].as<int>(), 20);

    EXPECT_EQ(y1["a"].as<int>(), 20);
    y1 = y2;
    EXPECT_EQ(y1["a"].as<int>(), 20);
}

TEST(yaml, force_insert_and_push_back)
{
    hj::yaml y;
    y.force_insert("k", "v");
    EXPECT_EQ(y["k"].as<std::string>(), "v");
    hj::yaml arr;
    arr.push_back(1);
    arr.push_back(2);
    EXPECT_EQ(arr[0].as<int>(), 1);
    EXPECT_EQ(arr[1].as<int>(), 2);
    arr.push_back(hj::yaml::load("x: 5"));
    EXPECT_EQ(arr[2]["x"].as<int>(), 5);
}

TEST(yaml, tag_and_scalar)
{
    hj::yaml y = hj::yaml::load("!!str tagged: value");
    EXPECT_TRUE(y.is_map());
    auto n = y["tagged"];
    EXPECT_TRUE(n.is_scalar());

    n.set_tag("!!str");
    EXPECT_FALSE(n.tag().empty());

    EXPECT_EQ(n.scalar(), "value");
}

TEST(yaml, invalid_load)
{
    hj::yaml y = hj::yaml::load("::not_yaml");

    EXPECT_TRUE(y.is_scalar());
    EXPECT_EQ(y.scalar(), "::not_yaml");

    std::ifstream fin("not_exist.yaml");
    hj::yaml      y2;
    y2.load(fin);

    EXPECT_TRUE(y2.is_null() || !y2);
}