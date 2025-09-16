#include "keygen.h"

#include <libcpp/crypto/rsa.hpp>

int rsa_key_maker::make(
    std::vector<std::string>& keys,
    const std::string& fmt,
    const std::string& mode,
    const int bits)
{
    auto key_fmt = _to_key_format(fmt);
    auto key_mode = _to_mode(mode);
    std::string pubkey, prikey;
    if (!libcpp::rsa::make_key_pair(pubkey, prikey, bits, key_fmt, key_mode))
        return ERR_KEYGEN_FAIL;

    keys.push_back(pubkey);
    keys.push_back(prikey);
    return OK;
}

libcpp::rsa::key_format rsa_key_maker::_to_key_format(const std::string& fmt)
{
    if (fmt == "x509")
        return libcpp::rsa::key_format::x509;
    else if (fmt == "pkcs1")
        return libcpp::rsa::key_format::pkcs1;
    else
        return libcpp::rsa::key_format::x509; // default
}

libcpp::rsa::mode rsa_key_maker::_to_mode(const std::string& mode)
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