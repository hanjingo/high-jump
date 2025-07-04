#include <gtest/gtest.h>
#include <libcpp/crypto/des.hpp>
#include <libcpp/log/logger.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>

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

TEST(des, encode)
{
    std::string str_src("hello world");
    std::string str_dst;
    std::string key("12345678");
    std::string iv("abcdefgh");

    // for stream ECB padding PKCS#5 test
    std::istringstream in(str_src);
    std::ostringstream out;
    ASSERT_EQ(libcpp::des::encode(out, 
                                  in, 
                                  reinterpret_cast<const unsigned char*>(key.c_str()), 
                                  key.size(), 
                                  libcpp::des::cipher::des_ecb, 
                                  libcpp::des::padding::des_pkcs5_padding), 
        true);
    ASSERT_STREQ(to_hex(out.str()).c_str(), "28DBA02EB5F6DD475D82E3681C83BB77");

    // ECB padding PKCS#5
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_pkcs5_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "28DBA02EB5F6DD475D82E3681C83BB77");

    // ECB padding PKCS#7
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_pkcs7_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "28DBA02EB5F6DD475D82E3681C83BB77");

    // ECB padding 0
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_zero_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "28DBA02EB5F6DD476042DAEBFA59687A");

    // ECB padding ISO10126 （random result）
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_iso10126_padding), true);

    // ECB padding ANSIX923
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_ansix923_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "28DBA02EB5F6DD47D33696D839C770B2");

    // ECB padding ISO/IEC 7816-4
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_iso_iec_7816_4_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "28DBA02EB5F6DD4706B5C56593DCBE2C");

    // ECB padding NOPADDING
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_no_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "28DBA02EB5F6DD476042DAEBFA59687A");

    // CBC padding PKCS#5
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_pkcs5_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "B72D0DC9E9433B0373FB9C7373EEE4D1");

    // CBC padding PKCS#7
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_pkcs7_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "B72D0DC9E9433B0373FB9C7373EEE4D1");

    // CBC padding 0
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_zero_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "B72D0DC9E9433B036393B0FC4D701C80");

    // CBC padding ISO10126 （random result）
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_iso10126_padding, iv), true);

    // CBC padding ANSIX923
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_ansix923_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "B72D0DC9E9433B037D1CFDBA8122DB79");

    // CBC padding ISO/IEC 7816-4
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "B72D0DC9E9433B03CAF03F0F1AA271F7");

    // CBC padding NOPADDING
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "B72D0DC9E9433B036393B0FC4D701C80");

    // CFB padding PKCS#5
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_pkcs5_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC248142F83354C93F");

    // CFB padding PKCS#7
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_pkcs7_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC248142F83354C93F");

    // CFB padding 0
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_zero_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC248142FD3651CC3A");

    // CFB padding ISO10126 （random result）
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_iso10126_padding, iv), true);

    // CFB padding ANSIX923
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_ansix923_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC248142FD3651CC3F");

    // CFB padding ISO/IEC 7816-4
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC2481427D3651CC3A");

    // CFB padding NOPADDING
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC248142");

    // OFB padding PKCS#5
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_pkcs5_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC3907D7ACE15B6C62");

    // OFB padding PKCS#7
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_pkcs7_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC3907D7ACE15B6C62");

    // OFB padding 0
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_zero_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC3907D7A9E45E6967");

    // OFB padding ISO10126 （random result）
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_iso10126_padding, iv), true);

    // OFB padding ANSIX923
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_ansix923_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC3907D7A9E45E6962");

    // OFB padding ISO/IEC 7816-4
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC3907D729E45E6967");

    // OFB padding NOPADDING
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC3907D7");

    // CTR padding PKCS#5
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_pkcs5_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC46D14F5AECDF0C30");

    // CTR padding PKCS#7
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_pkcs7_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC46D14F5AECDF0C30");

    // CTR padding 0
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_zero_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC46D14F5FE9DA0935");

    // CTR padding ISO10126 （random result）
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_iso10126_padding, iv), true);

    // CTR padding ANSIX923
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_ansix923_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC46D14F5FE9DA0930");

    // CTR padding ISO/IEC 7816-4
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC46D14FDFE9DA0935");

    // CTR padding NOPADDING
    str_dst.clear();
    ASSERT_EQ(libcpp::des::encode(str_dst, str_src, key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "FCB12F07AC95C1FC46D14F");
}

