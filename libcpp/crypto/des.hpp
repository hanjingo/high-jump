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

// disable msvc safe check warning
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// support deprecated api for low version openssl
#ifndef OPENSSL_SUPPRESS_DEPRECATED
#define OPENSSL_SUPPRESS_DEPRECATED
#endif

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <openssl/des.h>

namespace libcpp
{

class des
{
public:
    enum class cipher
    {
        des_ecb,
        des_cbc,
        des_cfb,
        des_ofb,
        des_ctr
    };

    enum class padding
    {
        des_pkcs5_padding,
        des_pkcs7_padding,
        des_zero_padding,
        des_iso10126_padding,
        des_ansix923_padding,
        des_iso_iec_7816_4_padding,
        des_no_padding
    };

public:
    // bytes -> encode bytes
    static bool encode(unsigned char* dst,
                       std::size_t& dst_len,
                       const unsigned char* src, 
                       const std::size_t src_len, 
                       const unsigned char* key, 
                       const std::size_t key_len, 
                       const cipher cip = cipher::des_ecb,
                       const padding pad_style = padding::des_zero_padding,
                       const unsigned char* iv = nullptr,
                       const std::size_t iv_len = 0)
    {
        if (!is_key_valid(key, key_len) || !is_iv_valid(cip, iv, iv_len))
            return false;

        std::vector<DES_key_schedule> key_schedules;
        std::size_t n = multiple_key_(key_schedules, key, key_len);
        std::size_t idx = 0;
        dst_len = 0;
        switch (cip)
        {
        case cipher::des_ecb: {
            for (; idx + 8 <= src_len; idx += 8) 
            {
                DES_cblock block;
                memcpy(block, src + idx, 8);
                ede_encode_(key_schedules, block);
                memcpy(dst + idx, block, 8);
            }
            std::size_t remain = src_len - idx;
            if (remain > 0) 
            {
                unsigned char last_block[8];
                padding_block_(last_block, 8, src + idx, remain, pad_style);
                DES_cblock block;
                memcpy(block, last_block, 8);
                ede_encode_(key_schedules, block);
                memcpy(dst + idx, block, 8);
                idx += 8;
            }
            dst_len = idx;
            break;
        }
        case cipher::des_cbc: {
            if (!iv) return false;
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            for (; idx + 8 <= src_len; idx += 8)
            {
                DES_cblock block;
                memcpy(block, src + idx, 8);
                for (int i = 0; i < 8; ++i)
                    block[i] ^= iv_block[i];

                ede_encode_(key_schedules, block);
                memcpy(dst + idx, block, 8);
                memcpy(iv_block, block, 8);
            }
            std::size_t remain = src_len - idx;
            if (remain > 0) 
            {
                unsigned char last_block[8];
                padding_block_(last_block, 8, src + idx, remain, pad_style);
                for (int i = 0; i < 8; ++i)
                    last_block[i] ^= iv_block[i];

                DES_cblock block;
                memcpy(block, last_block, 8);
                ede_encode_(key_schedules, block);
                memcpy(dst + idx, block, 8);
                idx += 8;
            }
            dst_len = idx;
            break;
        }
        case cipher::des_cfb: {
            if (!iv) return false;
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            std::size_t full_blocks = src_len / 8;
            long remain = static_cast<long>(src_len % 8);
            for (std::size_t i = 0; i < full_blocks; ++i) 
            {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                ede_encode_(key_schedules, block);
                for (int k = 0; k < 8; ++k)
                    dst[i * 8 + k] = src[i * 8 + k] ^ block[k];

                memcpy(iv_block, dst + i * 8, 8);
            }
            idx = full_blocks * 8;
            if (is_need_padding_(pad_style)) 
            {
                unsigned char last_block[8] = {0};
                if (remain > 0)
                    padding_block_(last_block, 8, src + idx, remain, pad_style);
                else
                    padding_block_(last_block, 8, nullptr, 0, pad_style);

                DES_cblock block;
                memcpy(block, iv_block, 8);
                ede_encode_(key_schedules, block);
                for (int k = 0; k < 8; ++k)
                    dst[idx + k] = last_block[k] ^ block[k];

                idx += 8;
            } 
            else if (remain > 0) 
            {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                ede_encode_(key_schedules, block);
                for (int k = 0; k < remain; ++k)
                    dst[idx + k] = src[idx + k] ^ block[k];

                idx += remain;
            }
            dst_len = idx;
            break;
        }
        case cipher::des_ofb: {
            if (!iv) return false;
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            std::size_t full_blocks = src_len / 8;
            long remain = static_cast<long>(src_len % 8);
            for (std::size_t i = 0; i < full_blocks; ++i) 
            {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                ede_encode_(key_schedules, block);
                for (int k = 0; k < 8; ++k)
                    dst[i * 8 + k] = src[i * 8 + k] ^ block[k];

                memcpy(iv_block, block, 8);
            }
            idx = full_blocks * 8;
            if (is_need_padding_(pad_style)) 
            {
                unsigned char last_block[8] = {0};
                if (remain > 0)
                    padding_block_(last_block, 8, src + idx, remain, pad_style);
                else
                    padding_block_(last_block, 8, nullptr, 0, pad_style);

                DES_cblock block;
                memcpy(block, iv_block, 8);
                ede_encode_(key_schedules, block);
                for (int k = 0; k < 8; ++k)
                    dst[idx + k] = last_block[k] ^ block[k];

                idx += 8;
            } 
            else if (remain > 0) 
            {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                ede_encode_(key_schedules, block);
                for (int k = 0; k < remain; ++k)
                    dst[idx + k] = src[idx + k] ^ block[k];

                idx += remain;
            }
            dst_len = idx;
            break;
        }
        case cipher::des_ctr: {
            if (!iv) return false;
            DES_cblock ctr_block;
            memcpy(ctr_block, iv, 8);
            idx = 0;
            std::size_t padded_len = src_len;
            if (is_need_padding_(pad_style)) 
            {
                std::size_t pad = 8 - (src_len % 8);
                if (pad == 0) pad = 8;
                padded_len = src_len + pad;
            }
            while (idx < padded_len) 
            {
                unsigned char block[8] = {0};
                if (idx + 8 <= src_len) 
                {
                    memcpy(block, src + idx, 8);
                } 
                else if (idx < src_len) 
                {
                    std::size_t remain_bytes = src_len - idx;
                    padding_block_(block, 8, src + idx, remain_bytes, pad_style);
                } 
                else 
                {
                    padding_block_(block, 8, nullptr, 0, pad_style);
                }
                DES_cblock keystream;
                memcpy(keystream, ctr_block, 8);
                ede_encode_(key_schedules, keystream);
                for (std::size_t i = 0; i < 8; ++i)
                    dst[idx + i] = block[i] ^ keystream[i];
                for (int i = 7; i >= 0; --i)
                    if (++ctr_block[i] != 0) {break;}
                idx += 8;
            }
            dst_len = padded_len;
            break;
        }
        }
        return dst_len > 0;
    }

