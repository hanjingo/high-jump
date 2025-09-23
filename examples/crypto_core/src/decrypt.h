#ifndef DECRYPT_H
#define DECRYPT_H

#include <string>
#include <vector>

#include <hj/crypto/aes.hpp>
#include <hj/crypto/base64.hpp>
#include <hj/crypto/des.hpp>
#include <hj/crypto/rsa.hpp>

#include "api.h"

class decryptor
{
public:
    virtual const std::string type() = 0;
    virtual const std::vector<std::string>& supported_paddings() = 0;
    virtual const std::vector<std::string>& supported_modes() = 0;
    virtual int decrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& mode,
        const std::string& key,
        const std::string& passwd,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt) = 0;
};

class decryptor_mgr
{
public:
    decryptor_mgr()
        :_decryptors{}
    {
    }
    ~decryptor_mgr()
    {
        _decryptors.clear();
    }

    static decryptor_mgr& instance()
    {
        static decryptor_mgr inst;
        return inst;
    }

    std::vector<std::string> supported_algos();
    int add(std::unique_ptr<decryptor>&& enc);
    int decrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& algo,
        const std::string& mode,
        const std::string& key,
        const std::string& passwd,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt);

private:
    std::vector<std::unique_ptr<decryptor>> _decryptors;
};

class aes_decryptor : public decryptor
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
    int decrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& mode,
        const std::string& key,
        const std::string& passwd,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt) override;

private:
    bool _is_mode_valid(const std::string& mode);
    bool _is_key_valid(
        const std::string& key,
        const std::string& mode);
    bool _is_padding_valid(const std::string& padding);
    bool _is_iv_valid(
        const std::string& iv, 
        const std::string& mode);
    bool _is_fmt_valid(const std::string& fmt);
};

class base64_decryptor : public decryptor
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
    int decrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& mode,
        const std::string& key,
        const std::string& passwd,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt) override;
};

class des_decryptor : public decryptor
{
public:
    inline const std::string type() override { return std::string("des"); }
    inline const std::vector<std::string>& supported_modes() override 
    { 
        static std::vector<std::string> modes{"ecb", "cbc", "cfb", "ofb", "ctr"};
        return modes; 
    }
    inline const std::vector<std::string>& supported_paddings() override 
    { 
        static std::vector<std::string> paddings{
            "pkcs5", "pkcs7", "zero", "iso10126", "ansix923", "iso_iec_7816_4", "none"};
        return paddings; 
    }
    int decrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& mode,
        const std::string& key,
        const std::string& passwd,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt) override;

private:
    bool _is_mode_valid(const std::string& mode);
    bool _is_key_valid(
        const std::string& key,
        const std::string& mode);
    bool _is_padding_valid(const std::string& padding);
    bool _is_iv_valid(
        const std::string& iv, 
        const std::string& mode);
    bool _is_fmt_valid(const std::string& fmt);
};

class rsa_decryptor : public decryptor
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
        static std::vector<std::string> paddings{
            "pkcs1", "no_padding", "pkcs1_oaep", "x931", "pkcs1_pss"};
        return paddings;
    }
    int decrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& mode,
        const std::string& key,
        const std::string& passwd,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt) override;

private:
    bool _is_key_valid(const std::string& key);
    bool _is_input_valid(
        const std::string& out,
        const std::string& in, 
        const std::string& content,
        const std::string& key,
        const std::string& padding);
    bool _is_padding_valid(const std::string& padding);
    bool _is_passwd_valid(const std::string& passwd);
    bool _is_fmt_valid(const std::string& fmt);
};

#endif