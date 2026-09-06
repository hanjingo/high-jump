#include <sstream>
#include <gtest/gtest.h>
#include <hj/encoding/xml.hpp>
#include <string>
#include <cstdio>

TEST(xml, load_from_string)
{
    const char       *text = "<root><item>42</item></root>";
    hj::xml::document x;
    ASSERT_TRUE(x.load(text));
    auto root = x.root();
    auto item = root.child("item");
    EXPECT_EQ(item.value(), "42");
}

TEST(xml, load_save_file)
{
    const char       *filename = "test.xml";
    hj::xml::document x;
    x.load("<root><foo>bar</foo></root>");
    ASSERT_TRUE(x.save_file(filename));

    hj::xml::document y;
    ASSERT_TRUE(y.load_file(filename));
    auto root = y.root();
    auto foo  = root.child("foo");
    EXPECT_EQ(foo.value(), "bar");
    std::remove(filename);
}

TEST(xml, node_and_attr)
{
    hj::xml::document x;
    x.load("<root></root>");
    auto root  = x.root();
    auto child = root.append_child("item");
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
    hj::xml::document x;
    x.load("<root></root>");
    auto root  = x.root();
    auto child = root.append_child("item");
    child.set_attr("id", "abc");
    child.set_attr("type", "test");
    EXPECT_EQ(child.attr("id"), "abc");
    EXPECT_EQ(child.attr("type"), "test");
}

TEST(xml, remove_child)
{
    hj::xml::document x;
    x.load("<root><a/><b/><c/></root>");
    auto root = x.root();
    EXPECT_TRUE(root.remove_child("b"));
    EXPECT_TRUE(root.child("b").empty());
    EXPECT_FALSE(root.remove_child("not_exist"));
}

TEST(xml, empty_node)
{
    hj::xml::document x;
    EXPECT_FALSE(x.empty());
    x.load("<root></root>");
    EXPECT_FALSE(x.empty());
}

TEST(xml, str_serialize)
{
    hj::xml::document x;
    x.load("<root><foo>bar</foo></root>");
    std::string xmlstr = x.str();
    EXPECT_NE(xmlstr.find("<foo>bar</foo>"), std::string::npos);

    hj::xml::document y;
    y.load(xmlstr.c_str());
    EXPECT_EQ(y.root().child("foo").value(), "bar");
}

TEST(xml_refactored, load_with_declaration)
{
    const char       *text = "<?xml version=\"1.0\" "
                             "encoding=\"UTF-8\"?><root><item>42</item></root>";
    hj::xml::document doc;
    auto              res = doc.load_string(text);
    ASSERT_TRUE(res);

    auto root = doc.root();
    EXPECT_EQ(root.name(), "root");
    EXPECT_EQ(root.child("item").value(), "42");
}

TEST(xml_refactored, set_attr_no_duplicate)
{
    hj::xml::document doc;
    doc.load_string("<root></root>");
    auto child = doc.root().append_child("item");

    child.set_attr("id", "100");
    child.set_attr("id", "200");

    EXPECT_EQ(child.attr("id"), "200");
    using format_flags = hj::xml::format_flags;
    std::string xml_str =
        doc.str("",
                static_cast<format_flags>(
                    static_cast<unsigned int>(format_flags::raw)
                    | static_cast<unsigned int>(format_flags::no_declaration)));
    EXPECT_EQ(xml_str, "<root><item id=\"200\"/></root>");
}

TEST(xml_refactored, move_semantics)
{
    hj::xml::document doc1;
    doc1.load_string("<root><foo>bar</foo></root>");

    hj::xml::document doc2 = std::move(doc1);
    EXPECT_TRUE(doc1.empty());
    EXPECT_FALSE(doc2.empty());
    EXPECT_EQ(doc2.root().child("foo").value(), "bar");
}