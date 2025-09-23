#include <gtest/gtest.h>
#include <hj/misc/fix.hpp>

TEST(FixTest, BuildAndParse) {
    hj::fix_builder builder;
    builder.begin();
    builder.add_string(35, "D"); // MsgType
    builder.add_string(49, "SENDER"); // SenderCompID
    builder.add_string(56, "TARGET"); // TargetCompID
    builder.add_int(34, 123); // MsgSeqNum
    builder.add_char(54, '1'); // Side
    builder.end();

    std::string fixmsg = builder.str();

    hj::fix_parser parser(fixmsg.data(), fixmsg.size());
    ASSERT_TRUE(parser.valid());
    ASSERT_TRUE(parser.complete());
    EXPECT_EQ(parser.get_string(35), "D");
    EXPECT_EQ(parser.get_string(49), "SENDER");
    EXPECT_EQ(parser.get_string(56), "TARGET");
    EXPECT_EQ(parser.get_int<int>(34), 123);
    EXPECT_EQ(parser.get_char(54), '1');
}

