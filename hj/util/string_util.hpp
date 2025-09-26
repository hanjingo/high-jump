/*
 *  This file is part of hj.
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

#ifndef STRING_UTIL_HPP
#define STRING_UTIL_HPP

#include <string>
#include <regex>
#include <vector>
#include <sstream>
#include <locale>
#include <codecvt>

namespace hj
{

namespace string_util
{

inline bool contains(const std::string& src, const std::string& sub)
{
    return src.find(sub) != std::string::npos;
}

inline bool end_with(const std::string& src, const std::string& suffix)
{
    if (src.length() >= suffix.length())
        return (0 == src.compare(src.length() - suffix.length(), suffix.length(), suffix));
    else
        return false;
}

inline std::string search(const std::string& src, const std::string& regex)
{
    std::smatch match;
    std::regex_search(src, match, std::regex(regex));
    return match[0];
}

inline std::vector<std::string> search_n(const std::string& src, const std::string& regex)
{
    std::regex pattern(regex);
    std::sregex_iterator begin(src.begin(), src.end(), pattern), end;
    std::vector<std::string> results;
    for (auto it = begin; it != end; ++it) 
        results.push_back(it->str());

    return results;
}

inline void search(const std::string& src, std::smatch& match, const std::string& regex)
{
    std::regex_search(src, match, std::regex(regex));
}

inline std::vector<std::string> split(const std::string& str, const std::string& regex)
{
    std::regex patten(regex);
    std::sregex_token_iterator first{str.begin(), str.end(), patten, -1}, last;
    return {first, last};
}

inline std::string& replace(std::string& str, const std::string& from, const std::string& to)
{
    str = std::regex_replace(str, std::regex(from), to);
    return str;
}

inline bool equal(const char* a, const char* b)
{
    return strcmp(a, b) == 0;
}

inline std::wstring to_wchar(const std::string& src)
{
#if defined(_MSC_VER)
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.from_bytes(src);
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(src);
#endif
}

inline std::wstring to_wstring(const std::string& src)
{
#if defined(_MSC_VER)
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.from_bytes(src);
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(src);
#endif
}

inline std::string from_wchar(const wchar_t* src)
{
    if (!src) 
        return std::string();

#if defined(_MSC_VER)
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(src);
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(src);
#endif
}

inline std::string from_wstring(const std::wstring& src)
{
#if defined(_MSC_VER)
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(src);
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(src);
#endif
}

inline std::string from_ptr_addr(const void* ptr, bool is_hex = true)
{
    std::ostringstream oss;
    if (is_hex)
        oss << std::hex << reinterpret_cast<std::uintptr_t>(ptr);
    else
        oss << reinterpret_cast<std::uintptr_t>(ptr);
    return oss.str();
}

} // namespace string_util
} // namespace hj

#endif // STRING_UTIL_HPP