#include "keygen.h"

#include <hj/crypto/rsa.hpp>

std::vector<std::string> keymaker_mgr::supported_algos()
{
    std::vector<std::string> algos;
    for(const auto &enc : _keymakers)
        algos.push_back(enc->type());

    return algos;
}

int keymaker_mgr::add(std::unique_ptr<keymaker> &&km)
{
    for(const auto &e : _keymakers)
        if(e->type() == km->type())
            return CRYPTO_ERR_FAIL;

    _keymakers.emplace_back(std::move(km));
    return OK;
}

int keymaker_mgr::make(std::vector<std::string> &outs,
                       const std::string        &algo,
                       const std::string        &fmt,
                       const std::string        &mode,
                       const int                 bits)
{
    for(const auto &e : _keymakers)
    {
        if(e->type() != algo)
            continue;

        return e->make(outs, fmt, mode, bits);
    }

    return CRYPTO_ERR_INVALID_ALGO;
}

// ----------------------------- RSA ------------------------------
int rsa_keymaker::make(std::vector<std::string> &keys,
                       const std::string        &fmt,
                       const std::string        &mode,
                       const int                 bits)
{
    auto        key_fmt  = _to_key_format(fmt);
    auto        key_mode = _to_mode(mode);
    std::string pubkey, prikey;
    if(!hj::rsa::make_key_pair(pubkey, prikey, bits, key_fmt, key_mode))
        return CRYPTO_ERR_KEYGEN_FAIL;

    keys.push_back(pubkey);
    keys.push_back(prikey);
    return OK;
}

hj::rsa::key_format rsa_keymaker::_to_key_format(const std::string &fmt)
{
    if(fmt == "x509")
        return hj::rsa::key_format::x509;
    else if(fmt == "pkcs1")
        return hj::rsa::key_format::pkcs1;
    else
        return hj::rsa::key_format::x509; // default
}

hj::rsa::mode rsa_keymaker::_to_mode(const std::string &mode)
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