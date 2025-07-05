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
                       const unsigned char* iv = nullptr)
    {
        if (!is_key_valid(key, key_len) || !is_iv_valid(cip, iv))
            return false;

        DES_cblock key_encrypt;
        memset(key_encrypt, 0, 8);
        if (key_len <= 8)
            memcpy(key_encrypt, key, key_len);
        else
            memcpy(key_encrypt, key, 8); 

        DES_key_schedule key_schedule;
        DES_set_key(&key_encrypt, &key_schedule);

        std::size_t idx = 0;
        dst_len = 0;
        switch (cip)
        {
        case cipher::des_ecb: {
            const_DES_cblock input;
            DES_cblock output;
            for (; idx + 8 <= src_len; idx += 8) 
            {
                memcpy(input, src + idx, 8);
                DES_ecb_encrypt(&input, &output, &key_schedule, DES_ENCRYPT);
                memcpy(dst + idx, output, 8);
            }

            std::size_t remain = src_len - idx;
            if (remain > 0) 
            {
                unsigned char last_block[8];
                padding_block_(last_block, 8, src + idx, remain, pad_style);
                DES_ecb_encrypt((const_DES_cblock*)last_block, &output, &key_schedule, DES_ENCRYPT);
                memcpy(dst + idx, output, 8);
                idx += 8;
            }
            dst_len = idx;
            break;
        }
        case cipher::des_cbc: {
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            for (; idx + 8 <= src_len; idx += 8)
                DES_ncbc_encrypt(src + idx, dst + idx, 8, &key_schedule, &iv_block, DES_ENCRYPT);

            std::size_t remain = src_len - idx;
            if (remain > 0) 
            {
                unsigned char last_block[8];
                padding_block_(last_block, 8, src + idx, remain, pad_style);
                DES_ncbc_encrypt(last_block, dst + idx, 8, &key_schedule, &iv_block, DES_ENCRYPT);
                idx += 8;
            }

            dst_len = idx;
            break;
        }
        case cipher::des_cfb: {
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            int num = 0;
            std::size_t full_blocks = src_len / 8;
            long remain = static_cast<long>(src_len % 8);
            for (std::size_t i = 0; i < full_blocks; ++i)
                DES_cfb64_encrypt(src + i * 8, dst + i * 8, 8, &key_schedule, &iv_block, &num, DES_ENCRYPT);

            idx = full_blocks * 8;
            if (is_need_padding_(pad_style)) 
            {
                unsigned char last_block[8];
                if (remain > 0)
                    padding_block_(last_block, 8, src + idx, remain, pad_style);
                else
                    padding_block_(last_block, 8, nullptr, 0, pad_style);

                DES_cfb64_encrypt(last_block, dst + idx, 8, &key_schedule, &iv_block, &num, DES_ENCRYPT);
                idx += 8;
            } else {
                if (remain > 0) 
                {
                    DES_cfb64_encrypt(src + idx, dst + idx, remain, &key_schedule, &iv_block, &num, DES_ENCRYPT);
                    idx += remain;
                }
            }
            dst_len = idx;
            break;
        }
        case cipher::des_ofb: {
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            int num = 0;
            std::size_t full_blocks = src_len / 8;
            long remain = static_cast<long>(src_len % 8);
            for (std::size_t i = 0; i < full_blocks; ++i)
                DES_ofb64_encrypt(src + i * 8, dst + i * 8, 8, &key_schedule, &iv_block, &num);

            idx = full_blocks * 8;
            if (is_need_padding_(pad_style)) 
            {
                unsigned char last_block[8];
                if (remain > 0)
                    padding_block_(last_block, 8, src + idx, remain, pad_style);
                else
                    padding_block_(last_block, 8, nullptr, 0, pad_style);

                DES_ofb64_encrypt(last_block, dst + idx, 8, &key_schedule, &iv_block, &num);
                idx += 8;
            } else {
                if (remain > 0) 
                {
                    DES_ofb64_encrypt(src + idx, dst + idx, remain, &key_schedule, &iv_block, &num);
                    idx += remain;
                }
            }
            dst_len = idx;
            break;
        }
        case cipher::des_ctr: {
            DES_cblock ctr_block;
            memcpy(ctr_block, iv, 8);
            idx = 0;
            std::size_t padded_len = src_len;
            bool need_padding = is_need_padding_(pad_style);
            if (need_padding) 
            {
                std::size_t pad = 8 - (src_len % 8);
                if (pad == 0) {pad = 8;}
                padded_len = src_len + pad;
            }

            while (idx < padded_len) 
            {
                unsigned char block[8] = {0};
                if (idx + 8 <= src_len) 
                {
                    memcpy(block, src + idx, 8);
                } else if (idx < src_len) {
                    std::size_t remain_bytes = src_len - idx;
                    padding_block_(block, 8, src + idx, remain_bytes, pad_style);
                } else {
                    padding_block_(block, 8, nullptr, 0, pad_style);
                }

                DES_cblock keystream;
                DES_ecb_encrypt(&ctr_block, &keystream, &key_schedule, DES_ENCRYPT);
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
                    iv_ptr))
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
                       const unsigned char* iv = nullptr)
    {
        if (!in || !out)
            return false;

        if (!is_key_valid(key, key_len) || !is_iv_valid(cip, iv))
            return false;

        constexpr std::size_t block_size = 8;
        char inbuf[block_size] = {0};
        char outbuf[block_size] = {0};

        DES_cblock key_encrypt;
        memset(key_encrypt, 0, 8);
        memcpy(key_encrypt, key, (key_len < 8) ? key_len : 8);
        DES_key_schedule key_schedule;
        DES_set_key_unchecked(&key_encrypt, &key_schedule);

        DES_cblock iv_block;
        if (cip != cipher::des_ecb && iv)
            memcpy(iv_block, iv, 8);

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
            } else {
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
                    DES_cblock output;
                    DES_ecb_encrypt((const_DES_cblock*)last_block, &output, &key_schedule, DES_ENCRYPT);
                    out.write(reinterpret_cast<const char*>(output), block_size);
                    break;
                }
                case cipher::des_cbc: {
                    DES_ncbc_encrypt(last_block, reinterpret_cast<unsigned char*>(outbuf), block_size, &key_schedule, &iv_block, DES_ENCRYPT);
                    out.write(outbuf, block_size);
                    break;
                }
                case cipher::des_cfb: {
                    int num = 0;
                    DES_cfb64_encrypt(last_block, reinterpret_cast<unsigned char*>(outbuf), block_size, &key_schedule, &iv_block, &num, DES_ENCRYPT);
                    out.write(outbuf, block_size);
                    break;
                }
                case cipher::des_ofb: {
                    int num = 0;
                    DES_ofb64_encrypt(last_block, reinterpret_cast<unsigned char*>(outbuf), block_size, &key_schedule, &iv_block, &num);
                    out.write(outbuf, block_size);
                    break;
                }
                case cipher::des_ctr: {
                    DES_cblock ctr_block;
                    memcpy(ctr_block, iv, 8);
                    DES_cblock keystream;
                    DES_ecb_encrypt(&ctr_block, &keystream, &key_schedule, DES_ENCRYPT);
                    for (std::size_t i = 0; i < block_size; ++i)
                        outbuf[i] = last_block[i] ^ keystream[i];
                    out.write(outbuf, block_size);
                    break;
                }
                }
                break;
            } else {
                switch (cip) {
                case cipher::des_ecb: {
                    DES_cblock input, output;
                    memcpy(input, inbuf, block_size);
                    DES_ecb_encrypt(&input, &output, &key_schedule, DES_ENCRYPT);
                    out.write(reinterpret_cast<const char*>(output), block_size);
                    break;
                }
                case cipher::des_cbc: {
                    DES_ncbc_encrypt(reinterpret_cast<unsigned char*>(inbuf), reinterpret_cast<unsigned char*>(outbuf), block_size, &key_schedule, &iv_block, DES_ENCRYPT);
                    out.write(outbuf, block_size);
                    break;
                }
                case cipher::des_cfb: {
                    int num = 0;
                    DES_cfb64_encrypt(reinterpret_cast<unsigned char*>(inbuf), reinterpret_cast<unsigned char*>(outbuf), block_size, &key_schedule, &iv_block, &num, DES_ENCRYPT);
                    out.write(outbuf, block_size);
                    break;
                }
                case cipher::des_ofb: {
                    int num = 0;
                    DES_ofb64_encrypt(reinterpret_cast<unsigned char*>(inbuf), reinterpret_cast<unsigned char*>(outbuf), block_size, &key_schedule, &iv_block, &num);
                    out.write(outbuf, block_size);
                    break;
                }
                case cipher::des_ctr: {
                    DES_cblock ctr_block;
                    memcpy(ctr_block, iv, 8);
                    DES_cblock keystream;
                    DES_ecb_encrypt(&ctr_block, &keystream, &key_schedule, DES_ENCRYPT);
                    for (std::size_t i = 0; i < block_size; ++i)
                        outbuf[i] = inbuf[i] ^ keystream[i];
                    out.write(outbuf, block_size);
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
                            const unsigned char* iv = nullptr)
    {
        std::ifstream src_file(src_file_path, std::ios::binary);
        if (!src_file.is_open())
            return false;

        std::ofstream dst_file(dst_file_path, std::ios::binary);
        if (!dst_file.is_open())
            return false;

        return encode(dst_file, src_file, key, key_len, cip, pad_style, iv);
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
                           iv_ptr);
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
                       const unsigned char* iv = nullptr)
    {
        if (!is_key_valid(key, key_len) || !is_iv_valid(cip, iv))
            return false;

        DES_cblock key_encrypt;
        memset(key_encrypt, 0, 8);
        if (key_len <= 8)
            memcpy(key_encrypt, key, key_len);
        else
            memcpy(key_encrypt, key, 8);

        DES_key_schedule key_schedule;
        DES_set_key_unchecked(&key_encrypt, &key_schedule);

        std::size_t idx = 0;
        dst_len = 0;
        switch (cip)
        {
        case cipher::des_ecb: {
            const_DES_cblock input;
            DES_cblock output;
            for (; idx < src_len; idx += 8)
            {
                memcpy(input, src + idx, 8);
                DES_ecb_encrypt(&input, &output, &key_schedule, DES_DECRYPT);
                memcpy(dst + idx, output, 8);
            }
            dst_len = idx;
            break;
        }
        case cipher::des_cbc: {
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            for (; idx < src_len; idx += 8)
                DES_ncbc_encrypt(src + idx, dst + idx, 8, &key_schedule, &iv_block, DES_DECRYPT);
            dst_len = idx;
            break;
        }
        case cipher::des_cfb: {
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            int num = 0;
            for (; idx + 8 <= src_len; idx += 8)
                DES_cfb64_encrypt(src + idx, dst + idx, 8, &key_schedule, &iv_block, &num, DES_DECRYPT);

            long remain = static_cast<long>(src_len - idx);
            if (remain > 0)
                DES_cfb64_encrypt(src + idx, dst + idx, remain, &key_schedule, &iv_block, &num, DES_DECRYPT);
        
            idx += remain;
            dst_len = idx;
            break;
        }
        case cipher::des_ofb: {
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            int num = 0;
            for (; idx + 8 <= src_len; idx += 8)
                DES_ofb64_encrypt(src + idx, dst + idx, 8, &key_schedule, &iv_block, &num);
        
            long remain = static_cast<long>(src_len - idx);
            if (remain > 0)
                DES_ofb64_encrypt(src + idx, dst + idx, remain, &key_schedule, &iv_block, &num);
        
            idx += remain;
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
                DES_ecb_encrypt(&ctr_block, &keystream, &key_schedule, DES_ENCRYPT);
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
                    iv_ptr))
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
                       const unsigned char* iv = nullptr)
    {
        if (!in || !out)
            return false;

        if (!is_key_valid(key, key_len) || !is_iv_valid(cip, iv))
            return false;

        constexpr std::size_t block_size = 8;
        char inbuf[block_size] = {0};
        char outbuf[block_size] = {0};

        DES_cblock key_encrypt;
        memset(key_encrypt, 0, 8);
        memcpy(key_encrypt, key, (key_len < 8) ? key_len : 8);
        DES_key_schedule key_schedule;
        DES_set_key_unchecked(&key_encrypt, &key_schedule);

        DES_cblock iv_block;
        if (cip != cipher::des_ecb && iv)
            memcpy(iv_block, iv, 8);

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

            bool is_last_block = false;
            if (in.tellg() == file_end)
                is_last_block = true;

            if (is_last_block) 
            {
                switch (cip) {
                    case cipher::des_ecb: {
                        DES_cblock input, output;
                        memcpy(input, inbuf, block_size);
                        DES_ecb_encrypt(&input, &output, &key_schedule, DES_DECRYPT);
                        memcpy(outbuf, output, block_size);
                        std::size_t out_len = block_size;
                        unpadding_block_(reinterpret_cast<unsigned char*>(outbuf), out_len, pad_style);
                        out.write(outbuf, out_len);
                        break;
                    }
                    case cipher::des_cbc: {
                        DES_ncbc_encrypt(reinterpret_cast<unsigned char*>(inbuf), reinterpret_cast<unsigned char*>(outbuf), block_size, &key_schedule, &iv_block, DES_DECRYPT);
                        std::size_t out_len = block_size;
                        unpadding_block_(reinterpret_cast<unsigned char*>(outbuf), out_len, pad_style);
                        out.write(outbuf, out_len);
                        break;
                    }
                    case cipher::des_cfb: {
                        int num = 0;
                        DES_cfb64_encrypt(reinterpret_cast<unsigned char*>(inbuf), reinterpret_cast<unsigned char*>(outbuf), block_size, &key_schedule, &iv_block, &num, DES_DECRYPT);
                        std::size_t out_len = block_size;
                        unpadding_block_(reinterpret_cast<unsigned char*>(outbuf), out_len, pad_style);
                        out.write(outbuf, out_len);
                        break;
                    }
                    case cipher::des_ofb: {
                        int num = 0;
                        DES_ofb64_encrypt(reinterpret_cast<unsigned char*>(inbuf), reinterpret_cast<unsigned char*>(outbuf), block_size, &key_schedule, &iv_block, &num);
                        std::size_t out_len = block_size;
                        unpadding_block_(reinterpret_cast<unsigned char*>(outbuf), out_len, pad_style);
                        out.write(outbuf, out_len);
                        break;
                    }
                    case cipher::des_ctr: {
                        DES_cblock ctr_block;
                        memcpy(ctr_block, iv, 8);
                        DES_cblock keystream;
                        DES_ecb_encrypt(&ctr_block, &keystream, &key_schedule, DES_ENCRYPT);
                        for (std::size_t i = 0; i < block_size; ++i)
                            outbuf[i] = inbuf[i] ^ keystream[i];

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
                        DES_cblock input, output;
                        memcpy(input, inbuf, block_size);
                        DES_ecb_encrypt(&input, &output, &key_schedule, DES_DECRYPT);
                        out.write(reinterpret_cast<const char*>(output), block_size);
                        break;
                    }
                    case cipher::des_cbc: {
                        DES_ncbc_encrypt(reinterpret_cast<unsigned char*>(inbuf), reinterpret_cast<unsigned char*>(outbuf), block_size, &key_schedule, &iv_block, DES_DECRYPT);
                        out.write(outbuf, block_size);
                        break;
                    }
                    case cipher::des_cfb: {
                        int num = 0;
                        DES_cfb64_encrypt(reinterpret_cast<unsigned char*>(inbuf), reinterpret_cast<unsigned char*>(outbuf), block_size, &key_schedule, &iv_block, &num, DES_DECRYPT);
                        out.write(outbuf, block_size);
                        break;
                    }
                    case cipher::des_ofb: {
                        int num = 0;
                        DES_ofb64_encrypt(reinterpret_cast<unsigned char*>(inbuf), reinterpret_cast<unsigned char*>(outbuf), block_size, &key_schedule, &iv_block, &num);
                        out.write(outbuf, block_size);
                        break;
                    }
                    case cipher::des_ctr: {
                        DES_cblock ctr_block;
                        memcpy(ctr_block, iv, 8);
                        DES_cblock keystream;
                        DES_ecb_encrypt(&ctr_block, &keystream, &key_schedule, DES_ENCRYPT);
                        for (std::size_t i = 0; i < block_size; ++i)
                            outbuf[i] = inbuf[i] ^ keystream[i];

                        out.write(outbuf, block_size);
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
                            const unsigned char* iv = nullptr)
    {
        std::ifstream src_file(src_file_path, std::ios::binary);
        if (!src_file.is_open())
            return false;

        std::ofstream dst_file(dst_file_path, std::ios::binary);
        if (!dst_file.is_open())
            return false;

        return decode(dst_file, src_file, key, key_len, cip, pad_style, iv);
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
                           iv_ptr);
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
        if (key == nullptr || key_len != 8)
            return false;

        return true;
    }

    static bool is_iv_valid(const cipher cip, const unsigned char* iv)
    {
        if (cip != cipher::des_ecb && iv == nullptr)
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