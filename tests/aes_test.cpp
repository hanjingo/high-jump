#include <gtest/gtest.h>
#include <hj/crypto/aes.hpp>

#include <filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>

std::string aes_test_to_hex(const std::string &src)
{
    std::stringstream ss;
    for(unsigned char c : src)
        ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
           << static_cast<int>(c);

    return ss.str();
}

std::string aes_test_to_bytes(const std::string &hex)
{
    std::string bytes;
    for(size_t i = 0; i < hex.length(); i += 2)
    {
        unsigned int      byte;
        std::stringstream ss;
        ss << std::hex << hex.substr(i, 2);
        ss >> byte;
        bytes.push_back(static_cast<char>(byte));
    }
    return bytes;
}

TEST(aes, encrypt)
{
    auto ok = hj::aes::error_code::ok;

    std::string str_src = "hello world 1234";
    std::string str_dst;
    std::string key128 = "1234567812345678";
    std::string key192 = "123456781234567812345678";
    std::string key256 = "12345678123456781234567812345678";
    std::string iv     = "abcdefghabcdefgh";

    hj::aes::options opt(reinterpret_cast<const unsigned char *>(key128.data()),
                         key128.size(),
                         reinterpret_cast<const unsigned char *>(iv.data()),
                         iv.size(),
                         hj::aes::mode::ecb,
                         hj::aes::padding::pkcs5);

    // ECB128 padding PKCS#5
    str_dst.clear();
    opt.mod       = hj::aes::mode::ecb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "8CAC1119CB4DD87A51FE8824C3694A38D96AA42B59151A9E9B5925FC9D95ADAF");

    // ECB128 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "8CAC1119CB4DD87A51FE8824C3694A38D96AA42B59151A9E9B5925FC9D95ADAF");

    // ECB128 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "8CAC1119CB4DD87A51FE8824C3694A389AE8FD02B340288A0E7BBFF0F0BA54D6");

    // ECB128 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // ECB128 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "8CAC1119CB4DD87A51FE8824C3694A3822C5EEA291A5D34ABB765D5B1842E78B");

    // ECB128 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "8CAC1119CB4DD87A51FE8824C3694A381FF14EEF93A23B23219C80D71AFC105E");

    // ECB128 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "8CAC1119CB4DD87A51FE8824C3694A38");

    // ECB192 padding PKCS#5
    str_dst.clear();
    opt.key       = reinterpret_cast<const unsigned char *>(key192.data());
    opt.key_len   = key192.size();
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "406E2B7A955B830CEB5E9E8867DEA8855DAE4B81861BE13861BF21861C5A3199");

    // ECB192 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "406E2B7A955B830CEB5E9E8867DEA8855DAE4B81861BE13861BF21861C5A3199");

    // ECB192 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "406E2B7A955B830CEB5E9E8867DEA885AD4748C9D990EF505B9F07885BE8478E");

    // ECB192 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // ECB192 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "406E2B7A955B830CEB5E9E8867DEA885EAD916782BFDA8941BD4CCFFA4CD2403");

    // ECB192 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "406E2B7A955B830CEB5E9E8867DEA885FE4110A9214DCB0194CB6AF8CDE8FE9D");

    // ECB192 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "406E2B7A955B830CEB5E9E8867DEA885");

    // ECB256 padding PKCS#5
    str_dst.clear();
    opt.key       = reinterpret_cast<const unsigned char *>(key256.data());
    opt.key_len   = key256.size();
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "45EE1BD897FD37E0F2E3843C7000E1CBEEBBDAED7324EC4BC70D1C0343337233");

    // ECB256 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "45EE1BD897FD37E0F2E3843C7000E1CBEEBBDAED7324EC4BC70D1C0343337233");

    // ECB256 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "45EE1BD897FD37E0F2E3843C7000E1CBA623C5073552282EAEE27EA60F009485");

    // ECB256 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // ECB256 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "45EE1BD897FD37E0F2E3843C7000E1CBBC84B644C8C6EA50D3FA26E25DEBCEA8");

    // ECB256 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "45EE1BD897FD37E0F2E3843C7000E1CB7AA312308994119CDE6DED1826EB9AA0");

    // ECB256 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "45EE1BD897FD37E0F2E3843C7000E1CB");

    // CBC128 padding PKCS#5
    str_dst.clear();
    opt.key       = reinterpret_cast<const unsigned char *>(key128.data());
    opt.key_len   = key128.size();
    opt.mod       = hj::aes::mode::cbc;
    opt.pad_style = hj::aes::padding::pkcs5;
    opt.iv        = reinterpret_cast<const unsigned char *>(iv.data());
    opt.iv_len    = iv.size();
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "742D1C448000B6A944D14E960E0248A56E5B78B1A23B996B46407E5F67B51627");

    // CBC128 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "742D1C448000B6A944D14E960E0248A56E5B78B1A23B996B46407E5F67B51627");

    // CBC128 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "742D1C448000B6A944D14E960E0248A5AB2682B74B7C1D2ED1487BA3EDFC01F1");

    // CBC128 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // CBC128 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "742D1C448000B6A944D14E960E0248A54120006A65A622296F06F8AEB859420D");

    // CBC128 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "742D1C448000B6A944D14E960E0248A5231CEE7AEE142F7A237B8496D352D77D");

    // CBC128 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "742D1C448000B6A944D14E960E0248A5");

    // CBC192 padding PKCS#5
    str_dst.clear();
    opt.key       = reinterpret_cast<const unsigned char *>(key192.data());
    opt.key_len   = key192.size();
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "5E559B496C47EA672844622A5B862DCDE14B48F0F48FBFD643B127F21B894CE5");

    // CBC192 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "5E559B496C47EA672844622A5B862DCDE14B48F0F48FBFD643B127F21B894CE5");

    // CBC192 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "5E559B496C47EA672844622A5B862DCDEA5BFE07335D832DF997C8E2F204F8DB");

    // CBC192 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // CBC192 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "5E559B496C47EA672844622A5B862DCD9638F956303AC21C9E8DB4FB049510A9");

    // CBC192 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "5E559B496C47EA672844622A5B862DCD325E9405B7B0E9E3E950CFA301EE0B64");

    // CBC192 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "5E559B496C47EA672844622A5B862DCD");

    // CBC256 padding PKCS#5
    str_dst.clear();
    opt.key       = reinterpret_cast<const unsigned char *>(key256.data());
    opt.key_len   = key256.size();
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "43D77E30642AE42D62852522EA1A70FD2D8756D35007D18C3ADFE10E4108A00B");

    // CBC256 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "43D77E30642AE42D62852522EA1A70FD2D8756D35007D18C3ADFE10E4108A00B");

    // CBC256 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "43D77E30642AE42D62852522EA1A70FD08AED410B31CA711A5CE231EBD07D66D");

    // CBC256 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // CBC256 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "43D77E30642AE42D62852522EA1A70FDAAE217B846F6510247CD52ADCAEEE0E0");

    // CBC256 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "43D77E30642AE42D62852522EA1A70FD95BB2F28E485E31A4393D9B8D9F64DF2");

    // CBC256 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "43D77E30642AE42D62852522EA1A70FD");

    // CFB128 padding PKCS#5
    str_dst.clear();
    opt.key       = reinterpret_cast<const unsigned char *>(key128.data());
    opt.key_len   = key128.size();
    opt.mod       = hj::aes::mode::cfb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D758309A1D7AB5CA1C7093A370C4F511A94AD");

    // CFB128 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D758309A1D7AB5CA1C7093A370C4F511A94AD");

    // CFB128 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D758319B1C7BB4CB1D7192A271C5F410A84BD");

    // CFB128 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // CFB128 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D758319B1C7BB4CB1D7192A271C5F410A84AD");

    // CFB128 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D758399B1C7BB4CB1D7192A271C5F410A84BD");

    // CFB128 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "A38D19F76F00C0C23B5726C2777D7583");

    // CFB192 padding PKCS#5
    str_dst.clear();
    opt.key       = reinterpret_cast<const unsigned char *>(key192.data());
    opt.key_len   = key192.size();
    opt.mod       = hj::aes::mode::cfb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5F87BF5553483056FB92A998EA07AB0D72");

    // CFB192 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5F87BF5553483056FB92A998EA07AB0D72");

    // CFB192 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5F97AF4543582046EB82B988FA17BB1D62");

    // CFB192 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // CFB192 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5F97AF4543582046EB82B988FA17BB1D72");

    // CFB192 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5F17AF4543582046EB82B988FA17BB1D62");

    // CFB192 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "2B9548B149F87264A6C0C48A55314C5F");

    // CFB256 padding PKCS#5
    str_dst.clear();
    opt.key       = reinterpret_cast<const unsigned char *>(key256.data());
    opt.key_len   = key256.size();
    opt.mod       = hj::aes::mode::cfb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65B652327F51EB9E5E6A97BF78814A61DC0");

    // CFB256 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65B652327F51EB9E5E6A97BF78814A61DC0");

    // CFB256 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65B753337E50EA9F5F6B96BE79804B60DD0");

    // CFB256 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // CFB256 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65B753337E50EA9F5F6B96BE79804B60DC0");

    // CFB256 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65BF53337E50EA9F5F6B96BE79804B60DD0");

    // CFB256 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "F804466E2B35C3323332DE6E633FA65B");

    // OFB128 padding PKCS#5
    str_dst.clear();
    opt.key       = reinterpret_cast<const unsigned char *>(key128.data());
    opt.key_len   = key128.size();
    opt.mod       = hj::aes::mode::ofb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D7583D19C72E677A12D656E839C70255DE6FA");

    // OFB128 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D7583D19C72E677A12D656E839C70255DE6FA");

    // OFB128 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D7583C18C62F667B13D757E938C60354DF6EA");

    // OFB128 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // OFB128 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D7583C18C62F667B13D757E938C60354DF6FA");

    // OFB128 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D7583418C62F667B13D757E938C60354DF6EA");

    // OFB128 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "A38D19F76F00C0C23B5726C2777D7583");

    // OFB192 padding PKCS#5
    str_dst.clear();
    opt.key       = reinterpret_cast<const unsigned char *>(key192.data());
    opt.key_len   = key192.size();
    opt.mod       = hj::aes::mode::ofb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5F8566926EAC19A95B135FB3B50EC29686");

    // OFB192 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5F8566926EAC19A95B135FB3B50EC29686");

    // OFB192 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5F9576827EBC09B94B034FA3A51ED28696");

    // OFB192 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // OFB192 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5F9576827EBC09B94B034FA3A51ED28686");

    // OFB192 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5F1576827EBC09B94B034FA3A51ED28696");

    // OFB192 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "2B9548B149F87264A6C0C48A55314C5F");

    // OFB256 padding PKCS#5
    str_dst.clear();
    opt.key       = reinterpret_cast<const unsigned char *>(key256.data());
    opt.key_len   = key256.size();
    opt.mod       = hj::aes::mode::ofb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65B4250ABF28FA751C889B12A11DDA870CD");

    // OFB256 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65B4250ABF28FA751C889B12A11DDA870CD");

    // OFB256 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65B5240BBE29FB741D899A13A01CDB860DD");

    // OFB256 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // OFB256 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65B5240BBE29FB741D899A13A01CDB860CD");

    // OFB256 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65BD240BBE29FB741D899A13A01CDB860DD");

    // OFB256 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "F804466E2B35C3323332DE6E633FA65B");

    // CTR128 padding PKCS#5
    str_dst.clear();
    opt.key       = reinterpret_cast<const unsigned char *>(key128.data());
    opt.key_len   = key128.size();
    opt.mod       = hj::aes::mode::ctr;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D7583FC10D8181B1EF01B41704AE789A34D7F");

    // CTR128 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D7583FC10D8181B1EF01B41704AE789A34D7F");

    // CTR128 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D7583EC00C8080B0EE00B51605AF799B35D6F");

    // CTR128 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // CTR128 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D7583EC00C8080B0EE00B51605AF799B35D7F");

    // CTR128 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "A38D19F76F00C0C23B5726C2777D75836C00C8080B0EE00B51605AF799B35D6F");

    // CTR128 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "A38D19F76F00C0C23B5726C2777D7583");

    // CTR192 padding PKCS#5
    str_dst.clear();
    opt.key       = reinterpret_cast<const unsigned char *>(key192.data());
    opt.key_len   = key192.size();
    opt.mod       = hj::aes::mode::ctr;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5F4D88D644EF60A935451AB6FD6F720B57");

    // CTR192 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5F4D88D644EF60A935451AB6FD6F720B57");

    // CTR192 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5F5D98C654FF70B925550AA6ED7F621B47");

    // CTR192 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // CTR192 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5F5D98C654FF70B925550AA6ED7F621B57");

    // CTR192 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "2B9548B149F87264A6C0C48A55314C5FDD98C654FF70B925550AA6ED7F621B47");

    // CTR192 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "2B9548B149F87264A6C0C48A55314C5F");

    // CTR256 padding PKCS#5
    str_dst.clear();
    opt.key       = reinterpret_cast<const unsigned char *>(key256.data());
    opt.key_len   = key256.size();
    opt.mod       = hj::aes::mode::ctr;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65B1D38A644453C4671441D516ECB540EE1");

    // CTR256 padding PKCS#7
    str_dst.clear();
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65B1D38A644453C4671441D516ECB540EE1");

    // CTR256 padding 0
    str_dst.clear();
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65B0D28B654552C5661540D417EDB441EF1");

    // CTR256 padding ISO10126 (random)
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);

    // CTR256 padding ANSIX923
    str_dst.clear();
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65B0D28B654552C5661540D417EDB441EE1");

    // CTR256 padding ISO/IEC 7816-4
    str_dst.clear();
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(
        aes_test_to_hex(str_dst).c_str(),
        "F804466E2B35C3323332DE6E633FA65B8D28B654552C5661540D417EDB441EF1");

    // CTR256 no padding
    str_dst.clear();
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::encrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "F804466E2B35C3323332DE6E633FA65B");
}

