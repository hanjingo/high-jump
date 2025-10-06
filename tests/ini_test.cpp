
#include <gtest/gtest.h>
#include <hj/encoding/ini.hpp>


TEST(ini, parse)
{
    char    text[] = "[section1] \na=123 \nb=abc \nc=123.456";
    hj::ini ini    = hj::ini::parse(text);
    ASSERT_EQ(ini.get_child("section1").get<int>("a"), 123);
    ASSERT_EQ(ini.get_child("section1").get<std::string>("b"), "abc");
    ASSERT_FLOAT_EQ(ini.get_child("section1").get<float>("c"), 123.456f);
}

TEST(ini, get_set)
{
    hj::ini ini;
    ini.put("section1.a", 42);
    ini.put("section1.b", "hello");
    ini.put("section1.c", 3.14);
    ASSERT_EQ(ini.get<int>("section1.a"), 42);
    ASSERT_EQ(ini.get<std::string>("section1.b"), "hello");
    ASSERT_DOUBLE_EQ(ini.get<double>("section1.c"), 3.14);
    ASSERT_EQ(ini.get<int>("section1.not_exist", 99), 99);
}

TEST(ini, set_method)
{
    hj::ini ini;
    ini.set("section2.x", 100);
    ini.set("section2.y", std::string("world"));
    ASSERT_EQ(ini.get<int>("section2.x"), 100);
    ASSERT_EQ(ini.get<std::string>("section2.y"), "world");
}

TEST(ini, str_and_parse_roundtrip)
{
    hj::ini ini;
    ini.put("person.name", "hanjingo");
    ini.put("person.age", 30);
    ini.put("person.income", 10000.123);
    std::string s    = ini.str();
    hj::ini     ini2 = hj::ini::parse(s.c_str());
    ASSERT_EQ(ini2.get<std::string>("person.name"), "hanjingo");
    ASSERT_EQ(ini2.get<int>("person.age"), 30);
    ASSERT_FLOAT_EQ(ini2.get<float>("person.income"), 10000.123f);
}

TEST(ini, read_file)
{
    char    text[] = "[person] \nname=hanjingo \nage=30 \nincome=10000.123";
    hj::ini cfg1   = hj::ini::parse(text);
    ASSERT_TRUE(cfg1.write_file("cfg1.ini"));

    hj::ini cfg2;
    ASSERT_TRUE(cfg2.read_file("cfg1.ini"));
    ASSERT_EQ(cfg2.get<std::string>("person.name"), "hanjingo");
    ASSERT_EQ(cfg2.get<int>("person.age"), 30);
    ASSERT_FLOAT_EQ(cfg2.get<float>("person.income"), 10000.123f);
}


TEST(ini, write_file)
{
    char    text[] = "[person] \nname=hanjingo \nage=30 \nincome=10000.123";
    hj::ini cfg1   = hj::ini::parse(text);
    cfg1.set("person.email", "hehehunanchina@live.com");
    ASSERT_TRUE(cfg1.write_file("cfg2.ini"));

    hj::ini cfg2;
    ASSERT_TRUE(cfg2.read_file("cfg2.ini"));
    ASSERT_EQ(cfg2.get<std::string>("person.email"), "hehehunanchina@live.com");
}


TEST(ini, str)
{
    char    text[] = "[person] \nname=hanjingo";
    hj::ini cfg1   = hj::ini::parse(text);
    cfg1.set("person.email", "hehehunanchina@live.com");
    hj::ini cfg2 = hj::ini::parse(cfg1.str().c_str());
    ASSERT_EQ(cfg2.get<std::string>("person.email"), "hehehunanchina@live.com");
}

TEST(ini, error_handling)
{
    hj::ini ini;
    ASSERT_FALSE(ini.read_file("not_exist_file.ini"));
    const char *bad_ini = "[section\na=1";
    hj::ini     ini2    = hj::ini::parse(bad_ini);
    ASSERT_TRUE(ini2.empty());
    ASSERT_EQ(ini.get<int>("no.such.key", 12345), 12345);
}