    // string -> encode string
    static bool encode(std::string& dst,
                       const std::string& src, 
                       const std::string& key,
                       const cipher cip = cipher::des_ecb,
                       const padding pad_style = padding::des_zero_padding,
                       const std::string& iv = std::string())
    {
        std::size_t dst_len = encode_len_reserve(src.size());
        dst.resize(dst_len);
        const unsigned char* iv_ptr = (cip == cipher::des_ecb) ? nullptr : reinterpret_cast<const unsigned char*>(iv.c_str());
        if (!encode(reinterpret_cast<unsigned char*>(const_cast<char*>(dst.data())),
                    dst_len,
                    reinterpret_cast<const unsigned char*>(src.c_str()),
                    src.size(),
                    reinterpret_cast<const unsigned char*>(key.c_str()),
                    key.size(),
                    cip,
                    pad_style,
                    iv_ptr,
                    iv.size()))
        {
            dst.clear();
            return false;
        }

        dst.resize(dst_len);
        return !dst.empty();
    }

    // stream -> encode stream
    static bool encode(std::ostream& out,
                       std::istream& in, 
                       const unsigned char* key, 
                       const std::size_t key_len,
                       const cipher cip = cipher::des_ecb,
                       const padding pad_style = padding::des_zero_padding,
                       const unsigned char* iv = nullptr,
                       const std::size_t iv_len = 0)
    {
        if (!in || !out)
            return false;

        if (!is_key_valid(key, key_len) || !is_iv_valid(cip, iv, iv_len))
            return false;

        std::vector<DES_key_schedule> key_schedules;
        std::size_t n = multiple_key_(key_schedules, key, key_len);
        constexpr std::size_t block_size = 8;
        char inbuf[block_size] = {0};
        char outbuf[block_size] = {0};
        DES_cblock iv_block;
        if (cip != cipher::des_ecb && cip != cipher::des_ctr && iv && iv_len >= 8)
            memcpy(iv_block, iv, 8);

        DES_cblock ctr_block;
        if (cip == cipher::des_ctr) 
        {
            if (!iv || iv_len < 8) 
                return false;

            memcpy(ctr_block, iv, 8);
        }

        while (true) 
        {
            in.read(inbuf, block_size);
            std::streamsize read_len = in.gcount();

            if (read_len == 0)
                break;

            bool is_last_block = false;
            if (read_len < (std::streamsize)block_size) 
            {
                is_last_block = true;
            } 
            else 
            {
                char peek;
                in.read(&peek, 1);
                if (in.gcount() == 0)
                    is_last_block = true;
                else
                    in.seekg(-1, std::ios::cur);
            }

            if (is_last_block) 
            {
                unsigned char last_block[block_size];
                padding_block_(last_block, block_size, reinterpret_cast<unsigned char*>(inbuf), static_cast<std::size_t>(read_len), pad_style);
                switch (cip) {
                case cipher::des_ecb: {
                    DES_cblock block;
                    memcpy(block, last_block, block_size);
                    ede_encode_(key_schedules, block);
                    out.write(reinterpret_cast<const char*>(block), block_size);
                    break;
                }
                case cipher::des_cbc: {
                    DES_cblock block;
                    memcpy(block, last_block, block_size);
                    for (int i = 0; i < 8; ++i)
                        block[i] ^= iv_block[i];

                    ede_encode_(key_schedules, block);
                    out.write(reinterpret_cast<const char*>(block), block_size);
                    memcpy(iv_block, block, 8);
                    break;
                }
                case cipher::des_cfb: {
                    DES_cblock block;
                    memcpy(block, iv_block, 8);
                    ede_encode_(key_schedules, block);
                    for (int k = 0; k < 8; ++k)
                        outbuf[k] = last_block[k] ^ block[k];

                    out.write(outbuf, block_size);
                    memcpy(iv_block, reinterpret_cast<unsigned char*>(outbuf), 8);
                    break;
                }
                case cipher::des_ofb: {
                    DES_cblock block;
                    memcpy(block, iv_block, 8);
                    ede_encode_(key_schedules, block);
                    for (int k = 0; k < 8; ++k)
                        outbuf[k] = last_block[k] ^ block[k];

                    out.write(outbuf, block_size);
                    memcpy(iv_block, block, 8);
                    break;
                }
                case cipher::des_ctr: {
                    DES_cblock keystream;
                    memcpy(keystream, ctr_block, 8);
                    ede_encode_(key_schedules, keystream);
                    for (std::size_t i = 0; i < block_size; ++i)
                        outbuf[i] = last_block[i] ^ keystream[i];

                    out.write(outbuf, block_size);
                    for (int i = 7; i >= 0; --i)
                        if (++ctr_block[i] != 0) {break;}

                    break;
                }
                }
                break;
            } else {
                switch (cip) {
                case cipher::des_ecb: {
                    DES_cblock block;
                    memcpy(block, inbuf, block_size);
                    ede_encode_(key_schedules, block);
                    out.write(reinterpret_cast<const char*>(block), block_size);
                    break;
                }
                case cipher::des_cbc: {
                    DES_cblock block;
                    memcpy(block, inbuf, block_size);
                    for (int i = 0; i < 8; ++i)
                        block[i] ^= iv_block[i];

                    ede_encode_(key_schedules, block);
                    out.write(reinterpret_cast<const char*>(block), block_size);
                    memcpy(iv_block, block, 8);
                    break;
                }
                case cipher::des_cfb: {
                    DES_cblock block;
                    memcpy(block, iv_block, 8);
                    ede_encode_(key_schedules, block);
                    for (int k = 0; k < 8; ++k)
                        outbuf[k] = inbuf[k] ^ block[k];

                    out.write(outbuf, block_size);
                    memcpy(iv_block, reinterpret_cast<unsigned char*>(outbuf), 8);
                    break;
                }
                case cipher::des_ofb: {
                    DES_cblock block;
                    memcpy(block, iv_block, 8);
                    ede_encode_(key_schedules, block);
                    for (int k = 0; k < 8; ++k)
                        outbuf[k] = inbuf[k] ^ block[k];

                    out.write(outbuf, block_size);
                    memcpy(iv_block, block, 8);
                    break;
                }
                case cipher::des_ctr: {
                    DES_cblock keystream;
                    memcpy(keystream, ctr_block, 8);
                    ede_encode_(key_schedules, keystream);
                    for (std::size_t i = 0; i < block_size; ++i)
                        outbuf[i] = inbuf[i] ^ keystream[i];

                    out.write(outbuf, block_size);
                    for (int i = 7; i >= 0; --i)
                        if (++ctr_block[i] != 0) break;

                    break;
                }
                }
            }
        }

        return true;
    }

