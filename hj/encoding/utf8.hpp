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

#ifndef HJ_ENCODING_UTF8_HPP
#define HJ_ENCODING_UTF8_HPP

#include <string>
#include <locale>
#include <iostream>
#include <stdexcept>
#include <cstring>

#if (__cplusplus >= 201703L)
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif
#else
#include <codecvt>
#endif

// Always include codecvt for compatibility functions
#if (__cplusplus < 201703L) || defined(HJ_FORCE_CODECVT)
#include <codecvt>
#endif

#ifndef UTF8_BUF_SZ
#define UTF8_BUF_SZ 4096
#endif

namespace hj
{
namespace utf8
{

enum class error_code
{
    ok = 0,
    buffer_overflow,
    invalid_input,
    conversion_failed
};

namespace detail
{
#if (__cplusplus >= 201703L)

#if defined(_WIN32) || defined(_WIN64)
inline std::string _decode(const std::wstring &str, error_code &ec) noexcept
{
    int size = WideCharToMultiByte(CP_UTF8,
                                   0,
                                   str.c_str(),
                                   (int) str.size(),
                                   NULL,
                                   0,
                                   NULL,
                                   NULL);
    if(size <= 0)
    {
        ec = error_code::conversion_failed;
        return std::string();
    }

    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8,
                        0,
                        str.c_str(),
                        (int) str.size(),
                        &result[0],
                        size,
                        NULL,
                        NULL);
    return result;
}

inline std::string _decode(const std::wstring &str) noexcept
{
    error_code ec = error_code::ok;
    return _decode(str, ec);
}

inline std::ostream &_decode(std::ostream &out, std::wistream &in) noexcept
{
    wchar_t wc;
    while(in.get(wc))
    {
        std::wstring wstr(1, wc);
        error_code   ec   = error_code::ok;
        std::string  utf8 = _decode(wstr, ec);
        out << utf8;
    }
    out.flush();
    return out;
}

inline unsigned char *_decode(unsigned char *out,
                              size_t        &out_len,
                              const wchar_t *in,
                              error_code    &ec) noexcept
{
    std::wstring wstr(in);
    std::string  str = _decode(wstr, ec);
    if(ec != error_code::ok)
        return nullptr;

    if(str.size() + 1 > out_len)
    {
        ec = error_code::buffer_overflow;
        return nullptr;
    }

    memcpy(out, str.data(), str.size());
    out[str.size()] = '\0';
    out_len         = str.size() + 1;
    return out;
}

inline unsigned char *
_decode(unsigned char *out, size_t &out_len, const wchar_t *in) noexcept
{
    error_code ec = error_code::ok;
    return _decode(out, out_len, in, ec);
}

inline std::wstring _encode(const std::string &str, error_code &ec) noexcept
{
    int size =
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int) str.size(), NULL, 0);
    if(size <= 0)
    {
        ec = error_code::conversion_failed;
        return std::wstring();
    }

    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8,
                        0,
                        str.c_str(),
                        (int) str.size(),
                        &result[0],
                        size);
    return result;
}

inline std::wstring _encode(const std::string &str) noexcept
{
    error_code ec = error_code::ok;
    return _encode(str, ec);
}

inline std::wostream &_encode(std::wostream &out, std::istream &in) noexcept
{
    char temp[UTF8_BUF_SZ];
    while(in.read(temp, UTF8_BUF_SZ) || in.gcount() > 0)
    {
        std::string  buffer(temp, static_cast<size_t>(in.gcount()));
        error_code   ec   = error_code::ok;
        std::wstring wstr = _encode(buffer, ec);
        if(ec != error_code::ok)
            return out;

        out.write(wstr.data(), wstr.size());
    }
    out.flush();
    return out;
}

inline wchar_t *_encode(wchar_t             *out,
                        size_t              &out_len,
                        const unsigned char *in,
                        error_code          &ec) noexcept
{
    std::string  str(reinterpret_cast<const char *>(in));
    std::wstring wstr = _encode(str, ec);
    if(ec != error_code::ok)
        return nullptr;

    if(wstr.size() + 1 > out_len)
    {
        ec = error_code::buffer_overflow;
        return nullptr;
    }

    memcpy(out, wstr.data(), wstr.size() * sizeof(wchar_t));
    out[wstr.size()] = L'\0';
    out_len          = wstr.size() + 1;
    return out;
}

inline wchar_t *
_encode(wchar_t *out, size_t &out_len, const unsigned char *in) noexcept
{
    error_code ec = error_code::ok;
    return _encode(out, out_len, in, ec);
}

#else
// POSIX/Other platforms
inline std::string _decode(const std::wstring &str, error_code &ec) noexcept
{
    std::mbstate_t state = std::mbstate_t();
    const wchar_t *src   = str.data();
    size_t         len   = std::wcsrtombs(NULL, &src, 0, &state);
    if(len == static_cast<size_t>(-1))
    {
        ec = error_code::conversion_failed;
        return std::string();
    }

    std::string result(len, 0);
    src = str.data();
    std::wcsrtombs(&result[0], &src, len, &state);
    return result;
}

