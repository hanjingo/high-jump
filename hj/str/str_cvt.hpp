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

#ifndef STR_CVT_HPP
#define STR_CVT_HPP

#include <algorithm>
#include <climits>
#include <cinttypes>
#include <cstdio>
#include <optional>
#include <string>
#include <string_view>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace hj::str::cvt_detail
{

#ifdef _WIN32
inline std::optional<std::wstring>
utf8_to_wide_impl(std::string_view utf8_str) noexcept
{
    if(utf8_str.empty())
        return std::wstring{};

    if(utf8_str.size() > static_cast<size_t>(INT_MAX))
        return std::nullopt;

    const int size = MultiByteToWideChar(CP_UTF8,
                                         MB_ERR_INVALID_CHARS,
                                         utf8_str.data(),
                                         (int) utf8_str.size(),
                                         nullptr,
                                         0);
    if(size <= 0)
        return std::nullopt;

    std::wstring result(size, 0);
    return MultiByteToWideChar(CP_UTF8,
                               MB_ERR_INVALID_CHARS,
                               utf8_str.data(),
                               (int) utf8_str.size(),
                               result.data(),
                               size)
                   > 0
               ? std::optional<std::wstring>(std::move(result))
               : std::nullopt;
}

inline std::optional<std::string>
wide_to_utf8_impl(std::wstring_view wide_str) noexcept
{
    if(wide_str.empty())
        return std::string{};

    if(wide_str.size() > static_cast<size_t>(INT_MAX))
        return std::nullopt;

    const int size = WideCharToMultiByte(CP_UTF8,
                                         WC_ERR_INVALID_CHARS,
                                         wide_str.data(),
                                         (int) wide_str.size(),
                                         nullptr,
                                         0,
                                         nullptr,
                                         nullptr);
    if(size <= 0)
        return std::nullopt;

    std::string result(size, 0);
    return WideCharToMultiByte(CP_UTF8,
                               WC_ERR_INVALID_CHARS,
                               wide_str.data(),
                               (int) wide_str.size(),
                               result.data(),
                               size,
                               nullptr,
                               nullptr)
                   > 0
               ? std::optional<std::string>(std::move(result))
               : std::nullopt;
}
#else
inline std::optional<std::wstring>
utf8_to_wide_impl(std::string_view utf8_str) noexcept
{
    if(utf8_str.empty())
        return std::wstring{};

    std::wstring result;
    result.reserve(utf8_str.size());
    const auto *ptr = reinterpret_cast<const unsigned char *>(utf8_str.data());
    const auto *end = ptr + utf8_str.size();
    while(ptr < end)
    {
        unsigned char c         = *ptr;
        uint32_t      codepoint = 0;
        size_t        remaining = 0;
        if(c < 0x80)
        {
            codepoint = c;
            remaining = 0;
            ptr += 1;
        } else if((c & 0xE0) == 0xC0)
        {
            codepoint = c & 0x1F;
            remaining = 1;
            ptr += 1;
        } else if((c & 0xF0) == 0xE0)
        {
            codepoint = c & 0x0F;
            remaining = 2;
            ptr += 1;
        } else if((c & 0xF8) == 0xF0)
        {
            codepoint = c & 0x07;
            remaining = 3;
            ptr += 1;
        } else
        {
            return std::nullopt;
        }

        if(static_cast<size_t>(end - ptr) < remaining)
            return std::nullopt;

        for(size_t i = 0; i < remaining; ++i)
        {
            unsigned char next_c = *ptr++;
            if((next_c & 0xC0) != 0x80)
                return std::nullopt;

            codepoint = (codepoint << 6) | (next_c & 0x3F);
        }

        if(codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF))
            return std::nullopt;
        if(remaining == 1 && codepoint < 0x80)
            return std::nullopt;
        if(remaining == 2 && codepoint < 0x800)
            return std::nullopt;
        if(remaining == 3 && codepoint < 0x10000)
            return std::nullopt;

        result.push_back(static_cast<wchar_t>(codepoint));
    }

    return result;
}

inline std::optional<std::string>
wide_to_utf8_impl(std::wstring_view wide_str) noexcept
{
    if(wide_str.empty())
        return std::string{};

    std::string result;
    result.reserve(wide_str.size() * 3);
    for(wchar_t wc : wide_str)
    {
        uint32_t cp = static_cast<uint32_t>(wc);
        if(cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
            return std::nullopt;

        if(cp < 0x80)
        {
            result.push_back(static_cast<char>(cp));
        } else if(cp < 0x800)
        {
            result.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if(cp < 0x10000)
        {
            result.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else
        {
            result.push_back(static_cast<char>(0xF0 | (cp >> 18)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }

    return result;
}
#endif

inline std::string from_ptr_addr_impl(const void *ptr,
                                      bool        is_hex = true) noexcept
{
    if(!ptr)
        return is_hex ? "0x0" : "0";

    char       buffer[32];
    const auto addr = reinterpret_cast<std::uintptr_t>(ptr);
    is_hex ? std::snprintf(buffer, sizeof(buffer), "0x%" PRIxPTR, addr)
           : std::snprintf(buffer, sizeof(buffer), "%" PRIuPTR, addr);
    return std::string{buffer};
}

} // namespace hj::str::cvt_detail


// --------------------------- string convert API ---------------------------
namespace hj::str
{
inline std::optional<std::wstring> try_to_wstring(std::string_view src) noexcept
{
    return cvt_detail::utf8_to_wide_impl(src);
}

inline std::optional<std::string>
try_from_wstring(std::wstring_view src) noexcept
{
    return cvt_detail::wide_to_utf8_impl(src);
}

inline std::optional<std::string>
try_from_wchar_opt(const wchar_t *src) noexcept
{
    if(!src)
        return std::nullopt;
    return cvt_detail::wide_to_utf8_impl(std::wstring_view(src));
}

inline std::wstring to_wstring(std::string_view src) noexcept
{
    auto res = cvt_detail::utf8_to_wide_impl(src);
    return res ? std::move(*res) : std::wstring{};
}

inline std::string from_wstring(std::wstring_view src) noexcept
{
    auto res = cvt_detail::wide_to_utf8_impl(src);
    return res ? std::move(*res) : std::string{};
}

inline std::string ascii_lower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A'))
                                      : static_cast<char>(c);
    });
    return str;
}

inline std::string ascii_upper(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return (c >= 'a' && c <= 'z') ? static_cast<char>(c - ('a' - 'A'))
                                      : static_cast<char>(c);
    });
    return str;
}

inline std::string from_ptr_addr(const void *ptr, bool is_hex = true) noexcept
{
    return cvt_detail::from_ptr_addr_impl(ptr, is_hex);
}

} // namespace hj::str

#endif // STR_CVT_HPP