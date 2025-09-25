#ifndef KEYGEN_H
#define KEYGEN_H

#include <string>
#include <vector>

#include "crypto_core.h"
#include <hj/crypto/rsa.hpp>

class keymaker
{
public:
    virtual std::string type() const = 0;
    virtual int make(
        std::vector<std::string>& keys,
        const std::string& fmt,
        const std::string& mode,
        const int bits) = 0;
};

class keymaker_mgr
{
public:
    keymaker_mgr()
        :_keymakers{}
    {
    }
    ~keymaker_mgr()
    {
        _keymakers.clear();
    }

    static keymaker_mgr& instance()
    {
        static keymaker_mgr inst;
        return inst;
    }

    std::vector<std::string> supported_algos();
    int add(std::unique_ptr<keymaker>&& km);
    int make(
        std::vector<std::string>& outs,
        const std::string& algo,
        const std::string& fmt,
        const std::string& mode,
        const int bits);

private:
    std::vector<std::unique_ptr<keymaker>> _keymakers;
};

class rsa_keymaker : public keymaker
{
public:
    inline std::string type() const override { return "rsa"; }
    inline std::vector<std::string>& supported_formats() 
    { 
        static std::vector<std::string> fmts{"x509", "pkcs1"};
        return fmts; 
    }
    inline std::vector<std::string>& supported_modes() 
    { 
        static std::vector<std::string> modes{
            "none",
            "aes_128_ecb", "aes_192_ecb", "aes_256_ecb",
            "aes_128_cbc", "aes_192_cbc", "aes_256_cbc",
            "aes_128_cfb", "aes_192_cfb", "aes_256_cfb",
            "aes_128_ofb", "aes_192_ofb", "aes_256_ofb"};
        return modes; 
    }
    int make(
        std::vector<std::string>& keys,
        const std::string& fmt,
        const std::string& mode,
        const int bits) override;

private:
    hj::rsa::key_format _to_key_format(const std::string& fmt);
    hj::rsa::mode _to_mode(const std::string& mode);
};

#endif