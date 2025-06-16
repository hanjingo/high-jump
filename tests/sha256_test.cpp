#include <gtest/gtest.h>
#include <libcpp/crypto/sha256.hpp>
#include <libcpp/crypto/base64.hpp>

TEST(sha256, encode)
{
    std::string data;
    ASSERT_EQ(libcpp::sha256::encode(std::string("123456"), data), true);
    ASSERT_EQ(data.empty(), false);

    unsigned char src[] = { 'a', 'b', 'c'};
    unsigned char dst[1024];
    unsigned long dst_len = 1024;
    ASSERT_EQ(libcpp::sha256::encode(src, 3, dst, dst_len), true);
    std::string sha256_encoded;
    sha256_encoded.assign((char*)dst, dst_len);
    std::string b64_encoded;
    ASSERT_EQ(libcpp::base64::encode(sha256_encoded, b64_encoded), true);
    ASSERT_STREQ(b64_encoded.c_str(), "ungWv48Bz+pBQUDeXa4iI7ADYaOWF3qctBD/YfIAFa0=");
}