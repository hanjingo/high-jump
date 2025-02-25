#include <gtest/gtest.h>
#include <libcpp/encoding/ini.hpp>

TEST(ini, parse)
{
    char text[] = "[section1] \na=123 \nb=abc \nc=123.456";
    libcpp::ini ini = libcpp::ini::parse(text);
    ASSERT_EQ(ini.get_child("section1").get<int>("a"), 123);
    ASSERT_EQ(ini.get_child("section1").get<std::string>("b") == std::string("abc"), true);
    ASSERT_EQ(ini.get_child("section1").get<float>("c") == float(123.456), true);
}

TEST(ini, read_file)
{
    libcpp::ini cfg;
    cfg.read_file("cfg.ini");
    ASSERT_EQ(cfg.get_child("person").get<std::string>("name") == std::string("hanjingo"), true);
    ASSERT_EQ(cfg.get_child("person").get<int>("age") == 30, true);
    ASSERT_EQ(cfg.get_child("person").get<float>("income") == float(10000.123), true);
}

TEST(ini, write_file)
{
    libcpp::ini cfg;
    cfg.read_file("cfg.ini");
    cfg.get_child("person").put("email", "hehehunanchina@live.com");
    cfg.write_file("cfg.ini");

    libcpp::ini tmp;
    tmp.read_file("cfg.ini");
    ASSERT_EQ(tmp.get_child("person").get<std::string>("email") == std::string("hehehunanchina@live.com"), true);
}