TEST(des, decode)
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
    ASSERT_EQ(libcpp::des::decode(out, 
                                  in, 
                                  reinterpret_cast<const unsigned char*>(key.c_str()), 
                                  key.size(), 
                                  libcpp::des::cipher::des_ecb, 
                                  libcpp::des::padding::des_pkcs5_padding), 
        true);
    ASSERT_STREQ(to_hex(out.str()).c_str(), "68656C6C6F20776F726C64");

    // ECB padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD475D82E3681C83BB77");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_pkcs5_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // ECB padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD475D82E3681C83BB77");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_pkcs7_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // ECB padding 0
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD476042DAEBFA59687A");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_zero_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // ECB padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD4721FF332B8A40E8D9");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_iso10126_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // ECB padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD476042DAEBFA59687A");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_ansix923_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C640000000000");

    // ECB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD4706B5C56593DCBE2C");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_iso_iec_7816_4_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // ECB padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("28DBA02EB5F6DD476042DAEBFA59687A");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_no_padding), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C640000000000");

    // CBC padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("B72D0DC9E9433B0373FB9C7373EEE4D1");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_pkcs5_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CBC padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("B72D0DC9E9433B0373FB9C7373EEE4D1");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_pkcs7_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CBC padding 0
    str_dst.clear();
    str_src = to_bytes("B72D0DC9E9433B036393B0FC4D701C80");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_zero_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CBC padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("B72D0DC9E9433B0368437984E8C9A32D");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_iso10126_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CBC padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("B72D0DC9E9433B037D1CFDBA8122DB79");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_ansix923_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CBC padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("B72D0DC9E9433B03CAF03F0F1AA271F7");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CBC padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("B72D0DC9E9433B036393B0FC4D701C80");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C640000000000");

    // CFB padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC248142F83354C93F");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_pkcs5_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CFB padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC248142F83354C93F");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_pkcs7_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CFB padding 0
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC248142FD3651CC3A");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_zero_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CFB padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC24814288857BCC3F");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_iso10126_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CFB padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC248142FD3651CC3F");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_ansix923_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CFB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC2481427D3651CC3A");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CFB padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC248142");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // OFB padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC3907D7ACE15B6C62");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_pkcs5_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // OFB padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC3907D7ACE15B6C62");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_pkcs7_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // OFB padding 0
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC3907D7A9E45E6967");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_zero_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // OFB padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC3907D759632FDF62");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_iso10126_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // OFB padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC3907D7A9E45E6962");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_ansix923_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // OFB padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC3907D729E45E6967");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // OFB padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC3907D7");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CTR padding PKCS#5
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC46D14F5AECDF0C30");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_pkcs5_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CTR padding PKCS#7
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC46D14F5AECDF0C30");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_pkcs7_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CTR padding 0
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC46D14F5FE9DA0935");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_zero_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CTR padding ISO10126 （random result）
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC46D14F6AF9AD1330");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_iso10126_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CTR padding ANSIX923
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC46D14F5FE9DA0930");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_ansix923_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CTR padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC46D14FDFE9DA0935");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");

    // CTR padding NOPADDING
    str_dst.clear();
    str_src = to_bytes("FCB12F07AC95C1FC46D14F");
    ASSERT_EQ(libcpp::des::decode(str_dst, str_src, key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_no_padding, iv), true);
    ASSERT_STREQ(to_hex(str_dst).c_str(), "68656C6C6F20776F726C64");
}

