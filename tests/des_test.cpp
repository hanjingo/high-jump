#include <gtest/gtest.h>
#include <libcpp/crypto/des.hpp>
#include <libcpp/crypto/md5.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>

std::string calc_file_md5(const std::string& file_path)
{
    std::ifstream ifs(file_path, std::ios::binary);
    if (!ifs.is_open())
        return "";

    std::string md5_bin;
    libcpp::md5::encode(md5_bin, ifs);
    return libcpp::md5::to_hex(md5_bin);
}

std::string to_hex(const std::string& src)
{
    std::stringstream ss;
    for (unsigned char c : src)
        ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(c);

    return ss.str();
}

std::string to_bytes(const std::string& hex) 
{
    std::string bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        unsigned int byte;
        std::stringstream ss;
        ss << std::hex << hex.substr(i, 2);
        ss >> byte;
        bytes.push_back(static_cast<char>(byte));
    }
    return bytes;
}

TEST(des, encrypt)
{
    std::string str_src("hello world 1234");
    std::string str_dst;
    std::string key("12345678");
    std::string iv("abcdefgh");

    // for stream ECB padding PKCS#5 test
    str_src = "hello world 1";
    std::istringstream in(str_src);
    std::ostringstream out;
    ASSERT_EQ(libcpp::des::encrypt(out, 
                                  in, 
                                  reinterpret_cast<const unsigned char*>(key.c_str()), 
                                  key.size(), 
                                  libcpp::des::mode::ecb, 
                                  libcpp::des::padding::pkcs5), 
        true);
    ASSERT_STREQ(to_hex(out.str()).c_str(), "28DBA02EB5F6DD47AA291D16D82146BF");

    // ECB padding PKCS#5
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::pkcs5), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "28DBA02EB5F6DD475D82E3681C83BB77");

    // ECB padding PKCS#7
    str_dst.clear();
    str_src = "hello world 1";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::pkcs7), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "28DBA02EB5F6DD47AA291D16D82146BF");

    // ECB padding 0
    str_dst.clear();
    str_src = "hello world 12";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::zero), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "28DBA02EB5F6DD47535FFDB06574A87D");

    // ECB padding ISO10126 （random result）
    str_dst.clear();
    str_src = "hello world 123";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::iso10126), true);

    // ECB padding ANSIX923
    str_dst.clear();
    str_src = "hello world 1234";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::ansix923), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "28DBA02EB5F6DD47BABFF98EFA6DB628030116F7E552E7B6");

    // ECB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = "hello world 12345";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::iso_iec_7816_4), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "28DBA02EB5F6DD47BABFF98EFA6DB628DE3491731DCB353C");

    // ECB padding NOPADDING
    str_dst.clear();
    str_src = "hello world 1234";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::no_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "28DBA02EB5F6DD47BABFF98EFA6DB628");

    str_dst.clear();
    str_src = "hello world 12345";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::no_padding), false);

    // CBC padding PKCS#5
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "B72D0DC9E9433B0373FB9C7373EEE4D1");

    // CBC padding PKCS#7
    str_dst.clear();
    str_src = "hello world 1";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "B72D0DC9E9433B03849FE655EE80DB80");

    // CBC padding 0
    str_dst.clear();
    str_src = "hello world 12";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "B72D0DC9E9433B0353E3BE76EB413AB0");

    // CBC padding ISO10126 （random result）
    str_dst.clear();
    str_src = "hello world 123";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::iso10126, iv), true);

    // CBC padding ANSIX923
    str_dst.clear();
    str_src = "hello world 1234";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "B72D0DC9E9433B031400731AF6B95E3BDF39AA4FBB8F6F63");

    // CBC padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = "hello world 12345";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "B72D0DC9E9433B031400731AF6B95E3BC113907DECCE3490");

    // CBC padding NOPADDING
    str_dst.clear();
    str_src = "hello world 1234";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "B72D0DC9E9433B031400731AF6B95E3B");

    // CFB padding PKCS#5
    str_dst.clear();
    str_src = "hello world 1234";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC248142DD0763FF0EC92210E9B43584E0");

    // CFB padding PKCS#7
    str_dst.clear();
    str_src = "hello world 123";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC248142DD0763FF3B");

    // CFB padding 0
    str_dst.clear();
    str_src = "hello world 12";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC248142DD0763CC3A");

    // CFB padding ISO10126 （random result）
    str_dst.clear();
    str_src = "hello world 1";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::iso10126, iv), true);

    // CFB padding ANSIX923
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC248142FD3651CC3F");

    // CFB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = "hello world 1";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC248142DD07D1CC3A");

    // CFB padding NOPADDING
    str_dst.clear();
    str_src = "hello world 1234";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC248142DD0763FF0E");

    // OFB padding PKCS#5
    str_dst.clear();
    str_src = "libcpp";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "F8BD2108B3C5B491");

    // OFB padding PKCS#7
    str_dst.clear();
    str_src = "libcpp1";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "F8BD2108B3C58792");

    // OFB padding 0
    str_dst.clear();
    str_src = "libcpp12";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "F8BD2108B3C587A14B6BB3A9E45E6967");

    // OFB padding ISO10126 （random result）
    str_dst.clear();
    str_src = "libcpp123";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::iso10126, iv), true);

    // OFB padding ANSIX923
    str_dst.clear();
    str_src = "libcpp1234";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "F8BD2108B3C587A1785FB3A9E45E6961");

    // OFB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = "libcpp12345";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "F8BD2108B3C587A1785F8629E45E6967");

    // OFB padding NOPADDING
    str_dst.clear();
    str_src = "1234567812345678";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "A5E6705FF68381AB7A59809DD1685E5F");

    str_dst.clear();
    str_src = "123456781234567";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::no_padding, iv), false);

    // CTR padding PKCS#5
    str_dst.clear();
    str_src = "1";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "A5D3446CC4B2B194");

    // CTR padding PKCS#7
    str_dst.clear();
    str_src = "12";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "A5E6456DC5B3B095");

    // CTR padding 0
    str_dst.clear();
    str_src = "123";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "A5E6706BC3B5B693");

    // CTR padding ISO10126 （random result）
    str_dst.clear();
    str_src = "1234";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::iso10126, iv), true);

    // CTR padding ANSIX923
    str_dst.clear();
    str_src = "12345";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "A5E6705FF6B5B690");

    // CTR padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = "123456";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "A5E6705FF6833693");

    // CTR padding NOPADDING
    str_dst.clear();
    str_src = "1234567";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::no_padding, iv), false);

    str_dst.clear();
    str_src = "12345678";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "A5E6705FF68381AB");
}

