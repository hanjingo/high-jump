#include <gtest/gtest.h>
#include <hj/crypto/sha.hpp>
#include <hj/crypto/base64.hpp>
#include <iomanip>

#include <fstream>
#include <cstdio>

TEST(sha256, encode)
{
    std::string data;
    ASSERT_EQ(hj::sha::encode(data,
                              std::string("123456"),
                              hj::sha::algorithm::sha256),
              true);
    ASSERT_EQ(data.empty(), false);
    ASSERT_STREQ(data.c_str(),
                 "\x8D\x96\x9E\xEFn\xCA\xD3\xC2\x9A:b\x92\x80\xE6\
\x86\xCF\f?]Z\x86\xAF\xF3\xCA\x12\x2\f\x92:\xDCl\x92");

    unsigned char src[] = {'a', 'b', 'c'};
    unsigned char dst[1024];
    std::size_t   dst_len = 1024;
    ASSERT_EQ(hj::sha::encode(dst, dst_len, src, 3, hj::sha::algorithm::sha256),
              true);
    std::string sha256_encoded;
    sha256_encoded.assign((char *) dst, dst_len);
    std::string b64_encoded;
    ASSERT_EQ(hj::base64::encode(b64_encoded, sha256_encoded), true);
    ASSERT_STREQ(b64_encoded.c_str(),
                 "ungWv48Bz+pBQUDeXa4iI7ADYaOWF3qctBD/YfIAFa0=");

    // stream test
    std::stringstream  in("123456");
    std::ostringstream out;
    hj::sha::encode(out, in, hj::sha::algorithm::sha256);
    std::string digest = out.str();
    ASSERT_EQ(digest.size(), 32);
    std::ostringstream hex;
    for(unsigned char c : digest)
        hex << std::hex << std::setw(2) << std::setfill('0') << (int) c;
    std::string hex_digest = hex.str();
    ASSERT_STREQ(hex_digest.c_str(),
                 "8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86\
aff3ca12020c923adc6c92");
}

TEST(sha256, encode_file)
{
    const char *input_file  = "sha_test_input.txt";
    const char *output_file = "sha_test_output.bin";
    {
        std::ofstream ofs(input_file, std::ios::binary);
        if(!ofs.is_open())
            GTEST_SKIP() << "Failed to create test input file.";
        ofs << "123456";
    }

    std::remove(output_file);
    ASSERT_TRUE(hj::sha::encode_file(output_file,
                                     input_file,
                                     hj::sha::algorithm::sha256));

    std::ifstream ifs(output_file, std::ios::binary);
    std::string   sha_bin((std::istreambuf_iterator<char>(ifs)),
                        std::istreambuf_iterator<char>());
    ASSERT_EQ(sha_bin.size(), 32);
    std::ostringstream hex;
    for(unsigned char c : sha_bin)
        hex << std::hex << std::setw(2) << std::setfill('0') << (int) c;
    std::string hex_digest = hex.str();
    ASSERT_STREQ(hex_digest.c_str(), "8d969eef6ecad3c29a3a629280e686cf0c3f5d\
5a86aff3ca12020c923adc6c92");

    std::remove(input_file);
    std::remove(output_file);
}