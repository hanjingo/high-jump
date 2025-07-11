/*
*  This file is part of libcpp.
*  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RSA_HPP
#define RSA_HPP

// disable msvc safe check warning
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// support deprecated api for low version openssl
#ifndef OPENSSL_SUPPRESS_DEPRECATED
#define OPENSSL_SUPPRESS_DEPRECATED
#endif

#include <string>
#include <fstream>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#ifndef RSA_MAX_KEY_LENGTH
#define RSA_MAX_KEY_LENGTH 4096
#endif

namespace libcpp
{

class rsa
{
public:
    enum class key_format 
    {
        x509,    // X.509 (-----BEGIN PUBLIC KEY-----) (-----BEGIN PRIVATE KEY-----)
        pkcs1,   // PKCS#1 (-----BEGIN RSA PUBLIC KEY-----) (-----BEGIN RSA PRIVATE KEY-----)
    };

    enum class cipher 
    {
        none, // no encryption

        // AES
        aes_128_ecb,
        aes_192_ecb,
        aes_256_ecb,

        aes_128_cbc,
        aes_192_cbc,
        aes_256_cbc,

        aes_128_cfb,
        aes_192_cfb,
        aes_256_cfb,

        aes_128_ofb,
        aes_192_ofb,
        aes_256_ofb,

        // 3DES
        des_ede3_ecb,
        des_ede3_cbc,
        des_ede3_cfb,
        des_ede3_ofb,
        des_ede_cbc,

        // DES (not recommended)
        des_ecb,
        des_cbc,
        des_cfb,
        des_ofb,

        // Blowfish
        bf_ecb,
        bf_cbc,
        bf_cfb,
        bf_ofb,

        // CAST5
        cast5_ecb,
        cast5_cbc,
        cast5_cfb,
        cast5_ofb,

        // RC2
        rc2_ecb,
        rc2_cbc,
        rc2_cfb,
        rc2_ofb,
    };

    enum class padding
    {
        pkcs1 = RSA_PKCS1_PADDING,
        no_padding = RSA_NO_PADDING,
        pkcs1_oaep = RSA_PKCS1_OAEP_PADDING,
        x931 = RSA_X931_PADDING,

#ifdef RSA_PKCS1_PSS_PADDING
        pkcs1_pss = RSA_PKCS1_PSS_PADDING,
#endif
    };

    // use pubkey to encrypt
    static bool encode(unsigned char* dst,
                       std::size_t& dst_len,
                       const unsigned char* src, 
                       const std::size_t src_len, 
                       const unsigned char* pubkey_pem,
                       const std::size_t pubkey_pem_len,
                       const padding padding = padding::pkcs1)
    {
        RSA* rsa = _load_public_key(pubkey_pem, pubkey_pem_len);
        if (!rsa)
            return false;

        if (dst_len < static_cast<std::size_t>(RSA_size(rsa)))
        {
            RSA_free(rsa);
            return false;
        }

        int len = RSA_public_encrypt((int)src_len, src, dst, rsa, static_cast<int>(padding));
        RSA_free(rsa);
        if (len == -1) 
            return false;

        dst_len = len;
        return true;
    }

    // use pubkey(string) to encrypt
    static bool encode(std::string& dst,
                       const std::string& src, 
                       const std::string& pubkey_pem,
                       const padding padding = padding::pkcs1)
    {
        dst.resize(encode_len_reserve(reinterpret_cast<const unsigned char*>(pubkey_pem.c_str()), pubkey_pem.size()));
        std::size_t dst_len = dst.size();
        if (!encode(reinterpret_cast<unsigned char*>(const_cast<char*>(dst.data())),
                    dst_len, 
                    reinterpret_cast<const unsigned char*>(src.c_str()),
                    src.size(),
                    reinterpret_cast<const unsigned char*>(pubkey_pem.c_str()),
                    pubkey_pem.size(),
                    padding))
        {
            dst.clear();
            return false;
        }

        dst.resize(dst_len);
        return true;
    }

    static bool encode(std::ostream& out,
                       std::istream& in,
                       const unsigned char* pubkey_pem,
                       const std::size_t pubkey_pem_len,
                       const padding padding = padding::pkcs1)
    {
        if (!in || !out)
            return false;

        RSA* rsa = _load_public_key(pubkey_pem, pubkey_pem_len);
        if (!rsa)
            return false;

        int key_size = RSA_size(rsa); // [1024/8, 2048/8, 4096/8]
        if (pubkey_pem_len < static_cast<std::size_t>(key_size))
        {
            RSA_free(rsa);
            return false;
        }

        int max_chunk = key_size;
        switch (padding)
        {
        case padding::pkcs1:      { max_chunk = key_size - RSA_PKCS1_PADDING_SIZE; break; }
        case padding::pkcs1_oaep: { max_chunk = key_size - 2 * 20 - 2; break; } // SHA-1
        case padding::x931:       { max_chunk = key_size - RSA_PKCS1_PADDING_SIZE; break; }
        case padding::no_padding: { max_chunk = key_size; break; }
        default:                  { RSA_free(rsa); return false; }
        }

        unsigned char inbuf[RSA_MAX_KEY_LENGTH];
        unsigned char outbuf[RSA_MAX_KEY_LENGTH];
        std::streamsize read_len;
        int write_len = 0;

        while (in)
        {
            in.read(reinterpret_cast<char*>(inbuf), max_chunk);
            read_len = in.gcount();
            if (read_len < 1)
                break;

            write_len = RSA_public_encrypt(static_cast<int>(read_len), inbuf, outbuf, rsa, static_cast<int>(padding));
            if (write_len == -1)
            {
                RSA_free(rsa);
                return false;
            }
            out.write(reinterpret_cast<char*>(outbuf), write_len);
        }
        
        RSA_free(rsa);
        return true;
    }

    // use pubkey to encrypt file
    // NOTE: this function not recommand for large file
    static bool encode_file(const char* dst_file_path,
                            const char* src_file_path,
                            const unsigned char* pubkey_pem,
                            const std::size_t pubkey_pem_len,
                            const padding padding = padding::pkcs1)
    {
        std::ifstream in(reinterpret_cast<const char*>(src_file_path), std::ios::binary);
        std::ofstream out(reinterpret_cast<const char*>(dst_file_path), std::ios::binary);
        return encode(out, in, pubkey_pem, pubkey_pem_len, padding);
    }

    // use pubkey(string) to encrypt file
    // NOTE: this function not recommand for large file
    static bool encode_file(const std::string& dst_file_path,
                            const std::string& src_file_path,
                            const std::string& pubkey_pem,
                            const padding padding = padding::pkcs1)
    {
        return encode_file(dst_file_path.c_str(),
                           src_file_path.c_str(),
                           reinterpret_cast<const unsigned char*>(pubkey_pem.c_str()),
                           pubkey_pem.size(),
                           padding);
    }

    // use prikey to decrypt
    static bool decode(unsigned char* dst,
                       std::size_t& dst_len,
                       const unsigned char* src,
                       const std::size_t src_len,
                       const unsigned char* prikey_pem,
                       const std::size_t prikey_pem_len,
                       const padding padding = padding::pkcs1,
                       const unsigned char* password = nullptr,
                       const std::size_t password_len = 0)
    {
        RSA* rsa = _load_private_key(prikey_pem, prikey_pem_len, password, password_len);
        if (!rsa)
            return false;

        if (dst_len < static_cast<std::size_t>(RSA_size(rsa)))
        {
            RSA_free(rsa);
            return false;
        }

        int len = RSA_private_decrypt(src_len, src, dst, rsa, static_cast<int>(padding));
        RSA_free(rsa);
        if (len == -1)
            return false;

        dst_len = len;
        return true;
    }

    // use prikey to decrypt
    static bool decode(std::string& dst,
                       const std::string& src,
                       const std::string& prikey_pem,
                       const padding padding = padding::pkcs1,
                       const std::string& password = "")
    {
        dst.resize(decode_len_reserve(reinterpret_cast<const unsigned char*>(prikey_pem.c_str()), prikey_pem.size()));

        unsigned char* password_ptr = (password.empty()) ? nullptr : reinterpret_cast<unsigned char*>(const_cast<char*>(password.c_str()));
        std::size_t password_len = password.size();
        std::size_t dst_len = dst.size();
        if(!decode(reinterpret_cast<unsigned char*>(const_cast<char*>(dst.data())),
                   dst_len,
                   reinterpret_cast<const unsigned char*>(src.c_str()),
                   src.size(),
                   reinterpret_cast<const unsigned char*>(prikey_pem.c_str()),
                   prikey_pem.size(),
                   padding,
                   password_ptr,
                   password_len))
        {
            dst.clear();
            return false;
        }

        dst.resize(dst_len);
        return true;
    }

    // use prikey to decrypt stream
    static bool decode(std::ostream& out,
                       std::istream& in,
                       const unsigned char* prikey_pem,
                       const std::size_t prikey_pem_len,
                       const padding padding = padding::pkcs1,
                       const unsigned char* password = nullptr,
                       const std::size_t password_len = 0)
    {
        if (!in || !out)
            return false;

        RSA* rsa = _load_private_key(prikey_pem, prikey_pem_len, password, password_len);
        if (!rsa)
            return false;

        std::size_t key_size = RSA_size(rsa); // [1024/8, 2048/8, 4096/8]
        if (prikey_pem_len < key_size)
        {
            RSA_free(rsa);
            return false;
        }

        unsigned char inbuf[RSA_MAX_KEY_LENGTH];
        unsigned char outbuf[RSA_MAX_KEY_LENGTH];
        std::streamsize read_len = 0;
        while (in)
        {
            in.read(reinterpret_cast<char*>(inbuf), key_size);
            read_len = in.gcount();
            if (read_len < 1)
                break;

            int write_len = RSA_private_decrypt(static_cast<int>(read_len), inbuf, outbuf, rsa, static_cast<int>(padding));
            if (write_len == -1)
            {
                RSA_free(rsa);
                return false;
            }
            out.write(reinterpret_cast<char*>(outbuf), write_len);
        }

        RSA_free(rsa);
        return true;
    }

    // use prikey to decrypt file
    static bool decode_file(const char* dst_file_path,
                            const char* src_file_path, 
                            const unsigned char* prikey_pem,
                            const std::size_t prikey_pem_len,
                            const padding padding = padding::pkcs1,
                            const unsigned char* password = nullptr,
                            const std::size_t password_len = 0)
    {
        std::ifstream in(reinterpret_cast<const char*>(src_file_path), std::ios::binary);
        std::ofstream out(reinterpret_cast<const char*>(dst_file_path), std::ios::binary);
        return decode(out, in, prikey_pem, prikey_pem_len, padding, password, password_len);
    }

    // use prikey(std::string) to decrypt file
    static bool decode_file(const std::string& dst_file_path,
                            const std::string& src_file_path,
                            const std::string& prikey_pem,
                            const padding padding = padding::pkcs1,
                            const std::string& password = "")
    {
        unsigned char* password_ptr = (password.empty()) ? nullptr : reinterpret_cast<unsigned char*>(const_cast<char*>(password.c_str()));
        std::size_t password_len = password.size();
        return decode_file(dst_file_path.c_str(),
                           src_file_path.c_str(),
                           reinterpret_cast<const unsigned char*>(prikey_pem.c_str()),
                           prikey_pem.size(),
                           padding,
                           password_ptr,
                           password_len);
    }

    // use prikey to signature
    static bool signature(unsigned char* dst,
                          std::size_t& dst_len,
                          const unsigned char* src, 
                          const std::size_t src_len, 
                          const unsigned char* prikey_pem,
                          const std::size_t prikey_pem_len,
                          const padding padding = padding::pkcs1,
                          const unsigned char* password = nullptr,
                          const std::size_t password_len = 0)
    {
        RSA* rsa = _load_private_key(prikey_pem, prikey_pem_len, password, password_len);
        if (!rsa)
            return false;

        if (dst_len < static_cast<std::size_t>(RSA_size(rsa)))
            return false;

        int len = RSA_private_encrypt(src_len, src, dst, rsa, static_cast<int>(padding));
        RSA_free(rsa);
        if (len == -1) 
            return false;

        dst_len = len;
        return true;
    }

    // use pubkey to verify signature
    static bool verify_signature(unsigned char* dst,
                                 std::size_t& dst_len,
                                 const unsigned char* src,
                                 const std::size_t src_len,
                                 const unsigned char* pubkey_pem,
                                 const std::size_t pubkey_pem_len,
                                 const padding padding = padding::pkcs1)
    {
        RSA* rsa = _load_public_key(pubkey_pem, pubkey_pem_len);
        if (!rsa)
            return false;

        if (dst_len < static_cast<std::size_t>(RSA_size(rsa)))
            return false;

        int len = RSA_public_decrypt(src_len, src, dst, rsa, static_cast<int>(padding));
        RSA_free(rsa);
        if (len == -1) 
            return false;

        dst_len = len;
        return true;
    }

    // generate rsa key pair
    static bool make_key_pair(unsigned char* pubkey_pem, 
                              std::size_t& pubkey_pem_len,
                              unsigned char* prikey_pem,
                              std::size_t& prikey_pem_len,
                              const std::size_t bits = 2048,
                              const key_format format = key_format::x509,
                              const cipher cip = cipher::none,
                              const unsigned char* password = nullptr,
                              const std::size_t password_len = 0)
    {
        if (!pubkey_pem || pubkey_pem_len < 256)
            return false;
        if (!prikey_pem || prikey_pem_len < 512)
            return false;
        // bits: 512, 1024, 2048, 3072, 4096
        if (!is_key_pair_bits_valid(bits))
            return false;
        if (cip != cipher::none && (!password || !_validate_password_for_cipher(cip, password_len)))
            return false;

        RSA* rsa = nullptr;
        BIGNUM* bn = nullptr;
        BIO* pub_bio = nullptr;
        BIO* pri_bio = nullptr;
        EVP_PKEY* pkey = nullptr;
        bool ret = false;
        char* pub_buf = nullptr;
        char* pri_buf = nullptr;
        long pub_len = 0;
        long pri_len = 0;
        do {
            bn = BN_new();
            if (!bn || !BN_set_word(bn, RSA_F4))
                break;

            rsa = RSA_new();
            if (!rsa || !RSA_generate_key_ex(rsa, bits, bn, nullptr))
                break;

            if (RSA_check_key(rsa) != 1)
                break;

            pub_bio = BIO_new(BIO_s_mem());
            pri_bio = BIO_new(BIO_s_mem());
            if (!pub_bio || !pri_bio)
                break;

            // write pubkey
            bool pub_success = false;
            switch (format) {
            case key_format::x509:
                // -----BEGIN PUBLIC KEY-----
                pub_success = PEM_write_bio_RSA_PUBKEY(pub_bio, rsa);
                break;
            case key_format::pkcs1:
                // -----BEGIN RSA PUBLIC KEY-----
                pub_success = PEM_write_bio_RSAPublicKey(pub_bio, rsa);
                break;
            default:
                break;
            }
            
            if (!pub_success)
                break;

            // write prikey
            bool pri_success = false;
            switch (format) {
            case key_format::x509:
                // -----BEGIN PRIVATE KEY-----
                pkey = EVP_PKEY_new();
                if (!pkey || !EVP_PKEY_set1_RSA(pkey, rsa)) 
                {
                    pri_success = false;
                    break;
                }
                else 
                {
                    // use password to encrypt private key
                    if (cip != cipher::none) 
                        pri_success = PEM_write_bio_PrivateKey(pri_bio, pkey, _select_cipher(cip), 
                            const_cast<unsigned char*>(password), static_cast<int>(password_len), nullptr, nullptr);
                    else 
                        pri_success = PEM_write_bio_PrivateKey(pri_bio, pkey, nullptr, nullptr, 0, nullptr, nullptr);
                }
                break;
            case key_format::pkcs1:
                // -----BEGIN RSA PRIVATE KEY-----
                // use password to encrypt private key
                if (cip != cipher::none) 
                    pri_success = PEM_write_bio_RSAPrivateKey(pri_bio, rsa, _select_cipher(cip),
                        const_cast<unsigned char*>(password), static_cast<int>(password_len), nullptr, nullptr);
                else
                    pri_success = PEM_write_bio_RSAPrivateKey(pri_bio, rsa, nullptr, nullptr, 0, nullptr, nullptr);

                break;
            default:
                break;
            }

            if (!pri_success)
                break;
            
            // read pubkey_pem
            pub_buf = nullptr;
            pub_len = BIO_get_mem_data(pub_bio, &pub_buf);
            if (pub_len < 1 || static_cast<std::size_t>(pub_len) > pubkey_pem_len)
                break;

            // read prikey_pem
            pri_buf = nullptr;
            pri_len = BIO_get_mem_data(pri_bio, &pri_buf);
            if (pri_len < 1 || static_cast<std::size_t>(pri_len) > prikey_pem_len)
                break;

            memcpy(pubkey_pem, pub_buf, static_cast<std::size_t>(pub_len));
            pubkey_pem[pub_len] = '\0';
            pubkey_pem_len = static_cast<std::size_t>(pub_len);

            memcpy(prikey_pem, pri_buf, static_cast<std::size_t>(pri_len));
            prikey_pem[pri_len] = '\0';
            prikey_pem_len = static_cast<std::size_t>(pri_len);
            
            ret = true;
        } while(false);

        if (pkey) 
            EVP_PKEY_free(pkey);
        if(pub_bio)
            BIO_free(pub_bio);
        if(pri_bio) 
            BIO_free(pri_bio);
        if(rsa)
            RSA_free(rsa);
        if(bn) 
            BN_free(bn);

        return ret;
    }

    static bool make_key_pair(std::string& pubkey_pem, 
                              std::string& prikey_pem,
                              const std::size_t bits = 2048,
                              const key_format format = key_format::x509,
                              const cipher cip = cipher::none,
                              const std::string& password = "")
    {
        pubkey_pem.resize(RSA_MAX_KEY_LENGTH);
        std::size_t pubkey_pem_len = pubkey_pem.size();
        prikey_pem.resize(RSA_MAX_KEY_LENGTH);
        std::size_t prikey_pem_len = prikey_pem.size();
        if (!make_key_pair(reinterpret_cast<unsigned char*>(const_cast<char*>(pubkey_pem.data())),
                           pubkey_pem_len,
                           reinterpret_cast<unsigned char*>(const_cast<char*>(prikey_pem.data())),
                           prikey_pem_len,
                           bits,
                           format,
                           cip,
                           reinterpret_cast<const unsigned char*>(password.c_str()),
                           password.size()))
        {
            pubkey_pem.clear();
            prikey_pem.clear();
            return false;
        }

        pubkey_pem.resize(pubkey_pem_len);
        prikey_pem.resize(prikey_pem_len);
        return true;
    }

    // reserve encode dst buf size
	static std::size_t encode_len_reserve(const unsigned char* pubkey_pem, const std::size_t pubkey_pem_len)
	{
		RSA* rsa = _load_public_key(pubkey_pem, pubkey_pem_len);
        if (!rsa)
            return 0;

        return RSA_size(rsa);
	}

    // reserve decode dst buf size
	static std::size_t decode_len_reserve(const unsigned char* prikey_pem, 
                                          const std::size_t prikey_pem_len,
                                          const unsigned char* password = nullptr,
                                          const std::size_t password_len = 0)
	{
		RSA* rsa = _load_private_key(prikey_pem, prikey_pem_len, password, password_len);
        if (!rsa)
            return 0;

        return RSA_size(rsa);
	}

    // check public key style
    static bool is_pubkey_valid(const unsigned char* pubkey_pem, 
                                const std::size_t pubkey_pem_len,
                                const padding padding = padding::pkcs1)
    {
        RSA* rsa = _load_public_key(pubkey_pem, pubkey_pem_len);
        if (!rsa)
            return false;

        bool valid = false;
        switch (static_cast<int>(padding)) {
        case RSA_PKCS1_PADDING:
        case RSA_PKCS1_OAEP_PADDING:
        case RSA_NO_PADDING:
#ifdef RSA_PKCS1_PSS_PADDING
        case RSA_PKCS1_PSS_PADDING:
#endif
            valid = true;
            break;
        default:
            valid = false;
            break;
        }

        RSA_free(rsa);
        return valid;
    }

    // check private key style
    static bool is_prikey_valid(const unsigned char* prikey_pem, 
                                const std::size_t prikey_pem_len,
                                const padding padding = padding::pkcs1,
                                const unsigned char* password = nullptr,
                                const std::size_t password_len = 0)
    {
        RSA* rsa = _load_private_key(prikey_pem, prikey_pem_len, password, password_len);
        if (!rsa)
            return false;

        bool valid = false;
        switch (static_cast<int>(padding)) {
        case RSA_PKCS1_PADDING:
        case RSA_PKCS1_OAEP_PADDING:
        case RSA_NO_PADDING:
#ifdef RSA_PKCS1_PSS_PADDING
        case RSA_PKCS1_PSS_PADDING:
#endif
            valid = true;
            break;
    default:
            valid = false;
            break;
        }

        RSA_free(rsa);
        return valid;
    }

    static bool is_key_pair_bits_valid(const std::size_t bits)
    {
        if (bits != 512 && bits != 1024 && bits != 2048 && bits != 3072 && bits != 4096) 
            return false;

        return true;
    }

private:
    static RSA* _load_public_key(const unsigned char* pubkey_pem, 
                                   const std::size_t pubkey_pem_len)
    {
        if (pubkey_pem == nullptr || pubkey_pem_len <= 0)
            return nullptr;
            
        BIO* bio = BIO_new_mem_buf(pubkey_pem, static_cast<int>(pubkey_pem_len));
        if (!bio) 
            return nullptr;

        // for pubkey style: 
        // -----BEGIN PUBLIC KEY-----
        // ...base64...
        // -----END PUBLIC KEY-----
        RSA* rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
        if (!rsa) 
        {
            // read fail, try another pubkey style: 
            // -----BEGIN RSA PUBLIC KEY-----
            // ...base64...
            // -----END RSA PUBLIC KEY-----
            BIO_reset(bio);
            rsa = PEM_read_bio_RSAPublicKey(bio, nullptr, nullptr, nullptr);
        }

        BIO_free(bio);
        return rsa;
    }

    static RSA* _load_public_key(const std::string& pubkey_pem) 
    {
        return _load_public_key(reinterpret_cast<const unsigned char*>(pubkey_pem.c_str()), pubkey_pem.size());    
    }

    static RSA* _load_private_key(const unsigned char* prikey_pem, 
                                  const int prikey_pem_len, 
                                  const unsigned char* password = nullptr, 
                                  const std::size_t password_len = 0)
    {
        if (prikey_pem == nullptr || prikey_pem_len <= 0)
            return nullptr;

        // for prikey style: 
        // -----BEGIN PRIVATE KEY-----
        // ...base64...
        // -----END PRIVATE KEY-----
        // or
        // -----BEGIN RSA PRIVATE KEY-----
        // ...base64...
        // -----END RSA PRIVATE KEY-----
        BIO* bio = BIO_new_mem_buf(prikey_pem, prikey_pem_len);
        if (!bio) 
            return nullptr;

        // try to read by PKCS#1 style first
        // if failed, try to read by X.509 style
        RSA* rsa = nullptr;
        if (password && password_len > 0)
            rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, 
                reinterpret_cast<char*>(const_cast<unsigned char*>(password)));
        else
            rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);
        
        if (!rsa) 
        {
            BIO_reset(bio);
            EVP_PKEY* pkey = nullptr;
            if (password && password_len > 0)
                pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, 
                    reinterpret_cast<char*>(const_cast<unsigned char*>(password)));
            else 
                pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
            
            if (pkey) 
            {
                rsa = EVP_PKEY_get1_RSA(pkey);
                EVP_PKEY_free(pkey);
            }
        }

        BIO_free(bio);
        return rsa;
    }

    static RSA* _load_private_key(const std::string& prikey_pem, const std::string& password = std::string())
    {
        if (prikey_pem.empty())
            return nullptr;

        const unsigned char* password_ptr = nullptr;
        std::size_t password_len = 0;
        if (!password.empty()) 
        {
            password_ptr = reinterpret_cast<const unsigned char*>(password.c_str());
            password_len = password.size();
        }

        return _load_private_key(reinterpret_cast<const unsigned char*>(prikey_pem.c_str()), prikey_pem.size(), 
                    password_ptr, password_len);
    }

    static const EVP_CIPHER* _select_cipher(const cipher cip)
    {
        switch (cip)
        {
        case cipher::none: { return nullptr; }

        // AES
        case cipher::aes_128_ecb: { return EVP_aes_128_cbc(); } // AES-128-CBC is the default for AES ECB
        case cipher::aes_192_ecb: { return EVP_aes_192_cbc(); } // AES-192-CBC is the default for AES ECB
        case cipher::aes_256_ecb: { return EVP_aes_256_cbc(); } // AES-256-CBC is the default for AES ECB

        case cipher::aes_128_cbc: { return EVP_aes_128_cbc(); }
        case cipher::aes_192_cbc: { return EVP_aes_192_cbc(); }
        case cipher::aes_256_cbc: { return EVP_aes_256_cbc(); }

        case cipher::aes_128_cfb: { return EVP_aes_128_cfb(); }
        case cipher::aes_192_cfb: { return EVP_aes_192_cfb(); }
        case cipher::aes_256_cfb: { return EVP_aes_256_cfb(); }

        case cipher::aes_128_ofb: { return EVP_aes_128_ofb(); }
        case cipher::aes_192_ofb: { return EVP_aes_192_ofb(); }
        case cipher::aes_256_ofb: { return EVP_aes_256_ofb(); }

        // 3DES
        case cipher::des_ede3_ecb: { return EVP_des_ede3_cbc(); } // 3DES-CBC is the default for 3DES ECB
        case cipher::des_ede3_cbc: { return EVP_des_ede3_cbc(); }
        case cipher::des_ede3_cfb: { return EVP_des_ede3_cfb(); }
        case cipher::des_ede3_ofb: { return EVP_des_ede3_ofb(); }
        case cipher::des_ede_cbc: { return EVP_des_ede_cbc(); }

        // DES (not recommended)
        case cipher::des_ecb: { return EVP_des_cbc(); } // DES-CBC is the default for DES ECB
        case cipher::des_cbc: { return EVP_des_cbc(); }
        case cipher::des_cfb: { return EVP_des_cfb(); }
        case cipher::des_ofb: { return EVP_des_ofb(); }

        // Blowfish
        case cipher::bf_ecb: { return EVP_bf_cbc(); } // Blowfish-CBC is the default for Blowfish ECB
        case cipher::bf_cbc: { return EVP_bf_cbc(); }
        case cipher::bf_cfb: { return EVP_bf_cfb(); }
        case cipher::bf_ofb: { return EVP_bf_ofb(); }

        // CAST5
        case cipher::cast5_ecb: { return EVP_cast5_cbc(); } // CAST5-CBC is the default for CAST5 ECB
        case cipher::cast5_cbc: { return EVP_cast5_cbc(); }
        case cipher::cast5_cfb: { return EVP_cast5_cfb(); }
        case cipher::cast5_ofb: { return EVP_cast5_ofb(); }

        // RC2
        case cipher::rc2_ecb: { return EVP_rc2_cbc(); } // RC2-CBC is the default for RC2 ECB
        case cipher::rc2_cbc: { return EVP_rc2_cbc(); }
        case cipher::rc2_cfb: { return EVP_rc2_cfb(); }
        case cipher::rc2_ofb: { return EVP_rc2_ofb(); }

        default: { return EVP_aes_256_cbc(); }
        }
    }

    static bool _validate_password_for_cipher(const cipher cip, std::size_t password_len)
    {
        switch (cip)
        {
        case cipher::none:
            return true; // no password required

        case cipher::des_ecb:
        case cipher::des_cbc:
        case cipher::des_cfb:
        case cipher::des_ofb:
        case cipher::des_ede_cbc:
        case cipher::des_ede3_ecb:
        case cipher::des_ede3_cbc:
        case cipher::des_ede3_cfb:
        case cipher::des_ede3_ofb:
            return password_len >= 8;
            
        case cipher::aes_128_ecb:
        case cipher::aes_128_cbc:
        case cipher::aes_128_cfb:
        case cipher::aes_128_ofb:
        case cipher::aes_192_ecb:
        case cipher::aes_192_cbc:
        case cipher::aes_192_cfb:
        case cipher::aes_192_ofb:
        case cipher::aes_256_ecb:
        case cipher::aes_256_cbc:
        case cipher::aes_256_cfb:
        case cipher::aes_256_ofb:
            return password_len >= 8;
            
        default:
            return password_len >= 4;
        }
    }
};

}

#endif