TEST(des, encrypt_n)
{
    std::string str_src("");
    std::string str_dst;
    std::string key("12345678abcdefgh00000000");
    std::string iv("abcdefgh");

    // for stream ECB padding PKCS#5 test
    str_src = "hello world";
    std::istringstream in(str_src);
    std::ostringstream out;
    ASSERT_EQ(libcpp::des::encrypt(out, 
                                  in, 
                                  reinterpret_cast<const unsigned char*>(key.c_str()), 
                                  key.size(), 
                                  libcpp::des::mode::ecb, 
                                  libcpp::des::padding::pkcs5), 
        true);
    ASSERT_STREQ(to_hex(out.str()).c_str(), "3E4665C52F935552F1C9C86E67880CBB");

    // ECB padding PKCS#5
    str_dst.clear();
    str_src = "1";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::pkcs5), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "40C3AD88D21D2BCA");

    // ECB padding PKCS#7
    str_dst.clear();
    str_src = "12";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::pkcs7), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "BFA6FD1DE34FB600");

    // ECB padding 0
    str_dst.clear();
    str_src = "123";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::zero), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "595C70C0662C2635");

    // ECB padding ISO10126 （random result）
    str_dst.clear();
    str_src = "1234";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::iso10126), true);

    // ECB padding ANSIX923
    str_dst.clear();
    str_src = "12345";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::ansix923), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "B2D791C38A1AD844");

    // ECB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = "123456";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::iso_iec_7816_4), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "A1A227D1256E7C2D");

    // ECB padding NOPADDING
    str_dst.clear();
    str_src = "12345678";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::no_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FE4ABFB5190D173D");

    str_dst.clear();
    str_src = "1234567";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::no_padding), false);

    // CBC padding PKCS#5
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "77910467EA549E870607FEDECED97190");

    // CBC padding PKCS#7
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "77910467EA549E870607FEDECED97190");

    // CBC padding 0
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "77910467EA549E875122F18A882D6318");

    // CBC padding ISO10126 （random result）
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::iso10126, iv), true);

    // CBC padding ANSIX923
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "77910467EA549E87D3A3E13D0A9DBC1D");

    // CBC padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "77910467EA549E876D62899FA608C319");

    // CBC padding NOPADDING
    str_dst.clear();
    str_src = "hello world 1234";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "77910467EA549E87E5C4991F4D218550");

    str_dst.clear();
    str_src = "hello world 123";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::no_padding, iv), false);

    // CFB padding PKCS#5
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644ED4F4B5090F2BA835F");

    // CFB padding PKCS#7
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644ED4F4B5090F2BA835F");

    // CFB padding 0
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644ED4F4B5095F7BF865A");

    // CFB padding ISO10126 （random result）
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::iso10126, iv), true);

    // CFB padding ANSIX923
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644ED4F4B5095F7BF865F");

    // CFB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644ED4F4B5015F7BF865A");

    // CFB padding NOPADDING
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::no_padding, iv), false);

    str_dst.clear();
    str_src = "hello world 4321";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644ED4F4B50B5C38CB46B");

    // OFB padding PKCS#5
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644ED66BC7B1B96753DD5");

    // OFB padding PKCS#7
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644ED66BC7B1B96753DD5");

    // OFB padding 0
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644ED66BC7B1E937038D0");

    // OFB padding ISO10126 （random result）
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::iso10126, iv), true);

    // OFB padding ANSIX923
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644ED66BC7B1E937038D5");

    // OFB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644ED66BC7B9E937038D0");

    // OFB padding NOPADDING
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::no_padding, iv), false);

    str_dst.clear();
    str_src = "hello world 123412345678";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644ED66BC7B3EA2420BE48977339416654194");

    // CTR padding PKCS#5
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644EDF85DF1975DDF811E");

    // CTR padding PKCS#7
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644EDF85DF1975DDF811E");

    // CTR padding 0
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644EDF85DF19258DA841B");

    // CTR padding ISO10126 （random result）
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::iso10126, iv), true);

    // CTR padding ANSIX923
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644EDF85DF19258DA841E");

    // CTR padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644EDF85DF11258DA841B");

    // CTR padding NOPADDING
    str_dst.clear();
    str_src = "hello world";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::no_padding, iv), false);

    str_dst.clear();
    str_src = "hello world 1234";
    ASSERT_EQ(libcpp::des::encrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "8BA580A7B50644EDF85DF1B269E8B72F");
}

