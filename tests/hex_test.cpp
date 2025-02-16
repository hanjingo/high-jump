#include <gtest/gtest.h>
#include <libcpp/encoding/hex.hpp>

TEST(hex, from_str)
{
    ASSERT_EQ(libcpp::hex::from_str<int>("FF"), 0xFF);
}

TEST(hex, to_str)
{
    ASSERT_STREQ(libcpp::hex::to_str(255).c_str(), "FF");
    ASSERT_STREQ(libcpp::hex::to_str(255, false).c_str(), "ff");
}