TEST(aes, decrypt)
{
    auto ok = hj::aes::error_code::ok;

    std::string str_src;
    std::string str_dst;
    std::string key128 = "1234567812345678";
    std::string key192 = "123456781234567812345678";
    std::string key256 = "12345678123456781234567812345678";
    std::string iv     = "abcdefghabcdefgh";

    hj::aes::options opt(reinterpret_cast<const unsigned char *>(key128.data()),
                         key128.size(),
                         reinterpret_cast<const unsigned char *>(iv.data()),
                         iv.size(),
                         hj::aes::mode::ecb,
                         hj::aes::padding::pkcs5);
    // ECB128 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "8CAC1119CB4DD87A51FE8824C3694A38D96AA42B59151A9E9B5925FC9D95ADAF");
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB128 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "8CAC1119CB4DD87A51FE8824C3694A38D96AA42B59151A9E9B5925FC9D95ADAF");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB128 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "8CAC1119CB4DD87A51FE8824C3694A389AE8FD02B340288A0E7BBFF0F0BA54D6");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB128 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "8CAC1119CB4DD87A51FE8824C3694A3808F0D406805B704BA680CAE2B0A95222");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB128 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "8CAC1119CB4DD87A51FE8824C3694A3822C5EEA291A5D34ABB765D5B1842E78B");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB128 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "8CAC1119CB4DD87A51FE8824C3694A381FF14EEF93A23B23219C80D71AFC105E");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB128 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("8CAC1119CB4DD87A51FE8824C3694A38");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB192 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "406E2B7A955B830CEB5E9E8867DEA8855DAE4B81861BE13861BF21861C5A3199");
    opt.key       = reinterpret_cast<const unsigned char *>(key192.data());
    opt.key_len   = key192.size();
    opt.mod       = hj::aes::mode::ecb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB192 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "406E2B7A955B830CEB5E9E8867DEA8855DAE4B81861BE13861BF21861C5A3199");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB192 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "406E2B7A955B830CEB5E9E8867DEA885AD4748C9D990EF505B9F07885BE8478E");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB192 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "406E2B7A955B830CEB5E9E8867DEA885CEB4E53004E6C030E4E54B6706BD6F0D");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB192 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "406E2B7A955B830CEB5E9E8867DEA885EAD916782BFDA8941BD4CCFFA4CD2403");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB192 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "406E2B7A955B830CEB5E9E8867DEA885FE4110A9214DCB0194CB6AF8CDE8FE9D");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB192 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("406E2B7A955B830CEB5E9E8867DEA885");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB256 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "45EE1BD897FD37E0F2E3843C7000E1CBEEBBDAED7324EC4BC70D1C0343337233");
    opt.key       = reinterpret_cast<const unsigned char *>(key256.data());
    opt.key_len   = key256.size();
    opt.mod       = hj::aes::mode::ecb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB256 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "45EE1BD897FD37E0F2E3843C7000E1CBEEBBDAED7324EC4BC70D1C0343337233");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB256 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "45EE1BD897FD37E0F2E3843C7000E1CBA623C5073552282EAEE27EA60F009485");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB256 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "45EE1BD897FD37E0F2E3843C7000E1CB73136ADE0EF900119625A65C3F12DDD4");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB256 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "45EE1BD897FD37E0F2E3843C7000E1CBBC84B644C8C6EA50D3FA26E25DEBCEA8");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB256 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "45EE1BD897FD37E0F2E3843C7000E1CB7AA312308994119CDE6DED1826EB9AA0");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // ECB256 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("45EE1BD897FD37E0F2E3843C7000E1CB");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC128 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "742D1C448000B6A944D14E960E0248A56E5B78B1A23B996B46407E5F67B51627");
    opt.key       = reinterpret_cast<const unsigned char *>(key128.data());
    opt.key_len   = key128.size();
    opt.mod       = hj::aes::mode::cbc;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC128 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "742D1C448000B6A944D14E960E0248A56E5B78B1A23B996B46407E5F67B51627");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC128 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "742D1C448000B6A944D14E960E0248A5AB2682B74B7C1D2ED1487BA3EDFC01F1");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC128 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "742D1C448000B6A944D14E960E0248A56E5B78B1A23B996B46407E5F67B51627");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC128 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "742D1C448000B6A944D14E960E0248A54120006A65A622296F06F8AEB859420D");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC128 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "742D1C448000B6A944D14E960E0248A5231CEE7AEE142F7A237B8496D352D77D");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC128 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("742D1C448000B6A944D14E960E0248A5");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC192 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "5E559B496C47EA672844622A5B862DCDE14B48F0F48FBFD643B127F21B894CE5");
    opt.key       = reinterpret_cast<const unsigned char *>(key192.data());
    opt.key_len   = key192.size();
    opt.mod       = hj::aes::mode::cbc;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC192 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "5E559B496C47EA672844622A5B862DCDE14B48F0F48FBFD643B127F21B894CE5");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC192 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "5E559B496C47EA672844622A5B862DCDEA5BFE07335D832DF997C8E2F204F8DB");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC192 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "5E559B496C47EA672844622A5B862DCD5EEC5CB2823E67A3E16F2829527ADCB4");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC192 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "5E559B496C47EA672844622A5B862DCD9638F956303AC21C9E8DB4FB049510A9");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC192 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "5E559B496C47EA672844622A5B862DCD325E9405B7B0E9E3E950CFA301EE0B64");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC192 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("5E559B496C47EA672844622A5B862DCD");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC256 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "43D77E30642AE42D62852522EA1A70FD2D8756D35007D18C3ADFE10E4108A00B");
    opt.key       = reinterpret_cast<const unsigned char *>(key256.data());
    opt.key_len   = key256.size();
    opt.mod       = hj::aes::mode::cbc;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC256 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "43D77E30642AE42D62852522EA1A70FD2D8756D35007D18C3ADFE10E4108A00B");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC256 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "43D77E30642AE42D62852522EA1A70FD08AED410B31CA711A5CE231EBD07D66D");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC256 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "43D77E30642AE42D62852522EA1A70FD61825A3CEA464AE381184C15E19F471E");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC256 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "43D77E30642AE42D62852522EA1A70FDAAE217B846F6510247CD52ADCAEEE0E0");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC256 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "43D77E30642AE42D62852522EA1A70FD95BB2F28E485E31A4393D9B8D9F64DF2");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CBC256 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("43D77E30642AE42D62852522EA1A70FD");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB128 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D758309A1D7AB5CA1C7093A370C4F511A94AD");
    opt.key       = reinterpret_cast<const unsigned char *>(key128.data());
    opt.key_len   = key128.size();
    opt.mod       = hj::aes::mode::cfb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB128 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D758309A1D7AB5CA1C7093A370C4F511A94AD");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB128 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D758319B1C7BB4CB1D7192A271C5F410A84BD");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB128 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D7583A3CD3C6812B8E894EAADFD62A7AF05AD");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB128 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D758319B1C7BB4CB1D7192A271C5F410A84AD");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB128 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D758399B1C7BB4CB1D7192A271C5F410A84BD");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB128 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("A38D19F76F00C0C23B5726C2777D7583");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB192 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F87BF5553483056FB92A998EA07AB0D72");
    opt.key       = reinterpret_cast<const unsigned char *>(key192.data());
    opt.key_len   = key192.size();
    opt.mod       = hj::aes::mode::cfb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB192 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F87BF5553483056FB92A998EA07AB0D72");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB192 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F97AF4543582046EB82B988FA17BB1D62");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB192 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5FFD30C922503DD6239F098C9F2A610172");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB192 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F97AF4543582046EB82B988FA17BB1D72");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB192 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F17AF4543582046EB82B988FA17BB1D62");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB192 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("2B9548B149F87264A6C0C48A55314C5F");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB256 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B652327F51EB9E5E6A97BF78814A61DC0");
    opt.key       = reinterpret_cast<const unsigned char *>(key256.data());
    opt.key_len   = key256.size();
    opt.mod       = hj::aes::mode::cfb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB256 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B652327F51EB9E5E6A97BF78814A61DC0");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB256 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B753337E50EA9F5F6B96BE79804B60DD0");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB256 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B75EAC69707766CE4C8D0CF69B53959C0");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB256 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B753337E50EA9F5F6B96BE79804B60DC0");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB256 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65BF53337E50EA9F5F6B96BE79804B60DD0");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CFB256 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("F804466E2B35C3323332DE6E633FA65B");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB128 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D7583D19C72E677A12D656E839C70255DE6FA");
    opt.key       = reinterpret_cast<const unsigned char *>(key128.data());
    opt.key_len   = key128.size();
    opt.mod       = hj::aes::mode::ofb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB128 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D7583D19C72E677A12D656E839C70255DE6FA");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB128 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D7583C18C62F667B13D757E938C60354DF6EA");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB128 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D758367C18AEE0175E3376A989934B5134FFA");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB128 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D7583C18C62F667B13D757E938C60354DF6FA");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB128 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D7583418C62F667B13D757E938C60354DF6EA");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB128 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("A38D19F76F00C0C23B5726C2777D7583");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB192 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F8566926EAC19A95B135FB3B50EC29686");
    opt.key       = reinterpret_cast<const unsigned char *>(key192.data());
    opt.key_len   = key192.size();
    opt.mod       = hj::aes::mode::ofb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB192 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F8566926EAC19A95B135FB3B50EC29686");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB192 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F9576827EBC09B94B034FA3A51ED28696");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB192 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F278777AFA67747EB1E08141284A9C586");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB192 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F9576827EBC09B94B034FA3A51ED28686");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB192 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F1576827EBC09B94B034FA3A51ED28696");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB192 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("2B9548B149F87264A6C0C48A55314C5F");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB256 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B4250ABF28FA751C889B12A11DDA870CD");
    opt.key       = reinterpret_cast<const unsigned char *>(key256.data());
    opt.key_len   = key256.size();
    opt.mod       = hj::aes::mode::ofb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB256 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B4250ABF28FA751C889B12A11DDA870CD");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB256 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B5240BBE29FB741D899A13A01CDB860DD");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB256 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B859CB116F8EDDC62F7D1D44763661FCD");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB256 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B5240BBE29FB741D899A13A01CDB860CD");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB256 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65BD240BBE29FB741D899A13A01CDB860DD");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // OFB256 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("F804466E2B35C3323332DE6E633FA65B");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR128 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D7583FC10D8181B1EF01B41704AE789A34D7F");
    opt.key       = reinterpret_cast<const unsigned char *>(key128.data());
    opt.key_len   = key128.size();
    opt.mod       = hj::aes::mode::ctr;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR128 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D7583FC10D8181B1EF01B41704AE789A34D7F");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR128 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D7583EC00C8080B0EE00B51605AF799B35D6F");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR128 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D7583ECBEC0E9135A36014A2AF4AC914C587F");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR128 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D7583EC00C8080B0EE00B51605AF799B35D7F");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR128 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "A38D19F76F00C0C23B5726C2777D75836C00C8080B0EE00B51605AF799B35D6F");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR128 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("A38D19F76F00C0C23B5726C2777D7583");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR192 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F4D88D644EF60A935451AB6FD6F720B57");
    opt.key       = reinterpret_cast<const unsigned char *>(key192.data());
    opt.key_len   = key192.size();
    opt.mod       = hj::aes::mode::ctr;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR192 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F4D88D644EF60A935451AB6FD6F720B57");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR192 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F5D98C654FF70B925550AA6ED7F621B47");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR192 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5FA57EAD45748F704D99FAAF5440CF0C57");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR192 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5F5D98C654FF70B925550AA6ED7F621B57");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR192 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "2B9548B149F87264A6C0C48A55314C5FDD98C654FF70B925550AA6ED7F621B47");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR192 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("2B9548B149F87264A6C0C48A55314C5F");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR256 padding PKCS#5
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B1D38A644453C4671441D516ECB540EE1");
    opt.key       = reinterpret_cast<const unsigned char *>(key256.data());
    opt.key_len   = key256.size();
    opt.mod       = hj::aes::mode::ctr;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR256 padding PKCS#7
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B1D38A644453C4671441D516ECB540EE1");
    opt.pad_style = hj::aes::padding::pkcs7;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR256 padding 0
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B0D28B654552C5661540D417EDB441EF1");
    opt.pad_style = hj::aes::padding::zero;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR256 padding ISO10126 (random)
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65BB38AD288657068CFC5D19F52A9CF36E1");
    opt.pad_style = hj::aes::padding::iso10126;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR256 padding ANSIX923
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B0D28B654552C5661540D417EDB441EE1");
    opt.pad_style = hj::aes::padding::ansix923;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR256 padding ISO/IEC 7816-4
    str_dst.clear();
    str_src = aes_test_to_bytes(
        "F804466E2B35C3323332DE6E633FA65B8D28B654552C5661540D417EDB441EF1");
    opt.pad_style = hj::aes::padding::iso_iec_7816_4;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");

    // CTR256 no padding
    str_dst.clear();
    str_src       = aes_test_to_bytes("F804466E2B35C3323332DE6E633FA65B");
    opt.pad_style = hj::aes::padding::no_padding;
    ASSERT_EQ(hj::aes::decrypt(str_dst, str_src, opt), ok);
    ASSERT_STREQ(aes_test_to_hex(str_dst).c_str(),
                 "68656C6C6F20776F726C642031323334");
}

