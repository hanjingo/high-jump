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
    // string -> ecb string
    static bool encode(const std::string& src, 
                       const std::string& key, 
                       std::string& dst,
                       std::size_t idx = 0)
    {
        std::size_t len = 0;
        dst.resize((src.size() / 8) * 8 + 8);
        if (!encode(src.c_str(), src.size(), key.c_str(), key.size(), const_cast<char*>(dst.c_str()), len, idx))
            return false;

        dst.resize(len);
        return true;
    }

    // string -> ecb bytes
    static bool encode(const std::string& src, 
                       const std::string& key, 
                       char* dst, 
                       std::size_t& dst_len,
                       std::size_t idx = 0)
    {
        return encode(src.c_str(), src.size(), key.c_str(), key.size(), dst, dst_len, idx);
    }

    // bytes -> ecb bytes (padding zero)
    static bool encode(const char* src, 
                       const std::size_t src_len, 
                       const char* key, 
                       const std::size_t key_len, 
                       char* dst,
                       std::size_t& dst_len,
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

    // file -> ecb file
    static bool encode_file(const std::string& src_file_path, 
                            const std::string& key, 
                            const std::string& dst_file_path)
    {
        std::ifstream src_file(src_file_path, std::ios::binary);
        if (!src_file.is_open())
            return false;

        std::ofstream dst_file(dst_file_path, std::ios::binary);
        if (!dst_file.is_open())
            return false;

        constexpr std::size_t block_size = 8;
        char inbuf[block_size] = {0};
        char outbuf[block_size] = {0};

        DES_cblock key_encrypt;
        memset(key_encrypt, 0, 8);
        memcpy(key_encrypt, key.data(), std::min<size_t>(key.size(), 8));
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

    // ecb string -> string
    static bool decode(const std::string& cipher, 
                       const std::string& key, 
                       std::string& dst, 
                       std::size_t idx = 0)
    {
        std::size_t len = 0;
        dst.resize((cipher.size() / 8) * 8 + 8);
        if (!decode(cipher.c_str(), cipher.size(), key.c_str(), key.size(), const_cast<char*>(dst.c_str()), len, idx))
            return false;

        dst.resize(len);
        return !dst.empty();
    }

    // ecb bytes -> string
    static bool decode(const char* cipher, 
                       const std::size_t cipher_len, 
                       const char* key, 
                       const std::size_t key_len, 
                       std::string& dst,
                       std::size_t idx = 0)
    {
        std::size_t len = 0;
        dst.resize((cipher_len / 8) * 8 + 8);
        if (!decode(cipher, cipher_len, key, key_len, const_cast<char*>(dst.c_str()), len, idx))
            return false;

        dst.resize(len);
        return !dst.empty();
    }

    // ecb bytes -> bytes
    static bool decode(const char* cipher, 
                       const std::size_t cipher_len, 
                       const char* key, 
                       const std::size_t key_len, 
                       char* dst,
                       std::size_t& dst_len,
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
            memcpy(dst + idx, output, 8);
        }

        if (cipher_len % 8 != 0)
        {
            idx -= 8;
            memset(input, 0, 8);
            memcpy(input, cipher + idx, cipher_len - idx);

            DES_ecb_encrypt(&input, &output, &key_schedule, DES_DECRYPT);
            memcpy(dst + idx, output, 8);
        }

        dst_len = idx;
        return dst_len > 0;
    }

    // ecb file -> file
    static bool decode_file(const std::string& src_file_path, 
                            const std::string& key, 
                            const std::string& dst_file_path)
    {
        std::ifstream src_file(src_file_path, std::ios::binary);
        if (!src_file.is_open())
            return false;

        std::ofstream dst_file(dst_file_path, std::ios::binary);
        if (!dst_file.is_open())
            return false;

        constexpr std::size_t block_size = 8;
        char inbuf[block_size] = {0};
        char outbuf[block_size] = {0};

        DES_cblock key_encrypt;
        memset(key_encrypt, 0, 8);
        memcpy(key_encrypt, key.data(), std::min<size_t>(key.size(), 8));
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
};

}

}

#endif