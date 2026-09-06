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
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
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

template <typename T>
inline constexpr bool is_byte_v =
    std::is_same_v<T, char> || std::is_same_v<T, unsigned char>
    || std::is_same_v<T, signed char> || std::is_same_v<T, std::byte>
    || std::is_same_v<T, uint8_t> || std::is_same_v<T, int8_t>;

inline bool atomic_rename(const std::string &temp_path,
                          const std::string &target_path) noexcept
{
#if defined(_WIN32)
    std::wstring wtemp   = std::filesystem::path(temp_path).wstring();
    std::wstring wtarget = std::filesystem::path(target_path).wstring();

    return MoveFileExW(wtemp.c_str(),
                       wtarget.c_str(),
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)
           != 0;
#else
    std::error_code ec;
    std::filesystem::rename(temp_path, target_path, ec);
    return !ec;
#endif
}

} // namespace detail

class hex
{
  public:
    static constexpr std::size_t buf_sz = 4096;

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
        if(!in.good() || !in.rdbuf())
            return false;

        char        buf[buf_sz];
        std::size_t total_len = 0;
        const auto  start_pos = in.tellg();

        while(true)
        {
            std::streamsize n = in.rdbuf()->sgetn(buf, buf_sz);
            if(n <= 0)
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

        bool valid = (!in.bad()) && (total_len > 0) && (total_len % 2 == 0);

        in.clear();
        if(start_pos != std::streampos(-1))
            in.seekg(start_pos);

        return valid;
    }

    static bool is_valid_file(const std::string &file_path)
    {
        std::ifstream file(file_path, std::ios::binary);
        if(!file.is_open() || !file.good())
            return false;

        return is_valid(file);
    }