TEST(aes, encrypt_file)
{
    auto ok = hj::aes::error_code::ok;

    std::string str_dst;
    std::string str_src;
    std::string key128 = "1234567812345678";
    std::string key192 = "123456781234567812345678";
    std::string key256 = "12345678123456781234567812345678";
    std::string iv     = "abcdefghabcdefgh";

    // ECB padding PKCS#5
    str_src = "./crypto.log";
    str_dst = "./aes_ecb_pkcs5_padding_encrypt.log";
    if(!std::filesystem::exists(str_src))
    {
        GTEST_SKIP() << "skip test aes encrypt_file not exist: " << str_src;
    }

    hj::aes::options opt;
    opt.key       = reinterpret_cast<const unsigned char *>(key128.data());
    opt.key_len   = key128.size();
    opt.mod       = hj::aes::mode::ecb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt_file(str_dst.c_str(), str_src.c_str(), opt), ok);
}

TEST(aes, decrypt_file)
{
    auto ok = hj::aes::error_code::ok;

    // encryptd file -> file
    std::string str_dst;
    std::string str_src;
    std::string key128 = "1234567812345678";
    std::string key192 = "123456781234567812345678";
    std::string key256 = "12345678123456781234567812345678";
    std::string iv     = "abcdefghabcdefgh";

    // ECB padding PKCS#5
    str_src = "./crypto.log";
    str_dst = "./aes_ecb_pkcs5_padding_encrypt1.log";
    if(!std::filesystem::exists(str_src))
    {
        GTEST_SKIP() << "skip test aes decrypt_file not exist: " << str_src;
    }

    hj::aes::options opt;
    opt.key       = reinterpret_cast<const unsigned char *>(key128.data());
    opt.key_len   = key128.size();
    opt.mod       = hj::aes::mode::ecb;
    opt.pad_style = hj::aes::padding::pkcs5;
    ASSERT_EQ(hj::aes::encrypt_file(str_dst.c_str(), str_src.c_str(), opt), ok);

    // ECB padding PKCS#5
    str_src = "./aes_ecb_pkcs5_padding_encrypt1.log";
    str_dst = "./aes_ecb_pkcs5_padding.log";
    ASSERT_EQ(hj::aes::decrypt_file(str_dst.c_str(), str_src.c_str(), opt), ok);
}

