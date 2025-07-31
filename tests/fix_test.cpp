#include <gtest/gtest.h>
#include <libcpp/misc/fix.hpp>

using libcpp::fix_message;

TEST(FixTest, BuildAndParseMessage) {
    fix_message msg;
    msg.start_message();
    msg.add_field(11, "ORDER123"); // ClOrdID
    msg.add_field(55, "AAPL");     // Symbol
    msg.add_field(54, "1");        // Side
    msg.add_field(38, "100");      // OrderQty
    std::string fix_str = msg.finish_message();

    // Parse the message
    fix_message parsed;
    parsed.parse(fix_str);

    EXPECT_EQ(parsed.get_field(11), "ORDER123");
    EXPECT_EQ(parsed.get_field(55), "AAPL");
    EXPECT_EQ(parsed.get_field(54), "1");
    EXPECT_EQ(parsed.get_field(38), "100");
}

TEST(FixTest, InvalidMessage) {
    fix_message msg;
    msg.parse("8=FIX.4.2|9=12|35=0|10=000|"); // Invalid checksum and delimiter
    EXPECT_EQ(msg.get_field(35), "0");
    EXPECT_EQ(msg.get_field(999), "");      // Non-existent field
}

