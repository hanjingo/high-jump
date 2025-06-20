#include <gtest/gtest.h>
#include <libcpp/crypto/aes.hpp>
#include <libcpp/log/logger.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>

TEST(aes, encode)
{
    std::string str_src = "hello world";
    std::string str_dst;
    std::string key = "12345678";
    std::string iv;
    ASSERT_EQ(libcpp::aes::encode(str_dst, str_src, key, iv), true);

    // // to hex style
    // std::stringstream ss;
    // for (unsigned char c : str_dst)
    //     ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(c);

    // ASSERT_STREQ(ss.str().c_str(), "28DBA02EB5F6DD476042DAEBFA59687A");
}

TEST(aes, decode)
{
    std::string str_src = "hello world";
    std::string str_dst;
    std::string str_encoded;
    std::string str_key("12345678");
    ASSERT_EQ(libcpp::aes::encode(str_encoded, str_src, str_key, std::string()), true);
    ASSERT_EQ(libcpp::aes::decode(str_dst, str_encoded, str_key, std::string()), true);
    ASSERT_STREQ(str_dst.c_str(), "hello world");
}

TEST(aes, encode_file)
{
    // file -> encoded file
    libcpp::logger::instance()->clear_sink();
    libcpp::logger::instance()->add_sink(libcpp::logger::create_rotate_file_sink("./aes.log", 1 * 1024 * 1024, 1, true));
    for (int i = 0; i < 1 * 1024; i++)
        libcpp::logger::instance()->info("{}", i);
    libcpp::logger::instance()->flush();

    std::string str_src = "./aes.log";
    std::string str_dst = "./aes_encode.log";
    std::string key = "12345678";
    std::string iv;
    ASSERT_EQ(libcpp::aes::encode_file(str_dst, str_src, key, iv), true); 
}

TEST(des, decode_file)
{
    // encoded file -> file
    std::string str_dst = "./aes_decode.log";
    std::string str_src = "./aes_encode.log";
    std::string key = "12345678";
    std::string iv;
    ASSERT_EQ(libcpp::aes::encode_file(str_dst, str_src, key, iv), true);
}