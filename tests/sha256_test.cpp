#include <gtest/gtest.h>
#include <libcpp/crypto/sha256.hpp>

TEST(sha256, encode)
{
    std::string ret = libcpp::sha256::encode(std::string("12345678"));
    ASSERT_EQ(ret.empty(), false);
}