TEST(des, decrypt)
{
    std::string str_src;
    std::string str_dst;
    std::string key("12345678");
    std::string iv("abcdefgh");

    // for stream ECB padding PKCS#5 test
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD475D82E3681C83BB77");
    std::istringstream in(str_src);
    std::ostringstream out;
    ASSERT_EQ(libcpp::des::decrypt(out, 
                                  in, 
                                  reinterpret_cast<const unsigned char*>(key.c_str()), 
                                  key.size(), 
                                  libcpp::des::mode::ecb, 
                                  libcpp::des::padding::pkcs5), 
        true);
    ASSERT_STREQ(to_hex(out.str()).c_str(), "68656C6C6F20776F726C64");

    // ECB padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD47AA291D16D82146BF");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::pkcs5), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C642031");

    // ECB padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD47AA291D16D82146BF");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::pkcs7), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C642031");

    // ECB padding 0
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD47535FFDB06574A87D");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::zero), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64203132");

    // ECB padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD4721FF332B8A40E8D9");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::iso10126), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // ECB padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD47BABFF98EFA6DB628030116F7E552E7B6");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::ansix923), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C642031323334");

    // ECB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD47BABFF98EFA6DB628DE3491731DCB353C");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::iso_iec_7816_4), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64203132333435");

    // ECB padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD47BABFF98EFA6DB628");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::no_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C642031323334");

    // CBC padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("B72D0DC9E9433B0373FB9C7373EEE4D1");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CBC padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("B72D0DC9E9433B03849FE655EE80DB80");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C642031");

    // CBC padding 0
    str_dst.clear();
    str_src = to_bytes("B72D0DC9E9433B0353E3BE76EB413AB0");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64203132");

    // CBC padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("B72D0DC9E9433B0368437984E8C9A32D");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::iso10126, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CBC padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("B72D0DC9E9433B031400731AF6B95E3BDF39AA4FBB8F6F63");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C642031323334");

    // CBC padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("B72D0DC9E9433B031400731AF6B95E3BC113907DECCE3490");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64203132333435");

    // CBC padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("B72D0DC9E9433B031400731AF6B95E3B");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C642031323334");

    // CFB padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC248142DD0763FF0EC92210E9B43584E0");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C642031323334");

    // CFB padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC248142DD0763FF3B");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C6420313233");

    // CFB padding 0
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC248142DD0763CC3A");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64203132");

    // CFB padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC24814288857BCC3F");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::iso10126, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CFB padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC248142FD3651CC3F");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CFB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC248142DD07D1CC3A");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C642031");

    // CFB padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC248142DD0763FF0E");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C642031323334");

    // OFB padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("F8BD2108B3C5B491");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "6C6962637070");

    // OFB padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("F8BD2108B3C58792");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "6C696263707031");

    // OFB padding 0
    str_dst.clear();
    str_src = to_bytes("F8BD2108B3C587A14B6BB3A9E45E6967");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "6C69626370703132");

    // OFB padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC3907D759632FDF62");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::iso10126, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // OFB padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("F8BD2108B3C587A1785FB3A9E45E6961");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "6C696263707031323334");

    // OFB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("F8BD2108B3C587A1785F8629E45E6967");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "6C69626370703132333435");

    // OFB padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("A5E6705FF68381AB7A59809DD1685E5F");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "31323334353637383132333435363738");

    // CTR padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("A5D3446CC4B2B194");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "31");

    // CTR padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("A5E6456DC5B3B095");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "3132");

    // CTR padding 0
    str_dst.clear();
    str_src = to_bytes("A5E6706BC3B5B693");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "313233");

    // CTR padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC46D14F6AF9AD1330");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::iso10126, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CTR padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("A5E6705FF6B5B690");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "3132333435");

    // CTR padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("A5E6705FF6833693");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "313233343536");

    // CTR padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("A5E6705FF68381AB");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "3132333435363738");
}

