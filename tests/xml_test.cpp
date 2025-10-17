#include <sstream>
#include <gtest/gtest.h>
#include <hj/encoding/xml.hpp>
#include <string>
#include <cstdio>

TEST(xml, load_from_string)
{
    const char *text = "<root><item>42</item></root>";
    hj::xml     x;
    ASSERT_TRUE(x.load(text));
    auto item = x.child("item");
    EXPECT_EQ(item.value(), "42");
}

TEST(xml, load_save_file)
{
    const char *filename = "test.xml";
    hj::xml     x;
    x.load("<root><foo>bar</foo></root>");
    ASSERT_TRUE(x.save_file(filename));
    hj::xml y;
    ASSERT_TRUE(y.load_file(filename));
    auto foo = y.child("foo");
    EXPECT_EQ(foo.value(), "bar");
    std::remove(filename);
}

TEST(xml, node_and_attr)
{
    hj::xml x;
    x.load("<root></root>");
    auto child = x.append_child("item");
    child.set_value("abc");
    child.set_attr("id", "123");
    EXPECT_EQ(child.value(), "abc");
    EXPECT_EQ(child.attr("id"), "123");
    EXPECT_EQ(child.name(), "item");
    child.set_name("item2");
    EXPECT_EQ(child.name(), "item2");
}

TEST(xml, attr_set_get)
{
    hj::xml x;
    x.load("<root></root>");
    auto child = x.append_child("item");
    child.set_attr("id", "abc");
    child.set_attr("type", "test");
    EXPECT_EQ(child.attr("id"), "abc");
    EXPECT_EQ(child.attr("type"), "test");
}

TEST(xml, remove_child)
{
    hj::xml x;
    x.load("<root><a/><b/><c/></root>");
    EXPECT_TRUE(x.remove_child("b"));
    EXPECT_TRUE(x.child("b").empty());
    EXPECT_FALSE(x.remove_child("not_exist"));
}

TEST(xml, empty_node)
{
    hj::xml x;
    EXPECT_FALSE(x.empty());
    x.load("<root></root>");
    EXPECT_FALSE(x.empty());
}

TEST(xml, str_serialize)
{
    hj::xml x;
    x.load("<root><foo>bar</foo></root>");
    std::string xmlstr = x.str();
    EXPECT_NE(xmlstr.find("<foo>bar</foo>"), std::string::npos);

    hj::xml y;
    y.load(xmlstr.c_str());
    EXPECT_EQ(y.child("foo").value(), "bar");
}