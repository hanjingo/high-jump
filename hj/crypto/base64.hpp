/*
 *  This file is part of high-jump(hj).
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

#ifndef BASE64_HPP
#define BASE64_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace hj
{

class base64
{
  public:
    static constexpr std::size_t buf_sz = 16384; // 16KB

    enum class error_code
    {
        ok = 0,
        invalid_input,
        buffer_overflow,
        encode_failed,
        decode_failed,
        file_open_failed,
        file_read_failed,
        file_write_failed
    };

    [[nodiscard]] inline static const char *to_string(error_code r) noexcept
    {
        switch(r)
        {
            case error_code::ok:
                return "ok";
            case error_code::invalid_input:
                return "invalid input";
            case error_code::buffer_overflow:
                return "buffer overflow";
            case error_code::encode_failed:
                return "encode failed";
            case error_code::decode_failed:
                return "decode failed";
            case error_code::file_open_failed:
                return "file open failed";
            case error_code::file_read_failed:
                return "file read failed";
            case error_code::file_write_failed:
                return "file write failed";
        }
        return "unknown";
    }

  private:
    static inline constexpr char enc_table[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    static inline constexpr int8_t dec_table[256] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57,
        58, 59, 60, 61, -1, -1, -1, -2, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,
        7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
        25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
        37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1};

  public:
    [[nodiscard]] static constexpr std::size_t
    encode_len_reserve(const std::size_t src_len) noexcept
    {
        return src_len / 3 * 4 + (src_len % 3 == 0 ? 0 : 4);
    }

    [[nodiscard]] static constexpr std::size_t
    decode_len_reserve(const std::size_t src_len) noexcept
    {
        return (src_len / 4) * 3;
    }

    // bytes -> base64 bytes
    [[nodiscard]] static error_code encode(unsigned char       *dst,
                                           std::size_t         &dst_len,
                                           const unsigned char *src,
                                           const std::size_t    src_len)
    {
        if(src_len == 0)
        {
            dst_len = 0;
            return error_code::ok;
        }
        if(!dst || !src)
            return error_code::invalid_input;

        std::size_t req_len = encode_len_reserve(src_len);
        if(dst_len < req_len)
            return error_code::buffer_overflow;

        std::size_t i = 0;
        std::size_t j = 0;
        for(; i + 2 < src_len; i += 3)
        {
            uint32_t val = (static_cast<uint32_t>(src[i]) << 16)
                           | (static_cast<uint32_t>(src[i + 1]) << 8)
                           | static_cast<uint32_t>(src[i + 2]);
            dst[j++]     = enc_table[(val >> 18) & 0x3F];
            dst[j++]     = enc_table[(val >> 12) & 0x3F];
            dst[j++]     = enc_table[(val >> 6) & 0x3F];
            dst[j++]     = enc_table[val & 0x3F];
        }

        if(i < src_len)
        {
            uint32_t val = static_cast<uint32_t>(src[i]) << 16;
            if(i + 1 < src_len)
                val |= static_cast<uint32_t>(src[i + 1]) << 8;

            dst[j++] = enc_table[(val >> 18) & 0x3F];
            dst[j++] = enc_table[(val >> 12) & 0x3F];
            dst[j++] = (i + 1 < src_len) ? enc_table[(val >> 6) & 0x3F] : '=';
            dst[j++] = '=';
        }

        dst_len = j;
        return error_code::ok;
    }

    // string -> base64 string
    [[nodiscard]] static error_code encode(std::string       &dst,
                                           const std::string &src)
    {
        if(src.empty())
        {
            dst.clear();
            return error_code::ok;
        }

        std::size_t dst_len = encode_len_reserve(src.size());
        dst.resize(dst_len);
        auto ec = encode(reinterpret_cast<unsigned char *>(dst.data()),
                         dst_len,
                         reinterpret_cast<const unsigned char *>(src.data()),
                         src.size());
        if(ec != error_code::ok)
        {
            dst.clear();
            return ec;
        }

        dst.resize(dst_len);
        return error_code::ok;
    }

    // stream -> base64 stream
    [[nodiscard]] static error_code encode(std::ostream &out, std::istream &in)
    {
        if(!in || !out)
            return error_code::invalid_input;

        constexpr std::size_t      in_buf_size  = (buf_sz / 3) * 3;
        constexpr std::size_t      out_buf_size = (in_buf_size / 3) * 4;
        std::vector<unsigned char> in_buf(in_buf_size);
        std::vector<unsigned char> out_buf(out_buf_size);
        while(true)
        {
            in.read(reinterpret_cast<char *>(in_buf.data()), in_buf_size);
            std::streamsize n = in.gcount();

            if(n > 0)
            {
                std::size_t out_len = out_buf.size();
                auto        ec      = encode(out_buf.data(),
                                             out_len,
                                             in_buf.data(),
                                             static_cast<std::size_t>(n));
                if(ec != error_code::ok)
                    return ec;

                out.write(reinterpret_cast<const char *>(out_buf.data()),
                          out_len);
                if(!out)
                    return error_code::file_write_failed;
            }

            if(in.eof())
                break;

            if(in.fail())
                return error_code::file_read_failed;
        }

        return error_code::ok;
    }

    // file -> base64 file
    [[nodiscard]] static error_code encode_file(const char *dst_file_path,
                                                const char *src_file_path)
    {
        if(!dst_file_path || !src_file_path
           || std::string(dst_file_path) == std::string(src_file_path))
            return error_code::invalid_input;

        std::ifstream in(src_file_path, std::ios::binary);
        if(!in.is_open())
            return error_code::file_open_failed;

        std::ofstream out(dst_file_path, std::ios::binary);
        if(!out.is_open())
            return error_code::file_open_failed;

        return encode(out, in);
    }

    // file -> base64 file
    [[nodiscard]] static error_code
    encode_file(const std::string &dst_file_path,
                const std::string &src_file_path)
    {
        return encode_file(dst_file_path.c_str(), src_file_path.c_str());
    }

    // base64 bytes -> bytes
    [[nodiscard]] static error_code decode(unsigned char       *dst,
                                           std::size_t         &dst_len,
                                           const unsigned char *src,
                                           const std::size_t    src_len)
    {
        if(src_len == 0)
        {
            dst_len = 0;
            return error_code::ok;
        }
        if(!dst || !src || src_len % 4 != 0)
            return error_code::invalid_input;

        std::size_t req_len = decode_len_reserve(src_len);
        if(dst_len < req_len)
            return error_code::buffer_overflow;

        std::size_t out_idx = 0;
        for(std::size_t i = 0; i < src_len; i += 4)
        {
            int8_t b0 = dec_table[src[i]];
            int8_t b1 = dec_table[src[i + 1]];
            int8_t b2 = dec_table[src[i + 2]];
            int8_t b3 = dec_table[src[i + 3]];

            if(b0 < 0 || b1 < 0)
                return error_code::decode_failed;

            uint32_t val = (static_cast<uint32_t>(b0) << 18)
                           | (static_cast<uint32_t>(b1) << 12);

            if(b2 >= 0)
            {
                val |= (static_cast<uint32_t>(b2) << 6);
                if(b3 >= 0)
                {
                    val |= static_cast<uint32_t>(b3);
                    dst[out_idx++] = (val >> 16) & 0xFF;
                    dst[out_idx++] = (val >> 8) & 0xFF;
                    dst[out_idx++] = val & 0xFF;
                } else if(b3 == -2)
                {
                    if(i + 4 != src_len)
                        return error_code::decode_failed;

                    if((b2 & 0x03) != 0)
                        return error_code::decode_failed;

                    dst[out_idx++] = (val >> 16) & 0xFF;
                    dst[out_idx++] = (val >> 8) & 0xFF;
                } else
                {
                    return error_code::decode_failed;
                }
            } else if(b2 == -2)
            {
                if(b3 != -2 || i + 4 != src_len)
                    return error_code::decode_failed;

                if((b1 & 0x0F) != 0)
                    return error_code::decode_failed;

                dst[out_idx++] = (val >> 16) & 0xFF;
            } else
            {
                return error_code::decode_failed;
            }
        }

        dst_len = out_idx;
        return error_code::ok;
    }

    // base64 string -> string
    [[nodiscard]] static error_code decode(std::string       &dst,
                                           const std::string &src)
    {
        if(src.empty())
        {
            dst.clear();
            return error_code::ok;
        }

        if(src.size() % 4 != 0)
            return error_code::invalid_input;

        std::size_t dst_len = decode_len_reserve(src.size());
        dst.resize(dst_len);
        auto ec = decode(reinterpret_cast<unsigned char *>(dst.data()),
                         dst_len,
                         reinterpret_cast<const unsigned char *>(src.data()),
                         src.size());

        if(ec != error_code::ok)
        {
            dst.clear();
            return ec;
        }

        dst.resize(dst_len);
        return error_code::ok;
    }

    [[nodiscard]] static error_code decode(std::ostream &out, std::istream &in)
    {
        if(!in || !out)
            return error_code::invalid_input;

        constexpr std::size_t      in_buf_size  = (buf_sz / 4) * 4;
        constexpr std::size_t      out_buf_size = (in_buf_size / 4) * 3;
        std::vector<unsigned char> in_buf(in_buf_size);
        std::vector<unsigned char> out_buf(out_buf_size);
        std::size_t                leftover = 0;
        bool                       has_eof  = false;

        while(!has_eof || leftover > 0)
        {
            if(!has_eof)
            {
                in.read(reinterpret_cast<char *>(in_buf.data() + leftover),
                        in_buf_size - leftover);
                std::streamsize n = in.gcount();
                if(n > 0)
                    leftover += static_cast<std::size_t>(n);

                if(in.eof())
                    has_eof = true;
                else if(in.fail())
                    return error_code::file_read_failed;
            }

            if(leftover == 0)
                break;

            std::size_t decode_bytes = (leftover / 4) * 4;
            if(has_eof && leftover % 4 != 0)
                return error_code::invalid_input;

            if(decode_bytes > 0)
            {
                std::size_t out_len = out_buf.size();
                auto        ec      = decode(out_buf.data(),
                                             out_len,
                                             in_buf.data(),
                                             decode_bytes);
                if(ec != error_code::ok)
                    return ec;

                out.write(reinterpret_cast<const char *>(out_buf.data()),
                          out_len);
                if(!out)
                    return error_code::file_write_failed;

                leftover -= decode_bytes;
                if(leftover > 0)
                {
                    std::memmove(in_buf.data(),
                                 in_buf.data() + decode_bytes,
                                 leftover);
                }
            } else if(has_eof)
            {
                break;
            }
        }

        return error_code::ok;
    }

    // base64 file -> file
    [[nodiscard]] static error_code decode_file(const char *dst_file_path,
                                                const char *src_file_path)
    {
        if(!dst_file_path || !src_file_path
           || std::string(dst_file_path) == std::string(src_file_path))
            return error_code::invalid_input;

        std::ifstream in(src_file_path, std::ios::binary);
        if(!in.is_open())
            return error_code::file_open_failed;

        std::ofstream out(dst_file_path, std::ios::binary);
        if(!out.is_open())
            return error_code::file_open_failed;

        return decode(out, in);
    }

    // base64 file -> file
    [[nodiscard]] static error_code
    decode_file(const std::string &dst_file_path,
                const std::string &src_file_path)
    {
        return decode_file(dst_file_path.c_str(), src_file_path.c_str());
    }

    [[nodiscard]] static bool is_valid(const unsigned char *buf,
                                       const std::size_t    len)
    {
        if(len == 0 || buf == nullptr || len % 4 != 0)
            return false;

        std::size_t pad = 0;
        for(std::size_t i = 0; i < len; ++i)
        {
            int8_t val = dec_table[buf[i]];
            if(val >= 0)
            {
                if(pad > 0)
                    return false;
            } else if(val == -2)
            {
                ++pad;
                if(pad > 2)
                    return false;
            } else
            {
                return false;
            }
        }

        if(pad == 1)
        {
            int8_t b2 = dec_table[buf[len - 2]];
            if(b2 < 0 || (b2 & 0x03) != 0)
                return false;
        } else if(pad == 2)
        {
            int8_t b1 = dec_table[buf[len - 3]];
            if(b1 < 0 || (b1 & 0x0F) != 0)
                return false;
        }

        return true;
    }

    [[nodiscard]] static bool is_valid(const std::string &str)
    {
        return is_valid(reinterpret_cast<const unsigned char *>(str.data()),
                        str.length());
    }

    [[nodiscard]] static bool is_valid(std::ifstream &in)
    {
        if(!in.is_open())
            return false;

        std::ifstream::pos_type start_pos = in.tellg();
        if(start_pos == std::ifstream::pos_type(-1))
            return false;

        unsigned char buf[buf_sz];
        std::size_t   total_len = 0;
        std::size_t   pad_count = 0;

        unsigned char last_four[4] = {0};

        while(true)
        {
            in.read(reinterpret_cast<char *>(buf), buf_sz);
            std::streamsize n = in.gcount();

            if(n > 0)
            {
                for(std::streamsize i = 0; i < n; ++i)
                {
                    int8_t val = dec_table[buf[i]];
                    if(val >= 0)
                    {
                        if(pad_count > 0)
                        {
                            in.clear();
                            in.seekg(start_pos);
                            return false;
                        }
                    } else if(val == -2)
                    {
                        pad_count++;
                        if(pad_count > 2)
                        {
                            in.clear();
                            in.seekg(start_pos);
                            return false;
                        }
                    } else
                    {
                        in.clear();
                        in.seekg(start_pos);
                        return false;
                    }

                    last_four[0] = last_four[1];
                    last_four[1] = last_four[2];
                    last_four[2] = last_four[3];
                    last_four[3] = buf[i];

                    total_len++;
                }
            }

            if(in.eof())
                break;

            if(in.fail())
            {
                in.clear();
                in.seekg(start_pos);
                return false;
            }
        }

        in.clear();
        in.seekg(start_pos);

        if(total_len == 0 || total_len % 4 != 0)
            return false;

        if(pad_count == 1)
        {
            int8_t b2 = dec_table[last_four[2]];
            if(b2 < 0 || (b2 & 0x03) != 0)
                return false;
        } else if(pad_count == 2)
        {
            int8_t b1 = dec_table[last_four[1]];
            if(b1 < 0 || (b1 & 0x0F) != 0)
                return false;
        }

        return true;
    }

    [[nodiscard]] static bool is_valid_file(const std::string &file_path)
    {
        std::ifstream file(file_path, std::ios::binary);
        if(!file.is_open())
            return false;

        return is_valid(file);
    }

  private:
    base64()                          = delete;
    ~base64()                         = delete;
    base64(const base64 &)            = delete;
    base64 &operator=(const base64 &) = delete;
    base64(base64 &&)                 = delete;
    base64 &operator=(base64 &&)      = delete;
};

class base64_category_impl : public std::error_category
{
  public:
    [[nodiscard]] const char *name() const noexcept override
    {
        return "hj::base64";
    }

    [[nodiscard]] std::string message(int ev) const override
    {
        return base64::to_string(static_cast<base64::error_code>(ev));
    }
};

[[nodiscard]] inline const std::error_category &base64_category() noexcept
{
    static const base64_category_impl instance;
    return instance;
}

[[nodiscard]] inline std::error_code
make_error_code(base64::error_code e) noexcept
{
    return {static_cast<int>(e), base64_category()};
}

} // namespace hj

template <>
struct std::is_error_code_enum<hj::base64::error_code> : std::true_type
{
};

#endif