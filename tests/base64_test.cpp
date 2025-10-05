#include <gtest/gtest.h>
#include <hj/crypto/base64.hpp>
#include <filesystem>

// for OpenSSL compatibility on Windows
#ifdef _WIN32
extern "C" {
#include <openssl/applink.c>
}
#endif

TEST(base64, encode)
{
    auto ok = hj::base64::error_code::ok;

    // string -> base64 string
    std::string str_dst;
    ASSERT_TRUE(
        hj::base64::encode(str_dst,
                           std::string("https://github.com/hanjingo/libcpp"))
        == ok);
    ASSERT_STREQ(str_dst.c_str(),
                 "aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==");

    // bytes -> base64 string
    unsigned char buf_dst[1024];
    std::size_t   buf_dst_len = 1024;
    unsigned char buf[]       = {'a', 'b', 'c', 'd', '1', '2', '3'};
    ASSERT_TRUE(hj::base64::encode(buf_dst, buf_dst_len, buf, 7) == ok);
    ASSERT_STREQ(std::string((char *) buf_dst, buf_dst_len).c_str(),
                 "YWJjZDEyMw==");

    // stream -> stream
    std::istringstream iss("https://github.com/hanjingo/libcpp");
    std::ostringstream oss;
    ASSERT_TRUE(hj::base64::encode(oss, iss) == ok);
    ASSERT_EQ(oss.str(), "aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==");
}

TEST(base64, decode)
{
    auto ok = hj::base64::error_code::ok;

    // base64 string -> string
    std::string str_dst;
    ASSERT_TRUE(
        hj::base64::decode(
            str_dst,
            std::string("aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA=="))
        == ok);
    ASSERT_STREQ(str_dst.c_str(), "https://github.com/hanjingo/libcpp");

    // base64 byte -> string
    unsigned char buf_dst[1024];
    std::size_t   buf_dst_len = 1024;
    unsigned char buf[]       = {'a',
                                 'G',
                                 'V',
                                 's',
                                 'b',
                                 'G',
                                 '8',
                                 'g',
                                 'b',
                                 'G',
                                 'l',
                                 'j',
                                 'c',
                                 'H',
                                 'A',
                                 '='};
    ASSERT_TRUE(hj::base64::decode(buf_dst, buf_dst_len, buf, 16) == ok);
    ASSERT_STREQ(
        std::string(reinterpret_cast<char *>(buf_dst), buf_dst_len).c_str(),
        "hello licpp");

    // stream -> stream
    std::istringstream iss("aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==");
    std::ostringstream oss;
    ASSERT_TRUE(hj::base64::decode(oss, iss) == ok);
    ASSERT_EQ(oss.str(), "https://github.com/hanjingo/libcpp");
}

TEST(base64, encode_file)
{
    auto ok = hj::base64::error_code::ok;

    // base64 file -> file
    std::string str_src = "./crypto.log";
    std::string str_dst = "./base64_file_test_encode.log";
    if(!std::filesystem::exists(str_src))
    {
        GTEST_SKIP() << "skip test base64 encrypt_file not exist: " << str_src;
    }
    ASSERT_TRUE(hj::base64::encode_file(str_dst, str_src) == ok);
}

TEST(base64, decode_file)
{
    auto ok = hj::base64::error_code::ok;

    std::string str_src = "./crypto.log";
    std::string str_dst = "./base64_file_test_encode1.log";
    if(!std::filesystem::exists(str_src))
    {
        GTEST_SKIP() << "skip test base64 decode_file not exist: " << str_src;
    }
    ASSERT_TRUE(hj::base64::encode_file(str_dst, str_src) == ok);

    str_src = "./base64_file_test_encode1.log";
    str_dst = "./base64_file_test_decode.log";
    if(!std::filesystem::exists(str_src))
    {
        GTEST_SKIP() << "skip test base64 decode_file not exist: " << str_src;
    }
    ASSERT_TRUE(hj::base64::decode_file(str_dst, str_src) == ok);
}

TEST(base64, is_valid)
{
    EXPECT_TRUE(hj::base64::is_valid("TWFu")); // "Man"
    EXPECT_TRUE(hj::base64::is_valid("TWE=")); // "Ma"
    EXPECT_TRUE(hj::base64::is_valid("TQ==")); // "M"
    EXPECT_TRUE(hj::base64::is_valid(
        "aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA=="));

    EXPECT_FALSE(hj::base64::is_valid("TWFu!"));
    EXPECT_FALSE(hj::base64::is_valid("TWFu$"));
    EXPECT_FALSE(hj::base64::is_valid("TWFu==="));
    EXPECT_FALSE(hj::base64::is_valid("TWFu"
                                      "=")); // 5 bytes, invalid
    EXPECT_FALSE(hj::base64::is_valid("TWF"));
    EXPECT_FALSE(hj::base64::is_valid(""));

    {
        std::string valid_b64   = "TWFu";
        std::string invalid_b64 = "TWFu!";
        std::string odd_b64     = "TWF";
        std::string empty_b64   = "";

        {
            std::ofstream ofs("tmp_valid_b64.txt", std::ios::binary);
            ofs << valid_b64;
        }
        {
            std::ofstream ofs("tmp_invalid_b64.txt", std::ios::binary);
            ofs << invalid_b64;
        }
        {
            std::ofstream ofs("tmp_odd_b64.txt", std::ios::binary);
            ofs << odd_b64;
        }
        {
            std::ofstream ofs("tmp_empty_b64.txt", std::ios::binary);
            ofs << empty_b64;
        }

        EXPECT_TRUE(hj::base64::is_valid_file("tmp_valid_b64.txt"));
        EXPECT_FALSE(hj::base64::is_valid_file("tmp_invalid_b64.txt"));
        EXPECT_FALSE(hj::base64::is_valid_file("tmp_odd_b64.txt"));
        EXPECT_FALSE(hj::base64::is_valid_file("tmp_empty_b64.txt"));

        std::ifstream fin1("tmp_valid_b64.txt", std::ios::binary);
        EXPECT_TRUE(hj::base64::is_valid(fin1));
        fin1.close();

        std::ifstream fin2("tmp_invalid_b64.txt", std::ios::binary);
        EXPECT_FALSE(hj::base64::is_valid(fin2));
        fin2.close();

        std::ifstream fin3("tmp_odd_b64.txt", std::ios::binary);
        EXPECT_FALSE(hj::base64::is_valid(fin3));
        fin3.close();

        std::ifstream fin4("tmp_empty_b64.txt", std::ios::binary);
        EXPECT_FALSE(hj::base64::is_valid(fin4));
        fin4.close();

        std::remove("tmp_valid_b64.txt");
        std::remove("tmp_invalid_b64.txt");
        std::remove("tmp_odd_b64.txt");
        std::remove("tmp_empty_b64.txt");
    }
}