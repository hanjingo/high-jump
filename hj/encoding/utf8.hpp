/*
 * This file is part of high-jump(hj).
 *
 * Copyright 2025-2026 hanjingo <hehehunanchina@live.com>
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

#ifndef HJ_ENCODING_UTF8_HPP
#define HJ_ENCODING_UTF8_HPP

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>

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

// -----------------------------------------------------------------------------
// Core Bitwise Unicode Transcoding Algorithms (No Locale, No OS APIs)
// -----------------------------------------------------------------------------

// Decode next UTF-8 sequence into a Unicode code point.
// Returns {code_point, bytes_read, success}
inline std::tuple<char32_t, size_t, bool>
decode_utf8_cp(std::string_view sv) noexcept
{
    if(sv.empty())
        return {0, 0, false};

    const auto *ptr = reinterpret_cast<const uint8_t *>(sv.data());
    size_t      len = sv.size();
    uint8_t     c0  = ptr[0];

    // 1-byte ASCII
    if(c0 <= 0x7F)
    {
        return {static_cast<char32_t>(c0), 1, true};
    }

    // 2-byte sequence
    if((c0 & 0xE0) == 0xC0)
    {
        if(len < 2 || (ptr[1] & 0xC0) != 0x80)
            return {0, 0, false};
        uint32_t cp = ((c0 & 0x1F) << 6) | (ptr[1] & 0x3F);
        if(cp < 0x80)
            return {0, 0, false}; // Overlong check
        return {static_cast<char32_t>(cp), 2, true};
    }

    // 3-byte sequence
    if((c0 & 0xF0) == 0xE0)
    {
        if(len < 3 || (ptr[1] & 0xC0) != 0x80 || (ptr[2] & 0xC0) != 0x80)
            return {0, 0, false};
        uint32_t cp =
            ((c0 & 0x0F) << 12) | ((ptr[1] & 0x3F) << 6) | (ptr[2] & 0x3F);
        if(cp < 0x800)
            return {0, 0, false}; // Overlong check
        if(cp >= 0xD800 && cp <= 0xDFFF)
            return {0, 0, false}; // Surrogate check
        return {static_cast<char32_t>(cp), 3, true};
    }

    // 4-byte sequence
    if((c0 & 0xF8) == 0xF0)
    {
        if(len < 4 || (ptr[1] & 0xC0) != 0x80 || (ptr[2] & 0xC0) != 0x80
           || (ptr[3] & 0xC0) != 0x80)
            return {0, 0, false};
        uint32_t cp = ((c0 & 0x07) << 18) | ((ptr[1] & 0x3F) << 12)
                      | ((ptr[2] & 0x3F) << 6) | (ptr[3] & 0x3F);
        if(cp < 0x10000 || cp > 0x10FFFF)
            return {0, 0, false}; // Range check
        return {static_cast<char32_t>(cp), 4, true};
    }

    return {0, 0, false};
}

// Encode a Unicode code point to UTF-8
// Writes bytes to 'out', returns written byte count (0 on error)
inline size_t encode_utf8_cp(char32_t cp, char *out) noexcept
{
    if(cp <= 0x7F)
    {
        out[0] = static_cast<char>(cp);
        return 1;
    }
    if(cp <= 0x7FF)
    {
        out[0] = static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
        out[1] = static_cast<char>(0x80 | (cp & 0x3F));
        return 2;
    }
    if(cp <= 0xFFFF)
    {
        if(cp >= 0xD800 && cp <= 0xDFFF)
            return 0; // Invalid surrogate
        out[0] = static_cast<char>(0xE0 | ((cp >> 12) & 0x0F));
        out[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out[2] = static_cast<char>(0x80 | (cp & 0x3F));
        return 3;
    }
    if(cp <= 0x10FFFF)
    {
        out[0] = static_cast<char>(0xF0 | ((cp >> 18) & 0x07));
        out[1] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        out[2] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out[3] = static_cast<char>(0x80 | (cp & 0x3F));
        return 4;
    }
    return 0;
}

// Read next Unicode code point from wchar_t (Handles UTF-16 surrogates or UTF-32)
inline std::tuple<char32_t, size_t, bool>
decode_wchar_cp(std::wstring_view wsv) noexcept
{
    if(wsv.empty())
        return {0, 0, false};

    if constexpr(sizeof(wchar_t) == 2) // Windows UTF-16 wchar_t
    {
        uint16_t w1 = static_cast<uint16_t>(wsv[0]);
        if(w1 >= 0xD800 && w1 <= 0xDBFF) // High surrogate
        {
            if(wsv.size() < 2)
                return {0, 0, false};
            uint16_t w2 = static_cast<uint16_t>(wsv[1]);
            if(w2 >= 0xDC00 && w2 <= 0xDFFF) // Low surrogate
            {
                char32_t cp = 0x10000 + (((w1 & 0x3FF) << 10) | (w2 & 0x3FF));
                return {cp, 2, true};
            }
            return {0, 0, false};
        }
        if(w1 >= 0xDC00 && w1 <= 0xDFFF)
            return {0, 0, false}; // Unmatched low
        return {static_cast<char32_t>(w1), 1, true};
    } else // POSIX 32-bit wchar_t (UTF-32)
    {
        char32_t cp = static_cast<char32_t>(wsv[0]);
        if(cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
            return {0, 0, false};
        return {cp, 1, true};
    }
}

// Encode Unicode code point to wchar_t array
inline size_t encode_wchar_cp(char32_t cp, wchar_t *out) noexcept
{
    if constexpr(sizeof(wchar_t) == 2) // Windows UTF-16
    {
        if(cp <= 0xFFFF)
        {
            if(cp >= 0xD800 && cp <= 0xDFFF)
                return 0;
            out[0] = static_cast<wchar_t>(cp);
            return 1;
        }
        if(cp <= 0x10FFFF)
        {
            cp -= 0x10000;
            out[0] = static_cast<wchar_t>(0xD800 | ((cp >> 10) & 0x3FF));
            out[1] = static_cast<wchar_t>(0xDC00 | (cp & 0x3FF));
            return 2;
        }
        return 0;
    } else // POSIX UTF-32
    {
        if(cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
            return 0;
        out[0] = static_cast<wchar_t>(cp);
        return 1;
    }
}

} // namespace detail

// -----------------------------------------------------------------------------
// Validation API
// -----------------------------------------------------------------------------

inline bool is_valid(std::string_view str) noexcept
{
    size_t idx = 0;
    while(idx < str.size())
    {
        auto [cp, read_bytes, ok] = detail::decode_utf8_cp(str.substr(idx));
        if(!ok)
            return false;
        idx += read_bytes;
    }
    return true;
}

// -----------------------------------------------------------------------------
// Decode APIs (std::wstring / wchar_t -> UTF-8 std::string / char array)
// -----------------------------------------------------------------------------

inline std::string decode(std::wstring_view wstr, error_code &ec) noexcept
{
    ec = error_code::ok;
    if(wstr.empty())
        return std::string();

    try
    {
        std::string res;
        res.reserve(wstr.size() * 3 / 2); // Intelligent pre-allocation

        size_t idx = 0;
        char   tmp[4];
        while(idx < wstr.size())
        {
            auto [cp, consumed, ok] = detail::decode_wchar_cp(wstr.substr(idx));
            if(!ok)
            {
                ec = error_code::conversion_failed;
                return std::string();
            }
            size_t bytes = detail::encode_utf8_cp(cp, tmp);
            if(bytes == 0)
            {
                ec = error_code::conversion_failed;
                return std::string();
            }
            res.append(tmp, bytes);
            idx += consumed;
        }
        return res;
    }
    catch(const std::bad_alloc &)
    {
        ec = error_code::conversion_failed;
        return std::string();
    }
}

inline std::string decode(std::wstring_view wstr) noexcept
{
    error_code ec = error_code::ok;
    return decode(wstr, ec);
}

// Zero-allocation Raw Buffer Decode API
inline unsigned char *decode(unsigned char *out,
                             size_t        &out_len,
                             const wchar_t *in,
                             error_code    &ec) noexcept
{
    ec = error_code::ok;
    if(!out || out_len == 0 || !in)
    {
        ec = error_code::invalid_input;
        return nullptr;
    }

    std::wstring_view wsv(in);
    size_t            idx      = 0;
    size_t            written  = 0;
    char             *char_out = reinterpret_cast<char *>(out);

    while(idx < wsv.size())
    {
        auto [cp, consumed, ok] = detail::decode_wchar_cp(wsv.substr(idx));
        if(!ok)
        {
            ec = error_code::conversion_failed;
            return nullptr;
        }

        char   tmp[4];
        size_t bytes = detail::encode_utf8_cp(cp, tmp);
        if(bytes == 0)
        {
            ec = error_code::conversion_failed;
            return nullptr;
        }

        if(written + bytes + 1
           > out_len) // Check buffer space (+1 for null terminator)
        {
            ec = error_code::buffer_overflow;
            return nullptr;
        }

        std::memcpy(char_out + written, tmp, bytes);
        written += bytes;
        idx += consumed;
    }

    char_out[written] = '\0';
    out_len           = written + 1; // Return total used bytes including '\0'
    return out;
}

inline unsigned char *
decode(unsigned char *out, size_t &out_len, const wchar_t *in) noexcept
{
    error_code ec = error_code::ok;
    return decode(out, out_len, in, ec);
}

// Stream Decode (wistream -> ostream)
inline std::ostream &decode(std::ostream &out, std::wistream &in)
{
    if(!out || !in)
        return out;

    wchar_t wc_buf[UTF8_BUF_SZ];
    char    utf8_buf[4];

    while(in.read(wc_buf, UTF8_BUF_SZ) || in.gcount() > 0)
    {
        std::wstring_view chunk(wc_buf, static_cast<size_t>(in.gcount()));
        size_t            idx = 0;

        while(idx < chunk.size())
        {
            auto [cp, consumed, ok] =
                detail::decode_wchar_cp(chunk.substr(idx));
            if(!ok)
            {
                out.put('?');
                idx += 1;
                continue;
            }
            size_t bytes = detail::encode_utf8_cp(cp, utf8_buf);
            if(bytes > 0)
            {
                out.write(utf8_buf, bytes);
            } else
            {
                out.put('?');
            }
            idx += consumed;
        }
    }
    out.flush();
    return out;
}

// -----------------------------------------------------------------------------
// Encode APIs (UTF-8 std::string_view / char array -> std::wstring / wchar_t)
// -----------------------------------------------------------------------------

inline std::wstring encode(std::string_view str, error_code &ec) noexcept
{
    ec = error_code::ok;
    if(str.empty())
        return std::wstring();

    try
    {
        std::wstring res;
        res.reserve(str.size());

        size_t  idx = 0;
        wchar_t tmp[2];
        while(idx < str.size())
        {
            auto [cp, consumed, ok] = detail::decode_utf8_cp(str.substr(idx));
            if(!ok)
            {
                ec = error_code::conversion_failed;
                return std::wstring();
            }
            size_t wchars = detail::encode_wchar_cp(cp, tmp);
            if(wchars == 0)
            {
                ec = error_code::conversion_failed;
                return std::wstring();
            }
            res.append(tmp, wchars);
            idx += consumed;
        }
        return res;
    }
    catch(const std::bad_alloc &)
    {
        ec = error_code::conversion_failed;
        return std::wstring();
    }
}

inline std::wstring encode(std::string_view str) noexcept
{
    error_code ec = error_code::ok;
    return encode(str, ec);
}

// Zero-allocation Raw Buffer Encode API
inline wchar_t *encode(wchar_t             *out,
                       size_t              &out_len,
                       const unsigned char *in,
                       error_code          &ec) noexcept
{
    ec = error_code::ok;
    if(!out || out_len == 0 || !in)
    {
        ec = error_code::invalid_input;
        return nullptr;
    }

    std::string_view sv(reinterpret_cast<const char *>(in));
    size_t           idx     = 0;
    size_t           written = 0;

    while(idx < sv.size())
    {
        auto [cp, consumed, ok] = detail::decode_utf8_cp(sv.substr(idx));
        if(!ok)
        {
            ec = error_code::conversion_failed;
            return nullptr;
        }

        wchar_t tmp[2];
        size_t  wchars = detail::encode_wchar_cp(cp, tmp);
        if(wchars == 0)
        {
            ec = error_code::conversion_failed;
            return nullptr;
        }

        if(written + wchars + 1 > out_len)
        {
            ec = error_code::buffer_overflow;
            return nullptr;
        }

        std::memcpy(out + written, tmp, wchars * sizeof(wchar_t));
        written += wchars;
        idx += consumed;
    }

    out[written] = L'\0';
    out_len      = written + 1;
    return out;
}

inline wchar_t *
encode(wchar_t *out, size_t &out_len, const unsigned char *in) noexcept
{
    error_code ec = error_code::ok;
    return encode(out, out_len, in, ec);
}

// Stream Encode (istream -> wostream) with Chunk-boundary Safe State
inline std::wostream &encode(std::wostream &out, std::istream &in)
{
    if(!out || !in)
        return out;

    char        chunk[UTF8_BUF_SZ];
    std::string pending_buf;
    wchar_t     wbuf[2];

    while(in.read(chunk, UTF8_BUF_SZ) || in.gcount() > 0)
    {
        pending_buf.append(chunk, static_cast<size_t>(in.gcount()));
        std::string_view sv(pending_buf);
        size_t           idx = 0;

        while(idx < sv.size())
        {
            auto [cp, consumed, ok] = detail::decode_utf8_cp(sv.substr(idx));
            if(!ok)
            {
                // If we hit the tail of the chunk, it might be an incomplete UTF-8 byte sequence
                if(sv.size() - idx < 4)
                {
                    break; // Save remaining bytes for next iteration
                }
                out.put(L'?');
                idx += 1;
                continue;
            }

            size_t wchars = detail::encode_wchar_cp(cp, wbuf);
            if(wchars > 0)
            {
                out.write(wbuf, wchars);
            } else
            {
                out.put(L'?');
            }
            idx += consumed;
        }

        pending_buf.erase(0, idx); // Retain incomplete bytes
    }

    // Flush leftover invalid trailing bytes if stream ends unexpectedly
    if(!pending_buf.empty())
    {
        out.put(L'?');
    }

    out.flush();
    return out;
}

} // namespace utf8
} // namespace hj

#endif // HJ_ENCODING_UTF8_HPP