#include "encrypt.h"

#include "comm.h"
#include "format.h"
#include <hj/io/filepath.hpp>

std::vector<std::string> encryptor_mgr::supported_algos() 
{ 
    std::vector<std::string> algos;
    for (const auto& enc : _encryptors)
        algos.push_back(enc->type());

    return algos;
}

int encryptor_mgr::add(std::unique_ptr<encryptor>&& enc)
{
    for (const auto& e : _encryptors)
        if (e->type() == enc->type())
            return CRYPTO_ERR_FAIL;

    _encryptors.emplace_back(std::move(enc));
    return CRYPTO_OK;
}

int encryptor_mgr::encrypt(
    std::string& out,
    const std::string& in,
    const std::string& content,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& fmt)
{
    for (const auto& e : _encryptors)
    {
        if (e->type() != algo)
            continue;

        return e->encrypt(out, in, content, mode, key, padding, iv, fmt);
    }

    return CRYPTO_ERR_INVALID_ALGO;
}

// ------------------------------------ AES -------------------------------------
int aes_encryptor::encrypt(
    std::string& out,
    const std::string& in,
    const std::string& content,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& fmt)
{
    if (!_is_mode_valid(mode))
        return CRYPTO_ERR_INVALID_MODE;

    if (!_is_key_valid(key, mode))
        return CRYPTO_ERR_INVALID_KEY;

    if (!_is_padding_valid(padding))
        return CRYPTO_ERR_INVALID_PADDING;

    if (!_is_iv_valid(iv, mode))
        return CRYPTO_ERR_INVALID_IV;

    if (!_is_fmt_valid(fmt))
        return CRYPTO_ERR_INVALID_FMT;

    if (!_is_input_valid(out, in, content, mode, key, padding))
        return CRYPTO_ERR_INVALID_INPUT;

    // do encrypt
    int err = CRYPTO_ERR_FAIL;
    auto mod = str_to_aes_mode(mode);
    auto pad = str_to_aes_padding(padding);
    if (out == "" && in == "") // encrypt: mem -> mem
    {
        std::string tmp;
        err = hj::aes::encrypt(tmp, content, key, mod, pad, iv) ? 
            CRYPTO_OK : CRYPTO_ERR_ENCRYPT_AES_FAILED;
        if (err == CRYPTO_OK)
            err = formator::format(out, tmp, fmt, fmt_target::memory);
    }
    else if (out != "" && in != "") // encrypt: file -> file
    {
        err = hj::aes::encrypt_file(out, in, key, mod, pad, iv) ? 
            CRYPTO_OK : CRYPTO_ERR_ENCRYPT_AES_FAILED;
        if (err == CRYPTO_OK)
            err = formator::format(out, out, fmt, fmt_target::file);
    }
    else if (out != "" && in == "") // encrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(content);
        err = hj::aes::encrypt(
            fout, 
            sin, 
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            key.size(), 
            mod, 
            pad, 
            reinterpret_cast<const unsigned char*>(iv.c_str()), 
            iv.size()) ? CRYPTO_OK : CRYPTO_ERR_ENCRYPT_AES_FAILED;
        if (err == CRYPTO_OK)
            err = formator::format(out, out, fmt, fmt_target::file);
    }
    else // encrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout;
        err = hj::aes::encrypt(
            sout, 
            fin, 
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            key.size(),
            mod, 
            pad, 
            reinterpret_cast<const unsigned char*>(iv.c_str()), 
            iv.size()) ? CRYPTO_OK : CRYPTO_ERR_ENCRYPT_AES_FAILED;

        if (err == CRYPTO_OK)
            err = formator::format(out, sout.str(), fmt, fmt_target::memory);
    }

    return err;
}

bool aes_encryptor::_is_fmt_valid(const std::string& fmt)
{
    return formator::is_supported_style(fmt);
}

bool aes_encryptor::_is_mode_valid(const std::string& mode)
{
    auto modes = supported_modes();
    return std::find(std::begin(modes), std::end(modes), mode) != std::end(modes);
}

bool aes_encryptor::_is_key_valid(
    const std::string& key,
    const std::string& mode)
{
    auto mod = str_to_aes_mode(mode);
    return hj::aes::is_key_valid(
        mod, 
        reinterpret_cast<const unsigned char*>(key.c_str()), 
        key.size());
}