inline std::string _decode(const std::wstring &str) noexcept
{
    error_code ec = error_code::ok;
    return _decode(str, ec);
}

inline std::ostream &_decode(std::ostream &out, std::wistream &in) noexcept
{
    wchar_t wc;
    while(in.get(wc))
    {
        std::wstring wstr(1, wc);
        std::string  utf8 = _decode(wstr);
        out << utf8;
    }
    out.flush();
    return out;
}

inline unsigned char *_decode(unsigned char *out,
                              size_t        &out_len,
                              const wchar_t *in,
                              error_code    &ec) noexcept
{
    std::wstring wstr(in);
    std::string  str = _decode(wstr, ec);
    if(ec != error_code::ok)
        return nullptr;

    if(str.size() + 1 > out_len)
    {
        ec = error_code::buffer_overflow;
        return nullptr;
    }

    memcpy(out, str.data(), str.size());
    out[str.size()] = '\0';
    out_len         = str.size() + 1;
    return out;
}

inline unsigned char *
_decode(unsigned char *out, size_t &out_len, const wchar_t *in) noexcept
{
    error_code ec = error_code::ok;
    return _decode(out, out_len, in, ec);
}

inline std::wstring _encode(const std::string &str, error_code &ec) noexcept
{
    std::mbstate_t state = std::mbstate_t();
    const char    *src   = str.data();
    size_t         len   = std::mbsrtowcs(NULL, &src, 0, &state);
    if(len == static_cast<size_t>(-1))
    {
        ec = error_code::conversion_failed;
        return std::wstring();
    }

    std::wstring result(len, 0);
    src = str.data();
    std::mbsrtowcs(&result[0], &src, len, &state);
    return result;
}

inline std::wstring _encode(const std::string &str) noexcept
{
    error_code ec = error_code::ok;
    return _encode(str, ec);
}

inline std::wostream &_encode(std::wostream &out, std::istream &in) noexcept
{
    char temp[UTF8_BUF_SZ];
    while(in.read(temp, UTF8_BUF_SZ) || in.gcount() > 0)
    {
        std::string  buffer(temp, static_cast<size_t>(in.gcount()));
        error_code   ec   = error_code::ok;
        std::wstring wstr = _encode(buffer, ec);
        if(ec != error_code::ok)
            return out;

        out.write(wstr.data(), wstr.size());
    }
    out.flush();
    return out;
}

inline wchar_t *_encode(wchar_t             *out,
                        size_t              &out_len,
                        const unsigned char *in,
                        error_code          &ec) noexcept
{
    std::string  str(reinterpret_cast<const char *>(in));
    std::wstring wstr = _encode(str, ec);
    if(ec != error_code::ok)
        return nullptr;

    if(wstr.size() + 1 > out_len)
    {
        ec = error_code::buffer_overflow;
        return nullptr;
    }

    memcpy(out, wstr.data(), wstr.size() * sizeof(wchar_t));
    out[wstr.size()] = L'\0';
    out_len          = wstr.size() + 1;
    return out;
}

inline wchar_t *
_encode(wchar_t *out, size_t &out_len, const unsigned char *in) noexcept
{
    error_code ec = error_code::ok;
    return _encode(out, out_len, in, ec);
}

#endif

#endif

} // namespace detail

inline bool is_valid(const std::string &str)
{
    // UTF-8 validation, industrial version: checks for overlong, surrogate, and invalid code points
    size_t i = 0, len = str.size();
    while(i < len)
    {
        unsigned char c = static_cast<unsigned char>(str[i]);
        if(c <= 0x7F)
        {
            i++;
            continue;
        } else if((c & 0xE0) == 0xC0)
        {
            if(i + 1 >= len || (str[i + 1] & 0xC0) != 0x80)
                return false;

            // Overlong encoding check
            if((c & 0xFE) == 0xC0)
                return false;
            i += 2;
        } else if((c & 0xF0) == 0xE0)
        {
            if(i + 2 >= len || (str[i + 1] & 0xC0) != 0x80
               || (str[i + 2] & 0xC0) != 0x80)
                return false;

            // Surrogate check
            unsigned int cp = ((c & 0x0F) << 12) | ((str[i + 1] & 0x3F) << 6)
                              | (str[i + 2] & 0x3F);
            if(cp >= 0xD800 && cp <= 0xDFFF)
                return false;

            i += 3;
        } else if((c & 0xF8) == 0xF0)
        {
            if(i + 3 >= len || (str[i + 1] & 0xC0) != 0x80
               || (str[i + 2] & 0xC0) != 0x80 || (str[i + 3] & 0xC0) != 0x80)
                return false;

            unsigned int cp = ((c & 0x07) << 18) | ((str[i + 1] & 0x3F) << 12)
                              | ((str[i + 2] & 0x3F) << 6)
                              | (str[i + 3] & 0x3F);
            if(cp > 0x10FFFF)
                return false;

            i += 4;
        } else
        {
            return false;
        }
    }
    return true;
}

