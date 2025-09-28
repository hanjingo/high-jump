#include <gtest/gtest.h>
#include <hj/encoding/ini.hpp>

TEST(ini, parse)
{
    char    text[] = "[section1] \na=123 \nb=abc \nc=123.456";
    hj::ini ini    = hj::ini::parse(text);
    ASSERT_EQ(ini.get_child("section1").get<int>("a"), 123);
    ASSERT_TRUE(ini.get_child("section1").get<std::string>("b")
                == std::string("abc"));
    ASSERT_FLOAT_EQ(ini.get_child("section1").get<float>("c"), 123.456f);
}

TEST(ini, read_file)
{
    char    text[] = "[person] \nname=hanjingo \nage=30 \nincome=10000.123";
    hj::ini cfg1   = hj::ini::parse(text);
    cfg1.write_file("cfg1.ini");

    hj::ini cfg2;
    cfg2.read_file("cfg1.ini");
    ASSERT_TRUE(cfg2.get_child("person").get<std::string>("name")
                == std::string("hanjingo"));
    ASSERT_EQ(cfg2.get_child("person").get<int>("age"), 30);
    ASSERT_FLOAT_EQ(cfg2.get_child("person").get<float>("income"), 10000.123f);
}

TEST(ini, write_file)
{
    char    text[] = "[person] \nname=hanjingo \nage=30 \nincome=10000.123";
    hj::ini cfg1   = hj::ini::parse(text);
    cfg1.get_child("person").put("email", "hehehunanchina@live.com");
    cfg1.write_file("cfg2.ini");

    hj::ini cfg2;
    cfg2.read_file("cfg2.ini");
    ASSERT_TRUE(cfg2.get_child("person").get<std::string>("email")
                == std::string("hehehunanchina@live.com"));
}

TEST(ini, str)
{
    char    text[] = "[person] \nname=hanjingo";
    hj::ini cfg1   = hj::ini::parse(text);
    cfg1.get_child("person").put("email", "hehehunanchina@live.com");

    hj::ini cfg2 = hj::ini::parse(cfg1.str().c_str());
    ASSERT_TRUE(cfg2.get_child("person").get<std::string>("email")
                == std::string("hehehunanchina@live.com"));
}