#ifndef RSA_HPP
#define RSA_HPP

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <string>
#include <vector>

namespace libcpp
{

class rsa
{
public:
    using key_t = RSA;

public:
    static key_t* load_public_key(const unsigned char* pubkey_pem, const int pubkey_pem_len)
    {
        BIO* bio = BIO_new_mem_buf(pubkey_pem, pubkey_pem_len);
        if (!bio) 
            return nullptr;

        RSA* rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        return rsa;
    }

    static key_t* load_public_key(const std::string& pubkey_pem) 
    {
        return load_public_key(reinterpret_cast<const unsigned char*>(pubkey_pem.c_str()), pubkey_pem.size());    
    }

    static key_t* load_private_key(const unsigned char* prikey_pem, const int prikey_pem_len)
    {
        BIO* bio = BIO_new_mem_buf(prikey_pem, prikey_pem_len);
        if (!bio) 
            return nullptr;

        RSA* rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        return rsa;
    }

    static key_t* load_private_key(const std::string& prikey_pem)
    {
        return load_private_key(reinterpret_cast<const unsigned char*>(prikey_pem.c_str()), prikey_pem.size());
    }
};

}

#endif