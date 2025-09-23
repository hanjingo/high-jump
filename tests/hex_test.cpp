#include <gtest/gtest.h>
#include <hj/encoding/hex.hpp>

TEST(hex, decode)
{
    int n1 = hj::hex::decode<int>("F");
    ASSERT_EQ(n1, 0xF);
    int n2 = hj::hex::decode<int>("FF");
    ASSERT_EQ(n2, 0xFF);
    int n3 = hj::hex::decode<int>("FFF");
    ASSERT_EQ(n3, 0xFFF);


    std::istringstream in1("F");
    std::istringstream in2("FF");
    std::istringstream in3("FFF");
    std::ostringstream out1;
    std::ostringstream out2;
    std::ostringstream out3;

    hj::hex::decode(out1, in1);
    auto str1 = out1.str();
    ASSERT_EQ(str1.size(), 1);
    ASSERT_EQ(static_cast<unsigned char>(str1[0]), 0x0F);

    hj::hex::decode(out2, in2);
    auto str2 = out2.str();
    ASSERT_EQ(str2.size(), 1);
    ASSERT_EQ(static_cast<unsigned char>(str2[0]), 0xFF);

    hj::hex::decode(out3, in3);
    auto str3 = out3.str();
    ASSERT_EQ(str3.size(), 2);
    ASSERT_EQ(static_cast<unsigned char>(str3[0]), 0x0F);  // "0F"
    ASSERT_EQ(static_cast<unsigned char>(str3[1]), 0xFF);  // "FF"
}

TEST(hex, encode)
{
    ASSERT_STREQ(hj::hex::encode(255, true).c_str(), "FF");
    ASSERT_STREQ(hj::hex::encode(255, false).c_str(), "ff");

    ASSERT_STREQ(hj::hex::encode(4095, true).c_str(), "FFF");
    ASSERT_STREQ(hj::hex::encode(4095, false).c_str(), "fff");

    std::istringstream in1("\xFF", std::ios::binary);
    std::ostringstream out1;
    hj::hex::encode(out1, in1, true);
    ASSERT_EQ(out1.str(), "FF");

    std::istringstream in2(std::string("\xFF\x0F", 2), std::ios::binary);
    std::ostringstream out2;
    hj::hex::encode(out2, in2, true);
    ASSERT_EQ(out2.str(), "FF0F");
}

TEST(hex, is_valid)
{
    EXPECT_TRUE(hj::hex::is_valid("00"));
    EXPECT_TRUE(hj::hex::is_valid("0A"));
    EXPECT_TRUE(hj::hex::is_valid("ff"));
    EXPECT_TRUE(hj::hex::is_valid("ABCDEFabcdef0123456789"));

    EXPECT_FALSE(hj::hex::is_valid("0G"));
    EXPECT_FALSE(hj::hex::is_valid("xyz"));
    EXPECT_FALSE(hj::hex::is_valid("12 34"));
    EXPECT_FALSE(hj::hex::is_valid("12-34"));

    EXPECT_FALSE(hj::hex::is_valid("F"));
    EXPECT_FALSE(hj::hex::is_valid("123"));

    EXPECT_FALSE(hj::hex::is_valid(""));

    {
        std::string valid_hex = "AABBCCDDEEFF";
        std::string invalid_hex = "AABBCCGG";
        std::string odd_hex = "AABBCCD";
        std::string empty_hex = "";

        {
            std::ofstream ofs("tmp_valid_hex.txt", std::ios::binary);
            ofs << valid_hex;
        }
        {
            std::ofstream ofs("tmp_invalid_hex.txt", std::ios::binary);
            ofs << invalid_hex;
        }
        {
            std::ofstream ofs("tmp_odd_hex.txt", std::ios::binary);
            ofs << odd_hex;
        }
        {
            std::ofstream ofs("tmp_empty_hex.txt", std::ios::binary);
            ofs << empty_hex;
        }

        EXPECT_TRUE(hj::hex::is_valid_file("tmp_valid_hex.txt"));
        EXPECT_FALSE(hj::hex::is_valid_file("tmp_invalid_hex.txt"));
        EXPECT_FALSE(hj::hex::is_valid_file("tmp_odd_hex.txt"));
        EXPECT_FALSE(hj::hex::is_valid_file("tmp_empty_hex.txt"));

        std::ifstream fin1("tmp_valid_hex.txt", std::ios::binary);
        EXPECT_TRUE(hj::hex::is_valid(fin1));
        fin1.close();

        std::ifstream fin2("tmp_invalid_hex.txt", std::ios::binary);
        EXPECT_FALSE(hj::hex::is_valid(fin2));
        fin2.close();

        std::ifstream fin3("tmp_odd_hex.txt", std::ios::binary);
        EXPECT_FALSE(hj::hex::is_valid(fin3));
        fin3.close();

        std::ifstream fin4("tmp_empty_hex.txt", std::ios::binary);
        EXPECT_FALSE(hj::hex::is_valid(fin4));
        fin4.close();

        std::remove("tmp_valid_hex.txt");
        std::remove("tmp_invalid_hex.txt");
        std::remove("tmp_odd_hex.txt");
        std::remove("tmp_empty_hex.txt");
    }
}