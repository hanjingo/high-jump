#include "decrypt.h"

#include <hj/io/filepath.hpp>

#include "comm.h"
#include "format.h"

std::vector<std::string> decryptor_mgr::supported_algos() 
{ 
    std::vector<std::string> algos;
    for (const auto& enc : _decryptors)
        algos.push_back(enc->type());

    return algos;
}

int decryptor_mgr::add(std::unique_ptr<decryptor>&& enc)
{
    for (const auto& e : _decryptors)
        if (e->type() == enc->type())
            return CRYPTO_ERR_FAIL;

    _decryptors.emplace_back(std::move(enc));
    return CRYPTO_OK;
}

int decryptor_mgr::decrypt(
    std::string& out,
    const std::string& in,
    const std::string& content,
    const std::string& algo,
    const std::string& mode,
    const std::string& key,
    const std::string& passwd,
    const std::string& padding,
    const std::string& iv,
    const std::string& fmt)
{
    for (const auto& e : _decryptors)
    {
        if (e->type() != algo)
            continue;

        return e->decrypt(out, in, content, mode, key, passwd, padding, iv, fmt);
    }

    return CRYPTO_ERR_INVALID_ALGO;
}

// ------------------------------------ AES -------------------------------------
int aes_decryptor::decrypt(
    std::string& out,
    const std::string& in,
    const std::string& content,
    const std::string& mode,
    const std::string& key,
    const std::string& passwd,
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

    auto mod = str_to_aes_mode(mode);
    auto pad = str_to_aes_padding(padding);
    int err = CRYPTO_ERR_FAIL;
    if (out == "" && in == "") // decrypt: mem -> mem
    {
        std::string tmp;
        err = formator::unformat(tmp, content, fmt, fmt_target::memory);
        if (err == CRYPTO_OK)
            err = hj::aes::decrypt(out, tmp, key, mod, pad, iv) ? 
                CRYPTO_OK : CRYPTO_ERR_DECRYPT_AES_FAILED;
    }
    else if (out != "" && in != "") // decrypt: file -> file
    {
        std::string tmp = in + ".tmp";
        err = formator::unformat(tmp, in, fmt, fmt_target::file);
        if (err == CRYPTO_OK)
            err = hj::aes::decrypt_file(out, tmp, key, mod, pad, iv) ? 
                CRYPTO_OK : CRYPTO_ERR_DECRYPT_AES_FAILED;
        hj::filepath::remove(tmp);
    }
    else if (out != "" && in == "") // decrypt: mem -> file
    {
        std::string tmp;
        err = formator::unformat(tmp, content, fmt, fmt_target::memory);
        if (err == CRYPTO_OK)
        {
            std::istringstream sin(tmp);
            std::ofstream fout(out, std::ios::binary);
            err = hj::aes::decrypt(
                fout, 
                sin, 
                reinterpret_cast<const unsigned char*>(key.c_str()), 
                key.size(),
                mod, 
                pad, 
                reinterpret_cast<const unsigned char*>(iv.c_str()), 
                iv.size()) ? CRYPTO_OK : CRYPTO_ERR_DECRYPT_AES_FAILED;
        }
    }
    else // decrypt: file -> mem
    {
        std::string tmp = in + ".tmp";
        err = formator::unformat(tmp, in, fmt, fmt_target::file);
        if (err == CRYPTO_OK)
        {
            std::ifstream fin(tmp, std::ios::binary);
            std::ostringstream sout(out);
            err = hj::aes::decrypt(
                sout, 
                fin, 
                reinterpret_cast<const unsigned char*>(key.c_str()), 
                key.size(),
                mod, 
                pad, 
                reinterpret_cast<const unsigned char*>(iv.c_str()), 
                iv.size()) ? CRYPTO_OK : CRYPTO_ERR_DECRYPT_AES_FAILED;
            out = sout.str();
        }
        hj::filepath::remove(tmp);
    }

    return err;
}

