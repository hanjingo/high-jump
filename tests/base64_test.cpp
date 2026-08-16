#include <gtest/gtest.h>
#include <hj/crypto/base64.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// for OpenSSL compatibility on Windows
#ifdef _WIN32
extern "C" {
#include <openssl/applink.c>
}
#endif

TEST(base64, rfc4648_test_vectors)
{
    auto ok = hj::base64::error_code::ok;

    struct TestVector
    {
        std::string plain;
        std::string encoded;
    };

    const std::vector<TestVector> vectors = {{"", ""},
                                             {"f", "Zg=="},
                                             {"fo", "Zm8="},
                                             {"foo", "Zm9v"},
                                             {"foob", "Zm9vYg=="},
                                             {"fooba", "Zm9vYmE="},
                                             {"foobar", "Zm9vYmFy"}};

    for(const auto &tv : vectors)
    {
        std::string enc_out;
        ASSERT_EQ(hj::base64::encode(enc_out, tv.plain), ok);
        EXPECT_EQ(enc_out, tv.encoded);

        std::string dec_out;
        ASSERT_EQ(hj::base64::decode(dec_out, tv.encoded), ok);
        EXPECT_EQ(dec_out, tv.plain);

        if(!tv.encoded.empty())
        {
            EXPECT_TRUE(hj::base64::is_valid(tv.encoded));
        } else
        {
            EXPECT_FALSE(hj::base64::is_valid(tv.encoded));
        }
    }
}

TEST(base64, encode)
{
    auto ok = hj::base64::error_code::ok;

    // string -> base64 string
    std::string str_dst;
    ASSERT_EQ(
        hj::base64::encode(str_dst,
                           std::string("https://github.com/hanjingo/libcpp")),
        ok);
    EXPECT_STREQ(str_dst.c_str(),
                 "aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==");

    // bytes -> base64 bytes
    unsigned char buf_dst[1024];
    std::size_t   buf_dst_len = sizeof(buf_dst);
    unsigned char buf[]       = {'a', 'b', 'c', 'd', '1', '2', '3'};
    ASSERT_EQ(hj::base64::encode(buf_dst, buf_dst_len, buf, 7), ok);
    EXPECT_EQ(std::string(reinterpret_cast<char *>(buf_dst), buf_dst_len),
              "YWJjZDEyMw==");

    // stream -> stream
    std::istringstream iss("https://github.com/hanjingo/libcpp");
    std::ostringstream oss;
    ASSERT_EQ(hj::base64::encode(oss, iss), ok);
    EXPECT_EQ(oss.str(), "aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==");
}

TEST(base64, decode_valid)
{
    auto ok = hj::base64::error_code::ok;

    // base64 string -> string
    std::string str_dst;
    ASSERT_EQ(
        hj::base64::decode(
            str_dst,
            std::string("aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==")),
        ok);
    EXPECT_STREQ(str_dst.c_str(), "https://github.com/hanjingo/libcpp");

    // base64 byte -> bytes
    unsigned char buf_dst[1024];
    std::size_t   buf_dst_len = sizeof(buf_dst);
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
    ASSERT_EQ(hj::base64::decode(buf_dst, buf_dst_len, buf, 16), ok);
    EXPECT_EQ(std::string(reinterpret_cast<char *>(buf_dst), buf_dst_len),
              "hello licpp");

    // stream -> stream
    std::istringstream iss("aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==");
    std::ostringstream oss;
    ASSERT_EQ(hj::base64::decode(oss, iss), ok);
    EXPECT_EQ(oss.str(), "https://github.com/hanjingo/libcpp");
}

TEST(base64, decode_invalid_rfc4648)
{
    std::string str_dst;

    EXPECT_EQ(hj::base64::decode(str_dst, "Zg"),
              hj::base64::error_code::invalid_input);
    EXPECT_EQ(hj::base64::decode(str_dst, "Zg="),
              hj::base64::error_code::invalid_input);
    EXPECT_EQ(hj::base64::decode(str_dst, "TWFu="),
              hj::base64::error_code::invalid_input);

    EXPECT_EQ(hj::base64::decode(str_dst, "Zg==\n"),
              hj::base64::error_code::invalid_input);
    EXPECT_EQ(hj::base64::decode(str_dst, "TWFu!"),
              hj::base64::error_code::invalid_input);

    EXPECT_EQ(hj::base64::decode(str_dst, "TW=u"),
              hj::base64::error_code::decode_failed);
    EXPECT_EQ(hj::base64::decode(str_dst, "=WFu"),
              hj::base64::error_code::decode_failed);

    EXPECT_EQ(hj::base64::decode(str_dst, "Zh=="),
              hj::base64::error_code::decode_failed);
    EXPECT_EQ(hj::base64::decode(str_dst, "Zm9="),
              hj::base64::error_code::decode_failed);
}

TEST(base64, encode_file)
{
    auto ok = hj::base64::error_code::ok;

    std::string str_src = "./crypto.log";
    std::string str_dst = "./base64_file_test_encode.log";

    if(!std::filesystem::exists(str_src))
    {
        std::ofstream ofs(str_src, std::ios::binary);
        ofs << "RFC 4648 Base64 Stream File Verification Standard Data Block";
        ofs.close();
    }

    ASSERT_EQ(hj::base64::encode_file(str_dst, str_src), ok);
    EXPECT_TRUE(std::filesystem::exists(str_dst));

    std::filesystem::remove(str_dst);
}

TEST(base64, decode_file)
{
    auto ok = hj::base64::error_code::ok;

    std::string str_src = "./tmp_file_for_decode_test.log";
    std::string str_enc = "./tmp_file_encoded.log";
    std::string str_dec = "./tmp_file_decoded.log";

    {
        std::ofstream ofs(str_src, std::ios::binary);
        ofs << "RFC4648_Strict_Decode_File_Test_Data_1234567890!@#$%^&*()";
    }

    ASSERT_EQ(hj::base64::encode_file(str_enc, str_src), ok);

    ASSERT_EQ(hj::base64::decode_file(str_dec, str_enc), ok);

    {
        std::ifstream f_orig(str_src, std::ios::binary);
        std::ifstream f_dec(str_dec, std::ios::binary);

        std::string orig_content((std::istreambuf_iterator<char>(f_orig)),
                                 std::istreambuf_iterator<char>());
        std::string dec_content((std::istreambuf_iterator<char>(f_dec)),
                                std::istreambuf_iterator<char>());

        EXPECT_EQ(orig_content, dec_content);
    }

    std::filesystem::remove(str_src);
    std::filesystem::remove(str_enc);
    std::filesystem::remove(str_dec);
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
    EXPECT_FALSE(hj::base64::is_valid("TWFu="));
    EXPECT_FALSE(hj::base64::is_valid("TWF"));
    EXPECT_FALSE(hj::base64::is_valid(""));

    EXPECT_FALSE(hj::base64::is_valid("Zh=="));
    EXPECT_FALSE(hj::base64::is_valid("Zm9="));

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

        std::filesystem::remove("tmp_valid_b64.txt");
        std::filesystem::remove("tmp_invalid_b64.txt");
        std::filesystem::remove("tmp_odd_b64.txt");
        std::filesystem::remove("tmp_empty_b64.txt");
    }
}