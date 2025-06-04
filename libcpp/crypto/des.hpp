#ifndef DES_HPP
#define DES_HPP

#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <openssl/des.h>

#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")

namespace libcpp
{

namespace des
{
using cblock_t = DES_cblock;
using const_cblock_t = const_DES_cblock;
using key_schedule_t = DES_key_schedule;

class ecb
{
public:
    ecb() {}
    ~ecb() {}

     template<typename T>
     inline ecb& operator<<(const T& src)
     {
         unsigned char buf[(sizeof(src) / 8) * 8 + 8];
         encrypt((unsigned char*)src, (unsigned char*)key_.data(), buf);
         return *this;
     }
    
     template<typename T>
     inline ecb& operator>>(T& dst)
     {
         decrypt()
         return *this;
     }

    static std::string encrypt(const std::string& src, const std::string& key)
    {
        std::vector<unsigned char> buf((src.size() / 8) * 8 + 8);
        std::size_t len = encrypt((unsigned char*)src.c_str(), src.size(), (unsigned char*)key.c_str(), key.size(), buf.data());
        return std::string(buf.begin(), buf.begin() + len);
    }

    static std::size_t encrypt(const unsigned char* src, 
                               const std::size_t src_len, 
                               const unsigned char* key, 
                               const std::size_t key_len, 
                               unsigned char* out,
                               std::size_t idx = 0)
    {
        cblock_t key_encrypt;
        memset(key_encrypt, 0, 8);

        if (key_len <= 8)
            memcpy(key_encrypt, key, key_len);
        else
            memcpy(key_encrypt, key, 8); 

        key_schedule_t key_schedule;
        DES_set_key_unchecked(&key_encrypt, &key_schedule);

        const_cblock_t input;
        cblock_t output;
        for (; idx < src_len; idx += 8)
        {
            memcpy(input, src + idx, 8);
            DES_ecb_encrypt(&input, &output, &key_schedule, DES_ENCRYPT);
            memcpy(out + idx, output, 8);
        }

        if (src_len % 8 != 0)
        {
            idx -= 8;
            memset(input, 0, 8);
            memcpy(input, src + idx, src_len - idx);

            DES_ecb_encrypt(&input, &output, &key_schedule, DES_ENCRYPT);
            memcpy(out + idx, output, 8);
        }

        return idx + 8;
    }

    static std::string decrypt(const std::string& cipher, const std::string& key)
    {
        std::vector<unsigned char> buf((cipher.size() / 8) * 8 + 8);
        std::size_t len = decrypt((unsigned char*)cipher.c_str(), cipher.size(), (unsigned char*)key.c_str(), key.size(), buf.data());
        return std::string(buf.begin(), buf.begin() + len);
    }

    static std::size_t decrypt(const unsigned char* cipher, 
                               const std::size_t cipher_len, 
                               const unsigned char* key, 
                               const std::size_t key_len, 
                               unsigned char* out,
                               std::size_t idx = 0)
    {
        cblock_t key_encrypt;
        memset(key_encrypt, 0, 8);

        if (key_len <= 8)
            memcpy(key_encrypt, key, key_len);
        else
            memcpy(key_encrypt, key, 8);

        key_schedule_t key_schedule;
        DES_set_key_unchecked(&key_encrypt, &key_schedule);

        const_cblock_t input;
        cblock_t output;

        for (; idx < cipher_len; idx += 8)
        {
            memcpy(input, cipher + idx, 8);
            DES_ecb_encrypt(&input, &output, &key_schedule, DES_DECRYPT);
            memcpy(out + idx, output, 8);
        }

        if (cipher_len % 8 != 0)
        {
            idx -= 8;
            memset(input, 0, 8);
            memcpy(input, cipher + idx, cipher_len - idx);

            DES_ecb_encrypt(&input, &output, &key_schedule, DES_DECRYPT);
            memcpy(out + idx, output, 8);
        }

        return idx;
    }
};

}

}

#endif