bool aes_decryptor::_is_mode_valid(const std::string& mode)
{
    auto modes = supported_modes();
    return std::find(std::begin(modes), std::end(modes), mode) != std::end(modes);
}

bool aes_decryptor::_is_key_valid(
    const std::string& key,
    const std::string& mode)
{
    auto mod = str_to_aes_mode(mode);
    return hj::aes::is_key_valid(
        mod, 
        reinterpret_cast<const unsigned char*>(key.c_str()), 
        key.size());
}

bool aes_decryptor::_is_padding_valid(const std::string& padding)
{
    auto paddings = supported_paddings();
    return std::find(std::begin(paddings), std::end(paddings), padding) != std::end(paddings);
}

bool aes_decryptor::_is_iv_valid(
    const std::string& iv, 
    const std::string& mode)
{
    const unsigned char* iv_ptr = (iv == "") ? nullptr : reinterpret_cast<const unsigned char*>(iv.c_str());
    auto mod = str_to_aes_mode(mode);
    return hj::aes::is_iv_valid(mod, iv_ptr, iv.size());
}

bool aes_decryptor::_is_fmt_valid(const std::string& fmt)
{
    return formator::is_supported_style(fmt);
}

// ------------------------------------ BASE64 -------------------------------------
int base64_decryptor::decrypt(
    std::string& out,
    const std::string& in,
    const std::string& content,
    const std::string& mode,
    const std::string& key,
    const std::string& passwd,
    const std::string& padding,
    const std::string& iv,
    const std::string& fmt)
{
    int err = CRYPTO_ERR_FAIL;
    if (out == "" && in == "") // decrypt: mem -> mem
    {
        err = hj::base64::decode(out, content) ? CRYPTO_OK : CRYPTO_ERR_DECRYPT_BASE64_FAILED;
    }
    else if (out != "" && in != "") // decrypt: file -> file
    {
        err = hj::base64::decode_file(out, in) ? CRYPTO_OK : CRYPTO_ERR_DECRYPT_BASE64_FAILED;
    }
    else if (out != "" && in == "") // decrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(content);
        err = hj::base64::decode(fout, sin);
    }
    else // decrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        err = hj::base64::decode(sout, fin) ? CRYPTO_OK : CRYPTO_ERR_DECRYPT_BASE64_FAILED;
        out = sout.str();
    }

    return err;
}

// ------------------------------------ DES -------------------------------------
int des_decryptor::decrypt(
    std::string& out,
    const std::string& in,
    const std::string& content,
    const std::string& mode,
    const std::string& key,
    const std::string& passwd,
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

    auto mod = str_to_des_mode(mode);
    auto pad = str_to_des_padding(padding);
    int err = CRYPTO_ERR_FAIL;
    if (out == "" && in == "") // decrypt: mem -> mem
    {
        err = hj::des::decrypt(out, content, key, mod, pad, iv) ? 
            CRYPTO_OK : CRYPTO_ERR_DECRYPT_DES_FAILED;
    }
    else if (out != "" && in != "") // decrypt: file -> file
    {
        err = hj::des::decrypt_file(out, in, key, mod, pad, iv) ? 
            CRYPTO_OK : CRYPTO_ERR_DECRYPT_DES_FAILED;
    }
    else if (out != "" && in == "") // decrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(content);
        err = hj::des::decrypt(
            fout, 
            sin, 
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            key.size(),
            mod, 
            pad, 
            reinterpret_cast<const unsigned char*>(iv.c_str()), 
            iv.size()) ? CRYPTO_OK : CRYPTO_ERR_DECRYPT_DES_FAILED;
    }
    else // decrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        err = hj::des::decrypt(
            sout, 
            fin, 
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            key.size(),
            mod, 
            pad, 
            reinterpret_cast<const unsigned char*>(iv.c_str()), 
            iv.size()) ? CRYPTO_OK : CRYPTO_ERR_DECRYPT_DES_FAILED;
        out = sout.str();
    }

    return err;
}

