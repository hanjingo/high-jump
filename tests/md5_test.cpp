#include <gtest/gtest.h>
#include <libcpp/crypto/md5.hpp>

#include <fstream>
#include <cstdio>

bool create_md5_test_file(const char* filename, const std::string& content)
{
    std::ofstream ofs(filename, std::ios::binary);
    if (ofs.is_open())
    {
        ofs << content;
        ofs.close();
        return true;
    }

    return false;
}

TEST(md5, encode)
{
    std::string dst;
    ASSERT_EQ(libcpp::md5::encode(dst, std::string("hehehunanchina@live.com")), true);
    ASSERT_STREQ(libcpp::md5::to_hex(dst).c_str(), "2da6acfccab34c8ac05295d0f4262b84");
    ASSERT_STREQ(libcpp::md5::to_hex(dst, true).c_str(), "2DA6ACFCCAB34C8AC05295D0F4262B84");

    // test for stream
    std::istringstream in("hehehunanchina@live.com");
    std::ostringstream out;
    libcpp::md5::encode(out, in);
    ASSERT_STREQ(libcpp::md5::to_hex(out.str()).c_str(), "2da6acfccab34c8ac05295d0f4262b84");
}

TEST(md5, encode_file)
{
    const char* input_file = "md5_test_input.txt";
    const char* output_file = "md5_test_output.bin";
    if (!create_md5_test_file(input_file, "hehehunanchina@live.com"))
        GTEST_SKIP() << "Failed to create test input file.";

    std::remove(output_file);
    ASSERT_TRUE(libcpp::md5::encode_file(output_file, input_file));

    std::ifstream ifs(output_file, std::ios::binary);
    std::string md5_bin((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ASSERT_EQ(md5_bin.size(), MD5_DIGEST_LENGTH);
    ASSERT_STREQ(libcpp::md5::to_hex(md5_bin).c_str(), "2da6acfccab34c8ac05295d0f4262b84");

    std::remove(input_file);
    std::remove(output_file);
}

TEST(md5, to_hex)
{
    std::string str = "hehehunanchina@live.com";
    ASSERT_STREQ(libcpp::md5::to_hex(str).c_str(), "6865686568756e616e6368696e61406c");
}

TEST(md5, encode_len_reserve)
{
    ASSERT_EQ(libcpp::md5::encode_len_reserve(), MD5_DIGEST_LENGTH);
}