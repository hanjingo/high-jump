#include <gtest/gtest.h>
#include <libcpp/crypto/sha256.hpp>

TEST(sha256, encode)
{
    std::string data;
    libcpp::sha256::encode(std::string("123456"), data);
    ASSERT_EQ(data.empty(), false);
}