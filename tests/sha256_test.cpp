#include <gtest/gtest.h>
#include <libcpp/crypto/sha256.hpp>
#include <libcpp/crypto/base64.hpp>

TEST(sha256, encode)
{
    std::string data;
    ASSERT_EQ(libcpp::sha256::encode(data, std::string("123456")), true);
    ASSERT_EQ(data.empty(), false);

    unsigned char src[] = { 'a', 'b', 'c'};
    unsigned char dst[1024];
    unsigned long dst_len = 1024;
    ASSERT_EQ(libcpp::sha256::encode(dst, dst_len, src, 3), true);
    std::string sha256_encoded;
    sha256_encoded.assign((char*)dst, dst_len);
    std::string b64_encoded;
    ASSERT_EQ(libcpp::base64::encode(b64_encoded, sha256_encoded), true);
    ASSERT_STREQ("ungWv48Bz+pBQUDeXa4iI7ADYaOWF3qctBD/YfIAFa0=", b64_encoded.c_str());
}