    // file -> encode file
    static bool encode_file(const char* dst_file_path,
                            const char* src_file_path,  
                            const unsigned char* key, 
                            const std::size_t key_len,
                            const cipher cip = cipher::des_ecb,
                            const padding pad_style = padding::des_zero_padding,
                            const unsigned char* iv = nullptr,
                            const std::size_t iv_len = 0)
    {
        std::ifstream src_file(src_file_path, std::ios::binary);
        if (!src_file.is_open())
            return false;

        std::ofstream dst_file(dst_file_path, std::ios::binary);
        if (!dst_file.is_open())
            return false;

        return encode(dst_file, src_file, key, key_len, cip, pad_style, iv, iv_len);
    }

    // file -> encode file
    static bool encode_file(const std::string& dst_file_path,
                            const std::string& src_file_path,
                            const std::string& key,
                            const cipher cip = cipher::des_ecb,
                            const padding pad_style = padding::des_zero_padding,
                            const std::string& iv = std::string())
    {
        const unsigned char* iv_ptr = (cip == cipher::des_ecb) ? nullptr : reinterpret_cast<const unsigned char*>(iv.c_str());
        return encode_file(dst_file_path.c_str(),
                           src_file_path.c_str(),  
                           reinterpret_cast<const unsigned char*>(key.c_str()), 
                           key.size(),
                           cip,
                           pad_style,
                           iv_ptr,
                           iv.size());
    }

