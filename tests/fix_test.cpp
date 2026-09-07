#include <gtest/gtest.h>
#include <hj/misc/fix.hpp>

TEST(fix, build_and_parse)
{
    hj::fix::builder builder;
    builder.begin("FIX.4.4");
    builder.add_string(35, "D");      // MsgType
    builder.add_string(49, "SENDER"); // SenderCompID
    builder.add_string(56, "TARGET"); // TargetCompID
    builder.add_int(34, 123);         // MsgSeqNum
    builder.add_char(54, '1');        // Side
    builder.end();

    std::string_view fixmsg = builder.view();

    hj::fix::parser parser(fixmsg);
    ASSERT_TRUE(parser.valid());
    ASSERT_TRUE(parser.complete());

    EXPECT_EQ(parser.get_string(35), "D");
    EXPECT_EQ(parser.get_string(49), "SENDER");
    EXPECT_EQ(parser.get_string(56), "TARGET");
    EXPECT_EQ(parser.get_int<int>(34), 123);
    EXPECT_EQ(parser.get_char(54), '1');
}

TEST(fix, external_buffer_builder)
{
    char             buf[256] = {};
    hj::fix::builder builder(buf, sizeof(buf));
    builder.begin();
    builder.add_string(35, "8");
    builder.add_int(34, 999);
    builder.end();

    hj::fix::parser parser(builder.view());
    EXPECT_EQ(parser.get_string(35), "8");
    EXPECT_EQ(parser.get_int<int>(34), 999);
}

TEST(fix, decimal_field)
{
    hj::fix::builder builder;
    builder.begin();
    builder.add_string(35, "D");
    builder.add_decimal(44, 12345, -2);
    builder.end();

    hj::fix::parser parser(builder.view());
    ASSERT_TRUE(parser.valid());
    ASSERT_TRUE(parser.complete());

    auto price = parser.get_string(44);
    ASSERT_TRUE(price.has_value());
    EXPECT_EQ(price.value(), "123.45");
}

TEST(fix, missing_tag_handling)
{
    hj::fix::builder builder;
    builder.begin();
    builder.add_string(35, "D");
    builder.add_int(34, 0);
    builder.end();

    hj::fix::parser parser(builder.view());
    ASSERT_TRUE(parser.valid());

    EXPECT_EQ(parser.get_int<int>(34), 0);

    EXPECT_FALSE(parser.get_string(999).has_value());
    EXPECT_FALSE(parser.get_int<int>(888).has_value());
    EXPECT_FALSE(parser.get_char(777).has_value());
}

TEST(fix, for_each_single_pass_scan)
{
    hj::fix::builder builder;
    builder.begin();
    builder.add_string(35, "D");
    builder.add_int(34, 100);
    builder.end();

    hj::fix::parser parser(builder.view());

    int tag_count = 0;
    parser.for_each([&tag_count](int tag, const auto &value) {
        tag_count++;
        if(tag == 35)
        {
            EXPECT_EQ(value.as_string(), "D");
        }
    });

    EXPECT_GT(tag_count, 0);
}