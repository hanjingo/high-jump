/*
 * This file is part of high-jump(hj).
 *
 * Copyright 2025 hanjingo <hehehunanchina@live.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HEX_HPP
#define HEX_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifndef HEX_BUF_SZ
#define HEX_BUF_SZ 4096
#endif

namespace hj
{

namespace detail
{

constexpr std::array<int8_t, 256> make_hex_table() noexcept
{
    std::array<int8_t, 256> table{};
    for(std::size_t i = 0; i < 256; ++i)
    {
        table[i] = -1;
    }
    for(int i = 0; i <= 9; ++i)
    {
        table[static_cast<std::size_t>('0' + i)] = static_cast<int8_t>(i);
    }
    for(int i = 0; i <= 5; ++i)
    {
        table[static_cast<std::size_t>('A' + i)] = static_cast<int8_t>(10 + i);
        table[static_cast<std::size_t>('a' + i)] = static_cast<int8_t>(10 + i);
    }
    return table;
}

inline constexpr std::array<int8_t, 256> kHexValues = make_hex_table();

inline constexpr char kHexCharsUpper[] = "0123456789ABCDEF";
inline constexpr char kHexCharsLower[] = "0123456789abcdef";

} // namespace detail

class hex
{
  public:
    template <typename T>
    struct encoder_wrapper
    {
        const T &value;
        bool     upper_case;

        encoder_wrapper(const T &val, bool upper = true)
            : value(val)
            , upper_case(upper)
        {
        }
    };

    template <typename T>
    struct decoder_wrapper
    {
        T &target;

        explicit decoder_wrapper(T &t)
            : target(t)
        {
        }
    };

    template <typename T>
    static encoder_wrapper<T> encoder(const T &value, bool upper_case = true)
    {
        return encoder_wrapper<T>(value, upper_case);
    }

    template <typename T>
    static decoder_wrapper<T> decoder(T &target)
    {
        return decoder_wrapper<T>(target);
    }

    static bool is_valid(const unsigned char *buf, std::size_t len) noexcept
    {
        if(!buf || len == 0 || len % 2 != 0)
            return false;

        for(std::size_t i = 0; i < len; ++i)
        {
            if(detail::kHexValues[buf[i]] < 0)
                return false;
        }

        return true;
    }

    static bool is_valid(std::string_view hex_str) noexcept
    {
        return is_valid(reinterpret_cast<const unsigned char *>(hex_str.data()),
                        hex_str.length());
    }

    static bool is_valid(std::istream &in)
    {
        if(!in.good())
            return false;

        char        buf[HEX_BUF_SZ];
        std::size_t total_len = 0;
        const auto  start_pos = in.tellg();
        while(in)
        {
            in.read(buf, HEX_BUF_SZ);
            std::streamsize n = in.gcount();
            if(n == 0)
                break;

            total_len += static_cast<std::size_t>(n);
            for(std::streamsize i = 0; i < n; ++i)
            {
                if(detail::kHexValues[static_cast<unsigned char>(buf[i])] >= 0)
                    continue;

                in.clear();
                if(start_pos != std::streampos(-1))
                    in.seekg(start_pos);

                return false;
            }
        }

        in.clear();
        if(start_pos != std::streampos(-1))
            in.seekg(start_pos);

        return total_len > 0 && (total_len % 2 == 0);
    }

    static bool is_valid_file(const std::string &file_path)
    {
        std::ifstream file(file_path, std::ios::binary);
        if(!file.is_open())
            return false;

        return is_valid(file);
    }

    static bool decode(std::string_view hex_str,
                       void            *out_buf,
                       std::size_t      out_len) noexcept
    {
        if(hex_str.length() % 2 != 0 || out_len < hex_str.length() / 2)
            return false;

        auto *out = static_cast<uint8_t *>(out_buf);
        for(std::size_t i = 0; i < hex_str.length(); i += 2)
        {
            int8_t high = detail::kHexValues[static_cast<uint8_t>(hex_str[i])];
            int8_t low =
                detail::kHexValues[static_cast<uint8_t>(hex_str[i + 1])];

            if(high < 0 || low < 0)
                return false;

            *out++ = static_cast<uint8_t>((high << 4) | low);
        }
        return true;
    }

    template <typename T = std::string>
    static T decode(std::string_view hex_str)
    {
        T result{};
        if(hex_str.empty() || hex_str.length() % 2 != 0)
            return result;

        if constexpr(std::is_integral_v<T>)
        {
            T val = 0;
            for(char c : hex_str)
            {
                int8_t v = detail::kHexValues[static_cast<uint8_t>(c)];
                if(v < 0)
                    return T{};

                val = static_cast<T>((val << 4) | v);
            }
            return val;
        } else
        {
            std::size_t out_len = hex_str.length() / 2;
            result.resize(out_len);
            if(!decode(hex_str, result.data(), out_len))
                result.clear();

            return result;
        }
    }

    static bool decode(std::ostream &out, std::istream &in)
    {
        if(!out.good() || !in.good())
            return false;

        char        in_buf[HEX_BUF_SZ];
        uint8_t     out_buf[HEX_BUF_SZ / 2];
        std::size_t carryover = 0;
        while(in)
        {
            in.read(in_buf + carryover, HEX_BUF_SZ - carryover);
            std::streamsize read_bytes =
                in.gcount() + static_cast<std::streamsize>(carryover);
            if(read_bytes == 0)
                break;

            std::size_t process_bytes = static_cast<std::size_t>(read_bytes);
            if(process_bytes % 2 != 0)
            {
                carryover = 1;
                process_bytes -= 1;
            } else
            {
                carryover = 0;
            }

            if(process_bytes > 0)
            {
                std::string_view chunk(in_buf, process_bytes);
                if(!decode(chunk, out_buf, process_bytes / 2))
                    return false;

                out.write(reinterpret_cast<char *>(out_buf), process_bytes / 2);
            }

            if(carryover > 0)
                in_buf[0] = in_buf[process_bytes];
        }

        return carryover == 0;
    }

    static bool decode_file(const std::string &out_path,
                            const std::string &in_path)
    {
        std::ifstream fin(in_path, std::ios::binary);
        std::ofstream fout(out_path, std::ios::binary);
        return decode(fout, fin);
    }

    static std::string
    encode(const void *data, std::size_t len, bool upper_case = true)
    {
        if(!data || len == 0)
            return {};

        const auto *byte_ptr = static_cast<const uint8_t *>(data);
        const char *table =
            upper_case ? detail::kHexCharsUpper : detail::kHexCharsLower;

        std::string result;
        result.resize(len * 2);
        char *out_ptr = &result[0];
        for(std::size_t i = 0; i < len; ++i)
        {
            uint8_t byte = byte_ptr[i];
            *out_ptr++   = table[byte >> 4];
            *out_ptr++   = table[byte & 0x0F];
        }
        return result;
    }

    template <typename T>
    static std::string encode(const T &value, bool upper_case = true)
    {
        if constexpr(std::is_integral_v<T>)
        {
            using UnsignedT = std::make_unsigned_t<T>;
            UnsignedT val   = static_cast<UnsignedT>(value);

            uint8_t     bytes[sizeof(T)];
            std::size_t valid_bytes = 0;
            for(int i = static_cast<int>(sizeof(T)) - 1; i >= 0; --i)
            {
                uint8_t b = static_cast<uint8_t>((val >> (i * 8)) & 0xFF);
                if(b != 0 || valid_bytes > 0 || i == 0)
                    bytes[valid_bytes++] = b;
            }
            return encode(bytes, valid_bytes, upper_case);
        } else if constexpr(std::is_convertible_v<T, std::string_view>)
        {
            std::string_view sv(value);
            return encode(sv.data(), sv.length(), upper_case);
        } else
        {
            return encode(std::data(value),
                          std::size(value) * sizeof(typename T::value_type),
                          upper_case);
        }
    }

    static bool
    encode(std::ostream &out, std::istream &in, bool upper_case = true)
    {
        if(!out.good() || !in.good())
            return false;

        char in_buf[HEX_BUF_SZ];
        while(in)
        {
            in.read(in_buf, HEX_BUF_SZ);
            std::streamsize n = in.gcount();
            if(n == 0)
                break;

            std::string encoded_chunk =
                encode(in_buf, static_cast<std::size_t>(n), upper_case);
            out.write(encoded_chunk.data(), encoded_chunk.length());
        }

        return true;
    }

    static bool encode_file(const std::string &out_path,
                            const std::string &in_path,
                            bool               upper_case = true)
    {
        std::ifstream fin(in_path, std::ios::binary);
        std::ofstream fout(out_path, std::ios::binary);
        return encode(fout, fin, upper_case);
    }

  private:
    hex()                       = delete;
    ~hex()                      = delete;
    hex(const hex &)            = delete;
    hex &operator=(const hex &) = delete;
    hex(hex &&)                 = delete;
    hex &operator=(hex &&)      = delete;
};

template <typename T>
inline std::ostream &operator<<(std::ostream                  &os,
                                const hex::encoder_wrapper<T> &wrapper)
{
    os << hex::encode(wrapper.value, wrapper.upper_case);
    return os;
}

template <typename T>
inline std::istream &operator>>(std::istream           &is,
                                hex::decoder_wrapper<T> wrapper)
{
    std::string hex_str;
    if(is >> hex_str)
    {
        wrapper.target = hex::decode<T>(hex_str);
        if(wrapper.target.empty() && !hex_str.empty())
            is.setstate(std::ios::failbit);
    }
    return is;
}

} // namespace hj

#endif // HEX_HPP