inline std::ostream &decode(std::ostream &out, std::wistream &in) noexcept
{
    if(!out || !in)
        return out;

#if (__cplusplus < 201703L)
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    wchar_t                                          wc;
    while(in.get(wc))
    {
        try
        {
            std::string utf8 = converter.to_bytes(wc);
            out << utf8;
        }
        catch(const std::exception &e)
        {
            out << "?"; // bad char
        }
    }
    out.flush();
    return out;
#else
    return detail::_decode(out, in);
#endif
}

inline std::string decode(const std::wstring &str, error_code &ec) noexcept
{
    if(str.empty())
    {
        ec = error_code::invalid_input;
        return std::string();
    }
#if (__cplusplus < 201703L)
    try
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
        return cvt.to_bytes(str);
    }
    catch(const std::exception &)
    {
        ec = error_code::conversion_failed;
        return std::string();
    }
#else
    return detail::_decode(str, ec);
#endif
}

inline std::string decode(const std::wstring &str) noexcept
{
    error_code ec = error_code::ok;
    return decode(str, ec);
}

inline unsigned char *decode(unsigned char *out,
                             size_t        &out_len,
                             const wchar_t *in,
                             error_code    &ec) noexcept
{
    if(out == nullptr || out_len < 1 || in == nullptr)
    {
        ec = error_code::invalid_input;
        return nullptr;
    }

#if (__cplusplus < 201703L)
    try
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::wstring                                     wstr(in);
        std::string str = converter.to_bytes(wstr);
        if(str.size() + 1 > out_len)
        {
            ec = error_code::buffer_overflow;
            return nullptr;
        }
        memcpy(out, str.data(), str.size());
        out[str.size()] = '\0'; // null-terminate
        out_len         = str.size() + 1;
        return out;
    }
    catch(const std::exception &)
    {
        ec = error_code::conversion_failed;
        return nullptr;
    }
#else
    return detail::_decode(out, out_len, in, ec);
#endif
}

inline std::wostream &encode(std::wostream &out, std::istream &in) noexcept
{
    if(!out || !in)
        return out;

#if (__cplusplus < 201703L)
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string                                      buffer;
    char                                             temp[UTF8_BUF_SZ];
    while(in.read(temp, UTF8_BUF_SZ) || in.gcount() > 0)
    {
        buffer.assign(temp, static_cast<size_t>(in.gcount()));
        try
        {
            std::wstring wstr = converter.from_bytes(buffer);
            out.write(wstr.data(), wstr.size());
        }
        catch(const std::exception &e)
        {
            wchar_t bad = L'?';
            out.write(&bad, 1);
            // Optionally log e.what()
        }
    }
    out.flush();
    return out;
#else
    return detail::_encode(out, in);
#endif
}

inline unsigned char *
decode(unsigned char *out, size_t &out_len, const wchar_t *in) noexcept
{
    error_code ec = error_code::ok;
    return decode(out, out_len, in, ec);
}

inline std::wstring encode(const std::string &str, error_code &ec) noexcept
{
    if(str.empty())
    {
        ec = error_code::invalid_input;
        return std::wstring();
    }
#if (__cplusplus < 201703L)
    try
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
        return cvt.from_bytes(str);
    }
    catch(const std::exception &)
    {
        ec = error_code::conversion_failed;
        return std::wstring();
    }
#else
    return detail::_encode(str, ec);
#endif
}

inline std::wstring encode(const std::string &str) noexcept
{
    error_code ec = error_code::ok;
    return encode(str, ec);
}

inline wchar_t *encode(wchar_t             *out,
                       size_t              &out_len,
                       const unsigned char *in,
                       error_code          &ec) noexcept
{
    if(out == nullptr || out_len < 1 || in == nullptr)
    {
        ec = error_code::invalid_input;
        return nullptr;
    }
#if (__cplusplus < 201703L)
    try
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::string  str(reinterpret_cast<const char *>(in));
        std::wstring wstr = converter.from_bytes(str);
        if(wstr.size() + 1 > out_len)
        {
            ec = error_code::buffer_overflow;
            return nullptr;
        }
        memcpy(out, wstr.data(), wstr.size() * sizeof(wchar_t));
        out[wstr.size()] = L'\0'; // null-terminate
        out_len          = wstr.size() + 1;
        return out;
    }
    catch(const std::exception &)
    {
        ec = error_code::conversion_failed;
        return nullptr;
    }
#else
    return detail::_encode(out, out_len, in, ec);
#endif
}

inline wchar_t *
encode(wchar_t *out, size_t &out_len, const unsigned char *in) noexcept
{
    error_code ec = error_code::ok;
    return encode(out, out_len, in, ec);
}

inline std::ios_base &cvt(std::ios_base &ios, error_code &ec) noexcept
{
    try
    {
#if (__cplusplus < 201703L)
        ios.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
#else
        // For C++17+, codecvt_utf8 is deprecated, use platform-specific methods
        ec = error_code::conversion_failed;
        return ios;
#endif
    }
    catch(...)
    {
        ec = error_code::conversion_failed;
        return ios;
    }

    return ios;
}

inline std::ios_base &cvt(std::ios_base &ios) noexcept
{
    error_code ec = error_code::ok;
    return cvt(ios, ec);
}

} // namespace utf8
} // namespace hj

#endif