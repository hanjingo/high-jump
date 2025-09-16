#ifndef KEYGEN_H
#define KEYGEN_H

#include <string>
#include <vector>

#include "api.h"
#include <libcpp/crypto/rsa.hpp>

class key_maker
{
public:
    virtual std::string type() const = 0;
    virtual int make(
        std::vector<std::string>& keys,
        const std::string& fmt,
        const std::string& mode,
        const int bits) = 0;
};

class rsa_key_maker : public key_maker
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
    libcpp::rsa::key_format _to_key_format(const std::string& fmt);
    libcpp::rsa::mode _to_mode(const std::string& mode);
};

#endif