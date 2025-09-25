#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include <hj/crypto/aes.hpp>
#include <hj/crypto/base64.hpp>
#include <hj/crypto/des.hpp>
#include <hj/crypto/md5.hpp>
#include <hj/crypto/sha.hpp>
#include <hj/crypto/rsa.hpp>

#include "crypto_core.h"

class encryptor
{
public:
    virtual const std::string type() = 0;
    virtual const std::vector<std::string>& supported_paddings() = 0;
    virtual const std::vector<std::string>& supported_modes() = 0;
    virtual int encrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& mode,
        const std::string& key,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt) = 0;
};

class encryptor_mgr
{
public:
    encryptor_mgr()
        :_encryptors{}
    {
    }
    ~encryptor_mgr()
    {
        _encryptors.clear();
    }

    static encryptor_mgr& instance()
    {
        static encryptor_mgr inst;
        return inst;
    }

    std::vector<std::string> supported_algos();
    int add(std::unique_ptr<encryptor>&& enc);
    int encrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& algo,
        const std::string& mode,
        const std::string& key,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt);

private:
    std::vector<std::unique_ptr<encryptor>> _encryptors;
};

class aes_encryptor : public encryptor
{
public:
    inline const std::string type() override { return std::string("aes"); }
    inline const std::vector<std::string>& supported_modes() override 
    { 
        static std::vector<std::string> modes{
            "ecb", "cbc", "cfb1", "cfb8", "cfb128", "cfb", "ofb", "ctr", "gcm", "ccm", "xts", 
            "wrap", "wrap_pad", "cbc_hmac_sha1", "cbc_hmac_sha256", "ocb"}; 
        return modes;
    }
    inline const std::vector<std::string>& supported_paddings() override 
    {
        static std::vector<std::string> paddings{
            "pkcs5", "pkcs7", "zero", "iso10126", "ansix923", "iso_iec_7816_4", "none"};
        return paddings;
    }
    int encrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& mode,
        const std::string& key,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt) override;

private:
    bool _is_fmt_valid(const std::string& fmt);
    bool _is_mode_valid(const std::string& mode);
    bool _is_key_valid(
        const std::string& key,
        const std::string& mode);
    bool _is_padding_valid(const std::string& padding);
    bool _is_iv_valid(
        const std::string& iv, 
        const std::string& mode);
    bool _is_input_valid(
        const std::string& out,
        const std::string& in, 
        const std::string& content,
        const std::string& mode,
        const std::string& key,
        const std::string& padding);
};

class base64_encryptor : public encryptor
{
public:
    inline const std::string type() override { return std::string("base64"); }
    inline const std::vector<std::string>& supported_modes() override 
    { 
        static std::vector<std::string> modes{};
        return modes; 
    }
    inline const std::vector<std::string>& supported_paddings() override 
    { 
        static std::vector<std::string> paddings{};
        return paddings; 
    }
    int encrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& mode,
        const std::string& key,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt) override;

private:
    bool _is_fmt_valid(const std::string& fmt);
};

class des_encryptor : public encryptor
{
public:
    inline const std::string type() override { return std::string("des"); }
    inline const std::vector<std::string>& supported_modes() override 
    { 
        static std::vector<std::string> modes{};
        return modes; 
    }
    inline const std::vector<std::string>& supported_paddings() override 
    { 
        static std::vector<std::string> paddings{};
        return paddings; 
    }
    int encrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& mode,
        const std::string& key,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt) override;

private:
    bool _is_fmt_valid(const std::string& fmt);
    bool _is_mode_valid(const std::string& mode);
    bool _is_key_valid(
        const std::string& key,
        const std::string& mode);
    bool _is_padding_valid(const std::string& padding);
    bool _is_iv_valid(
        const std::string& iv, 
        const std::string& mode);
    bool _is_input_valid(
        const std::string& out,
        const std::string& in, 
        const std::string& content,
        const std::string& padding);
};

class md5_encryptor : public encryptor
{
public:
    inline const std::string type() override { return std::string("md5"); }
    inline const std::vector<std::string>& supported_modes() override 
    { 
        static std::vector<std::string> modes{};
        return modes; 
    }
    inline const std::vector<std::string>& supported_paddings() override 
    { 
        static std::vector<std::string> paddings{};
        return paddings; 
    }
    int encrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& mode,
        const std::string& key,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt) override;

private:
    bool _is_fmt_valid(const std::string& fmt);
};

class sha256_encryptor : public encryptor
{
public:
    inline const std::string type() override { return std::string("sha256"); }
    inline const std::vector<std::string>& supported_modes() override 
    { 
        static std::vector<std::string> modes{};
        return modes; 
    }
    inline const std::vector<std::string>& supported_paddings() override 
    { 
        static std::vector<std::string> paddings{};
        return paddings; 
    }
    int encrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& mode,
        const std::string& key,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt) override;

private:
    bool _is_fmt_valid(const std::string& fmt);
};

class rsa_encryptor : public encryptor
{
public:
    inline const std::string type() override { return std::string("rsa"); }
    inline const std::vector<std::string>& supported_modes() override 
    { 
        static std::vector<std::string> modes{};
        return modes; 
    }
    inline const std::vector<std::string>& supported_paddings() override 
    { 
        static std::vector<std::string> paddings{};
        return paddings; 
    }
    int encrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& mode,
        const std::string& key,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt) override;

private:
    bool _is_fmt_valid(const std::string& fmt);
    bool _is_key_valid(const std::string& key);
    bool _is_padding_valid(const std::string& padding);
    bool _is_input_valid(
        const std::string& out,
        const std::string& in, 
        const std::string& content,
        const std::string& key,
        const std::string& padding);
};

#endif