bool des_decryptor::_is_mode_valid(const std::string& mode)
{
    auto modes = supported_modes();
    return std::find(std::begin(modes), std::end(modes), mode) != std::end(modes);
}

bool des_decryptor::_is_key_valid(
    const std::string& key,
    const std::string& mode)
{
    return hj::des::is_key_valid(
        reinterpret_cast<const unsigned char*>(key.c_str()), 
        key.size());
}

bool des_decryptor::_is_padding_valid(const std::string& padding)
{
    auto paddings = supported_paddings();
    return std::find(std::begin(paddings), std::end(paddings), padding) != std::end(paddings);
}

bool des_decryptor::_is_iv_valid(
    const std::string& iv, 
    const std::string& mode)
{
    const unsigned char* iv_ptr = (iv == "") ? nullptr : reinterpret_cast<const unsigned char*>(iv.c_str());
    auto mod = str_to_des_mode(mode);
    return hj::des::is_iv_valid(mod, iv_ptr, iv.size());
}

bool des_decryptor::_is_fmt_valid(const std::string& fmt)
{
    return formator::is_supported_style(fmt);
}

// ------------------------------------ RSA -------------------------------------
int rsa_decryptor::decrypt(
    std::string& out,
    const std::string& in,
    const std::string& content,
    const std::string& mode,
    const std::string& key,
    const std::string& passwd,
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

    auto pad = str_to_rsa_padding(padding);
    int err = CRYPTO_ERR_FAIL;
    if (out == "" && in == "") // decrypt: mem -> mem
    {
        err = hj::rsa::decrypt(out, content, key, pad, "") ? 
            CRYPTO_OK : CRYPTO_ERR_DECRYPT_RSA_FAILED;
    }
    else if (out != "" && in != "") // decrypt: file -> file
    {
        err = hj::rsa::decrypt_file(out, in, key, pad, "") ? 
            CRYPTO_OK : CRYPTO_ERR_DECRYPT_RSA_FAILED;
    }
    else if (out != "" && in == "") // decrypt: mem -> file
    {
        std::ofstream fout(out, std::ios::binary);
        std::istringstream sin(content);
        err = hj::rsa::decrypt(
            fout, 
            sin, 
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            key.size(),
            pad, 
            nullptr, 
            0) ? CRYPTO_OK : CRYPTO_ERR_DECRYPT_RSA_FAILED;
    }
    else // decrypt: file -> mem
    {
        std::ifstream fin(in, std::ios::binary);
        std::ostringstream sout(out);
        err = hj::rsa::decrypt(
            sout, 
            fin, 
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            key.size(),
            pad, 
            nullptr, 
            0) ? CRYPTO_OK : CRYPTO_ERR_DECRYPT_RSA_FAILED;
        out = sout.str();
    }

    return err;
}

bool rsa_decryptor::_is_key_valid(const std::string& key)
{
    return hj::rsa::is_key_pair_bits_valid(key.size() * 8);
}

bool rsa_decryptor::_is_padding_valid(const std::string& padding)
{
    auto paddings = supported_paddings();
    return std::find(std::begin(paddings), std::end(paddings), padding) != std::end(paddings);
}

bool rsa_decryptor::_is_input_valid(
    const std::string& out,
    const std::string& in, 
    const std::string& content,
    const std::string& key,
    const std::string& padding)
{
    // to mem
    if (out == "")
    {
        auto pad = str_to_rsa_padding(padding);
        return hj::rsa::is_cipher_valid(content, pad, key);
    }

    // to file
    std::ifstream fin(in, std::ios::binary);
    auto pad = str_to_rsa_padding(padding);
    return hj::rsa::is_cipher_valid(
        fin, 
        pad, 
        reinterpret_cast<const unsigned char*>(key.c_str()),
        key.size());
}

bool rsa_decryptor::_is_fmt_valid(const std::string& fmt)
{
    return formator::is_supported_style(fmt);
}