#include <gtest/gtest.h>
#include <libcpp/crypto/sha.hpp>
#include <libcpp/crypto/base64.hpp>
#include <iomanip>

TEST(sha256, encode)
{
    std::string data;
    ASSERT_EQ(libcpp::sha::encode(data, std::string("123456"), libcpp::sha::algorithm::sha256), true);
    ASSERT_EQ(data.empty(), false);
    ASSERT_STREQ(data.c_str(), "\x8D\x96\x9E\xEFn\xCA\xD3\xC2\x9A:b\x92\x80\xE6\x86\xCF\f?]Z\x86\xAF\xF3\xCA\x12\x2\f\x92:\xDCl\x92");

    unsigned char src[] = { 'a', 'b', 'c'};
    unsigned char dst[1024];
    std::size_t dst_len = 1024;
    ASSERT_EQ(libcpp::sha::encode(dst, dst_len, src, 3, libcpp::sha::algorithm::sha256), true);
    std::string sha256_encoded;
    sha256_encoded.assign((char*)dst, dst_len);
    std::string b64_encoded;
    ASSERT_EQ(libcpp::base64::encode(b64_encoded, sha256_encoded), true);
    ASSERT_STREQ(b64_encoded.c_str(), "ungWv48Bz+pBQUDeXa4iI7ADYaOWF3qctBD/YfIAFa0=");

    // stream test
    std::stringstream in("123456");
    std::ostringstream out;
    libcpp::sha::encode(out, in, libcpp::sha::algorithm::sha256);
    std::string digest = out.str();
    ASSERT_EQ(digest.size(), 32);
    std::ostringstream hex;
    for (unsigned char c : digest)
        hex << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    std::string hex_digest = hex.str();
    ASSERT_STREQ(hex_digest.c_str(), "8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92");
}