bool aes_encryptor::_is_padding_valid(const std::string& padding)
{
    auto paddings = supported_paddings();
    return std::find(std::begin(paddings), std::end(paddings), padding) != std::end(paddings);
}

bool aes_encryptor::_is_iv_valid(
    const std::string& iv, 
    const std::string& mode)
{
    const unsigned char* iv_ptr = (iv == "") ? nullptr : reinterpret_cast<const unsigned char*>(iv.c_str());
    return hj::aes::is_iv_valid(str_to_aes_mode(mode), iv_ptr, iv.size());
}

bool aes_encryptor::_is_input_valid(
    const std::string& out,
    const std::string& in, 
    const std::string& content,
    const std::string& mode,
    const std::string& key,
    const std::string& padding)
{
    if (in == "" && content == "")
        return false;

    auto mod = str_to_aes_mode(mode);
    auto pad = str_to_aes_padding(padding);
    // to mem
    if (out == "")
        return hj::aes::is_plain_valid(mod, key.size(), pad, content.size());

    // to file
    if (!hj::filepath::is_exist(in))
        return false;
    std::ifstream fin(in, std::ios::binary);
    return hj::aes::is_plain_valid(mod, key.size(), pad, fin);
}

// ------------------------------------ BASE64 -------------------------------------
int base64_encryptor::encrypt(
    std::string& out,
    const std::string& in,
    const std::string& content,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& fmt)
{
    if (!_is_fmt_valid(fmt))
        return CRYPTO_ERR_INVALID_FMT;

    int err = CRYPTO_ERR_FAIL;
    if (out == "" && in == "") // encrypt: mem -> mem
    {
        err = hj::base64::encode(out, content) ? 
            CRYPTO_OK : CRYPTO_ERR_ENCRYPT_BASE64_FAILED;
    }
    else if (out != "" && in != "") // encrypt: file -> file
    {
        err = hj::base64::encode_file(out, in) ? 
            CRYPTO_OK : CRYPTO_ERR_ENCRYPT_BASE64_FAILED;
    }
    else if (out != "" && in == "") // encrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(content);
        err = hj::base64::encode(fout, sin) ? 
            CRYPTO_OK : CRYPTO_ERR_ENCRYPT_BASE64_FAILED;
    }
    else // encrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        err = hj::base64::encode(sout, fin) ? 
            CRYPTO_OK : CRYPTO_ERR_ENCRYPT_BASE64_FAILED;
        out = sout.str();
    }

    return err;
}

bool base64_encryptor::_is_fmt_valid(const std::string& fmt)
{
    return formator::is_supported_style(fmt);
}

// ------------------------------------ DES -------------------------------------
int des_encryptor::encrypt(
    std::string& out,
    const std::string& in,
    const std::string& content,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& fmt)
{
    if (!_is_mode_valid(mode))
        return CRYPTO_ERR_INVALID_MODE;

    if (!_is_key_valid(key, mode))
        return CRYPTO_ERR_INVALID_KEY;

    if (!_is_padding_valid(padding))
        return CRYPTO_ERR_INVALID_PADDING;

    if (!_is_iv_valid(iv, mode))
        return CRYPTO_ERR_INVALID_IV;

    if (!_is_fmt_valid(fmt))
        return CRYPTO_ERR_INVALID_FMT;

    if (_is_input_valid(out, in, content, padding))
        return CRYPTO_ERR_INVALID_INPUT;

    int err = CRYPTO_ERR_FAIL;
    auto mod = str_to_des_mode(mode);
    auto pad = str_to_des_padding(padding);
    if (out == "" && in == "") // encrypt: mem -> mem
    {
        err = hj::des::encrypt(out, content, key, mod, pad, iv) ? 
            CRYPTO_OK : CRYPTO_ERR_ENCRYPT_DES_FAILED;
    }
    else if (out != "" && in != "") // encrypt: file -> file
    {
        err = hj::des::encrypt_file(out, in, key, mod, pad, iv) ? 
            CRYPTO_OK : CRYPTO_ERR_ENCRYPT_DES_FAILED;
    }
    else if (out != "" && in == "") // encrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(content);
        err = hj::des::encrypt(
            fout, 
            sin, 
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            key.size(),
            mod, 
            pad, 
            reinterpret_cast<const unsigned char*>(iv.c_str()), 
            iv.size()) ? CRYPTO_OK : CRYPTO_ERR_ENCRYPT_DES_FAILED;
    }
    else // encrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        err = hj::des::encrypt(
            sout, 
            fin, 
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            key.size(),
            mod, 
            pad, 
            reinterpret_cast<const unsigned char*>(iv.c_str()), 
            iv.size()) ? CRYPTO_OK : CRYPTO_ERR_ENCRYPT_DES_FAILED;
        out = sout.str();
    }

    return err;
}