TEST(aes, keygen)
{
    auto          ok      = hj::aes::error_code::ok;
    unsigned char key[32] = {0};
    ASSERT_EQ(hj::aes::keygen(key, sizeof(key)), ok);

    ASSERT_EQ(hj::aes::keygen(key, 16), ok);
    ASSERT_EQ(hj::aes::keygen(key, 24), ok);
    ASSERT_EQ(hj::aes::keygen(key, 32), ok);
    ASSERT_FALSE(hj::aes::keygen(key, 15) == ok);

    unsigned char passwd[7] = "passwd";
    unsigned char salt[5]   = "salt";
    ASSERT_EQ(hj::aes::keygen(key, 32, passwd, 7, salt, 5), ok);
}

TEST(aes, is_key_valid)
{
    auto ok = hj::aes::error_code::ok;

    unsigned char key[32] = {0};
    ASSERT_TRUE(hj::aes::is_key_valid(hj::aes::mode::ecb, key, sizeof(key)));
    ASSERT_TRUE(hj::aes::is_key_valid(hj::aes::mode::ecb, key, 16));
    ASSERT_TRUE(hj::aes::is_key_valid(hj::aes::mode::ecb, key, 24));
    ASSERT_TRUE(hj::aes::is_key_valid(hj::aes::mode::ecb, key, 32));
    ASSERT_FALSE(hj::aes::is_key_valid(hj::aes::mode::ecb, key, 15));
    ASSERT_FALSE(hj::aes::is_key_valid(hj::aes::mode::ecb, key, 25));
    ASSERT_FALSE(hj::aes::is_key_valid(hj::aes::mode::ecb, key, 33));
}

