#include <gtest/gtest.h>
#include <libcpp/encoding/hex.hpp>

TEST(hex, from)
{
    ASSERT_EQ(libcpp::hex::from<int>("F"), 0xF);
    ASSERT_EQ(libcpp::hex::from<int>("FF"), 0xFF);
    ASSERT_EQ(libcpp::hex::from<int>("FFF"), 0xFFF);

    std::istringstream in1("F");
    std::istringstream in2("FF");
    std::istringstream in3("FFF");
    std::ostringstream out1;
    std::ostringstream out2;
    std::ostringstream out3;

    libcpp::hex::from(out1, in1, libcpp::hex::upper_case);
    auto str1 = out1.str();
    ASSERT_EQ(str1.size(), 1);
    ASSERT_EQ(static_cast<unsigned char>(str1[0]), 0x0F);

    libcpp::hex::from(out2, in2, libcpp::hex::upper_case);
    auto str2 = out2.str();
    ASSERT_EQ(str2.size(), 1);
    ASSERT_EQ(static_cast<unsigned char>(str2[0]), 0xFF);

    libcpp::hex::from(out3, in3, libcpp::hex::upper_case);
    auto str3 = out3.str();
    ASSERT_EQ(str3.size(), 2);
    ASSERT_EQ(static_cast<unsigned char>(str3[0]), 0xFF);
    ASSERT_EQ(static_cast<unsigned char>(str3[1]), 0x0F);
}

TEST(hex, to)
{
    ASSERT_STREQ(libcpp::hex::to(255, libcpp::hex::upper_case).c_str(), "FF");
    ASSERT_STREQ(libcpp::hex::to(255, libcpp::hex::lower_case).c_str(), "ff");

    ASSERT_STREQ(libcpp::hex::to(4095, libcpp::hex::upper_case).c_str(), "FFF");
    ASSERT_STREQ(libcpp::hex::to(4095, libcpp::hex::lower_case).c_str(), "fff");

    std::istringstream in1("\xFF", std::ios::binary);
    std::ostringstream out1;
    libcpp::hex::to(out1, in1, libcpp::hex::upper_case);
    ASSERT_EQ(out1.str(), "FF");

    std::istringstream in2(std::string("\xFF\x0F", 2), std::ios::binary);
    std::ostringstream out2;
    libcpp::hex::to(out2, in2, libcpp::hex::upper_case);
    ASSERT_EQ(out2.str(), "FF0F");
}