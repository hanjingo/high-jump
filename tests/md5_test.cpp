#include <gtest/gtest.h>
#include <libcpp/crypto/md5.hpp>

TEST(md5, calc)
{
    std::string dst;
    libcpp::md5::encode("hehehunanchina@live.com", dst);
    ASSERT_STREQ(dst.c_str(), "2da6acfccab34c8ac05295d0f4262b84");

    libcpp::md5::encode("hehehunanchina@live.com", dst, libcpp::md5::upper_case);
    ASSERT_STREQ(dst.c_str(), "2DA6ACFCCAB34C8AC05295D0F4262B84");
}