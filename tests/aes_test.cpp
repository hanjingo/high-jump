#include <gtest/gtest.h>
#include <libcpp/crypto/aes.hpp>
#include <libcpp/log/logger.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>

TEST(aes, encode)
{
    std::string str_dst;
    ASSERT_EQ(libcpp::aes::encode(std::string("hello world"), str_dst, std::string("12345678"), std::string()), true);

    // // to hex style
    // std::stringstream ss;
    // for (unsigned char c : str_dst)
    //     ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(c);

    // ASSERT_STREQ(ss.str().c_str(), "28DBA02EB5F6DD476042DAEBFA59687A");
}

TEST(aes, decode)
{
    std::string str_dst;
    std::string str_encoded;
    std::string str_key("12345678");
    ASSERT_EQ(libcpp::aes::encode(std::string("hello world"), str_encoded, str_key, std::string()), true);
    ASSERT_EQ(libcpp::aes::decode(str_encoded, str_dst, str_key, std::string()), true);
    ASSERT_STREQ(str_dst.c_str(), "hello world");
}

TEST(aes, encode_file)
{
    // base64 file -> file
    libcpp::logger::instance()->clear_sink();
    libcpp::logger::instance()->add_sink(libcpp::logger::create_rotate_file_sink("./aes_file_test.log", 1024 * 1024 * 1024, 1, true));
    for (int i = 0; i < 1 * 1024 * 1024; i++)
        libcpp::logger::instance()->info("{}", i);
    libcpp::logger::instance()->flush();

    ASSERT_EQ(
        libcpp::aes::encode_file(
            std::string("./ecb_file_test.log"), 
            std::string("./ecb_file_test_encode.log"),
            std::string("12345678"),
            std::string("")), 
        true);
}

// TEST(des, ecb_decode_file)
// {
//     ASSERT_EQ(
//         libcpp::des::ecb::decode_file(
//             std::string("./ecb_file_test_encode.log"), 
//             std::string("12345678"), 
//             std::string("./ecb_file_test1.log")), 
//         true);
// }