bool des_encryptor::_is_input_valid(
    const std::string& out,
    const std::string& in, 
    const std::string& content,
    const std::string& padding)
{
    if (in == "" && content == "")
        return false;

    auto pad = str_to_des_padding(padding);
    // to mem
    if (out == "")
        return hj::des::is_plain_valid(reinterpret_cast<const unsigned char*>(content.c_str()), 
            content.size(), pad);
    
    // to file
    std::ifstream fin(in, std::ios::binary);
    return hj::des::is_plain_valid(fin, pad);
}

bool des_encryptor::_is_fmt_valid(const std::string& fmt)
{
    return formator::is_supported_style(fmt);
}

bool des_encryptor::_is_mode_valid(const std::string& mode)
{
    auto modes = supported_modes();
    return std::find(std::begin(modes), std::end(modes), mode) != std::end(modes);
}

bool des_encryptor::_is_key_valid(
    const std::string& key,
    const std::string& mode)
{
    return hj::des::is_key_valid(reinterpret_cast<const unsigned char*>(key.c_str()), key.size());
}

bool des_encryptor::_is_padding_valid(const std::string& padding)
{
    auto paddings = supported_paddings();
    return std::find(std::begin(paddings), std::end(paddings), padding) != std::end(paddings);
}

bool des_encryptor::_is_iv_valid(
    const std::string& iv, 
    const std::string& mode)
{
    const unsigned char* iv_ptr = (iv == "") ? nullptr : reinterpret_cast<const unsigned char*>(iv.c_str());
    auto mod = str_to_des_mode(mode);
    return hj::des::is_iv_valid(mod, iv_ptr, iv.size());
}

// ------------------------------------ MD5 -------------------------------------
int md5_encryptor::encrypt(
    std::string& out,
    const std::string& in,
    const std::string& content,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& fmt)
{
    if (!_is_fmt_valid(fmt))
        return CRYPTO_ERR_INVALID_FMT;

    int err = CRYPTO_ERR_FAIL;
    if (out == "" && in == "") // encrypt: mem -> mem
    {
        err = hj::md5::encode(out, content) ? CRYPTO_OK : CRYPTO_ERR_ENCRYPT_MD5_FAILED;
    }
    else if (out != "" && in != "") // encrypt: file -> file
    {
        err = hj::md5::encode_file(out, in) ? CRYPTO_OK : CRYPTO_ERR_ENCRYPT_MD5_FAILED;
    }
    else if (out != "" && in == "") // encrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(content);
        err = hj::md5::encode(fout, sin) ? CRYPTO_OK : CRYPTO_ERR_ENCRYPT_MD5_FAILED;
    }
    else // encrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        err = hj::md5::encode(sout, fin) ? CRYPTO_OK : CRYPTO_ERR_ENCRYPT_MD5_FAILED;
        out = sout.str();
    }

    return err;
}

bool md5_encryptor::_is_fmt_valid(const std::string& fmt)
{
    return formator::is_supported_style(fmt);
}

