#ifndef AES_HPP
#define AES_HPP

#include <fstream>
#include <iostream>

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#ifndef AES_BUF_SIZE
#define AES_BUF_SIZE 4096
#endif

namespace libcpp
{

class aes
{
public:
    enum cipher 
    {
        aes_128_ecb,
        aes_128_cbc,
        aes_128_cfb1,
        aes_128_cfb8,
        aes_128_cfb128,
        aes_128_ofb,
        aes_128_ctr,
        aes_128_ccm,
        aes_128_gcm,
        aes_128_xts,
        aes_128_wrap,
        aes_128_wrap_pad,
# ifndef OPENSSL_NO_OCB
        aes_128_ocb,
# endif

        aes_192_ecb,
        aes_192_cbc,
        aes_192_cfb1,
        aes_192_cfb8,
        aes_192_cfb128,
        aes_192_ofb,
        aes_192_ctr,
        aes_192_ccm,
        aes_192_gcm,
        aes_192_wrap,
        aes_192_wrap_pad,
# ifndef OPENSSL_NO_OCB
        aes_192_ocb,
# endif

        aes_256_ecb,
        aes_256_cbc,
        aes_256_cfb1,
        aes_256_cfb8,
        aes_256_cfb128,
        aes_256_ofb,
        aes_256_ctr,
        aes_256_ccm,
        aes_256_gcm,
        aes_256_xts,
        aes_256_wrap,
        aes_256_wrap_pad,
# ifndef OPENSSL_NO_OCB
        aes_256_ocb,
#endif

