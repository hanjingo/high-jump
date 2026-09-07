#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <hj/encoding/xml.hpp>
#include <sstream>
#include <string>
#include <vector>

TEST(xml_options, parse_options_fluent_bitwise)
{
    const char *xml_with_comments_and_cdata = R"(
        <?xml version="1.0"?>
        <!-- my comment -->
        <root>
            <![CDATA[raw cdata content]]>
        </root>
    )";

    auto opts = hj::xml::parse_options::declaration
                | hj::xml::parse_options::comments
                | hj::xml::parse_options::cdata;

    hj::xml::document doc;
    ASSERT_TRUE(doc.load(xml_with_comments_and_cdata, opts));

    auto root = doc.root();
    EXPECT_FALSE(root.empty());
    EXPECT_EQ(root.first_child().type(), hj::xml::node_type::cdata);
    EXPECT_EQ(root.first_child().value(), "raw cdata content");
}

TEST(xml_mutation, set_api_returns_bool_and_propagates_failure)
{
    hj::xml::document doc;
    doc.load("<root><item id=\"1\">old_value</item></root>");

    auto item = doc.root().child("item");

    EXPECT_TRUE(item.set_name("new_item"));
    EXPECT_EQ(item.name(), "new_item");

    EXPECT_TRUE(item.set_value("new_value"));
    EXPECT_EQ(item.value(), "new_value");

    EXPECT_TRUE(item.set_attr("id", "100"));
    EXPECT_EQ(item.attr("id"), "100");

    EXPECT_TRUE(item.set_text("updated_text"));
    EXPECT_EQ(item.text().get(), "updated_text");

    hj::xml::node null_node;
    EXPECT_FALSE(null_node.set_name("invalid"));
    EXPECT_FALSE(null_node.set_value("invalid"));
    EXPECT_FALSE(null_node.set_text("invalid"));
    EXPECT_FALSE(null_node.set_attr("key", "val"));

    hj::xml::attribute null_attr;
    EXPECT_FALSE(null_attr.set_name("key"));
    EXPECT_FALSE(null_attr.set_value("val"));
}

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

