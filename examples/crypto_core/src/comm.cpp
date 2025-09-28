#include "comm.h"

#include "crypto_core.h"

hj::aes::mode str_to_aes_mode(const std::string &mode)
{
    if(mode == "ecb")
        return hj::aes::mode::ecb;
    else if(mode == "cbc")
        return hj::aes::mode::cbc;
    else if(mode == "cfb1")
        return hj::aes::mode::cfb1;
    else if(mode == "cfb8")
        return hj::aes::mode::cfb8;
    else if(mode == "cfb128")
        return hj::aes::mode::cfb128;
    else if(mode == "cfb")
        return hj::aes::mode::cfb;
    else if(mode == "ofb")
        return hj::aes::mode::ofb;
    else if(mode == "ctr")
        return hj::aes::mode::ctr;
    else if(mode == "gcm")
        return hj::aes::mode::gcm;
    else if(mode == "ccm")
        return hj::aes::mode::ccm;
    else if(mode == "xts")
        return hj::aes::mode::xts;
    else if(mode == "wrap")
        return hj::aes::mode::wrap;
    else if(mode == "wrap_pad")
        return hj::aes::mode::wrap_pad;
    else if(mode == "cbc_hmac_sha1")
        return hj::aes::mode::cbc_hmac_sha1;
    else if(mode == "cbc_hmac_sha256")
        return hj::aes::mode::cbc_hmac_sha256;
    else if(mode == "ocb")
        return hj::aes::mode::ocb;
    else
        return hj::aes::mode::ecb; // default
}

hj::aes::padding str_to_aes_padding(const std::string &padding)
{
    if(padding == "pkcs5")
        return hj::aes::padding::pkcs5;
    else if(padding == "pkcs7")
        return hj::aes::padding::pkcs7;
    else if(padding == "zero")
        return hj::aes::padding::zero;
    else if(padding == "iso10126")
        return hj::aes::padding::iso10126;
    else if(padding == "ansix923")
        return hj::aes::padding::ansix923;
    else if(padding == "iso_iec_7816_4")
        return hj::aes::padding::iso_iec_7816_4;
    else if(padding == "no_padding")
        return hj::aes::padding::no_padding;
    else
        return hj::aes::padding::pkcs7; // default
}

hj::des::mode str_to_des_mode(const std::string &mode)
{
    if(mode == "ecb")
        return hj::des::mode::ecb;
    else if(mode == "cbc")
        return hj::des::mode::cbc;
    else if(mode == "cfb")
        return hj::des::mode::cfb;
    else if(mode == "ofb")
        return hj::des::mode::ofb;
    else if(mode == "ctr")
        return hj::des::mode::ctr;
    else
        return hj::des::mode::ecb; // default
}

hj::des::padding str_to_des_padding(const std::string &padding)
{
    if(padding == "pkcs5")
        return hj::des::padding::pkcs5;
    else if(padding == "pkcs7")
        return hj::des::padding::pkcs7;
    else if(padding == "zero")
        return hj::des::padding::zero;
    else if(padding == "iso10126")
        return hj::des::padding::iso10126;
    else if(padding == "ansix923")
        return hj::des::padding::ansix923;
    else if(padding == "iso_iec_7816_4")
        return hj::des::padding::iso_iec_7816_4;
    else if(padding == "no_padding")
        return hj::des::padding::no_padding;
    else
        return hj::des::padding::pkcs7; // default
}

hj::rsa::padding str_to_rsa_padding(const std::string &padding)
{
    if(padding == "pkcs1")
        return hj::rsa::padding::pkcs1;
    else if(padding == "pkcs1_oaep")
        return hj::rsa::padding::pkcs1_oaep;
    else if(padding == "no_padding")
        return hj::rsa::padding::no_padding;
    else if(padding == "x931")
        return hj::rsa::padding::x931;
    else if(padding == "pkcs1_pss")
        return hj::rsa::padding::pkcs1_pss;
    else
        return hj::rsa::padding::pkcs1; // default
}

hj::rsa::key_format str_to_rsa_key_format(const std::string &fmt)
{
    if(fmt == "x509")
        return hj::rsa::key_format::x509;
    else if(fmt == "pkcs1")
        return hj::rsa::key_format::pkcs1;
    else
        return hj::rsa::key_format::x509; // default
}

hj::rsa::mode str_to_rsa_mode(const std::string &mode)
{
    if(mode == "none")
        return hj::rsa::mode::none;
    else if(mode == "aes_128_ecb")
        return hj::rsa::mode::aes_128_ecb;
    else if(mode == "aes_192_ecb")
        return hj::rsa::mode::aes_192_ecb;
    else if(mode == "aes_256_ecb")
        return hj::rsa::mode::aes_256_ecb;
    else if(mode == "aes_128_cbc")
        return hj::rsa::mode::aes_128_cbc;
    else if(mode == "aes_192_cbc")
        return hj::rsa::mode::aes_192_cbc;
    else if(mode == "aes_256_cbc")
        return hj::rsa::mode::aes_256_cbc;
    else if(mode == "aes_128_cfb")
        return hj::rsa::mode::aes_128_cfb;
    else if(mode == "aes_192_cfb")
        return hj::rsa::mode::aes_192_cfb;
    else if(mode == "aes_256_cfb")
        return hj::rsa::mode::aes_256_cfb;
    else if(mode == "aes_128_ofb")
        return hj::rsa::mode::aes_128_ofb;
    else if(mode == "aes_192_ofb")
        return hj::rsa::mode::aes_192_ofb;
    else if(mode == "aes_256_ofb")
        return hj::rsa::mode::aes_256_ofb;
    else
        return hj::rsa::mode::none; // default
}