    // encode bytes -> bytes
    static bool decode(unsigned char* dst,
                       std::size_t& dst_len,
                       const unsigned char* src, 
                       const std::size_t src_len, 
                       const unsigned char* key, 
                       const std::size_t key_len, 
                       const cipher cip = cipher::des_ecb,
                       const padding pad_style = padding::des_zero_padding,
                       const unsigned char* iv = nullptr,
                       const std::size_t iv_len = 0)
    {
        if (!is_key_valid(key, key_len) || !is_iv_valid(cip, iv, iv_len))
            return false;

        std::vector<DES_key_schedule> key_schedules;
        std::size_t n = multiple_key_(key_schedules, key, key_len);
        std::size_t idx = 0;
        dst_len = 0;
        switch (cip)
        {
        case cipher::des_ecb: {
            for (; idx < src_len; idx += 8) 
            {
                DES_cblock block;
                memcpy(block, src + idx, 8);
                ede_decode_(key_schedules, block);
                memcpy(dst + idx, block, 8);
            }
            dst_len = idx;
            break;
        }
        case cipher::des_cbc: {
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            for (; idx < src_len; idx += 8) 
            {
                DES_cblock block, tmp;
                memcpy(block, src + idx, 8);
                memcpy(tmp, block, 8);
                ede_decode_(key_schedules, block);
                for (int i = 0; i < 8; ++i)
                    block[i] ^= iv_block[i];

                memcpy(dst + idx, block, 8);
                memcpy(iv_block, tmp, 8);
            }
            dst_len = idx;
            break;
        }
        case cipher::des_cfb: {
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            std::size_t full_blocks = src_len / 8;
            long remain = static_cast<long>(src_len % 8);
            for (std::size_t i = 0; i < full_blocks; ++i) 
            {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                ede_encode_(key_schedules, block); // same with encode
                for (int k = 0; k < 8; ++k)
                    dst[i * 8 + k] = src[i * 8 + k] ^ block[k];

                memcpy(iv_block, src + i * 8, 8);
            }
            idx = full_blocks * 8;
            if (remain > 0) 
            {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                ede_encode_(key_schedules, block); // same with encode
                for (int k = 0; k < remain; ++k)
                    dst[idx + k] = src[idx + k] ^ block[k];

                idx += remain;
            }
            dst_len = idx;
            break;
        }
        case cipher::des_ofb: {
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            std::size_t full_blocks = src_len / 8;
            long remain = static_cast<long>(src_len % 8);
            for (std::size_t i = 0; i < full_blocks; ++i) 
            {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                ede_encode_(key_schedules, block); // same with encode
                for (int k = 0; k < 8; ++k)
                    dst[i * 8 + k] = src[i * 8 + k] ^ block[k];

                memcpy(iv_block, block, 8);
            }
            idx = full_blocks * 8;
            if (remain > 0) {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                ede_encode_(key_schedules, block); // same with encode
                for (int k = 0; k < remain; ++k)
                    dst[idx + k] = src[idx + k] ^ block[k];

                idx += remain;
            }
            dst_len = idx;
            break;
        }
        case cipher::des_ctr: {
            DES_cblock ctr_block;
            memcpy(ctr_block, iv, 8);
            idx = 0;
            while (idx < src_len) 
            {
                DES_cblock keystream;
                memcpy(keystream, ctr_block, 8);
                ede_encode_(key_schedules, keystream); // same with encode
                std::size_t chunk = (src_len - idx >= 8) ? 8 : (src_len - idx);
                for (std::size_t i = 0; i < chunk; ++i)
                    dst[idx + i] = src[idx + i] ^ keystream[i];

                for (int i = 7; i >= 0; --i)
                    if (++ctr_block[i] != 0) break;

                idx += chunk;
            }
            dst_len = idx;
            break;
        }
        }

        unpadding_block_(dst, dst_len, pad_style);
        return dst_len > 0;
    }

