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

#ifndef UTF8_HPP
#define UTF8_HPP

#include <codecvt>
#include <iostream>
#include <locale>
#include <string>

#ifndef UTF8_BUF_SZ
#define UTF8_BUF_SZ 4096
#endif

namespace libcpp
{

namespace utf8
{

bool is_valid (const std::string &str)
{
    int c, i, ix, n, j;
    for (i = 0, ix = str.length (); i < ix; i++) {
        c = (unsigned char) str[i];
        if (0x00 <= c && c <= 0x7F)
            n = 0;
        else if ((c & 0xE0) == 0xC0)
            n = 1;
        else if ((c & 0xF0) == 0xE0)
            n = 2;
        else if ((c & 0xF8) == 0xF0)
            n = 3;
        else
            return false;
        for (j = 0; j < n && i < ix; j++)
            if ((++i == ix) || ((str[i] & 0xC0) != 0x80))
                return false;
    }
    return true;
}

std::ostream &decode (std::ostream &out, std::wistream &in)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
    wchar_t wc;
    while (in.get (wc)) {
        try {
            std::string utf8 = converter.to_bytes (wc);
            out << utf8;
        }
        catch (...) {
            out << "?"; // bad char
        }
    }

    out.flush ();
    return out;
}

std::string decode (const std::wstring &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t> > cvt;
    return cvt.to_bytes (str);
}

unsigned char *decode (unsigned char *out, const wchar_t *in)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
    std::wstring wstr (in);
    std::string str = converter.to_bytes (wstr);
    memcpy (out, str.data (), str.size ());
    out[str.size ()] = '\0'; // null-terminate
    return out;
}

std::wostream &encode (std::wostream &out, std::istream &in)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
    std::string buffer;
    char temp[UTF8_BUF_SZ];
    while (in.read (temp, UTF8_BUF_SZ) || in.gcount () > 0) {
        buffer.assign (temp, static_cast<size_t> (in.gcount ()));
        try {
            std::wstring wstr = converter.from_bytes (buffer);
            out.write (wstr.data (), wstr.size ());
        }
        catch (...) {
            wchar_t bad = L'?';
            out.write (&bad, 1);
        }
    }
    out.flush ();
    return out;
}

std::wstring encode (const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t> > cvt;
    return cvt.from_bytes (str);
}

wchar_t *encode (wchar_t *out, const unsigned char *in)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
    std::string str (reinterpret_cast<const char *> (in));
    std::wstring wstr = converter.from_bytes (str);
    memcpy (out, wstr.data (), wstr.size () * sizeof (wchar_t));
    out[wstr.size ()] = L'\0'; // null-terminate
    return out;
}

inline std::ios_base &cvt (std::ios_base &ios)
{
    try {
        ios.imbue (
          std::locale (std::locale (), new std::codecvt_utf8<wchar_t>));
    }
    catch (...) {
        throw "not support this stream type";
    }

    return ios;
}

} // namespace utf8
} // namespace libcpp

#endif