#include <gtest/gtest.h>
#include <libcpp/encoding/hex.hpp>

TEST (hex, decode)
{
    int n1 = libcpp::hex::decode<int> ("F");
    ASSERT_EQ (n1, 0xF);
    int n2 = libcpp::hex::decode<int> ("FF");
    ASSERT_EQ (n2, 0xFF);
    int n3 = libcpp::hex::decode<int> ("FFF");
    ASSERT_EQ (n3, 0xFFF);


    std::istringstream in1 ("F");
    std::istringstream in2 ("FF");
    std::istringstream in3 ("FFF");
    std::ostringstream out1;
    std::ostringstream out2;
    std::ostringstream out3;

    libcpp::hex::decode (out1, in1);
    auto str1 = out1.str ();
    ASSERT_EQ (str1.size (), 1);
    ASSERT_EQ (static_cast<unsigned char> (str1[0]), 0x0F);

    libcpp::hex::decode (out2, in2);
    auto str2 = out2.str ();
    ASSERT_EQ (str2.size (), 1);
    ASSERT_EQ (static_cast<unsigned char> (str2[0]), 0xFF);

    libcpp::hex::decode (out3, in3);
    auto str3 = out3.str ();
    ASSERT_EQ (str3.size (), 2);
    ASSERT_EQ (static_cast<unsigned char> (str3[0]), 0x0F); // "0F"
    ASSERT_EQ (static_cast<unsigned char> (str3[1]), 0xFF); // "FF"
}

TEST (hex, encode)
{
    ASSERT_STREQ (libcpp::hex::encode (255, true).c_str (), "FF");
    ASSERT_STREQ (libcpp::hex::encode (255, false).c_str (), "ff");

    ASSERT_STREQ (libcpp::hex::encode (4095, true).c_str (), "FFF");
    ASSERT_STREQ (libcpp::hex::encode (4095, false).c_str (), "fff");

    std::istringstream in1 ("\xFF", std::ios::binary);
    std::ostringstream out1;
    libcpp::hex::encode (out1, in1, true);
    ASSERT_EQ (out1.str (), "FF");

    std::istringstream in2 (std::string ("\xFF\x0F", 2), std::ios::binary);
    std::ostringstream out2;
    libcpp::hex::encode (out2, in2, true);
    ASSERT_EQ (out2.str (), "FF0F");
}