    // encode string -> string
    static bool decode(std::string& dst,
                       const std::string& src, 
                       const std::string& key,
                       const cipher cip = cipher::des_ecb,
                       const padding pad_style = padding::des_zero_padding,
                       const std::string& iv = std::string())
    {
        dst.resize(decode_len_reserve(src.size()));
        std::size_t dst_len = dst.size();
        const unsigned char* iv_ptr = (cip == cipher::des_ecb) ? nullptr : reinterpret_cast<const unsigned char*>(iv.c_str());
        if (!decode(reinterpret_cast<unsigned char*>(const_cast<char*>(dst.data())),
                    dst_len,
                    reinterpret_cast<const unsigned char*>(src.c_str()),
                    src.size(),
                    reinterpret_cast<const unsigned char*>(key.c_str()),
                    key.size(),
                    cip,
                    pad_style,
                    iv_ptr,
                    iv.size()))
        {
            dst.clear();
            return false;
        }

        dst.resize(dst_len);
        return !dst.empty();
    }

    // decode stream -> decode stream
    static bool decode(std::ostream& out,
                       std::istream& in, 
                       const unsigned char* key, 
                       const std::size_t key_len,
                       const cipher cip = cipher::des_ecb,
                       const padding pad_style = padding::des_zero_padding,
                       const unsigned char* iv = nullptr,
                       const std::size_t iv_len = 0)
    {
        if (!in || !out)
            return false;

        if (!is_key_valid(key, key_len) || !is_iv_valid(cip, iv, iv_len))
            return false;

        std::vector<DES_key_schedule> key_schedules;
        std::size_t n = multiple_key_(key_schedules, key, key_len);

        constexpr std::size_t block_size = 8;
        char inbuf[block_size] = {0};
        char outbuf[block_size] = {0};
        DES_cblock iv_block;
        if (cip != cipher::des_ecb && iv && iv_len >= 8)
            memcpy(iv_block, iv, 8);

        DES_cblock ctr_block;
        if (cip == cipher::des_ctr && iv && iv_len >= 8)
            memcpy(ctr_block, iv, 8);

        std::streampos file_end;
        in.seekg(0, std::ios::end);
        file_end = in.tellg();
        in.seekg(0, std::ios::beg);

        while (in.tellg() < file_end) 
        {
            in.read(inbuf, block_size);
            std::streamsize read_len = in.gcount();
            if (read_len == 0)
                break;

            bool is_last_block = (in.tellg() == file_end);

            if (is_last_block) 
            {
                switch (cip) {
                case cipher::des_ecb: {
                    DES_cblock block;
                    memcpy(block, inbuf, block_size);
                    ede_decode_(key_schedules, block);
                    memcpy(outbuf, block, block_size);
                    std::size_t out_len = block_size;
                    unpadding_block_(reinterpret_cast<unsigned char*>(outbuf), out_len, pad_style);
                    out.write(outbuf, out_len);
                    break;
                }
                case cipher::des_cbc: {
                    DES_cblock block, tmp;
                    memcpy(block, inbuf, block_size);
                    memcpy(tmp, block, block_size);
                    ede_decode_(key_schedules, block);
                    for (int i = 0; i < 8; ++i)
                        block[i] ^= iv_block[i];

                    memcpy(outbuf, block, block_size);
                    memcpy(iv_block, tmp, 8);
                    std::size_t out_len = block_size;
                    unpadding_block_(reinterpret_cast<unsigned char*>(outbuf), out_len, pad_style);
                    out.write(outbuf, out_len);
                    break;
                }
                case cipher::des_cfb: {
                    DES_cblock block;
                    memcpy(block, iv_block, 8);
                    ede_encode_(key_schedules, block); // same with encode
                    for (int k = 0; k < 8; ++k)
                        outbuf[k] = inbuf[k] ^ block[k];

                    memcpy(iv_block, inbuf, 8);
                    std::size_t out_len = block_size;
                    unpadding_block_(reinterpret_cast<unsigned char*>(outbuf), out_len, pad_style);
                    out.write(outbuf, out_len);
                    break;
                }
                case cipher::des_ofb: {
                    DES_cblock block;
                    memcpy(block, iv_block, 8);
                    ede_encode_(key_schedules, block); // same with encode
                    for (int k = 0; k < 8; ++k)
                        outbuf[k] = inbuf[k] ^ block[k];

                    memcpy(iv_block, block, 8);
                    std::size_t out_len = block_size;
                    unpadding_block_(reinterpret_cast<unsigned char*>(outbuf), out_len, pad_style);
                    out.write(outbuf, out_len);
                    break;
                }
                case cipher::des_ctr: {
                    DES_cblock keystream;
                    memcpy(keystream, ctr_block, 8);
                    ede_encode_(key_schedules, keystream); // same with encode
                    for (std::size_t i = 0; i < block_size; ++i)
                        outbuf[i] = inbuf[i] ^ keystream[i];

                    for (int i = 7; i >= 0; --i)
                        if (++ctr_block[i] != 0) {break;}

                    std::size_t out_len = block_size;
                    unpadding_block_(reinterpret_cast<unsigned char*>(outbuf), out_len, pad_style);
                    out.write(outbuf, out_len);
                    break;
                }
                }
            } 
            else 
            {
                switch (cip) {
                case cipher::des_ecb: {
                    DES_cblock block;
                    memcpy(block, inbuf, block_size);
                    ede_decode_(key_schedules, block);
                    out.write(reinterpret_cast<const char*>(block), block_size);
                    break;
                }
                case cipher::des_cbc: {
                    DES_cblock block, tmp;
                    memcpy(block, inbuf, block_size);
                    memcpy(tmp, block, block_size);
                    ede_decode_(key_schedules, block);
                    for (int i = 0; i < 8; ++i)
                        block[i] ^= iv_block[i];

                    out.write(reinterpret_cast<const char*>(block), block_size);
                    memcpy(iv_block, tmp, 8);
                    break;
                }
                case cipher::des_cfb: {
                    DES_cblock block;
                    memcpy(block, iv_block, 8);
                    ede_encode_(key_schedules, block); // same with encode
                    for (int k = 0; k < 8; ++k)
                        outbuf[k] = inbuf[k] ^ block[k];

                    out.write(outbuf, block_size);
                    memcpy(iv_block, inbuf, 8);
                    break;
                }
                case cipher::des_ofb: {
                    DES_cblock block;
                    memcpy(block, iv_block, 8);
                    ede_encode_(key_schedules, block); // same with encode
                    for (int k = 0; k < 8; ++k)
                        outbuf[k] = inbuf[k] ^ block[k];

                    out.write(outbuf, block_size);
                    memcpy(iv_block, block, 8);
                    break;
                }
                case cipher::des_ctr: {
                    DES_cblock keystream;
                    memcpy(keystream, ctr_block, 8);
                    ede_encode_(key_schedules, keystream); // same with encode
                    for (std::size_t i = 0; i < block_size; ++i)
                        outbuf[i] = inbuf[i] ^ keystream[i];

                    out.write(outbuf, block_size);
                    for (int i = 7; i >= 0; --i)
                        if (++ctr_block[i] != 0) {break;}
                        
                    break;
                }
                }
            }
        }

        return true;
    }

