#include <gtest/gtest.h>
#include <libcpp/crypto/des.hpp>

TEST(des, ecb)
{
    ASSERT_STREQ(
        libcpp::des::ecb::decrypt(libcpp::des::ecb::encrypt("1234", "1"), "1").c_str(), 
        "1234");
}