TEST(des, decrypt_n)
{
    std::string str_src;
    std::string str_dst;
    std::string key("12345678abcdefgh00000000");
    std::string iv("abcdefgh");

    // for stream ECB padding PKCS#5 test
    str_dst.clear();
    str_src = to_bytes("3E4665C52F935552F1C9C86E67880CBB");
    std::istringstream in(str_src);
    std::ostringstream out;
    ASSERT_EQ(libcpp::des::decrypt(out, 
                                  in, 
                                  reinterpret_cast<const unsigned char*>(key.c_str()), 
                                  key.size(), 
                                  libcpp::des::mode::ecb, 
                                  libcpp::des::padding::pkcs5), 
        true);
    ASSERT_STREQ(to_hex(out.str()).c_str(), "68656C6C6F20776F726C64");

    // ECB padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("3E4665C52F935552F1C9C86E67880CBB");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::pkcs5), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // ECB padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("3E4665C52F935552F1C9C86E67880CBB");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::pkcs7), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // ECB padding 0
    str_dst.clear();
    str_src = to_bytes("3E4665C52F935552ED4059A86631C2ED");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::zero), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // ECB padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("8C4EE7F2FF1D139E");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::iso10126), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "31323334");

    // ECB padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("3E4665C52F9355523559BB7793D1D266");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::ansix923), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // ECB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("3E4665C52F93555275E23EDBE984B84C");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::iso_iec_7816_4), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // ECB padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("FE4ABFB5190D173D");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ecb, libcpp::des::padding::no_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "3132333435363738");

    // CBC padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("77910467EA549E870607FEDECED97190");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CBC padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("77910467EA549E870607FEDECED97190");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CBC padding 0
    str_dst.clear();
    str_src = to_bytes("77910467EA549E875122F18A882D6318");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CBC padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("77910467EA549E870607FEDECED97190");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::iso10126, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CBC padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("77910467EA549E87D3A3E13D0A9DBC1D");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CBC padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("77910467EA549E876D62899FA608C319");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CBC padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("77910467EA549E87E5C4991F4D218550");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cbc, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C642031323334");

    // CFB padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644ED4F4B5090F2BA835F");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CFB padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644ED4F4B5090F2BA835F");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CFB padding 0
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644ED4F4B5095F7BF865A");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CFB padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644ED4F4B50");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::iso10126, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CFB padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644ED4F4B5095F7BF865F");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CFB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644ED4F4B5015F7BF865A");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CFB padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644ED4F4B50B5C38CB46B");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::cfb, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C642034333231");

    // OFB padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644ED66BC7B1B96753DD5");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // OFB padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644ED66BC7B1B96753DD5");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // OFB padding 0
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644ED66BC7B1E937038D0");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // OFB padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644ED66BC7B");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::iso10126, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // OFB padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644ED66BC7B1E937038D5");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // OFB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644ED66BC7B9E937038D0");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // OFB padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644ED66BC7B3EA2420BE48977339416654194");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ofb, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C6420313233343132333435363738");

    // CTR padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644EDF85DF1975DDF811E");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::pkcs5, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CTR padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644EDF85DF1975DDF811E");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::pkcs7, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CTR padding 0
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644EDF85DF19258DA841B");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::zero, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CTR padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644EDF85DF1");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::iso10126, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CTR padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644EDF85DF19258DA841E");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::ansix923, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CTR padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644EDF85DF11258DA841B");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::iso_iec_7816_4, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CTR padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("8BA580A7B50644EDF85DF1B269E8B72F");
    ASSERT_EQ(libcpp::des::decrypt(str_dst, str_src, key, libcpp::des::mode::ctr, libcpp::des::padding::no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C642031323334");
}


TEST(des, encrypt_file)
{
    std::string str_src("./crypto.log");
    std::string str_src_nopadding("./crypto_nopadding.log");
    std::string key("12345678");
    std::string iv("abcdefgh");

    // ECB padding PKCS#5
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ecb_pkcs5_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ecb, libcpp::des::padding::pkcs5), true);

    // ECB padding PKCS#7
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ecb_pkcs7_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ecb, libcpp::des::padding::pkcs7), true);

    // ECB padding 0
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ecb_zero_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ecb, libcpp::des::padding::zero), true);

    // ECB padding ISO10126 （random result）
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ecb_iso10126_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ecb, libcpp::des::padding::iso10126), true);

    // ECB padding ANSIX923
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ecb_ansix923_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ecb, libcpp::des::padding::ansix923), true);

    // ECB padding ISO/IEC 7816-4
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ecb_iso_iec_7816_4_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ecb, libcpp::des::padding::iso_iec_7816_4), true);

    // ECB padding NOPADDING
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ecb_no_padding_encrypt.log"), 
        str_src_nopadding, key, libcpp::des::mode::ecb, libcpp::des::padding::no_padding), true);

    // CBC padding PKCS#5
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_cbc_pkcs5_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::cbc, libcpp::des::padding::pkcs5, iv), true);

    // CBC padding PKCS#7
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_cbc_pkcs7_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::cbc, libcpp::des::padding::pkcs7, iv), true);

    // CBC padding 0
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_cbc_zero_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::cbc, libcpp::des::padding::zero, iv), true);

    // CBC padding ISO10126 （random result）
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_cbc_iso10126_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::cbc, libcpp::des::padding::iso10126, iv), true);

    // CBC padding ANSIX923
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_cbc_ansix923_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::cbc, libcpp::des::padding::ansix923, iv), true);

    // CBC padding ISO/IEC 7816-4
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_cbc_iso_iec_7816_4_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::cbc, libcpp::des::padding::iso_iec_7816_4, iv), true);

    // CBC padding NOPADDING
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_cbc_no_padding_encrypt.log"), str_src_nopadding, 
        key, libcpp::des::mode::cbc, libcpp::des::padding::no_padding, iv), true);

    // CFB padding PKCS#5
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_cfb_pkcs5_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::cfb, libcpp::des::padding::pkcs5, iv), true);

    // CFB padding PKCS#7
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_cfb_pkcs7_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::cfb, libcpp::des::padding::pkcs7, iv), true);

    // CFB padding 0
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_cfb_zero_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::cfb, libcpp::des::padding::zero, iv), true);

    // CFB padding ISO10126 （random result）
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_cfb_iso10126_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::cfb, libcpp::des::padding::iso10126, iv), true);

    // CFB padding ANSIX923
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_cfb_ansix923_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::cfb, libcpp::des::padding::ansix923, iv), true);

    // CFB padding ISO/IEC 7816-4
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_cfb_iso_iec_7816_4_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::cfb, libcpp::des::padding::iso_iec_7816_4, iv), true);

    // CFB padding NOPADDING
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_cfb_no_padding_encrypt.log"), str_src_nopadding, 
        key, libcpp::des::mode::cfb, libcpp::des::padding::no_padding, iv), true);

    // OFB padding PKCS#5
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ofb_pkcs5_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ofb, libcpp::des::padding::pkcs5, iv), true);

    // OFB padding PKCS#7
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ofb_pkcs7_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ofb, libcpp::des::padding::pkcs7, iv), true);

    // OFB padding 0
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ofb_zero_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ofb, libcpp::des::padding::zero, iv), true);

    // OFB padding ISO10126 （random result）
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ofb_iso10126_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ofb, libcpp::des::padding::iso10126, iv), true);

    // OFB padding ANSIX923
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ofb_ansix923_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ofb, libcpp::des::padding::ansix923, iv), true);

    // OFB padding ISO/IEC 7816-4
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ofb_iso_iec_7816_4_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ofb, libcpp::des::padding::iso_iec_7816_4, iv), true);

    // OFB padding NOPADDING
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ofb_no_padding_encrypt.log"), str_src_nopadding, 
        key, libcpp::des::mode::ofb, libcpp::des::padding::no_padding, iv), true);

    // CTR padding PKCS#5
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ctr_pkcs5_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ctr, libcpp::des::padding::pkcs5, iv), true);

    // CTR padding PKCS#7
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ctr_pkcs7_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ctr, libcpp::des::padding::pkcs7, iv), true);

    // CTR padding 0
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ctr_zero_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ctr, libcpp::des::padding::zero, iv), true);

    // CTR padding ISO10126 （random result）
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ctr_iso10126_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ctr, libcpp::des::padding::iso10126, iv), true);

    // CTR padding ANSIX923
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ctr_ansix923_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ctr, libcpp::des::padding::ansix923, iv), true);

    // CTR padding ISO/IEC 7816-4
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ctr_iso_iec_7816_4_padding_encrypt.log"), str_src, 
        key, libcpp::des::mode::ctr, libcpp::des::padding::iso_iec_7816_4, iv), true);

    // CTR padding NOPADDING
    ASSERT_EQ(libcpp::des::encrypt_file(std::string("./des_ctr_no_padding_encrypt.log"), str_src_nopadding, 
        key, libcpp::des::mode::ctr, libcpp::des::padding::no_padding, iv), true);
}