    // decode file -> file
    static bool decode_file(const char* dst_file_path,
                            const char* src_file_path,  
                            const unsigned char* key, 
                            const std::size_t key_len,
                            const cipher cip = cipher::des_ecb,
                            const padding pad_style = padding::des_zero_padding,
                            const unsigned char* iv = nullptr,
                            const std::size_t iv_len = 0)
    {
        std::ifstream src_file(src_file_path, std::ios::binary);
        if (!src_file.is_open())
            return false;

        std::ofstream dst_file(dst_file_path, std::ios::binary);
        if (!dst_file.is_open())
            return false;

        return decode(dst_file, src_file, key, key_len, cip, pad_style, iv, iv_len);
    }

    // encode file -> file
    static bool decode_file(const std::string& dst_file_path,
                            const std::string& src_file_path,
                            const std::string& key,
                            const cipher cip = cipher::des_ecb,
                            const padding pad_style = padding::des_zero_padding,
                            const std::string& iv = std::string())
    {
        const unsigned char* iv_ptr = (cip == cipher::des_ecb) ? nullptr : reinterpret_cast<const unsigned char*>(iv.c_str());
        return decode_file(dst_file_path.c_str(),
                           src_file_path.c_str(),  
                           reinterpret_cast<const unsigned char*>(key.c_str()), 
                           key.size(),
                           cip,
                           pad_style,
                           iv_ptr,
                           iv.size());
    }

