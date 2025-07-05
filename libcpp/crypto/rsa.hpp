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
    // use pubkey to encrypt
    static bool encode(unsigned char* dst,
                       std::size_t& dst_len,
                       const unsigned char* src, 
                       const std::size_t src_len, 
                       const unsigned char* pubkey_pem,
                       const std::size_t pubkey_pem_len,
                       const int padding = RSA_PKCS1_PADDING)
    {
        RSA* rsa = _load_public_key(pubkey_pem, pubkey_pem_len);
        if (!rsa)
            return false;

        if (dst_len < static_cast<std::size_t>(RSA_size(rsa)))
        {
            RSA_free(rsa);
            return false;
        }

        int len = RSA_public_encrypt((int)src_len, src, dst, rsa, padding);
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
                       const int padding = RSA_PKCS1_PADDING)
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
                       const int padding = RSA_PKCS1_PADDING)
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
        case RSA_PKCS1_PADDING:      { max_chunk = key_size - RSA_PKCS1_PADDING_SIZE; break; }
        case RSA_PKCS1_OAEP_PADDING: { max_chunk = key_size - 2 * 20 - 2; break; } // SHA-1
        case RSA_X931_PADDING:       { max_chunk = key_size - RSA_PKCS1_PADDING_SIZE; break; }
        case RSA_NO_PADDING:         { max_chunk = key_size; break; }
        default:                     { RSA_free(rsa); return false; }
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

            write_len = RSA_public_encrypt(static_cast<int>(read_len), inbuf, outbuf, rsa, padding);
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
                            int padding = RSA_PKCS1_PADDING)
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
                            int padding = RSA_PKCS1_PADDING)
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
                       const int padding = RSA_PKCS1_PADDING)
    {
        RSA* rsa = _load_private_key(prikey_pem, prikey_pem_len);
        if (!rsa)
            return false;

        if (dst_len < static_cast<std::size_t>(RSA_size(rsa)))
        {
            RSA_free(rsa);
            return false;
        }

        int len = RSA_private_decrypt(src_len, src, dst, rsa, padding);
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
                       const int padding = RSA_PKCS1_PADDING)
    {
        dst.resize(decode_len_reserve(reinterpret_cast<const unsigned char*>(prikey_pem.c_str()), prikey_pem.size()));
        std::size_t dst_len = dst.size();
        if(!decode(reinterpret_cast<unsigned char*>(const_cast<char*>(dst.data())),
                   dst_len,
                   reinterpret_cast<const unsigned char*>(src.c_str()),
                   src.size(),
                   reinterpret_cast<const unsigned char*>(prikey_pem.c_str()),
                   prikey_pem.size(),
                   padding))
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
                       const int padding = RSA_PKCS1_PADDING)
    {
        if (!in || !out)
            return false;

        BIO* bio = BIO_new_mem_buf(prikey_pem, prikey_pem_len);
        if (!bio)
            return false;

        RSA* rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        if (!rsa)
            return false;

        int key_size = RSA_size(rsa); // [1024/8, 2048/8, 4096/8]
        if (prikey_pem_len < static_cast<std::size_t>(key_size))
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

            int write_len = RSA_private_decrypt(static_cast<int>(read_len), inbuf, outbuf, rsa, padding);
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
                            int padding = RSA_PKCS1_PADDING)
    {
        std::ifstream in(reinterpret_cast<const char*>(src_file_path), std::ios::binary);
        std::ofstream out(reinterpret_cast<const char*>(dst_file_path), std::ios::binary);
        return decode(out, in, prikey_pem, prikey_pem_len, padding);
    }

    // use prikey(std::string) to decrypt file
    static bool decode_file(const std::string& dst_file_path,
                            const std::string& src_file_path,
                            const std::string& prikey_pem,
                            int padding = RSA_PKCS1_PADDING)
    {
        return decode_file(dst_file_path.c_str(),
                           src_file_path.c_str(),
                           reinterpret_cast<const unsigned char*>(prikey_pem.c_str()),
                           prikey_pem.size(),
                           padding);
    }

    // use prikey to signature
    static bool signature(unsigned char* dst,
                          std::size_t& dst_len,
                          const unsigned char* src, 
                          const std::size_t src_len, 
                          const unsigned char* prikey_pem,
                          const std::size_t prikey_pem_len,
                          const int padding = RSA_PKCS1_PADDING)
    {
        RSA* rsa = _load_private_key(prikey_pem, prikey_pem_len);
        if (!rsa)
            return false;

        if (dst_len < static_cast<std::size_t>(RSA_size(rsa)))
            return false;

        int len = RSA_private_encrypt(src_len, src, dst, rsa, padding);
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
                                 const int padding = RSA_PKCS1_PADDING)
    {
        RSA* rsa = _load_public_key(pubkey_pem, pubkey_pem_len);
        if (!rsa)
            return false;

        if (dst_len < static_cast<std::size_t>(RSA_size(rsa)))
            return false;

        int len = RSA_public_decrypt(src_len, src, dst, rsa, padding);
        RSA_free(rsa);
        if (len == -1) 
            return false;

        dst_len = len;
        return true;
    }

    // generate rsa key pair
    static bool make_key_pair_x509(unsigned char* pubkey_pem, 
                                   std::size_t& pubkey_pem_len,
                                   unsigned char* prikey_pem,
                                   std::size_t& prikey_pem_len,
                                   const std::size_t bits = 2048)
    {
        // bits: 515, 1024, 2048, 3072, 4096
        RSA* rsa = nullptr;
        BIGNUM* bn = nullptr;
        BIO* pub_bio = nullptr;
        BIO* pri_bio = nullptr;
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

            pub_bio = BIO_new(BIO_s_mem());
            pri_bio = BIO_new(BIO_s_mem());
            if (!pub_bio || !pri_bio)
                break;

            if (!PEM_write_bio_RSA_PUBKEY(pub_bio, rsa))
                break;

            if (!PEM_write_bio_RSAPrivateKey(pri_bio, rsa, nullptr, nullptr, 0, nullptr, nullptr))
                break;
            
            // read pubkey_pem
            pub_buf = nullptr;
            pub_len = BIO_get_mem_data(pub_bio, &pub_buf);
            if (pub_len < 1 || pubkey_pem_len < (std::size_t)pub_len)
                break;

            memcpy(pubkey_pem, pub_buf, pub_len);
            pubkey_pem_len = (std::size_t)pub_len;

            // read prikey_pem
            pri_buf = nullptr;
            pri_len = BIO_get_mem_data(pri_bio, &pri_buf);
            if (pri_len < 1 || prikey_pem_len < (std::size_t)pri_len)
                break;

            memcpy(prikey_pem, pri_buf, pri_len);
            prikey_pem_len = (std::size_t)pri_len;
            ret = true;
        } while(0);

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

    static bool make_key_pair_x509(std::string& pubkey_pem, 
                                   std::string& prikey_pem,
                                   const std::size_t bits = 2048)
    {
        pubkey_pem.resize(RSA_MAX_KEY_LENGTH);
        std::size_t pubkey_pem_len = pubkey_pem.size();
        prikey_pem.resize(RSA_MAX_KEY_LENGTH);
        std::size_t prikey_pem_len = prikey_pem.size();
        if (!make_key_pair_x509(reinterpret_cast<unsigned char*>(const_cast<char*>(pubkey_pem.data())),
                                pubkey_pem_len,
                                reinterpret_cast<unsigned char*>(const_cast<char*>(prikey_pem.data())),
                                prikey_pem_len,
                                bits))
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
	static std::size_t decode_len_reserve(const unsigned char* prikey_pem, const std::size_t prikey_pem_len)
	{
		RSA* rsa = _load_private_key(prikey_pem, prikey_pem_len);
        if (!rsa)
            return 0;

        return RSA_size(rsa);
	}

    // check public key style
    static bool is_pubkey_valid(const unsigned char* pubkey_pem, 
                                const std::size_t pubkey_pem_len,
                                const int padding)
    {
        if (pubkey_pem == nullptr || pubkey_pem_len == 0)
            return false;

        BIO* bio = BIO_new_mem_buf(pubkey_pem, static_cast<int>(pubkey_pem_len));
        if (!bio)
            return false;

        // standarm padding style
        RSA* rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
        if (!rsa) 
        {
            // try PKCS#1 padding style
            BIO_reset(bio);
            rsa = PEM_read_bio_RSAPublicKey(bio, nullptr, nullptr, nullptr);
        }

        BIO_free(bio);
        if (!rsa)
            return false;

        bool valid = false;
        switch (padding) {
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
                                const int padding)
    {
        if (prikey_pem == nullptr || prikey_pem_len == 0)
            return false;

        BIO* bio = BIO_new_mem_buf(prikey_pem, static_cast<int>(prikey_pem_len));
        if (!bio)
            return false;

        RSA* rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);
        if (!rsa) 
        {
            BIO_reset(bio);
            EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
            if (pkey) 
            {
                rsa = EVP_PKEY_get1_RSA(pkey);
                EVP_PKEY_free(pkey);
            }
        }

        BIO_free(bio);
        if (!rsa)
            return false;

        bool valid = false;
        switch (padding) {
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

private:
    static RSA* _load_public_key(const unsigned char* pubkey_pem, 
                                   const std::size_t pubkey_pem_len)
    {
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

    static RSA* _load_private_key(const unsigned char* prikey_pem, const int prikey_pem_len)
    {
        BIO* bio = BIO_new_mem_buf(prikey_pem, prikey_pem_len);
        if (!bio) 
            return nullptr;

        RSA* rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        return rsa;
    }

    static RSA* _load_private_key(const std::string& prikey_pem)
    {
        return _load_private_key(reinterpret_cast<const unsigned char*>(prikey_pem.c_str()), prikey_pem.size());
    }
};

}

#endif