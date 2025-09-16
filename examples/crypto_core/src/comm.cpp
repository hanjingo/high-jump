#include "comm.h"

#include "api.h"

libcpp::aes::mode str_to_aes_mode(const std::string& mode)
{
    if (mode == "ecb")
        return libcpp::aes::mode::ecb;
    else if (mode == "cbc")
        return libcpp::aes::mode::cbc;
    else if (mode == "cfb1")
        return libcpp::aes::mode::cfb1;
    else if (mode == "cfb8")
        return libcpp::aes::mode::cfb8;
    else if (mode == "cfb128")
        return libcpp::aes::mode::cfb128;
    else if (mode == "cfb")
        return libcpp::aes::mode::cfb;
    else if (mode == "ofb")
        return libcpp::aes::mode::ofb;
    else if (mode == "ctr")
        return libcpp::aes::mode::ctr;
    else if (mode == "gcm")
        return libcpp::aes::mode::gcm;
    else if (mode == "ccm")
        return libcpp::aes::mode::ccm;
    else if (mode == "xts")
        return libcpp::aes::mode::xts;
    else if (mode == "wrap")
        return libcpp::aes::mode::wrap;
    else if (mode == "wrap_pad")
        return libcpp::aes::mode::wrap_pad;
    else if (mode == "cbc_hmac_sha1")
        return libcpp::aes::mode::cbc_hmac_sha1;
    else if (mode == "cbc_hmac_sha256")
        return libcpp::aes::mode::cbc_hmac_sha256;
    else if (mode == "ocb")
        return libcpp::aes::mode::ocb;
    else
        return libcpp::aes::mode::ecb; // default
}

libcpp::aes::padding str_to_aes_padding(const std::string& padding)
{
    if (padding == "pkcs5")
        return libcpp::aes::padding::pkcs5;
    else if (padding == "pkcs7")
        return libcpp::aes::padding::pkcs7;
    else if (padding == "zero")
        return libcpp::aes::padding::zero;
    else if (padding == "iso10126")
        return libcpp::aes::padding::iso10126;
    else if (padding == "ansix923")
        return libcpp::aes::padding::ansix923;
    else if (padding == "iso_iec_7816_4")
        return libcpp::aes::padding::iso_iec_7816_4;
    else if (padding == "no_padding")
        return libcpp::aes::padding::no_padding;
    else
        return libcpp::aes::padding::pkcs7; // default
}

libcpp::des::mode str_to_des_mode(const std::string& mode)
{
    if (mode == "ecb")
        return libcpp::des::mode::ecb;
    else if (mode == "cbc")
        return libcpp::des::mode::cbc;
    else if (mode == "cfb")
        return libcpp::des::mode::cfb;
    else if (mode == "ofb")
        return libcpp::des::mode::ofb;
    else if (mode == "ctr")
        return libcpp::des::mode::ctr;
    else
        return libcpp::des::mode::ecb; // default
}

libcpp::des::padding str_to_des_padding(const std::string& padding)
{
    if (padding == "pkcs5")
        return libcpp::des::padding::pkcs5;
    else if (padding == "pkcs7")
        return libcpp::des::padding::pkcs7;
    else if (padding == "zero")
        return libcpp::des::padding::zero;
    else if (padding == "iso10126")
        return libcpp::des::padding::iso10126;
    else if (padding == "ansix923")
        return libcpp::des::padding::ansix923;
    else if (padding == "iso_iec_7816_4")
        return libcpp::des::padding::iso_iec_7816_4;
    else if (padding == "no_padding")
        return libcpp::des::padding::no_padding;
    else
        return libcpp::des::padding::pkcs7; // default
}

libcpp::rsa::padding str_to_rsa_padding(const std::string& padding)
{
    if (padding == "pkcs1")
        return libcpp::rsa::padding::pkcs1;
    else if (padding == "pkcs1_oaep")
        return libcpp::rsa::padding::pkcs1_oaep;
    else if (padding == "no_padding")
        return libcpp::rsa::padding::no_padding;
    else if (padding == "x931")
        return libcpp::rsa::padding::x931;
    else if (padding == "pkcs1_pss")
        return libcpp::rsa::padding::pkcs1_pss;
    else
        return libcpp::rsa::padding::pkcs1; // default
}

libcpp::rsa::key_format str_to_rsa_key_format(const std::string& fmt)
{
    if (fmt == "x509")
        return libcpp::rsa::key_format::x509;
    else if (fmt == "pkcs1")
        return libcpp::rsa::key_format::pkcs1;
    else
        return libcpp::rsa::key_format::x509; // default
}

libcpp::rsa::mode str_to_rsa_mode(const std::string& mode)
{
    if (mode == "none")
        return libcpp::rsa::mode::none;
    else if (mode == "aes_128_ecb")
        return libcpp::rsa::mode::aes_128_ecb;
    else if (mode == "aes_192_ecb")
        return libcpp::rsa::mode::aes_192_ecb;
    else if (mode == "aes_256_ecb")
        return libcpp::rsa::mode::aes_256_ecb;
    else if (mode == "aes_128_cbc")
        return libcpp::rsa::mode::aes_128_cbc;
    else if (mode == "aes_192_cbc")
        return libcpp::rsa::mode::aes_192_cbc;
    else if (mode == "aes_256_cbc")
        return libcpp::rsa::mode::aes_256_cbc;
    else if (mode == "aes_128_cfb")
        return libcpp::rsa::mode::aes_128_cfb;
    else if (mode == "aes_192_cfb")
        return libcpp::rsa::mode::aes_192_cfb;
    else if (mode == "aes_256_cfb")
        return libcpp::rsa::mode::aes_256_cfb;
    else if (mode == "aes_128_ofb")
        return libcpp::rsa::mode::aes_128_ofb;
    else if (mode == "aes_192_ofb")
        return libcpp::rsa::mode::aes_192_ofb;
    else if (mode == "aes_256_ofb")
        return libcpp::rsa::mode::aes_256_ofb;
    else
        return libcpp::rsa::mode::none; // default
}