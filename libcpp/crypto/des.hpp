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

#ifndef DES_HPP
#define DES_HPP

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <openssl/des.h>

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
    // bytes -> ecb bytes (padding zero)
    static bool encode(const unsigned char* src, 
                       const unsigned long src_len, 
                       unsigned char* dst,
                       unsigned long& dst_len,
                       unsigned char* key, 
                       unsigned long key_len, 
                       unsigned long idx = 0)
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
            memcpy(dst + idx, output, 8);
        }

        if (src_len % 8 != 0)
        {
            idx -= 8;
            memset(input, 0, 8);
            memcpy(input, src + idx, src_len - idx);

            DES_ecb_encrypt(&input, &output, &key_schedule, DES_ENCRYPT);
            memcpy(dst + idx, output, 8);
        }

        dst_len = idx + 8;
        return dst_len > 0;
    }

    // bytes -> ecb bytes
    static bool encode(const char* src, 
                       const unsigned long src_len, 
                       char* dst,
                       unsigned long& dst_len,
                       char* key, 
                       unsigned long key_len, 
                       unsigned long idx = 0)
    {
        return encode(reinterpret_cast<const unsigned char*>(src),
                      src_len,
                      reinterpret_cast<unsigned char*>(dst),
                      dst_len,
                      reinterpret_cast<unsigned char*>(key),
                      key_len,
                      idx);
    }

    // string -> ecb string
    static bool encode(const std::string& src, 
                       const std::string& key, 
                       std::string& dst,
                       unsigned long idx = 0)
    {
        unsigned long len = 0;
        dst.resize((src.size() / 8) * 8 + 8);
        if (!encode(src.c_str(), src.size(), dst.data(), len, const_cast<char*>(key.c_str()), key.size(), idx))
        {
            dst.clear();
            return false;
        }

        dst.resize(len);
        return true;
    }

    // file -> ecb file
    static bool encode_file(const unsigned char* src_file_path,  
                            const unsigned char* dst_file_path,
                            const unsigned char* key, 
                            const unsigned long key_len)
    {
        std::ifstream src_file(reinterpret_cast<const char*>(src_file_path), std::ios::binary);
        if (!src_file.is_open())
            return false;

        std::ofstream dst_file(reinterpret_cast<const char*>(dst_file_path), std::ios::binary);
        if (!dst_file.is_open())
            return false;

        constexpr unsigned long block_size = 8;
        char inbuf[block_size] = {0};
        char outbuf[block_size] = {0};

        DES_cblock key_encrypt;
        memset(key_encrypt, 0, 8);
        memcpy(key_encrypt, key, std::min<size_t>(key_len, 8));
        DES_key_schedule key_schedule;
        DES_set_key_unchecked(&key_encrypt, &key_schedule);

        while (true) 
        {
            src_file.read(inbuf, block_size);
            std::streamsize read_len = src_file.gcount();
            
            if (read_len == 0)
                break;

            // padding 0
            if (read_len < (std::streamsize)block_size)
                memset(inbuf + read_len, 0, block_size - read_len);

            DES_ecb_encrypt(
                reinterpret_cast<const_DES_cblock*>(inbuf),
                reinterpret_cast<DES_cblock*>(outbuf),
                &key_schedule,
                DES_ENCRYPT);

            dst_file.write(outbuf, block_size);
            if (read_len < (std::streamsize)block_size)
                break;
        }
        return true;
    }

    // file -> ecb file
    static bool encode_file(const std::string& src_file_path,
                            const std::string& dst_file_path,
                            const std::string& key)
    {
        return encode_file(reinterpret_cast<const unsigned char*>(src_file_path.c_str()), 
                            reinterpret_cast<const unsigned char*>(dst_file_path.c_str()), 
                            reinterpret_cast<const unsigned char*>(key.c_str()), 
                            key.size());
    }

    // ecb bytes -> bytes
    static bool decode(const unsigned char* src, 
                       const unsigned long src_len, 
                       unsigned char* dst,
                       unsigned long& dst_len,
                       unsigned char* key, 
                       unsigned long key_len, 
                       unsigned long idx = 0)
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
            DES_ecb_encrypt(&input, &output, &key_schedule, DES_DECRYPT);
            memcpy(dst + idx, output, 8);
        }

        if (src_len % 8 != 0)
        {
            idx -= 8;
            memset(input, 0, 8);
            memcpy(input, src + idx, src_len - idx);

            DES_ecb_encrypt(&input, &output, &key_schedule, DES_DECRYPT);
            memcpy(dst + idx, output, 8);
        }

        dst_len = idx;
        return dst_len > 0;
    }

    // ecb bytes -> bytes
    static bool decode(const char* src, 
                       const unsigned long src_len, 
                       char* dst,
                       unsigned long& dst_len,
                       char* key, 
                       unsigned long key_len, 
                       unsigned long idx = 0)
    {
        return decode(reinterpret_cast<const unsigned char*>(src),
                        src_len,
                        reinterpret_cast<unsigned char*>(dst),
                        dst_len,
                        reinterpret_cast<unsigned char*>(key),
                        key_len,
                        idx);
    }

    // ecb string -> string
    static bool decode(const std::string& src, 
                       std::string& dst, 
                       std::string& key, 
                       unsigned long idx = 0)
    {
        unsigned long len = 0;
        dst.resize((src.size() / 8) * 8 + 8);
        if (!decode(src.c_str(), src.size(), dst.data(), len, const_cast<char*>(key.c_str()), key.size(),  idx))
        {
            dst.clear();
            return false;
        }

        dst.resize(len);
        return !dst.empty();
    }

    
    // ecb file -> file
    static bool decode_file(const unsigned char* src_file_path, 
                            const unsigned char* dst_file_path,
                            const unsigned char* key, 
                            const unsigned long key_len)
    {
        std::ifstream src_file(reinterpret_cast<const char*>(src_file_path), std::ios::binary);
        if (!src_file.is_open())
            return false;

        std::ofstream dst_file(reinterpret_cast<const char*>(dst_file_path), std::ios::binary);
        if (!dst_file.is_open())
            return false;

        constexpr unsigned long block_size = 8;
        char inbuf[block_size] = {0};
        char outbuf[block_size] = {0};

        DES_cblock key_encrypt;
        memset(key_encrypt, 0, 8);
        memcpy(key_encrypt, key, std::min<size_t>(key_len, 8));
        DES_key_schedule key_schedule;
        DES_set_key_unchecked(&key_encrypt, &key_schedule);
        while (true) 
        {
            src_file.read(inbuf, block_size);
            std::streamsize read_len = src_file.gcount();

            if (read_len == 0)
                break;

            DES_ecb_encrypt(
                reinterpret_cast<const_DES_cblock*>(inbuf),
                reinterpret_cast<DES_cblock*>(outbuf),
                &key_schedule,
                DES_DECRYPT);

            dst_file.write(outbuf, read_len);
            if (read_len < (std::streamsize)block_size)
                break;
        }

        return true;
    }

    // ecb file -> file
    static bool decode_file(const char* src_file_path, 
                            const char* dst_file_path,
                            const char* key, 
                            const unsigned long key_len)
    {
        return decode_file(reinterpret_cast<const unsigned char*>(src_file_path),
                            reinterpret_cast<const unsigned char*>(dst_file_path),
                            reinterpret_cast<const unsigned char*>(key),
                            key_len);
    }

    static bool decode_file(const std::string& src_file_path,
                            const std::string& dst_file_path,
                            const std::string& key)
    {
        return decode_file(src_file_path.c_str(), dst_file_path.c_str(), key.c_str(), key.size());
    }

private:
    ecb() = default;
    ~ecb() = default;
    ecb(const ecb&) = delete;
    ecb& operator=(const ecb&) = delete;
    ecb(ecb&&) = delete;
    ecb& operator=(ecb&&) = delete;
};

}

}

#endif