        aes_128_cbc_hmac_sha1,
        aes_256_cbc_hmac_sha1,
        aes_128_cbc_hmac_sha256,
        aes_256_cbc_hmac_sha256
    };

public:
    static bool encode(const unsigned char* src, 
                       const unsigned long src_len, 
                       unsigned char* dst,
                       unsigned long& dst_len,
                       const unsigned char* key, 
                       const unsigned long key_len,
                       const unsigned char* iv,
                       const cipher cip = aes_256_cbc)
    {
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
            return false;

        if (1 != EVP_EncryptInit_ex(ctx, _select_cipher(cip), NULL, key, iv))
        {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        int len = 0;
        if (1 != EVP_EncryptUpdate(ctx, dst, &len, src, src_len))
        {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        dst_len = len;
        if (1 != EVP_EncryptFinal_ex(ctx, dst + len, &len))
        {
            dst_len = 0;
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        dst_len += len;
        EVP_CIPHER_CTX_free(ctx);
        return true;
    }

    static bool encode(const std::string& src, 
                       std::string& dst, 
                       const std::string& key, 
                       const std::string& iv, 
                       const cipher cip = aes_256_cbc)
    {
        dst.resize(src.size() + EVP_CIPHER_block_size(_select_cipher(cip)));
        unsigned long dst_len = dst.size();
        if (!encode(reinterpret_cast<const unsigned char*>(src.c_str()), 
                    src.size(), 
                    reinterpret_cast<unsigned char*>(const_cast<char*>(dst.data())), 
                    dst_len, 
                    reinterpret_cast<const unsigned char*>(key.c_str()), 
                    key.size(), 
                    reinterpret_cast<const unsigned char*>(iv.c_str()), 
                    cip))
            return false;

        dst.resize(dst_len);
        return true;
    }

    static bool encode_file(const unsigned char* src_file_path, 
                            const unsigned char* dst_file_path,
                            const unsigned char* key, 
                            const unsigned long key_len,
                            const unsigned char* iv,
                            const cipher cip = aes_256_cbc)
    {
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
            return false;

        if (1 != EVP_EncryptInit_ex(ctx, _select_cipher(cip), NULL, 
                reinterpret_cast<const unsigned char*>(key), reinterpret_cast<const unsigned char*>(iv)))
        {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        std::ifstream in(reinterpret_cast<const char*>(src_file_path), std::ios::binary);
        std::ofstream out(reinterpret_cast<const char*>(dst_file_path), std::ios::binary);
        if (!in || !out)
        {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        unsigned char inbuf[AES_BUF_SIZE];
        unsigned char outbuf[AES_BUF_SIZE + EVP_MAX_BLOCK_LENGTH];
        int outlen = 0;
        std::streamsize inlen = 0;
        while (in)
        {
            in.read((char*)inbuf, AES_BUF_SIZE);
            inlen = in.gcount();
            if (inlen <= 0)
                break;

            if (1 != EVP_EncryptUpdate(ctx, outbuf, &outlen, inbuf, in.gcount()))
            {
                EVP_CIPHER_CTX_free(ctx);
                return false;
            }
            out.write(reinterpret_cast<char*>(outbuf), outlen);
        }

        if (1 != EVP_EncryptFinal_ex(ctx, outbuf, &outlen))
        {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        out.write(reinterpret_cast<char*>(outbuf), outlen);

        EVP_CIPHER_CTX_free(ctx);
        return true;
    }

    static bool encode_file(const std::string& src_file_path,
                            const std::string& dst_file_path,
                            const std::string& key,
                            const std::string& iv,
                            const cipher cip = aes_256_cbc)
    {
        return encode_file(reinterpret_cast<const unsigned char*>(src_file_path.c_str()),
                           reinterpret_cast<const unsigned char*>(dst_file_path.c_str()),
                           reinterpret_cast<const unsigned char*>(key.c_str()),
                           key.size(),
                           reinterpret_cast<const unsigned char*>(iv.c_str()),
                           cip);
    }

    static bool decode(const unsigned char* src,
                       const unsigned long src_len,
                       unsigned char* dst,
                       unsigned long& dst_len,
                       const unsigned char* key,
                       const unsigned long key_len,
                       const unsigned char* iv,
                       const cipher cip = aes_256_cbc)
    {
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
            return false;

        if (1 != EVP_DecryptInit_ex(ctx, _select_cipher(cip), NULL, key, iv))
        {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        int len;
        if (1 != EVP_DecryptUpdate(ctx, dst, &len, src, src_len))
        {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        dst_len = len;
        if (1 != EVP_DecryptFinal_ex(ctx, dst + len, &len))
        {
            dst_len = 0;
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        dst_len += len;
        EVP_CIPHER_CTX_free(ctx);
        return true;
    }

    static bool decode(const std::string& src, 
                       std::string& dst, 
                       const std::string& key, 
                       const std::string& iv, 
                       const cipher cip = aes_256_cbc)
    {
        dst.resize(src.size());
        unsigned long dst_len = dst.size();
        if (!decode(reinterpret_cast<const unsigned char*>(src.c_str()), 
                    src.size(), 
                    reinterpret_cast<unsigned char*>(const_cast<char*>(dst.data())),
                    dst_len, 
                    reinterpret_cast<const unsigned char*>(key.c_str()), 
                    key.size(), 
                    reinterpret_cast<const unsigned char*>(iv.c_str()), 
                    cip))
        {
            dst.clear();
            return false;
        }

        dst.resize(dst_len);
        return true;
    }

    static bool decode_file(const unsigned char* src_file_path,
                            const unsigned char* dst_file_path,
                            const unsigned char* key,
                            const unsigned long key_len,
                            const unsigned char* iv,
                            const cipher cip = aes_256_cbc)
    {
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
            return false;

        if (1 != EVP_DecryptInit_ex(ctx, _select_cipher(cip), NULL, 
                reinterpret_cast<const unsigned char*>(key), reinterpret_cast<const unsigned char*>(iv)))
        {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        std::ifstream in(reinterpret_cast<const char*>(src_file_path), std::ios::binary);
        std::ofstream out(reinterpret_cast<const char*>(dst_file_path), std::ios::binary);
        if (!in || !out)
        {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        unsigned char inbuf[AES_BUF_SIZE];
        unsigned char outbuf[AES_BUF_SIZE + EVP_MAX_BLOCK_LENGTH];
        int outlen = 0;
        std::streamsize inlen = 0;
        while (in)
        {
            in.read((char*)inbuf, AES_BUF_SIZE);
            inlen = in.gcount();
            if (inlen <= 0)
                break;

            if (1 != EVP_DecryptUpdate(ctx, outbuf, &outlen, inbuf, in.gcount()))
            {
                EVP_CIPHER_CTX_free(ctx);
                return false;
            }
            out.write(reinterpret_cast<char*>(outbuf), outlen);
        }

        if (1 != EVP_DecryptFinal_ex(ctx, outbuf, &outlen))
        {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        out.write(reinterpret_cast<char*>(outbuf), outlen);

        EVP_CIPHER_CTX_free(ctx);
        return true;
    }

    static bool decode_file(const std::string& src_file_path,
                            const std::string& dst_file_path,
                            const std::string& key,
                            const std::string& iv,
                            const cipher cip = aes_256_cbc)
    {
        return decode_file(reinterpret_cast<const unsigned char*>(src_file_path.c_str()),
                           reinterpret_cast<const unsigned char*>(dst_file_path.c_str()),
                           reinterpret_cast<const unsigned char*>(key.c_str()),
                           key.size(),
                           reinterpret_cast<const unsigned char*>(iv.c_str()),
                           cip);
    }

private:
    static const EVP_CIPHER* _select_cipher(const cipher cip)
    {
        switch (cip)
        {
        case cipher::aes_128_ecb: { return EVP_aes_128_ecb(); }
        case cipher::aes_128_cbc: { return EVP_aes_128_cbc(); }
        case cipher::aes_128_cfb1: { return EVP_aes_128_cfb1(); }
        case cipher::aes_128_cfb8: { return EVP_aes_128_cfb8(); }
        case cipher::aes_128_cfb128: { return EVP_aes_128_cfb128(); }
        case cipher::aes_128_ofb: { return EVP_aes_128_ofb(); }
        case cipher::aes_128_ctr: { return EVP_aes_128_ctr(); }
        case cipher::aes_128_ccm: { return EVP_aes_128_ccm(); }
        case cipher::aes_128_gcm: { return EVP_aes_128_gcm(); }
        case cipher::aes_128_xts: { return EVP_aes_128_xts(); }
        case cipher::aes_128_wrap: { return EVP_aes_128_wrap(); }
        case cipher::aes_128_wrap_pad: { return EVP_aes_128_wrap_pad(); }
# ifndef OPENSSL_NO_OCB
        case cipher::aes_128_ocb: { return EVP_aes_128_ocb(); }
# endif

        case cipher::aes_192_ecb: { return EVP_aes_192_ecb(); }
        case cipher::aes_192_cbc: { return EVP_aes_192_cbc(); }
        case cipher::aes_192_cfb1: { return EVP_aes_192_cfb1(); }
        case cipher::aes_192_cfb8: { return EVP_aes_192_cfb8(); }
        case cipher::aes_192_cfb128: { return EVP_aes_192_cfb128(); }
        case cipher::aes_192_ofb: { return EVP_aes_192_ofb(); }
        case cipher::aes_192_ctr: { return EVP_aes_192_ctr(); }
        case cipher::aes_192_ccm: { return EVP_aes_192_ccm(); }
        case cipher::aes_192_gcm: { return EVP_aes_192_gcm(); }
        case cipher::aes_192_wrap: { return EVP_aes_192_wrap(); }
        case cipher::aes_192_wrap_pad: { return EVP_aes_192_wrap_pad(); }
# ifndef OPENSSL_NO_OCB
        case cipher::aes_192_ocb: { return EVP_aes_192_ocb(); }
#endif

        case cipher::aes_256_ecb: { return EVP_aes_256_ecb(); }
        case cipher::aes_256_cbc: { return EVP_aes_256_cbc(); }
        case cipher::aes_256_cfb1: { return EVP_aes_256_cfb1(); }
        case cipher::aes_256_cfb8: { return EVP_aes_256_cfb8(); }
        case cipher::aes_256_cfb128: { return EVP_aes_256_cfb128(); }
        case cipher::aes_256_ofb: { return EVP_aes_256_ofb(); }
        case cipher::aes_256_ctr: { return EVP_aes_256_ctr(); }
        case cipher::aes_256_ccm: { return EVP_aes_256_ccm(); }
        case cipher::aes_256_gcm: { return EVP_aes_256_gcm(); }
        case cipher::aes_256_xts: { return EVP_aes_256_xts(); }
        case cipher::aes_256_wrap: { return EVP_aes_256_wrap(); }
        case cipher::aes_256_wrap_pad: { return EVP_aes_256_wrap_pad(); }
# ifndef OPENSSL_NO_OCB
        case cipher::aes_256_ocb: { return EVP_aes_256_ocb(); }
#endif

        case aes_128_cbc_hmac_sha1: { return EVP_aes_128_cbc_hmac_sha1(); }
        case aes_256_cbc_hmac_sha1: { return EVP_aes_256_cbc_hmac_sha1(); }
        case aes_128_cbc_hmac_sha256: { return EVP_aes_128_cbc_hmac_sha256(); }
        case aes_256_cbc_hmac_sha256: { return EVP_aes_256_cbc_hmac_sha256(); }
        }
    }

private:
    aes() = default;
    ~aes() = default;
    aes(const aes&) = delete;
    aes& operator=(const aes&) = delete;
    aes(aes&&) = delete;
    aes& operator=(aes&&) = delete;
};

}

#endif