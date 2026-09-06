#include <gtest/gtest.h>
#include <hj/encoding/hex.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdio>

TEST(hex, decode)
{
    int n1 = hj::hex::decode<int>("0F");
    ASSERT_EQ(n1, 0xF);
    int n2 = hj::hex::decode<int>("FF");
    ASSERT_EQ(n2, 0xFF);
    int n3 = hj::hex::decode<int>("0FFF");
    ASSERT_EQ(n3, 0xFFF);

    int n_odd = hj::hex::decode<int>("F");
    ASSERT_EQ(n_odd, 0);

    std::string str_decoded = hj::hex::decode<std::string>("48656C6C6F");
    ASSERT_EQ(str_decoded, "Hello");

    std::vector<uint8_t> vec_decoded =
        hj::hex::decode<std::vector<uint8_t>>("010203FF");
    ASSERT_EQ(vec_decoded.size(), 4);
    ASSERT_EQ(vec_decoded[0], 0x01);
    ASSERT_EQ(vec_decoded[3], 0xFF);

    std::istringstream in1("0F");
    std::istringstream in2("FF");
    std::istringstream in3("0FFF");
    std::ostringstream out1;
    std::ostringstream out2;
    std::ostringstream out3;

    EXPECT_TRUE(hj::hex::decode(out1, in1));
    auto res1 = out1.str();
    ASSERT_EQ(res1.size(), 1);
    ASSERT_EQ(static_cast<unsigned char>(res1[0]), 0x0F);

    EXPECT_TRUE(hj::hex::decode(out2, in2));
    auto res2 = out2.str();
    ASSERT_EQ(res2.size(), 1);
    ASSERT_EQ(static_cast<unsigned char>(res2[0]), 0xFF);

    EXPECT_TRUE(hj::hex::decode(out3, in3));
    auto res3 = out3.str();
    ASSERT_EQ(res3.size(), 2);
    ASSERT_EQ(static_cast<unsigned char>(res3[0]), 0x0F);
    ASSERT_EQ(static_cast<unsigned char>(res3[1]), 0xFF);

    std::istringstream in_odd("FFF");
    std::ostringstream out_odd;
    EXPECT_FALSE(hj::hex::decode(out_odd, in_odd));
}

TEST(hex, encode)
{
    ASSERT_STREQ(hj::hex::encode(255, true).c_str(), "FF");
    ASSERT_STREQ(hj::hex::encode(255, false).c_str(), "ff");

    ASSERT_STREQ(hj::hex::encode(4095, true).c_str(), "0FFF");
    ASSERT_STREQ(hj::hex::encode(4095, false).c_str(), "0fff");

    ASSERT_EQ(hj::hex::encode("Hello", true), "48656C6C6F");

    std::vector<uint8_t> bytes = {0x00, 0x1A, 0x2B, 0xFF};
    ASSERT_EQ(hj::hex::encode(bytes, false), "001a2bff");

    std::istringstream in1("\xFF", std::ios::binary);
    std::ostringstream out1;
    EXPECT_TRUE(hj::hex::encode(out1, in1, true));
    ASSERT_EQ(out1.str(), "FF");

    std::istringstream in2(std::string("\xFF\x0F", 2), std::ios::binary);
    std::ostringstream out2;
    EXPECT_TRUE(hj::hex::encode(out2, in2, true));
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
        std::string valid_hex   = "AABBCCDDEEFF";
        std::string invalid_hex = "AABBCCGG";
        std::string odd_hex     = "AABBCCD";
        std::string empty_hex   = "";

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

TEST(hex, file_encode_decode)
{
    std::string raw_data =
        "High-Jump C++17 Hex Benchmark & Correctness Test Data Stream.";
    std::string in_file  = "tmp_raw_data.bin";
    std::string hex_file = "tmp_encoded.hex";
    std::string out_file = "tmp_decoded.bin";

    {
        std::ofstream ofs(in_file, std::ios::binary);
        ofs << raw_data;
    }

    EXPECT_TRUE(hj::hex::encode_file(hex_file, in_file, true));
    EXPECT_TRUE(hj::hex::is_valid_file(hex_file));

    EXPECT_TRUE(hj::hex::decode_file(out_file, hex_file));

    std::ifstream fin(out_file, std::ios::binary);
    std::string   restored_data((std::istreambuf_iterator<char>(fin)),
                                std::istreambuf_iterator<char>());
    ASSERT_EQ(raw_data, restored_data);

    fin.close();
    std::remove(in_file.c_str());
    std::remove(hex_file.c_str());
    std::remove(out_file.c_str());
}

TEST(hex, stream_operators)
{
    std::ostringstream oss;
    oss << hj::hex::encoder("Hello", true);
    ASSERT_EQ(oss.str(), "48656C6C6F");

    std::ostringstream oss_lower;
    oss_lower << hj::hex::encoder("Hello", false);
    ASSERT_EQ(oss_lower.str(), "48656c6c6f");

    std::vector<uint8_t> bytes = {0x01, 0x02, 0xFF};
    std::ostringstream   oss_vec;
    oss_vec << hj::hex::encoder(bytes, true);
    ASSERT_EQ(oss_vec.str(), "0102FF");

    std::istringstream iss("48656C6C6F");
    std::string        decoded_str;
    iss >> hj::hex::decoder(decoded_str);
    ASSERT_EQ(decoded_str, "Hello");
    EXPECT_FALSE(iss.fail());

    std::istringstream iss_bad("48656C6C6G");
    std::string        bad_str;
    iss_bad >> hj::hex::decoder(bad_str);
    EXPECT_TRUE(iss_bad.fail());
}