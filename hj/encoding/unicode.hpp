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

#ifndef UNICODE_HPP
#define UNICODE_HPP

#include <string>
#include <locale>
#include <stdexcept>

#if (__cplusplus >= 201703L)
#if defined(_WIN32) || defined(_WIN64)
#ifndef NOMINMAX
#define NOMINMAX
#endif
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
    if(str.empty())
        return L"";
    std::mbstate_t state = std::mbstate_t();
    const char    *src   = str.data();
    size_t         len   = std::mbsrtowcs(nullptr, &src, 0, &state);
    if(len == static_cast<size_t>(-1))
        throw std::runtime_error("utf8->wstring failed");
    std::wstring wstr(len, 0);
    src = str.data();
    std::mbsrtowcs(&wstr[0], &src, len, &state);
    return wstr;
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
    if(wstr.empty())
        return "";
    std::mbstate_t state = std::mbstate_t();
    const wchar_t *src   = wstr.data();
    size_t         len   = std::wcsrtombs(nullptr, &src, 0, &state);
    if(len == static_cast<size_t>(-1))
        throw std::runtime_error("wstring->utf8 failed");
    std::string str(len, 0);
    src = wstr.data();
    std::wcsrtombs(&str[0], &src, len, &state);
    return str;
#endif

#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    return cvt.to_bytes(wstr);

#endif
}

} // namespace unicode
} // namespace hj

#endif