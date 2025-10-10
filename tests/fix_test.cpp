#include <gtest/gtest.h>
#include <hj/misc/fix.hpp>

TEST(fix, build_and_arse)
{
    hj::fix::builder builder;
    builder.begin();
    builder.add_string(35, "D");      // MsgType
    builder.add_string(49, "SENDER"); // SenderCompID
    builder.add_string(56, "TARGET"); // TargetCompID
    builder.add_int(34, 123);         // MsgSeqNum
    builder.add_char(54, '1');        // Side
    builder.end();

    std::string fixmsg = builder.str();

    hj::fix::parser parser(fixmsg.data(), fixmsg.size());
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
    std::string     fixmsg = builder.str();
    hj::fix::parser parser(fixmsg.data(), fixmsg.size());
    EXPECT_EQ(parser.get_string(35), "8");
    EXPECT_EQ(parser.get_int<int>(34), 999);
}

TEST(fix, decimal_field)
{
    hj::fix::builder builder;
    builder.begin();
    builder.add_string(35, "D"); // required
    builder.add_decimal(44, 12345, -2);
    builder.end();
    std::string     fixmsg = builder.str();
    hj::fix::parser parser(fixmsg.data(), fixmsg.size());
    ASSERT_TRUE(parser.valid());
    ASSERT_TRUE(parser.complete());
    EXPECT_NE(parser.get_string(44).find("123.45"), std::string::npos);
}

TEST(fix, missing_tag)
{
    hj::fix::builder builder;
    builder.begin();
    builder.add_string(35, "D"); // required
    builder.add_string(49, "A");
    builder.end();
    std::string     fixmsg = builder.str();
    hj::fix::parser parser(fixmsg.data(), fixmsg.size());
    ASSERT_TRUE(parser.valid());
    ASSERT_TRUE(parser.complete());
    EXPECT_EQ(parser.get_string(999), "");
    EXPECT_EQ(parser.get_int<int>(888), 0);
    EXPECT_EQ(parser.get_char(777), '\0');
}

TEST(fix, empty_message)
{
    hj::fix::parser parser("", 0);
    EXPECT_EQ(parser.get_string(1), "");
    EXPECT_EQ(parser.get_int<int>(1), 0);
    EXPECT_EQ(parser.get_char(1), '\0');
}