TEST(des, encode_file)
{
    std::string str_src("./des.log");
    std::string key("12345678");
    std::string iv("abcdefgh");
    libcpp::logger::instance()->clear_sink();
    libcpp::logger::instance()->add_sink(libcpp::logger::create_rotate_file_sink("./des.log", 1 * 1024 * 1024, 1, true));
    for (int i = 0; i < 1 * 1024; i++)
        libcpp::logger::instance()->info("{}", i);
    libcpp::logger::instance()->flush();

    // ECB padding PKCS#5
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ecb_pkcs5_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_pkcs5_padding), true);

    // ECB padding PKCS#7
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ecb_pkcs7_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_pkcs7_padding), true);

    // ECB padding 0
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ecb_zero_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_zero_padding), true);

    // ECB padding ISO10126 （random result）
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ecb_iso10126_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_iso10126_padding), true);

    // ECB padding ANSIX923
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ecb_ansix923_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_ansix923_padding), true);

    // ECB padding ISO/IEC 7816-4
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ecb_iso_iec_7816_4_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_iso_iec_7816_4_padding), true);

    // ECB padding NOPADDING
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ecb_no_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_no_padding), true);

    // CBC padding PKCS#5
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_cbc_pkcs5_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_pkcs5_padding, iv), true);

    // CBC padding PKCS#7
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_cbc_pkcs7_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_pkcs7_padding, iv), true);

    // CBC padding 0
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_cbc_zero_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_zero_padding, iv), true);

    // CBC padding ISO10126 （random result）
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_cbc_iso10126_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_iso10126_padding, iv), true);

    // CBC padding ANSIX923
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_cbc_ansix923_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_ansix923_padding, iv), true);

    // CBC padding ISO/IEC 7816-4
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_cbc_iso_iec_7816_4_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);

    // CBC padding NOPADDING
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_cbc_no_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_no_padding, iv), true);

    // CFB padding PKCS#5
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_cfb_pkcs5_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_pkcs5_padding, iv), true);

    // CFB padding PKCS#7
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_cfb_pkcs7_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_pkcs7_padding, iv), true);

    // CFB padding 0
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_cfb_zero_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_zero_padding, iv), true);

    // CFB padding ISO10126 （random result）
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_cfb_iso10126_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_iso10126_padding, iv), true);

    // CFB padding ANSIX923
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_cfb_ansix923_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_ansix923_padding, iv), true);

    // CFB padding ISO/IEC 7816-4
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_cfb_iso_iec_7816_4_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);

    // CFB padding NOPADDING
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_cfb_no_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_no_padding, iv), true);

    // OFB padding PKCS#5
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ofb_pkcs5_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_pkcs5_padding, iv), true);

    // OFB padding PKCS#7
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ofb_pkcs7_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_pkcs7_padding, iv), true);

    // OFB padding 0
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ofb_zero_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_zero_padding, iv), true);

    // OFB padding ISO10126 （random result）
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ofb_iso10126_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_iso10126_padding, iv), true);

    // OFB padding ANSIX923
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ofb_ansix923_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_ansix923_padding, iv), true);

    // OFB padding ISO/IEC 7816-4
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ofb_iso_iec_7816_4_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);

    // OFB padding NOPADDING
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ofb_no_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_no_padding, iv), true);

    // CTR padding PKCS#5
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ctr_pkcs5_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_pkcs5_padding, iv), true);

    // CTR padding PKCS#7
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ctr_pkcs7_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_pkcs7_padding, iv), true);

    // CTR padding 0
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ctr_zero_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_zero_padding, iv), true);

    // CTR padding ISO10126 （random result）
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ctr_iso10126_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_iso10126_padding, iv), true);

    // CTR padding ANSIX923
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ctr_ansix923_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_ansix923_padding, iv), true);

    // CTR padding ISO/IEC 7816-4
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ctr_iso_iec_7816_4_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);

    // CTR padding NOPADDING
    ASSERT_EQ(libcpp::des::encode_file(std::string("./des_ctr_no_padding_encode.log"), str_src, 
        key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_no_padding, iv), true);
}