// ------------------------------------ SHA256 -------------------------------------
int sha256_encryptor::encrypt(
    std::string& out,
    const std::string& in,
    const std::string& content,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& fmt)
{
    if (!_is_fmt_valid(fmt))
        return CRYPTO_ERR_INVALID_FMT;

    int err = CRYPTO_ERR_FAIL;
    if (out == "" && in == "") // encrypt: mem -> mem
    {
        err = hj::sha::encode(out, content, hj::sha::algorithm::sha256) ? 
            CRYPTO_OK : CRYPTO_ERR_ENCRYPT_SHA256_FAILED;
    }
    else if (out != "" && in != "") // encrypt: file -> file
    {
        err = hj::sha::encode_file(out, in, hj::sha::algorithm::sha256) ? 
            CRYPTO_OK : CRYPTO_ERR_ENCRYPT_SHA256_FAILED;
    }
    else if (out != "" && in == "") // encrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(content);
        err = hj::sha::encode(fout, sin, hj::sha::algorithm::sha256) ? 
            CRYPTO_OK : CRYPTO_ERR_ENCRYPT_SHA256_FAILED;
    }
    else // encrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        err = hj::sha::encode(sout, fin, hj::sha::algorithm::sha256) ? 
            CRYPTO_OK : CRYPTO_ERR_ENCRYPT_SHA256_FAILED;
        out = sout.str();
    }

    return err;
}

bool sha256_encryptor::_is_fmt_valid(const std::string& fmt)
{
    return formator::is_supported_style(fmt);
}

// ------------------------------------ RSA -------------------------------------
int rsa_encryptor::encrypt(
    std::string& out,
    const std::string& in,
    const std::string& content,
    const std::string& mode,
    const std::string& key,
    const std::string& padding,
    const std::string& iv,
    const std::string& fmt)
{
    if (!_is_key_valid(key))
        return CRYPTO_ERR_INVALID_KEY;

    if (!_is_padding_valid(padding))
        return CRYPTO_ERR_INVALID_PADDING;

    if (!_is_fmt_valid(fmt))
        return CRYPTO_ERR_INVALID_FMT;

    if (!_is_input_valid(out, in, content, key, padding))
        return CRYPTO_ERR_INVALID_INPUT;

    int err = CRYPTO_ERR_FAIL;
    auto pad = str_to_rsa_padding(padding);
    if (out == "" && in == "") // encrypt: mem -> mem
    {
        err = hj::rsa::encrypt(out, content, key, pad) ? 
            CRYPTO_OK : CRYPTO_ERR_ENCRYPT_RSA_FAILED;
    }
    else if (out != "" && in != "") // encrypt: file -> file
    {
        err = hj::rsa::encrypt_file(out, in, key, pad) ? 
            CRYPTO_OK : CRYPTO_ERR_ENCRYPT_RSA_FAILED;
    }
    else if (out != "" && in == "") // encrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(content);
        err = hj::rsa::encrypt(
            fout, 
            sin, 
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            key.size(), 
            pad) ? CRYPTO_OK : CRYPTO_ERR_ENCRYPT_RSA_FAILED;
    }
    else // encrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        err = hj::rsa::encrypt(
            sout, 
            fin, 
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            key.size(), 
            pad) ? CRYPTO_OK : CRYPTO_ERR_ENCRYPT_RSA_FAILED;
        out = sout.str();
    }

    return err;
}

bool rsa_encryptor::_is_fmt_valid(const std::string& fmt)
{
    return formator::is_supported_style(fmt);
}

bool rsa_encryptor::_is_key_valid(const std::string& key)
{
    return hj::rsa::is_key_pair_bits_valid(key.size() * 8);
}

bool rsa_encryptor::_is_padding_valid(const std::string& padding)
{
    auto paddings = supported_paddings();
    return std::find(std::begin(paddings), std::end(paddings), padding) != std::end(paddings);
}

bool rsa_encryptor::_is_input_valid(
    const std::string& out,
    const std::string& in, 
    const std::string& content,
    const std::string& key,
    const std::string& padding)
{
    if (in == "" && content == "")
        return false;

    std::ifstream fin(in, std::ios::binary);
    auto pad = str_to_rsa_padding(padding);
    // to mem
    if (out == "")
        return hj::rsa::is_plain_valid(
            content.size(), 
            pad, 
            reinterpret_cast<const unsigned char*>(key.c_str()),
            key.size()); 

    // to file
    return hj::rsa::is_plain_valid(
        fin, 
        pad, 
        reinterpret_cast<const unsigned char*>(key.c_str()),
        key.size());
}