TEST(aes, is_iv_valid)
{
    std::string str16 = std::string(16, 'a');
    std::string str24 = std::string(24, 'b');
    std::string str32 = std::string(32, 'c');

    ASSERT_TRUE(hj::aes::is_iv_valid(hj::aes::mode::ecb, nullptr, 0));
    ASSERT_FALSE(hj::aes::is_iv_valid(
        hj::aes::mode::ecb,
        reinterpret_cast<const unsigned char *>(str16.c_str()),
        str16.size()));

    ASSERT_TRUE(hj::aes::is_iv_valid(
        hj::aes::mode::cbc,
        reinterpret_cast<const unsigned char *>(str16.c_str()),
        str16.size()));
    ASSERT_FALSE(hj::aes::is_iv_valid(
        hj::aes::mode::cbc,
        reinterpret_cast<const unsigned char *>(str24.c_str()),
        str24.size()));

    ASSERT_TRUE(hj::aes::is_iv_valid(
        hj::aes::mode::gcm,
        reinterpret_cast<const unsigned char *>(str16.c_str()),
        str16.size()));
    ASSERT_TRUE(hj::aes::is_iv_valid(
        hj::aes::mode::gcm,
        reinterpret_cast<const unsigned char *>(str24.c_str()),
        str24.size()));
    ASSERT_TRUE(hj::aes::is_iv_valid(
        hj::aes::mode::gcm,
        reinterpret_cast<const unsigned char *>(str32.c_str()),
        str32.size()));

    ASSERT_FALSE(hj::aes::is_iv_valid(
        hj::aes::mode::cbc,
        reinterpret_cast<const unsigned char *>(str32.c_str()),
        str32.size()));
}

