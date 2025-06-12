#include <gtest/gtest.h>
#include <libcpp/crypto/md5.hpp>

TEST(md5, encode)
{
    std::string dst;
    ASSERT_EQ(libcpp::md5::encode(std::string("hehehunanchina@live.com"), dst), true);
    ASSERT_STREQ(libcpp::md5::to_hex(dst).c_str(), "2da6acfccab34c8ac05295d0f4262b84");
    ASSERT_STREQ(libcpp::md5::to_hex(dst, true).c_str(), "2DA6ACFCCAB34C8AC05295D0F4262B84");

}