    static bool try_decode(std::string_view hex_str,
                           void            *out_buf,
                           std::size_t      out_len) noexcept
    {
        if(hex_str.empty() || hex_str.length() % 2 != 0
           || out_len < hex_str.length() / 2)
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
    static std::optional<T> try_decode(std::string_view hex_str) noexcept
    {
        if(hex_str.empty() || hex_str.length() % 2 != 0)
            return std::nullopt;

        if constexpr(std::is_integral_v<T>)
        {
            static_assert(std::is_unsigned_v<T>,
                          "Hex decode only supports unsigned integral types to "
                          "avoid sign extension ambiguity.");

            if(hex_str.length() > sizeof(T) * 2)
                return std::nullopt;

            T           val     = 0;
            constexpr T max_val = std::numeric_limits<T>::max();

            for(char c : hex_str)
            {
                int8_t v = detail::kHexValues[static_cast<uint8_t>(c)];
                if(v < 0)
                    return std::nullopt;

                if(val > (max_val >> 4))
                    return std::nullopt;

                val = static_cast<T>((val << 4) | static_cast<T>(v));
            }
            return val;
        } else
        {
            try
            {
                T           result;
                std::size_t out_len = hex_str.length() / 2;
                result.resize(out_len);
                if(!try_decode(hex_str, result.data(), out_len))
                    return std::nullopt;

                return result;
            }
            catch(const std::bad_alloc &)
            {
                return std::nullopt;
            }
        }
    }

    template <typename T = std::string>
    static T decode(std::string_view hex_str)
    {
        auto opt = try_decode<T>(hex_str);
        if(!opt.has_value())
            throw std::invalid_argument(
                "Invalid, empty, or overflowing hex string for decoding.");

        return *opt;
    }

    static bool decode(std::ostream &out, std::istream &in)
    {
        if(!out.good() || !in.good() || !in.rdbuf())
            return false;

        char        in_buf[buf_sz];
        uint8_t     out_buf[buf_sz / 2];
        std::size_t carryover     = 0;
        std::size_t total_decoded = 0;

        while(true)
        {
            const std::size_t capacity = buf_sz - carryover;

            std::streamsize read_bytes =
                in.rdbuf()->sgetn(in_buf + carryover,
                                  static_cast<std::streamsize>(capacity));

            if(read_bytes < 0)
                return false;

            const std::size_t total_bytes =
                carryover + static_cast<std::size_t>(read_bytes);

            if(read_bytes == 0)
            {
                if(carryover != 0)
                    return false;
                break;
            }

            const std::size_t process_bytes = total_bytes - (total_bytes % 2);

            if(process_bytes > 0)
            {
                std::string_view chunk(in_buf, process_bytes);
                if(!try_decode(chunk, out_buf, process_bytes / 2))
                    return false;

                out.write(reinterpret_cast<char *>(out_buf),
                          static_cast<std::streamsize>(process_bytes / 2));

                if(!out)
                    return false;

                total_decoded += process_bytes / 2;
            }

            carryover = total_bytes - process_bytes;
            if(carryover > 0)
            {
                in_buf[0] = in_buf[process_bytes];
            }
        }

        return !in.bad() && carryover == 0 && total_decoded > 0 && !out.fail();
    }

    static bool decode_file(const std::string &out_path,
                            const std::string &in_path)
    {
        std::ifstream fin(in_path, std::ios::binary);
        if(!fin.is_open() || !fin.good())
            return false;

        static std::atomic<uint64_t> seq_counter{0};
        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        std::string temp_path = out_path + ".tmp." + std::to_string(now) + "."
                                + std::to_string(seq_counter.fetch_add(1));

        bool success = false;
        {
            std::ofstream fout(temp_path, std::ios::binary);
            if(!fout.is_open() || !fout.good())
                return false;

            success = decode(fout, fin);
            fout.flush();
        }

        if(success)
        {
            if(detail::atomic_rename(temp_path, out_path))
                return true;
        }

        std::error_code ec;
        std::filesystem::remove(temp_path, ec);
        return false;
    }

    static bool try_encode(char       *out_buf,
                           std::size_t out_len,
                           const void *data,
                           std::size_t len,
                           bool        upper_case = true) noexcept
    {
        constexpr std::size_t max_bytes =
            std::numeric_limits<std::size_t>::max() / 2;
        if(len > max_bytes)
            return false;

        if(!data || !out_buf || out_len < len * 2)
            return false;

        const auto *byte_ptr = static_cast<const uint8_t *>(data);
        const char *table =
            upper_case ? detail::kHexCharsUpper : detail::kHexCharsLower;

        char *out_ptr = out_buf;
        for(std::size_t i = 0; i < len; ++i)
        {
            uint8_t byte = byte_ptr[i];
            *out_ptr++   = table[byte >> 4];
            *out_ptr++   = table[byte & 0x0F];
        }
        return true;
    }

    static std::string
    encode(const void *data, std::size_t len, bool upper_case)
    {
        if(!data || len == 0)
            return {};

        constexpr std::size_t max_bytes =
            std::numeric_limits<std::size_t>::max() / 2;
        if(len > max_bytes)
            throw std::overflow_error(
                "Input size exceeds maximum encodable hex limit.");

        std::string result;
        result.resize(len * 2);
        try_encode(result.data(), result.length(), data, len, upper_case);
        return result;
    }

    static std::string encode(const char *str, bool upper_case = true)
    {
        return encode(std::string_view(str ? str : ""), upper_case);
    }

    static std::string encode(std::string_view sv, bool upper_case = true)
    {
        return encode(static_cast<const void *>(sv.data()),
                      sv.length(),
                      upper_case);
    }

    template <typename T,
              typename = std::enable_if_t<
                  !std::is_pointer_v<std::decay_t<T>>
                  && !std::is_array_v<std::remove_reference_t<T>>
                  && !std::is_convertible_v<T, std::string_view>>>
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
            return encode(static_cast<const void *>(bytes),
                          valid_bytes,
                          upper_case);
        } else
        {
            using ElementType = typename T::value_type;
            static_assert(detail::is_byte_v<ElementType>,
                          "Hex encode container fallback only supports "
                          "byte-like elements "
                          "(e.g., uint8_t, char, std::byte).");

            return encode(static_cast<const void *>(std::data(value)),
                          std::size(value),
                          upper_case);
        }
    }

    static bool
    encode(std::ostream &out, std::istream &in, bool upper_case = true)
    {
        if(!out.good() || !in.good() || !in.rdbuf())
            return false;

        char in_buf[buf_sz];

        while(true)
        {
            std::streamsize n = in.rdbuf()->sgetn(in_buf, buf_sz);

            if(n < 0)
                return false;

            if(n == 0)
                break;

            std::string encoded_chunk =
                encode(static_cast<const void *>(in_buf),
                       static_cast<std::size_t>(n),
                       upper_case);

            out.write(encoded_chunk.data(),
                      static_cast<std::streamsize>(encoded_chunk.length()));

            if(!out)
                return false;
        }

        return !in.bad() && !out.fail();
    }

    static bool encode_file(const std::string &out_path,
                            const std::string &in_path,
                            bool               upper_case = true)
    {
        std::ifstream fin(in_path, std::ios::binary);
        if(!fin.is_open() || !fin.good())
            return false;

        static std::atomic<uint64_t> seq_counter{0};
        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        std::string temp_path = out_path + ".tmp." + std::to_string(now) + "."
                                + std::to_string(seq_counter.fetch_add(1));

        bool success = false;
        {
            std::ofstream fout(temp_path, std::ios::binary);
            if(!fout.is_open() || !fout.good())
                return false;

            success = encode(fout, fin, upper_case);
            fout.flush();
        }

        if(success)
        {
            if(detail::atomic_rename(temp_path, out_path))
                return true;
        }

        std::error_code ec;
        std::filesystem::remove(temp_path, ec);
        return false;
    }

  private:
    hex()                       = delete;
    ~hex()                      = delete;
    hex(const hex &)            = delete;
    hex &operator=(hex &&)      = delete;
    hex(hex &&)                 = delete;
    hex &operator=(const hex &) = delete;
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
        auto opt = hex::try_decode<T>(hex_str);
        if(opt.has_value())
        {
            wrapper.target = std::move(*opt);
        } else
        {
            is.setstate(std::ios::failbit);
        }
    }
    return is;
}

} // namespace hj

#endif // HEX_HPP