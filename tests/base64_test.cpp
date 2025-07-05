#include <gtest/gtest.h>
#include <libcpp/crypto/base64.hpp>

#include <libcpp/log/logger.hpp>

// for OpenSSL compatibility on Windows
#ifdef _WIN32
extern "C" 
{
    #include <openssl/applink.c>
}
#endif

TEST(base64, encode)
{
    // string -> base64 string
    std::string str_dst;
    ASSERT_EQ(libcpp::base64::encode(str_dst, std::string("https://github.com/hanjingo/libcpp")), true);
    ASSERT_STREQ(str_dst.c_str(), "aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==");

    // bytes -> base64 string
    unsigned char buf_dst[1024];
    std::size_t buf_dst_len = 1024;
    unsigned char buf[] = { 'a', 'b', 'c', 'd', '1', '2', '3' };
    ASSERT_EQ(libcpp::base64::encode(buf_dst, buf_dst_len, buf, 7), true);
    ASSERT_STREQ(std::string((char*)buf_dst, buf_dst_len).c_str(), "YWJjZDEyMw==");

    // stream -> stream
    std::istringstream iss("https://github.com/hanjingo/libcpp");
    std::ostringstream oss;
    ASSERT_TRUE(libcpp::base64::encode(oss, iss));
    ASSERT_EQ(oss.str(), "aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==");
}

TEST(base64, decode)
{
    // base64 string -> string
    std::string str_dst;
    ASSERT_EQ(libcpp::base64::decode(str_dst, std::string("aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==")), true);
    ASSERT_STREQ(str_dst.c_str(), "https://github.com/hanjingo/libcpp");

    // base64 byte -> string
    unsigned char buf_dst[1024];
    std::size_t buf_dst_len = 1024;
    unsigned char buf[] = { 'a', 'G', 'V', 's', 'b', 'G', '8', 'g', 'b', 'G', 'l', 'j', 'c', 'H', 'A', '=' };
    ASSERT_EQ(libcpp::base64::decode(buf_dst, buf_dst_len, buf, 16), true);
    ASSERT_STREQ(std::string(reinterpret_cast<char*>(buf_dst), buf_dst_len).c_str(), "hello licpp");

    // stream -> stream
    std::istringstream iss("aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==");
    std::ostringstream oss;
    ASSERT_TRUE(libcpp::base64::decode(oss, iss));
    ASSERT_EQ(oss.str(), "https://github.com/hanjingo/libcpp");
}

TEST(base64, encode_file)
{
    // base64 file -> file
    libcpp::logger::instance()->clear_sink();
    libcpp::logger::instance()->add_sink(libcpp::logger::create_rotate_file_sink("./base64_file_test.log", 10 * 1024 * 1024, 1, true));
    for (int i = 0; i < 1 * 1024; i++)
        libcpp::logger::instance()->info("{}", i);
    libcpp::logger::instance()->flush();

    ASSERT_EQ(libcpp::base64::encode_file(std::string("./base64_file_test_encode.log"), 
        std::string("./base64_file_test.log")), true);
}

TEST(base64, decode_file)
{
    ASSERT_EQ(libcpp::base64::decode_file(std::string("./base64_file_test_decode.log"), std::string("./base64_file_test_encode.log")), true);
}