    // reserve encode dst buf size
	static std::size_t encode_len_reserve(const std::size_t src_len)
	{
        return (src_len / 8) * 8 + 8;
	}

	// reserve decode dst buf size
	static std::size_t decode_len_reserve(const std::size_t src_len)
	{
		return (src_len / 8) * 8 + 8;
	}

    static bool is_key_valid(const unsigned char* key, const std::size_t key_len)
    {
        if (key == nullptr || key_len < 8 || key_len % 8 != 0)
            return false;

        return true;
    }

    static bool is_iv_valid(const cipher cip, const unsigned char* iv, const std::size_t iv_len)
    {
        // iv is required for CBC, CFB, OFB, and CTR modes
        if (cip != cipher::des_ecb && (iv == nullptr || iv_len != 8))
            return false;

        return true;
    }

private:
    // padding block
    static void padding_block_(unsigned char* dst, 
                               const std::size_t dst_len,
                               const unsigned char* src, 
                               const std::size_t src_len,
                               const padding pad_style)
    {
        memset(dst, 0, dst_len);
        memcpy(dst, src, src_len);
        unsigned char pad_val = static_cast<unsigned char>(dst_len - src_len);
        
        switch (pad_style) {
        case padding::des_pkcs5_padding:
        case padding::des_pkcs7_padding:
            for (std::size_t i = src_len; i < dst_len; ++i)
                dst[i] = pad_val;
            break;
        case padding::des_zero_padding:
            break;
        case padding::des_iso10126_padding:
            for (std::size_t i = src_len; i < dst_len - 1; ++i)
                dst[i] = static_cast<unsigned char>(rand() % 256);

            dst[dst_len - 1] = pad_val;
            break;
        case padding::des_ansix923_padding:
            dst[dst_len - 1] = pad_val;
            break;
        case padding::des_iso_iec_7816_4_padding:
            dst[src_len] = 0x80;
            break;
        case padding::des_no_padding:
            break;
        default:
            break;
        }
    }

    // unpadding block
    static void unpadding_block_(unsigned char* buf, std::size_t& len, const padding pad_style)
    {
        switch (pad_style) {
        case padding::des_pkcs5_padding:
        case padding::des_pkcs7_padding: {
            if (len == 0) 
                return;

            unsigned char pad = buf[len - 1];
            if (pad == 0 || pad > 8)
                return;

            for (std::size_t i = len - pad; i < len; ++i)
                if (buf[i] != pad) {return;}
            
            len = len - pad;
            return;
        }
        case padding::des_zero_padding: {
            while (len > 0 && buf[len - 1] == 0)
                --len;

            return;
        }
        case padding::des_iso10126_padding: {
            if (len == 0) 
                return;
        
            unsigned char pad = buf[len - 1];
            if (pad == 0 || pad > 8) 
                return;

            len = len - pad;
            return;
        }
        case padding::des_ansix923_padding: {
            if (len == 0) 
                return;

            unsigned char pad = buf[len - 1];
            if (pad == 0 || pad > 8) 
                return;

            for (std::size_t i = len - pad; i < len - 1; ++i)
                if (buf[i] != 0) {return;}

            len = len - pad;
            return;
        }
        case padding::des_iso_iec_7816_4_padding: {
            int i = static_cast<int>(len - 1);
            while (i >= 0 && buf[i] == 0) 
                --i;

            if (i >= 0 && buf[i] == 0x80)
                len = static_cast<std::size_t>(i);
            
            return;
        }
        case padding::des_no_padding:
            return;
        default:
            return;
        }
    }

