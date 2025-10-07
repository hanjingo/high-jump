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

#ifndef UNICODE_HPP
#define UNICODE_HPP

#include <string>
#include <locale>
#include <stdexcept>

#if (__cplusplus >= 201703L)
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif
#else
#include <codecvt>
#endif

#if (__cplusplus < 201703L) || defined(HJ_FORCE_CODECVT)
#include <codecvt>
#endif

namespace hj
{
namespace unicode
{

inline std::wstring from_utf8(const std::string &str)
{
#if (__cplusplus >= 201703L)
#if defined(_WIN32) || defined(_WIN64)
    if(str.empty())
        return L"";
    int len = MultiByteToWideChar(CP_UTF8,
                                  0,
                                  str.data(),
                                  (int) str.size(),
                                  nullptr,
                                  0);
    if(len <= 0)
        throw std::runtime_error("utf8->wstring failed");
    std::wstring wstr(len, 0);
    MultiByteToWideChar(CP_UTF8,
                        0,
                        str.data(),
                        (int) str.size(),
                        &wstr[0],
                        len);
    return wstr;
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    return cvt.from_bytes(str);
#endif

#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    return cvt.from_bytes(str);

#endif
}

inline std::string to_utf8(const std::wstring &wstr)
{
#if (__cplusplus >= 201703L)
#if defined(_WIN32) || defined(_WIN64)
    if(wstr.empty())
        return "";
    int len = WideCharToMultiByte(CP_UTF8,
                                  0,
                                  wstr.data(),
                                  (int) wstr.size(),
                                  nullptr,
                                  0,
                                  nullptr,
                                  nullptr);
    if(len <= 0)
        throw std::runtime_error("wstring->utf8 failed");
    std::string str(len, 0);
    WideCharToMultiByte(CP_UTF8,
                        0,
                        wstr.data(),
                        (int) wstr.size(),
                        &str[0],
                        len,
                        nullptr,
                        nullptr);
    return str;
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    return cvt.to_bytes(wstr);
#endif

#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    return cvt.to_bytes(wstr);

#endif
}

} // namespace unicode
} // namespace hj

#endif