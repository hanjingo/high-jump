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
    enum class mode
    {
        ecb,
        cbc,
        cfb,
        ofb,
        ctr
    };

    enum class padding
    {
        pkcs5,
        pkcs7,
        zero,
        iso10126,
        ansix923,
        iso_iec_7816_4,
        no_padding
    };

    static constexpr std::size_t block_size = 8;

public:
    // bytes -> encrypt bytes
    static bool encrypt(unsigned char* dst,
                        std::size_t& dst_len,
                        const unsigned char* src, 
                        const std::size_t src_len, 
                        const unsigned char* key, 
                        const std::size_t key_len, 
                        const mode mod = mode::ecb,
                        const padding pad_style = padding::zero,
                        const unsigned char* iv = nullptr,
                        const std::size_t iv_len = 0)
    {
        if (!is_key_valid(key, key_len) || !is_iv_valid(mod, iv, iv_len))
            return false;

        if (!is_plain_valid(src, src_len, pad_style))
            return false;

        std::vector<DES_key_schedule> key_schedules;
        std::size_t n = _multiple_key(key_schedules, key, key_len);
        std::size_t idx = 0;
        dst_len = 0;
        switch (mod)
        {
        case mode::ecb: {
            for (; idx + 8 <= src_len; idx += 8) 
            {
                DES_cblock block;
                memcpy(block, src + idx, 8);
                _ede_encrypt(key_schedules, block);
                memcpy(dst + idx, block, 8);
            }
            std::size_t remain = src_len - idx;
            if (pad_style == padding::no_padding && remain > 0) 
                return false;

            if (remain > 0 || _is_need_padding(pad_style)) 
            {
                unsigned char last_block[8];
                _padding_block(last_block, 8, src + idx, remain, pad_style);
                DES_cblock block;
                memcpy(block, last_block, 8);
                _ede_encrypt(key_schedules, block);
                memcpy(dst + idx, block, 8);
                idx += 8;
            }
            dst_len = idx;
            break;
        }
        case mode::cbc: {
            if (!iv) return false;
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            for (; idx + 8 <= src_len; idx += 8)
            {
                DES_cblock block;
                memcpy(block, src + idx, 8);
                for (int i = 0; i < 8; ++i)
                    block[i] ^= iv_block[i];

                _ede_encrypt(key_schedules, block);
                memcpy(dst + idx, block, 8);
                memcpy(iv_block, block, 8);
            }
            std::size_t remain = src_len - idx;
            if (pad_style == padding::no_padding && remain > 0) 
                return false;

            if (remain > 0 || _is_need_padding(pad_style)) 
            {
                unsigned char last_block[8];
                _padding_block(last_block, 8, src + idx, remain, pad_style);
                for (int i = 0; i < 8; ++i)
                    last_block[i] ^= iv_block[i];

                DES_cblock block;
                memcpy(block, last_block, 8);
                _ede_encrypt(key_schedules, block);
                memcpy(dst + idx, block, 8);
                idx += 8;
            }
            dst_len = idx;
            break;
        }
        case mode::cfb: {
            if (!iv) return false;
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            std::size_t full_blocks = src_len / 8;
            long remain = static_cast<long>(src_len % 8);
            for (std::size_t i = 0; i < full_blocks; ++i) 
            {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                _ede_encrypt(key_schedules, block);
                for (int k = 0; k < 8; ++k)
                    dst[i * 8 + k] = src[i * 8 + k] ^ block[k];

                memcpy(iv_block, dst + i * 8, 8);
            }
            idx = full_blocks * 8;
            if (_is_need_padding(pad_style)) 
            {
                unsigned char last_block[8] = {0};
                if (remain > 0)
                    _padding_block(last_block, 8, src + idx, remain, pad_style);
                else
                    _padding_block(last_block, 8, nullptr, 0, pad_style);

                DES_cblock block;
                memcpy(block, iv_block, 8);
                _ede_encrypt(key_schedules, block);
                for (int k = 0; k < 8; ++k)
                    dst[idx + k] = last_block[k] ^ block[k];

                idx += 8;
            } 
            else if (remain > 0) 
            {
                if (pad_style == padding::no_padding) 
                    return false;

                DES_cblock block;
                memcpy(block, iv_block, 8);
                _ede_encrypt(key_schedules, block);
                for (int k = 0; k < remain; ++k)
                    dst[idx + k] = src[idx + k] ^ block[k];

                idx += remain;
            }
            dst_len = idx;
            break;
        }
        case mode::ofb: {
            if (!iv) return false;
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            std::size_t full_blocks = src_len / 8;
            long remain = static_cast<long>(src_len % 8);
            if (pad_style == padding::no_padding && remain > 0) 
                return false;

            for (std::size_t i = 0; i < full_blocks; ++i) 
            {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                _ede_encrypt(key_schedules, block);
                for (int k = 0; k < 8; ++k)
                    dst[i * 8 + k] = src[i * 8 + k] ^ block[k];

                memcpy(iv_block, block, 8);
            }
            idx = full_blocks * 8;
            if (_is_need_padding(pad_style)) 
            {
                unsigned char last_block[8] = {0};
                if (remain > 0)
                    _padding_block(last_block, 8, src + idx, remain, pad_style);
                else
                    _padding_block(last_block, 8, nullptr, 0, pad_style);

                DES_cblock block;
                memcpy(block, iv_block, 8);
                _ede_encrypt(key_schedules, block);
                for (int k = 0; k < 8; ++k)
                    dst[idx + k] = last_block[k] ^ block[k];

                idx += 8;
            } 
            else if (remain > 0) 
            {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                _ede_encrypt(key_schedules, block);
                for (int k = 0; k < remain; ++k)
                    dst[idx + k] = src[idx + k] ^ block[k];

                idx += remain;
            }
            dst_len = idx;
            break;
        }
        case mode::ctr: {
            if (!iv) return false;
            DES_cblock ctr_block;
            memcpy(ctr_block, iv, 8);
            idx = 0;
            std::size_t padded_len = src_len;
            if (_is_need_padding(pad_style)) 
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
                    _padding_block(block, 8, src + idx, remain_bytes, pad_style);
                } 
                else 
                {
                    _padding_block(block, 8, nullptr, 0, pad_style);
                }
                DES_cblock keystream;
                memcpy(keystream, ctr_block, 8);
                _ede_encrypt(key_schedules, keystream);
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

    // string -> encrypt string
    static bool encrypt(std::string& dst,
                        const std::string& src, 
                        const std::string& key,
                        const mode mod = mode::ecb,
                        const padding pad_style = padding::zero,
                        const std::string& iv = std::string())
    {
        std::size_t dst_len = encrypt_len_reserve(src.size());
        dst.resize(dst_len);
        const unsigned char* iv_ptr = (mod == mode::ecb) ? nullptr : reinterpret_cast<const unsigned char*>(iv.c_str());
        if (!encrypt(reinterpret_cast<unsigned char*>(const_cast<char*>(dst.data())),
                     dst_len,
                     reinterpret_cast<const unsigned char*>(src.c_str()),
                     src.size(),
                     reinterpret_cast<const unsigned char*>(key.c_str()),
                     key.size(),
                     mod,
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

    // stream -> encrypt stream
    static bool encrypt(std::ostream& out,
                        std::istream& in, 
                        const unsigned char* key, 
                        const std::size_t key_len,
                        const mode mod = mode::ecb,
                        const padding pad_style = padding::zero,
                        const unsigned char* iv = nullptr,
                        const std::size_t iv_len = 0)
    {
        if (!in || !out)
            return false;

        if (!is_key_valid(key, key_len) || !is_iv_valid(mod, iv, iv_len))
            return false;

        std::vector<DES_key_schedule> key_schedules;
        std::size_t n = _multiple_key(key_schedules, key, key_len);

        unsigned char inbuf[block_size] = {0};
        unsigned char outbuf[block_size] = {0};
        DES_cblock iv_block;
        if (mod != mode::ecb && iv && iv_len >= 8)
            memcpy(iv_block, iv, 8);

        DES_cblock ctr_block;
        if (mod == mode::ctr && iv && iv_len >= 8)
            memcpy(ctr_block, iv, 8);

        while (true) {
            in.read(reinterpret_cast<char*>(inbuf), block_size);
            std::streamsize read_len = in.gcount();

            if (read_len == 0)
                break;

            // check if this is the last block
            bool is_last_block = false;
            if (read_len < block_size) 
            {
                is_last_block = true;
            } 
            else 
            {
                // peek one byte to check if there's more data
                char peek_byte;
                if (!in.read(&peek_byte, 1))
                    is_last_block = true;
                else
                    in.seekg(-1, std::ios::cur);  // rewind
            }

            if (!is_last_block) 
            {
                _stream_block_encrypt(key_schedules, mod, out, iv_block, ctr_block, inbuf, outbuf);
                continue;
            }

            // handle the last block
            if (pad_style == padding::no_padding)
            {
                if (read_len != block_size)
                    return false;
                
                _stream_block_encrypt(key_schedules, mod, out, iv_block, ctr_block, inbuf, outbuf);
                return true;
            }

            // padding for the last block
            if (read_len == block_size) 
            {
                // handler current block
                _stream_block_encrypt(key_schedules, mod, out, iv_block, ctr_block, inbuf, outbuf);
                
                // padding block
                unsigned char padding_block[block_size];
                _padding_block(padding_block, block_size, nullptr, 0, pad_style);
                _stream_block_encrypt(key_schedules, mod, out, iv_block, ctr_block, padding_block, outbuf);
            } 
            else 
            {
                // padding for the last block
                unsigned char padded_block[block_size];
                _padding_block(padded_block, block_size, inbuf, read_len, pad_style);
                _stream_block_encrypt(key_schedules, mod, out, iv_block, ctr_block, padded_block, outbuf);
            }
            
            break;
        }

        return true;
    }

    // file -> encrypt file
    static bool encrypt_file(const char* dst_file_path,
                             const char* src_file_path,  
                             const unsigned char* key, 
                             const std::size_t key_len,
                             const mode mod = mode::ecb,
                             const padding pad_style = padding::zero,
                             const unsigned char* iv = nullptr,
                             const std::size_t iv_len = 0)
    {
        std::ifstream src_file(src_file_path, std::ios::binary);
        if (!src_file.is_open())
            return false;

        std::ofstream dst_file(dst_file_path, std::ios::binary);
        if (!dst_file.is_open())
            return false;

        return encrypt(dst_file, src_file, key, key_len, mod, pad_style, iv, iv_len);
    }

    // file -> encrypt file
    static bool encrypt_file(const std::string& dst_file_path,
                             const std::string& src_file_path,
                             const std::string& key,
                             const mode mod = mode::ecb,
                             const padding pad_style = padding::zero,
                             const std::string& iv = std::string())
    {
        const unsigned char* iv_ptr = (mod == mode::ecb) ? nullptr : reinterpret_cast<const unsigned char*>(iv.c_str());
        return encrypt_file(dst_file_path.c_str(),
                            src_file_path.c_str(),  
                            reinterpret_cast<const unsigned char*>(key.c_str()), 
                            key.size(),
                            mod,
                            pad_style,
                            iv_ptr,
                            iv.size());
    }

    // decrypt bytes -> bytes
    static bool decrypt(unsigned char* dst,
                        std::size_t& dst_len,
                        const unsigned char* src, 
                        const std::size_t src_len, 
                        const unsigned char* key, 
                        const std::size_t key_len, 
                        const mode mod = mode::ecb,
                        const padding pad_style = padding::zero,
                        const unsigned char* iv = nullptr,
                        const std::size_t iv_len = 0)
    {
        if (!is_key_valid(key, key_len) || !is_iv_valid(mod, iv, iv_len))
            return false;

        std::vector<DES_key_schedule> key_schedules;
        std::size_t n = _multiple_key(key_schedules, key, key_len);
        std::size_t idx = 0;
        dst_len = 0;
        switch (mod)
        {
        case mode::ecb: {
            for (; idx < src_len; idx += 8) 
            {
                DES_cblock block;
                memcpy(block, src + idx, 8);
                _ede_decrypt(key_schedules, block);
                memcpy(dst + idx, block, 8);
            }
            dst_len = idx;
            break;
        }
        case mode::cbc: {
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            for (; idx < src_len; idx += 8) 
            {
                DES_cblock block, tmp;
                memcpy(block, src + idx, 8);
                memcpy(tmp, block, 8);
                _ede_decrypt(key_schedules, block);
                for (int i = 0; i < 8; ++i)
                    block[i] ^= iv_block[i];

                memcpy(dst + idx, block, 8);
                memcpy(iv_block, tmp, 8);
            }
            dst_len = idx;
            break;
        }
        case mode::cfb: {
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            std::size_t full_blocks = src_len / 8;
            long remain = static_cast<long>(src_len % 8);
            for (std::size_t i = 0; i < full_blocks; ++i) 
            {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                _ede_encrypt(key_schedules, block); // same with encrypt
                for (int k = 0; k < 8; ++k)
                    dst[i * 8 + k] = src[i * 8 + k] ^ block[k];

                memcpy(iv_block, src + i * 8, 8);
            }
            idx = full_blocks * 8;
            if (remain > 0) 
            {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                _ede_encrypt(key_schedules, block); // same with encrypt
                for (int k = 0; k < remain; ++k)
                    dst[idx + k] = src[idx + k] ^ block[k];

                idx += remain;
            }
            dst_len = idx;
            break;
        }
        case mode::ofb: {
            DES_cblock iv_block;
            memcpy(iv_block, iv, 8);
            std::size_t full_blocks = src_len / 8;
            long remain = static_cast<long>(src_len % 8);
            for (std::size_t i = 0; i < full_blocks; ++i) 
            {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                _ede_encrypt(key_schedules, block); // same with encrypt
                for (int k = 0; k < 8; ++k)
                    dst[i * 8 + k] = src[i * 8 + k] ^ block[k];

                memcpy(iv_block, block, 8);
            }
            idx = full_blocks * 8;
            if (remain > 0) {
                DES_cblock block;
                memcpy(block, iv_block, 8);
                _ede_encrypt(key_schedules, block); // same with encrypt
                for (int k = 0; k < remain; ++k)
                    dst[idx + k] = src[idx + k] ^ block[k];

                idx += remain;
            }
            dst_len = idx;
            break;
        }
        case mode::ctr: {
            DES_cblock ctr_block;
            memcpy(ctr_block, iv, 8);
            idx = 0;
            while (idx < src_len) 
            {
                DES_cblock keystream;
                memcpy(keystream, ctr_block, 8);
                _ede_encrypt(key_schedules, keystream); // same with encrypt
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

        _unpadding_block(dst, dst_len, pad_style);
        return dst_len > 0;
    }

    // decrypt string -> string
    static bool decrypt(std::string& dst,
                        const std::string& src, 
                        const std::string& key,
                        const mode mod = mode::ecb,
                        const padding pad_style = padding::zero,
                        const std::string& iv = std::string())
    {
        dst.resize(decrypt_len_reserve(src.size()));
        std::size_t dst_len = dst.size();
        const unsigned char* iv_ptr = (mod == mode::ecb) ? nullptr : reinterpret_cast<const unsigned char*>(iv.c_str());
        if (!decrypt(reinterpret_cast<unsigned char*>(const_cast<char*>(dst.data())),
                     dst_len,
                     reinterpret_cast<const unsigned char*>(src.c_str()),
                     src.size(),
                     reinterpret_cast<const unsigned char*>(key.c_str()),
                     key.size(),
                     mod,
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

    // decrypt stream -> stream
    static bool decrypt(std::ostream& out,
                        std::istream& in, 
                        const unsigned char* key, 
                        const std::size_t key_len,
                        const mode mod = mode::ecb,
                        const padding pad_style = padding::zero,
                        const unsigned char* iv = nullptr,
                        const std::size_t iv_len = 0)
    {
        if (!in || !out)
            return false;

        if (!is_key_valid(key, key_len) || !is_iv_valid(mod, iv, iv_len))
            return false;

        std::vector<DES_key_schedule> key_schedules;
        std::size_t n = _multiple_key(key_schedules, key, key_len);

        unsigned char inbuf[block_size] = {0};
        unsigned char outbuf[block_size] = {0};
        unsigned char prev_outbuf[block_size] = {0};
        
        DES_cblock iv_block;
        if (mod != mode::ecb && iv && iv_len >= 8)
            memcpy(iv_block, iv, 8);

        DES_cblock ctr_block;
        if (mod == mode::ctr && iv && iv_len >= 8)
            memcpy(ctr_block, iv, 8);

        bool has_previous_block = false;
        bool is_first_block = true;

        while (in) 
        {
            in.read(reinterpret_cast<char*>(inbuf), block_size);
            std::streamsize read_len = in.gcount();
            
            if (read_len == 0)
                break;

            if (read_len != block_size)
                return false;

            // decrypt the current block
            switch (mod) {
                case mode::ecb: {
                    DES_cblock block;
                    memcpy(block, inbuf, block_size);
                    _ede_decrypt(key_schedules, block);
                    memcpy(outbuf, block, block_size);
                    break;
                }
                case mode::cbc: {
                    DES_cblock block, tmp;
                    memcpy(block, inbuf, block_size);
                    memcpy(tmp, block, block_size);
                    _ede_decrypt(key_schedules, block);
                    for (int i = 0; i < block_size; ++i)
                        block[i] ^= iv_block[i];
                    memcpy(outbuf, block, block_size);
                    memcpy(iv_block, tmp, block_size);
                    break;
                }
                case mode::cfb: {
                    DES_cblock block;
                    memcpy(block, iv_block, block_size);
                    _ede_encrypt(key_schedules, block);
                    for (int k = 0; k < block_size; ++k)
                        outbuf[k] = inbuf[k] ^ block[k];
                    memcpy(iv_block, inbuf, block_size);
                    break;
                }
                case mode::ofb: {
                    DES_cblock block;
                    memcpy(block, iv_block, block_size);
                    _ede_encrypt(key_schedules, block);
                    for (int k = 0; k < block_size; ++k)
                        outbuf[k] = inbuf[k] ^ block[k];
                    memcpy(iv_block, block, block_size);
                    break;
                }
                case mode::ctr: {
                    DES_cblock keystream;
                    memcpy(keystream, ctr_block, block_size);
                    _ede_encrypt(key_schedules, keystream);
                    for (std::size_t i = 0; i < block_size; ++i)
                        outbuf[i] = inbuf[i] ^ keystream[i];
                    for (int i = block_size - 1; i >= 0; --i)
                        if (++ctr_block[i] != 0) { break; }

                    break;
                }
                default: return false; // unsupported algorithm
            }

            // check if this is the last block
            char peek_byte;
            bool has_next_block = false;
            if (in.read(&peek_byte, 1)) 
            {
                in.seekg(-1, std::ios::cur);  // rewind
                has_next_block = true;
            }

            // If there is a previous block, we can safely output it now (since it's not the last block)
            if (has_previous_block)
                out.write(reinterpret_cast<char*>(prev_outbuf), block_size);

            // If this is the last block, we need to perform unpadding
            if (!has_next_block) 
            {
                std::size_t final_len = block_size;
                if (pad_style != padding::no_padding) 
                    _unpadding_block(outbuf, final_len, pad_style);

                out.write(reinterpret_cast<char*>(outbuf), final_len);
                break;
            }

            // save the current block as the previous block for the next iteration
            memcpy(prev_outbuf, outbuf, block_size);
            has_previous_block = true;
            is_first_block = false;
        }

        return true;
    }

    // decrypt file -> file
    static bool decrypt_file(const char* dst_file_path,
                             const char* src_file_path,  
                             const unsigned char* key, 
                             const std::size_t key_len,
                             const mode mod = mode::ecb,
                             const padding pad_style = padding::zero,
                             const unsigned char* iv = nullptr,
                             const std::size_t iv_len = 0)
    {
        std::ifstream src_file(src_file_path, std::ios::binary);
        if (!src_file.is_open())
            return false;

        std::ofstream dst_file(dst_file_path, std::ios::binary);
        if (!dst_file.is_open())
            return false;

        return decrypt(dst_file, src_file, key, key_len, mod, pad_style, iv, iv_len);
    }

    // decrypt file -> file
    static bool decrypt_file(const std::string& dst_file_path,
                             const std::string& src_file_path,
                             const std::string& key,
                             const mode mod = mode::ecb,
                             const padding pad_style = padding::zero,
                             const std::string& iv = std::string())
    {
        const unsigned char* iv_ptr = (mod == mode::ecb) ? nullptr : reinterpret_cast<const unsigned char*>(iv.c_str());
        return decrypt_file(dst_file_path.c_str(),
                            src_file_path.c_str(),  
                            reinterpret_cast<const unsigned char*>(key.c_str()), 
                            key.size(),
                            mod,
                            pad_style,
                            iv_ptr,
                            iv.size());
    }

    // reserve encrypt dst buf size
	static std::size_t encrypt_len_reserve(const std::size_t src_len)
	{
        return (src_len / 8) * 8 + 8;
	}

	// reserve decrypt dst buf size
	static std::size_t decrypt_len_reserve(const std::size_t src_len)
	{
		return (src_len / 8) * 8 + 8;
	}

    static bool is_key_valid(const unsigned char* key, const std::size_t key_len)
    {
        if (key == nullptr || key_len < 8 || key_len % 8 != 0)
            return false;

        return true;
    }

    static bool is_iv_valid(const mode mod, const unsigned char* iv, const std::size_t iv_len)
    {
        // iv is required for CBC, CFB, OFB, and CTR modes
        if (mod != mode::ecb && (iv == nullptr || iv_len != 8))
            return false;

        return true;
    }

    static bool is_plain_valid(const unsigned char* src, 
                               const std::size_t src_len, 
                               const padding pad_style)
    {
        if (src == nullptr || src_len == 0)
            return false;

        if (pad_style == padding::no_padding && src_len % 8 != 0)
            return false;

        return true;
    }

    static bool is_plain_valid(std::istream& in, 
                               const padding pad_style)
    {
        if (pad_style == padding::no_padding) 
        {
            std::streampos current_pos = in.tellg();
            in.seekg(0, std::ios::end);
            std::streampos end_pos = in.tellg();
            in.seekg(current_pos);
            std::size_t total_length = static_cast<std::size_t>(end_pos - current_pos);
            if (total_length == 0 || total_length % 8 != 0) 
                return false;
        }

        return true;
    }

private:
    // padding block
    static void _padding_block(unsigned char* dst, 
                               const std::size_t dst_len,
                               const unsigned char* src, 
                               const std::size_t src_len,
                               const padding pad_style)
    {
        memset(dst, 0, dst_len);
        if (src && src_len > 0)
            memcpy(dst, src, std::min(src_len, dst_len));

        unsigned char pad_val = static_cast<unsigned char>(dst_len - src_len);
        switch (pad_style) {
        case padding::pkcs5:
        case padding::pkcs7: {
            for (std::size_t i = src_len; i < dst_len; ++i)
                dst[i] = pad_val;
            break;
        }
        case padding::iso10126: {
            for (std::size_t i = src_len; i < dst_len - 1; ++i)
                dst[i] = static_cast<unsigned char>(rand() % 256);

            dst[dst_len - 1] = pad_val;
            break;
        }
        case padding::ansix923: {
            dst[dst_len - 1] = pad_val;
            break;
        }
        case padding::iso_iec_7816_4: {
            if (src_len < dst_len) 
                dst[src_len] = 0x80;
            break;
        }
        case padding::zero:
        case padding::no_padding:
            break;
        default:
            break;
        }
    }

    // unpadding block
    static void _unpadding_block(unsigned char* buf, std::size_t& len, const padding pad_style)
    {
        switch (pad_style) {
        case padding::pkcs5:
        case padding::pkcs7: {
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
        case padding::zero: {
            while (len > 0 && buf[len - 1] == 0)
                --len;

            return;
        }
        case padding::iso10126: {
            if (len == 0) 
                return;
        
            unsigned char pad = buf[len - 1];
            if (pad == 0 || pad > 8) 
                return;

            len = len - pad;
            return;
        }
        case padding::ansix923: {
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
        case padding::iso_iec_7816_4: {
            int i = static_cast<int>(len - 1);
            while (i >= 0 && buf[i] == 0) 
                --i;

            if (i >= 0 && buf[i] == 0x80)
                len = static_cast<std::size_t>(i);
            
            return;
        }
        case padding::no_padding:
            return;
        default:
            return;
        }
    }

    static bool _is_need_padding(const padding pad_style)
    {
        switch (pad_style)
        {
        case padding::pkcs5:
        case padding::pkcs7:
        case padding::iso10126:
        case padding::ansix923:
        case padding::iso_iec_7816_4:
        case padding::zero:
            return true;
        case padding::no_padding:
            return false;
        default:
            return false;
        }
    }

    static std::size_t _multiple_key(std::vector<DES_key_schedule>& key_schedules, 
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

    static void _ede_encrypt(std::vector<DES_key_schedule>& key_schedules, DES_cblock& block)
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

    static void _ede_decrypt(std::vector<DES_key_schedule>& key_schedules, DES_cblock& block)
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

    static void _stream_block_encrypt(std::vector<DES_key_schedule>& key_schedules,
                                     const mode mod,
                                     std::ostream& out,
                                     unsigned char* iv_block,
                                     unsigned char* ctr_block,
                                     unsigned char* inbuf,
                                     unsigned char* outbuf)
    {
        switch (mod) {
            case mode::ecb: {
                DES_cblock block;
                memcpy(block, inbuf, block_size);
                _ede_encrypt(key_schedules, block);
                out.write(reinterpret_cast<const char*>(block), block_size);
                break;
            }
            case mode::cbc: {
                DES_cblock block;
                memcpy(block, inbuf, block_size);
                for (int i = 0; i < block_size; ++i)
                    block[i] ^= iv_block[i];

                _ede_encrypt(key_schedules, block);
                out.write(reinterpret_cast<const char*>(block), block_size);
                memcpy(iv_block, block, block_size);
                break;
            }
            case mode::cfb: {
                DES_cblock block;
                memcpy(block, iv_block, block_size);
                _ede_encrypt(key_schedules, block);
                for (int k = 0; k < block_size; ++k)
                    outbuf[k] = inbuf[k] ^ block[k];

                out.write(reinterpret_cast<const char*>(outbuf), block_size);
                memcpy(iv_block, outbuf, block_size);
                break;
            }
            case mode::ofb: {
                DES_cblock block;
                memcpy(block, iv_block, block_size);
                _ede_encrypt(key_schedules, block);
                for (int k = 0; k < block_size; ++k)
                    outbuf[k] = inbuf[k] ^ block[k];

                out.write(reinterpret_cast<const char*>(outbuf), block_size);
                memcpy(iv_block, block, block_size);
                break;
            }
            case mode::ctr: {
                DES_cblock keystream;
                memcpy(keystream, ctr_block, block_size);
                _ede_encrypt(key_schedules, keystream);
                for (std::size_t i = 0; i < block_size; ++i)
                    outbuf[i] = inbuf[i] ^ keystream[i];

                out.write(reinterpret_cast<const char*>(outbuf), block_size);
                for (int i = block_size - 1; i >= 0; --i)
                    if (++ctr_block[i] != 0) break;

                break;
            }
            default: break;
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