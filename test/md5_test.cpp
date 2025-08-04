#include <gtest/gtest.h>
#include <libcpp/crypto/md5.hpp>

TEST(md5, encode)
{
    std::string dst;
    ASSERT_EQ(libcpp::md5::encode(dst, std::string("hehehunanchina@live.com")),
              true);
    ASSERT_STREQ(libcpp::md5::to_hex(dst).c_str(),
                 "2da6acfccab34c8ac05295d0f4262b84");
    ASSERT_STREQ(libcpp::md5::to_hex(dst, true).c_str(),
                 "2DA6ACFCCAB34C8AC05295D0F4262B84");

    // test for stream
    std::istringstream in("hehehunanchina@live.com");
    std::ostringstream out;
    libcpp::md5::encode(out, in);
    ASSERT_STREQ(libcpp::md5::to_hex(out.str()).c_str(),
                 "2da6acfccab34c8ac05295d0f4262b84");
}

TEST(md5, to_hex)
{
    std::string str = "hehehunanchina@live.com";
    ASSERT_STREQ(libcpp::md5::to_hex(str).c_str(),
                 "6865686568756e616e6368696e61406c");
}

TEST(md5, encode_len_reserve)
{
    ASSERT_EQ(libcpp::md5::encode_len_reserve(), MD5_DIGEST_LENGTH);
}