TEST(xml_boundary, attribute_boundaries)
{
    hj::xml::document doc;
    doc.load("<root><child id=\"1\"/></root>");
    auto child = doc.root().child("child");

    EXPECT_TRUE(child.attr("missing").empty());
    EXPECT_FALSE(child.attribute_node("missing"));

    EXPECT_TRUE(child.set_attr("id", "1"));
    EXPECT_TRUE(child.set_attr("id", "2"));
    EXPECT_TRUE(child.set_attr("id", "3"));

    EXPECT_EQ(child.attr("id"), "3");

    int attr_count = 0;
    for(auto a : child.attributes())
    {
        (void) a;
        attr_count++;
    }
    EXPECT_EQ(attr_count, 1);
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

TEST(xml, empty_document)
{
    hj::xml::document doc;
    EXPECT_FALSE(doc.empty());
    EXPECT_FALSE(doc.root());
    EXPECT_TRUE(doc.root().empty());
    EXPECT_EQ(doc.root().type(), hj::xml::node_type::null);
}

TEST(xml_lifecycle, string_view_dangling_and_move_semantics)
{
    std::string_view val_view;
    std::string_view attr_view;

    hj::xml::document doc1;
    doc1.load("<root attr=\"hello\">world</root>");

    auto root = doc1.root();
    val_view  = root.value();
    attr_view = root.attr("attr");

    EXPECT_EQ(val_view, "world");
    EXPECT_EQ(attr_view, "hello");

    hj::xml::document doc2 = std::move(doc1);

    EXPECT_TRUE(doc1.empty());
    EXPECT_FALSE(doc1.root());

    EXPECT_EQ(val_view, "world");
    EXPECT_EQ(attr_view, "hello");

    EXPECT_EQ(doc2.root().value(), "world");
    EXPECT_EQ(doc2.root().attr("attr"), "hello");
}

TEST(xml_flags, format_flags_behavior)
{
    using hj::xml::format_flags;

    {
        hj::xml::document doc;
        doc.load("<root><item id=\"100\"/></root>");
        auto        flags = format_flags::raw | format_flags::no_declaration
                            | format_flags::attribute_single_quote;
        std::string xml   = doc.str("", flags);
        EXPECT_EQ(xml, "<root><item id='100'/></root>");
    }

    {
        hj::xml::document doc;
        doc.load("<root><item a=\"1\" b=\"2\"/></root>");
        auto        flags = format_flags::indent | format_flags::no_declaration
                            | format_flags::indent_attributes;
        std::string xml   = doc.str("  ", flags);
        EXPECT_NE(xml.find("\n  <item\n    a=\"1\"\n    b=\"2\" />"),
                  std::string::npos);
    }

    {
        hj::xml::document doc;
        doc.load("<root>A &amp; B</root>");

        std::string xml_default =
            doc.str("", format_flags::raw | format_flags::no_declaration);
        EXPECT_EQ(xml_default, "<root>A &amp; B</root>");

        std::string xml_no_escape =
            doc.str("",
                    format_flags::raw | format_flags::no_declaration
                        | format_flags::no_escapes);
        EXPECT_EQ(xml_no_escape, "<root>A & B</root>");
    }
}

TEST(xml_encoding, multi_encoding_support)
{
    {
        const char *utf8_text =
            "<?xml version=\"1.0\" "
            "encoding=\"UTF-8\"?><root><item>中文测试</item></root>";
        hj::xml::document doc;
        auto              res =
            doc.load_string(utf8_text, hj::xml::parse_options::by_default);
        ASSERT_TRUE(res);
        EXPECT_EQ(doc.root().child("item").value(), "中文测试");
    }

    {
        const unsigned char utf16_le_bom[] = {
            0xFF, 0xFE, '<', 0x00, '?', 0x00, 'x', 0x00, 'm', 0x00, 'l', 0x00,
            ' ',  0x00, 'v', 0x00, 'e', 0x00, 'r', 0x00, 's', 0x00, 'i', 0x00,
            'o',  0x00, 'n', 0x00, '=', 0x00, '"', 0x00, '1', 0x00, '.', 0x00,
            '0',  0x00, '"', 0x00, '?', 0x00, '>', 0x00,

            '<',  0x00, 'r', 0x00, 'o', 0x00, 'o', 0x00, 't', 0x00, '>', 0x00,

            '<',  0x00, 'd', 0x00, 'a', 0x00, 't', 0x00, 'a', 0x00, '>', 0x00,
            'O',  0x00, 'K', 0x00, '<', 0x00, '/', 0x00, 'd', 0x00, 'a', 0x00,
            't',  0x00, 'a', 0x00, '>', 0x00,

            '<',  0x00, '/', 0x00, 'r', 0x00, 'o', 0x00, 'o', 0x00, 't', 0x00,
            '>',  0x00};

        std::string_view  buffer(reinterpret_cast<const char *>(utf16_le_bom),
                                 sizeof(utf16_le_bom));
        hj::xml::document doc;
        auto              res = doc.load(buffer);
        ASSERT_TRUE(res);
        EXPECT_EQ(doc.root().child("data").value(), "OK");
    }

    {
        std::stringstream ss;
        hj::xml::document doc_write;
        doc_write.load("<root><city>Zürich</city></root>");

        bool save_ok = doc_write.save(ss,
                                      "\t",
                                      hj::xml::format_flags::by_default,
                                      hj::xml::encoding::latin1);
        ASSERT_TRUE(save_ok);

        ss.seekg(0);

        hj::xml::document doc_read;
        auto parse_res = doc_read.load(ss,
                                       hj::xml::parse_options::by_default,
                                       hj::xml::encoding::latin1);
        ASSERT_TRUE(parse_res);
        EXPECT_EQ(doc_read.root().child("city").value(), "Zürich");
    }
}

TEST(xml, parse_failure_paths)
{
    hj::xml::document doc;

    {
        const char *malformed_xml = "<root>";
        auto        res           = doc.load(malformed_xml);

        ASSERT_FALSE(res);
        EXPECT_EQ(res.status(), hj::xml::parse_status::end_element_mismatch);
        EXPECT_STREQ(res.description(), "Start-end tags mismatch");
        EXPECT_EQ(res.offset(), 5);
    }

    {
        const char *mismatch_xml = "<root><item></other></root>";
        auto        res          = doc.load(mismatch_xml);

        ASSERT_FALSE(res);
        EXPECT_EQ(res.status(), hj::xml::parse_status::end_element_mismatch);
        EXPECT_STREQ(res.description(), "Start-end tags mismatch");
        EXPECT_EQ(res.offset(), 14);
    }

    {
        const char *bad_attr_xml = "<root id=123/>";
        auto        res          = doc.load(bad_attr_xml);

        ASSERT_FALSE(res);
        EXPECT_EQ(res.status(), hj::xml::parse_status::bad_attribute);
        EXPECT_EQ(res.offset(), 9);
    }
}

TEST(xml_conversion, attr_as_and_value_as)
{
    const char *xml = R"(
        <server host="127.0.0.1" port="8080" timeout="1.5" enabled="true">
            <max_threads>64</max_threads>
            <ratio>0.85</ratio>
            <debug>yes</debug>
        </server>
    )";

    hj::xml::document doc;
    ASSERT_TRUE(doc.load(xml));
    auto server = doc.root();

    EXPECT_EQ(server.attr_as<int>("port"), 8080);
    EXPECT_EQ(server.attr_as<double>("timeout"), 1.5);
    EXPECT_EQ(server.attr_as<bool>("enabled"), true);
    EXPECT_EQ(server.attr_as<std::string>("host"), "127.0.0.1");

    EXPECT_FALSE(server.attr_as<int>("non_existent").has_value());
    EXPECT_FALSE(server.attr_as<int>("host").has_value());

    EXPECT_EQ(server.child("max_threads").value_as<int>(), 64);
    EXPECT_DOUBLE_EQ(server.child("ratio").value_as<double>().value(), 0.85);
    EXPECT_EQ(server.child("debug").value_as<bool>(), true);

    EXPECT_FALSE(server.child("missing").value_as<int>().has_value());
}