TEST(des, decode_file)
{
    std::string str_dst;
    std::string str_src;
    std::string key("12345678");
    std::string iv("abcdefgh");
    libcpp::logger::instance()->clear_sink();
    libcpp::logger::instance()->add_sink(libcpp::logger::create_rotate_file_sink("./des.log", 1 * 1024 * 1024, 1, true));
    for (int i = 0; i < 1 * 1024; i++)
        libcpp::logger::instance()->info("{}", i);
    libcpp::logger::instance()->flush();

    // ECB padding PKCS#5
    str_src = "./des_ecb_pkcs5_padding_encode.log";
    str_dst = "./des_ecb_pkcs5_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_pkcs5_padding), true);

    // ECB padding PKCS#7 
    str_src = "./des_ecb_pkcs7_padding_encode.log";
    str_dst = "./des_ecb_pkcs7_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_pkcs7_padding), true);

    // ECB padding 0 
    str_src = "./des_ecb_zero_padding_encode.log";
    str_dst = "./des_ecb_zero_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_zero_padding), true);

    // ECB padding ISO10126 （random result） 
    str_src = "./des_ecb_iso10126_padding_encode.log";
    str_dst = "./des_ecb_iso10126_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_iso10126_padding), true);

    // ECB padding ANSIX923 
    str_src = "./des_ecb_ansix923_padding_encode.log";
    str_dst = "./des_ecb_ansix923_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_ansix923_padding), true);

    // ECB padding ISO/IEC 7816-4 
    str_src = "./des_ecb_iso_iec_7816_4_padding_encode.log";
    str_dst = "./des_ecb_iso_iec_7816_4_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_iso_iec_7816_4_padding), true);

    // ECB padding NOPADDING 
    str_src = "./des_ecb_no_padding_encode.log";
    str_dst = "./des_ecb_no_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ecb, libcpp::des::padding::des_no_padding), true);

    // CBC padding PKCS#5 
    str_src = "./des_cbc_pkcs5_padding_encode.log";
    str_dst = "./des_cbc_pkcs5_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_pkcs5_padding, iv), true);

    // CBC padding PKCS#7 
    str_src = "./des_cbc_pkcs7_padding_encode.log";
    str_dst = "./des_cbc_pkcs7_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_pkcs7_padding, iv), true);

    // CBC padding 0 
    str_src = "./des_cbc_zero_padding_encode.log";
    str_dst = "./des_cbc_zero_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_zero_padding, iv), true);

    // CBC padding ISO10126 （random result） 
    str_src = "./des_cbc_iso10126_padding_encode.log";
    str_dst = "./des_cbc_iso10126_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_iso10126_padding, iv), true);

    // CBC padding ANSIX923 
    str_src = "./des_cbc_ansix923_padding_encode.log";
    str_dst = "./des_cbc_ansix923_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_ansix923_padding, iv), true);

    // CBC padding ISO/IEC 7816-4 
    str_src = "./des_cbc_iso_iec_7816_4_padding_encode.log";
    str_dst = "./des_cbc_iso_iec_7816_4_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);

    // CBC padding NOPADDING 
    str_src = "./des_cbc_no_padding_encode.log";
    str_dst = "./des_cbc_no_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_cbc, libcpp::des::padding::des_no_padding, iv), true);

    // CFB padding PKCS#5 
    str_src = "./des_cfb_pkcs5_padding_encode.log";
    str_dst = "./des_cfb_pkcs5_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_pkcs5_padding, iv), true);

    // CFB padding PKCS#7 
    str_src = "./des_cfb_pkcs7_padding_encode.log";
    str_dst = "./des_cfb_pkcs7_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_pkcs7_padding, iv), true);

    // CFB padding 0 
    str_src = "./des_cfb_zero_padding_encode.log";
    str_dst = "./des_cfb_zero_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_zero_padding, iv), true);

    // CFB padding ISO10126 （random result） 
    str_src = "./des_cfb_iso10126_padding_encode.log";
    str_dst = "./des_cfb_iso10126_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_iso10126_padding, iv), true);

    // CFB padding ANSIX923 
    str_src = "./des_cfb_ansix923_padding_encode.log";
    str_dst = "./des_cfb_ansix923_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_ansix923_padding, iv), true);

    // CFB padding ISO/IEC 7816-4 
    str_src = "./des_cfb_iso_iec_7816_4_padding_encode.log";
    str_dst = "./des_cfb_iso_iec_7816_4_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);

    // CFB padding NOPADDING 
    str_src = "./des_cfb_no_padding_encode.log";
    str_dst = "./des_cfb_no_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_cfb, libcpp::des::padding::des_no_padding, iv), true);

    // OFB padding PKCS#5 
    str_src = "./des_ofb_pkcs5_padding_encode.log";
    str_dst = "./des_ofb_pkcs5_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src,  
        key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_pkcs5_padding, iv), true);

    // OFB padding PKCS#7 
    str_src = "./des_ofb_pkcs7_padding_encode.log";
    str_dst = "./des_ofb_pkcs7_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_pkcs7_padding, iv), true);

    // OFB padding 0 
    str_src = "./des_ofb_zero_padding_encode.log";
    str_dst = "./des_ofb_zero_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_zero_padding, iv), true);

    // OFB padding ISO10126 （random result） 
    str_src = "./des_ofb_iso10126_padding_encode.log";
    str_dst = "./des_ofb_iso10126_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_iso10126_padding, iv), true);

    // OFB padding ANSIX923 
    str_src = "./des_ofb_ansix923_padding_encode.log";
    str_dst = "./des_ofb_ansix923_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src,  
        key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_ansix923_padding, iv), true);

    // OFB padding ISO/IEC 7816-4 
    str_src = "./des_ofb_iso_iec_7816_4_padding_encode.log";
    str_dst = "./des_ofb_iso_iec_7816_4_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);

    // OFB padding NOPADDING 
    str_src = "./des_ofb_no_padding_encode.log";
    str_dst = "./des_ofb_no_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ofb, libcpp::des::padding::des_no_padding, iv), true);

    // CTR padding PKCS#5 
    str_src = "./des_ctr_pkcs5_padding_encode.log";
    str_dst = "./des_ctr_pkcs5_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src,  
        key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_pkcs5_padding, iv), true);

    // CTR padding PKCS#7 
    str_src = "./des_ctr_pkcs7_padding_encode.log";
    str_dst = "./des_ctr_pkcs7_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_pkcs7_padding, iv), true);

    // CTR padding 0 
    str_src = "./des_ctr_zero_padding_encode.log";
    str_dst = "./des_ctr_zero_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_zero_padding, iv), true);

    // CTR padding ISO10126 （random result） 
    str_src = "./des_ctr_iso10126_padding_encode.log";
    str_dst = "./des_ctr_iso10126_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_iso10126_padding, iv), true);

    // CTR padding ANSIX923 
    str_src = "./des_ctr_ansix923_padding_encode.log";
    str_dst = "./des_ctr_ansix923_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_ansix923_padding, iv), true);

    // CTR padding ISO/IEC 7816-4 
    str_src = "./des_ctr_iso_iec_7816_4_padding_encode.log";
    str_dst = "./des_ctr_iso_iec_7816_4_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_iso_iec_7816_4_padding, iv), true);

    // CTR padding NOPADDING 
    str_src = "./des_ctr_no_padding_encode.log";
    str_dst = "./des_ctr_no_padding.log";
    ASSERT_EQ(libcpp::des::decode_file(str_dst, str_src, 
        key, libcpp::des::cipher::des_ctr, libcpp::des::padding::des_no_padding, iv), true);
}