TEST(des, decrypt_file)
{
    std::string md5_src;
    std::string md5_dst;
    std::string str_dst;
    std::string str_src;
    std::string key("12345678");
    std::string iv("abcdefgh");

    str_src = "./des_ecb_pkcs5_padding_encrypt.log";
    str_dst = "./des_ecb_pkcs5_padding.log";

    ASSERT_TRUE(libcpp::des::encrypt_file(str_src, "./crypto.log", 
        key, libcpp::des::mode::ecb, libcpp::des::padding::pkcs5));

    // ECB padding PKCS#5
    ASSERT_TRUE(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ecb, libcpp::des::padding::pkcs5));
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // ECB padding PKCS#7 
    str_src = "./des_ecb_pkcs7_padding_encrypt.log";
    str_dst = "./des_ecb_pkcs7_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ecb, libcpp::des::padding::pkcs7), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // ECB padding 0 
    str_src = "./des_ecb_zero_padding_encrypt.log";
    str_dst = "./des_ecb_zero_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ecb, libcpp::des::padding::zero), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // ECB padding ISO10126 （random result） 
    str_src = "./des_ecb_iso10126_padding_encrypt.log";
    str_dst = "./des_ecb_iso10126_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ecb, libcpp::des::padding::iso10126), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // ECB padding ANSIX923 
    str_src = "./des_ecb_ansix923_padding_encrypt.log";
    str_dst = "./des_ecb_ansix923_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ecb, libcpp::des::padding::ansix923), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // ECB padding ISO/IEC 7816-4 
    str_src = "./des_ecb_iso_iec_7816_4_padding_encrypt.log";
    str_dst = "./des_ecb_iso_iec_7816_4_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ecb, libcpp::des::padding::iso_iec_7816_4), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // ECB padding NOPADDING 
    str_src = "./des_ecb_no_padding_encrypt.log";
    str_dst = "./des_ecb_no_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ecb, libcpp::des::padding::no_padding), true);
    md5_src = calc_file_md5("./crypto_nopadding.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CBC padding PKCS#5 
    str_src = "./des_cbc_pkcs5_padding_encrypt.log";
    str_dst = "./des_cbc_pkcs5_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::cbc, libcpp::des::padding::pkcs5, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CBC padding PKCS#7 
    str_src = "./des_cbc_pkcs7_padding_encrypt.log";
    str_dst = "./des_cbc_pkcs7_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::cbc, libcpp::des::padding::pkcs7, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CBC padding 0 
    str_src = "./des_cbc_zero_padding_encrypt.log";
    str_dst = "./des_cbc_zero_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::cbc, libcpp::des::padding::zero, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CBC padding ISO10126 （random result） 
    str_src = "./des_cbc_iso10126_padding_encrypt.log";
    str_dst = "./des_cbc_iso10126_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::cbc, libcpp::des::padding::iso10126, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CBC padding ANSIX923 
    str_src = "./des_cbc_ansix923_padding_encrypt.log";
    str_dst = "./des_cbc_ansix923_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::cbc, libcpp::des::padding::ansix923, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CBC padding ISO/IEC 7816-4 
    str_src = "./des_cbc_iso_iec_7816_4_padding_encrypt.log";
    str_dst = "./des_cbc_iso_iec_7816_4_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::cbc, libcpp::des::padding::iso_iec_7816_4, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CBC padding NOPADDING 
    str_src = "./des_cbc_no_padding_encrypt.log";
    str_dst = "./des_cbc_no_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::cbc, libcpp::des::padding::no_padding, iv), true);
    md5_src = calc_file_md5("./crypto_nopadding.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CFB padding PKCS#5 
    str_src = "./des_cfb_pkcs5_padding_encrypt.log";
    str_dst = "./des_cfb_pkcs5_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::cfb, libcpp::des::padding::pkcs5, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CFB padding PKCS#7 
    str_src = "./des_cfb_pkcs7_padding_encrypt.log";
    str_dst = "./des_cfb_pkcs7_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::cfb, libcpp::des::padding::pkcs7, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CFB padding 0 
    str_src = "./des_cfb_zero_padding_encrypt.log";
    str_dst = "./des_cfb_zero_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::cfb, libcpp::des::padding::zero, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CFB padding ISO10126 （random result） 
    str_src = "./des_cfb_iso10126_padding_encrypt.log";
    str_dst = "./des_cfb_iso10126_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::cfb, libcpp::des::padding::iso10126, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CFB padding ANSIX923 
    str_src = "./des_cfb_ansix923_padding_encrypt.log";
    str_dst = "./des_cfb_ansix923_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::cfb, libcpp::des::padding::ansix923, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CFB padding ISO/IEC 7816-4 
    str_src = "./des_cfb_iso_iec_7816_4_padding_encrypt.log";
    str_dst = "./des_cfb_iso_iec_7816_4_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::cfb, libcpp::des::padding::iso_iec_7816_4, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CFB padding NOPADDING 
    str_src = "./des_cfb_no_padding_encrypt.log";
    str_dst = "./des_cfb_no_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::cfb, libcpp::des::padding::no_padding, iv), true);
    md5_src = calc_file_md5("./crypto_nopadding.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // OFB padding PKCS#5 
    str_src = "./des_ofb_pkcs5_padding_encrypt.log";
    str_dst = "./des_ofb_pkcs5_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src,  
        key, libcpp::des::mode::ofb, libcpp::des::padding::pkcs5, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // OFB padding PKCS#7 
    str_src = "./des_ofb_pkcs7_padding_encrypt.log";
    str_dst = "./des_ofb_pkcs7_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ofb, libcpp::des::padding::pkcs7, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // OFB padding 0 
    str_src = "./des_ofb_zero_padding_encrypt.log";
    str_dst = "./des_ofb_zero_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ofb, libcpp::des::padding::zero, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // OFB padding ISO10126 （random result） 
    str_src = "./des_ofb_iso10126_padding_encrypt.log";
    str_dst = "./des_ofb_iso10126_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ofb, libcpp::des::padding::iso10126, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // OFB padding ANSIX923 
    str_src = "./des_ofb_ansix923_padding_encrypt.log";
    str_dst = "./des_ofb_ansix923_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src,  
        key, libcpp::des::mode::ofb, libcpp::des::padding::ansix923, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // OFB padding ISO/IEC 7816-4 
    str_src = "./des_ofb_iso_iec_7816_4_padding_encrypt.log";
    str_dst = "./des_ofb_iso_iec_7816_4_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ofb, libcpp::des::padding::iso_iec_7816_4, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // OFB padding NOPADDING 
    str_src = "./des_ofb_no_padding_encrypt.log";
    str_dst = "./des_ofb_no_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ofb, libcpp::des::padding::no_padding, iv), true);
    md5_src = calc_file_md5("./crypto_nopadding.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CTR padding PKCS#5 
    str_src = "./des_ctr_pkcs5_padding_encrypt.log";
    str_dst = "./des_ctr_pkcs5_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src,  
        key, libcpp::des::mode::ctr, libcpp::des::padding::pkcs5, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CTR padding PKCS#7 
    str_src = "./des_ctr_pkcs7_padding_encrypt.log";
    str_dst = "./des_ctr_pkcs7_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ctr, libcpp::des::padding::pkcs7, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CTR padding 0 
    str_src = "./des_ctr_zero_padding_encrypt.log";
    str_dst = "./des_ctr_zero_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ctr, libcpp::des::padding::zero, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CTR padding ISO10126 （random result） 
    str_src = "./des_ctr_iso10126_padding_encrypt.log";
    str_dst = "./des_ctr_iso10126_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ctr, libcpp::des::padding::iso10126, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CTR padding ANSIX923 
    str_src = "./des_ctr_ansix923_padding_encrypt.log";
    str_dst = "./des_ctr_ansix923_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ctr, libcpp::des::padding::ansix923, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CTR padding ISO/IEC 7816-4 
    str_src = "./des_ctr_iso_iec_7816_4_padding_encrypt.log";
    str_dst = "./des_ctr_iso_iec_7816_4_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ctr, libcpp::des::padding::iso_iec_7816_4, iv), true);
    md5_src = calc_file_md5("./crypto.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);

    // CTR padding NOPADDING 
    str_src = "./des_ctr_no_padding_encrypt.log";
    str_dst = "./des_ctr_no_padding.log";
    ASSERT_EQ(libcpp::des::decrypt_file(str_dst, str_src, 
        key, libcpp::des::mode::ctr, libcpp::des::padding::no_padding, iv), true);
    md5_src = calc_file_md5("./crypto_nopadding.log");
    md5_dst = calc_file_md5(str_dst.c_str());
    ASSERT_TRUE(md5_src == md5_dst);
}