    static bool is_need_padding_(const padding pad_style)
    {
        switch (pad_style)
        {
        case padding::des_pkcs5_padding:
        case padding::des_pkcs7_padding:
        case padding::des_iso10126_padding:
        case padding::des_ansix923_padding:
        case padding::des_iso_iec_7816_4_padding:
        case padding::des_zero_padding:
            return true;
        default:
            return false;
        }
    }

    static std::size_t multiple_key_(std::vector<DES_key_schedule>& key_schedules, 
                                     const unsigned char* key, 
                                     const std::size_t key_len)
    {
        std::size_t n = key_len / 8;
        key_schedules.resize(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            DES_cblock k;
            memcpy(k, key + i * 8, 8);
            DES_set_key_unchecked(&k, &key_schedules[i]);
        }
        return n;
    }

    static void ede_encode_(std::vector<DES_key_schedule>& key_schedules, DES_cblock& block)
    {
        // E(K1) -> D(K2) -> E(K3) -> ...
        if (key_schedules.size() == 1) 
        {
            DES_ecb_encrypt(&block, &block, &key_schedules[0], DES_ENCRYPT);
        } 
        else if (key_schedules.size() == 2) 
        {
            DES_ecb_encrypt(&block, &block, &key_schedules[0], DES_ENCRYPT);
            DES_ecb_encrypt(&block, &block, &key_schedules[1], DES_DECRYPT);
            DES_ecb_encrypt(&block, &block, &key_schedules[0], DES_ENCRYPT);
        } 
        else if (key_schedules.size() == 3) 
        {
            DES_ecb_encrypt(&block, &block, &key_schedules[0], DES_ENCRYPT);
            DES_ecb_encrypt(&block, &block, &key_schedules[1], DES_DECRYPT);
            DES_ecb_encrypt(&block, &block, &key_schedules[2], DES_ENCRYPT);
        } 
        else 
        {
            for (std::size_t i = 0; i < key_schedules.size(); ++i) 
            {
                int mode = (i % 2 == 0) ? DES_ENCRYPT : DES_DECRYPT;
                DES_ecb_encrypt(&block, &block, &key_schedules[i], mode);
            }
        }
    }

    static void ede_decode_(std::vector<DES_key_schedule>& key_schedules, DES_cblock& block)
    {
        // ... -> D(K3) -> E(K2) -> D(K1)
        if (key_schedules.size() == 1) 
        {
            DES_ecb_encrypt(&block, &block, &key_schedules[0], DES_DECRYPT);
        } 
        else if (key_schedules.size() == 2) 
        {
            DES_ecb_encrypt(&block, &block, &key_schedules[0], DES_DECRYPT);
            DES_ecb_encrypt(&block, &block, &key_schedules[1], DES_ENCRYPT);
            DES_ecb_encrypt(&block, &block, &key_schedules[0], DES_DECRYPT);
        } 
        else if (key_schedules.size() == 3) 
        {
            DES_ecb_encrypt(&block, &block, &key_schedules[2], DES_DECRYPT);
            DES_ecb_encrypt(&block, &block, &key_schedules[1], DES_ENCRYPT);
            DES_ecb_encrypt(&block, &block, &key_schedules[0], DES_DECRYPT);
        } 
        else 
        {
            for (std::size_t i = key_schedules.size(); i-- > 0; ) 
            {
                int mode = (i % 2 == 0) ? DES_DECRYPT : DES_ENCRYPT;
                DES_ecb_encrypt(&block, &block, &key_schedules[i], mode);
            }
        }
    }

private:
    des() = default;
    ~des() = default;
    des(const des&) = delete;
    des& operator=(const des&) = delete;
    des(des&&) = delete;
    des& operator=(des&&) = delete;
};

}

#endif