TEST(aes, is_plain_valid)
{
    ASSERT_TRUE(hj::aes::is_plain_valid(hj::aes::mode::ecb,
                                        16,
                                        hj::aes::padding::pkcs7,
                                        0));
    ASSERT_TRUE(hj::aes::is_plain_valid(hj::aes::mode::ecb,
                                        16,
                                        hj::aes::padding::pkcs7,
                                        16));
    ASSERT_TRUE(hj::aes::is_plain_valid(hj::aes::mode::ecb,
                                        16,
                                        hj::aes::padding::pkcs7,
                                        32));

    ASSERT_FALSE(hj::aes::is_plain_valid(hj::aes::mode::ecb,
                                         16,
                                         hj::aes::padding::no_padding,
                                         15));
}

TEST(aes, is_stream_mode)
{
    ASSERT_TRUE(hj::aes::is_stream_mode(hj::aes::mode::cfb1));
    ASSERT_TRUE(hj::aes::is_stream_mode(hj::aes::mode::cfb8));
    ASSERT_TRUE(hj::aes::is_stream_mode(hj::aes::mode::cfb128));
    ASSERT_TRUE(hj::aes::is_stream_mode(hj::aes::mode::ofb));
    ASSERT_TRUE(hj::aes::is_stream_mode(hj::aes::mode::ctr));
    ASSERT_FALSE(hj::aes::is_stream_mode(hj::aes::mode::ecb));
    ASSERT_FALSE(hj::aes::is_stream_mode(hj::aes::mode::cbc));
}

TEST(aes, is_aead_mode)
{
    ASSERT_TRUE(hj::aes::is_aead_mode(hj::aes::mode::gcm));
    ASSERT_TRUE(hj::aes::is_aead_mode(hj::aes::mode::ccm));
    ASSERT_FALSE(hj::aes::is_aead_mode(hj::aes::mode::ecb));
    ASSERT_FALSE(hj::aes::is_aead_mode(hj::aes::mode::cbc));
}