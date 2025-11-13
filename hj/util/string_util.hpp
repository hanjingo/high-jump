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

#ifndef STRING_UTIL_HPP
#define STRING_UTIL_HPP

#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <cstring>
#include <cstdint>
#include <cinttypes>
#include <cstdio>
#include <cctype>
#include <mutex>
#include <regex>

#ifdef _WIN32
#include <windows.h>

#else
#include <locale>
#include <cwchar>

#endif

#if !defined(_MSC_VER)
#include <cerrno>
// Provide a minimal, portable implementation of strncpy_s for non-MSVC
// environments. Many files in this project call strncpy_s(dest, destsz, src, count).
// This implementation copies at most `count` bytes from `src` into `dest`,
// ensures null-termination and never writes past `destsz` bytes.
static inline int strncpy_s(char *dest, size_t destsz, const char *src, size_t count)
{
    if(dest == nullptr || src == nullptr || destsz == 0)
        return EINVAL;

    // compute how many bytes are actually available to copy (reserve one for '\0')
    size_t max_copy = (destsz > 0) ? destsz - 1 : 0;

    // don't read beyond the actual src content
    size_t src_len = std::strlen(src);
    size_t to_copy = (count < src_len) ? count : src_len;
    if(to_copy > max_copy)
        to_copy = max_copy;

    if(to_copy > 0)
        std::memcpy(dest, src, to_copy);
    dest[to_copy] = '\0';

    return 0; // success
}
#endif

namespace hj
{
namespace detail
{
class regex_cache
{
  private:
    static constexpr size_t MAX_CACHE_SIZE = 32;
    struct cache_entry
    {
        std::string pattern;
        std::regex  compiled;
        size_t      access_count = 0;
    };

    mutable std::vector<cache_entry> _cache;
    mutable std::mutex               _mu;

  public:
    static inline regex_cache &instance()
    {
        static regex_cache inst;
        return inst;
    }

    inline const std::regex &get_regex(const std::string &pattern) const
    {
        std::lock_guard<std::mutex> lock(_mu);
        for(auto &entry : _cache)
        {
            if(entry.pattern == pattern)
            {
                ++entry.access_count;
                return entry.compiled;
            }
        }

        if(_cache.size() >= MAX_CACHE_SIZE)
        {
            auto min_it = std::min_element(
                _cache.begin(),
                _cache.end(),
                [](const cache_entry &a, const cache_entry &b) {
                    return a.access_count < b.access_count;
                });
            if(min_it != _cache.end())
                _cache.erase(min_it);
        }

        _cache.emplace_back(cache_entry{pattern, std::regex(pattern), 1});
        return _cache.back().compiled;
    }

    inline void clear() noexcept
    {
        std::lock_guard<std::mutex> lock(_mu);
        _cache.clear();
    }
};

// Platform-specific UTF-8 conversion implementation
#ifdef _WIN32
inline std::optional<std::wstring>
utf8_to_wide_impl(std::string_view utf8_str) noexcept
{
    if(utf8_str.empty())
        return std::wstring{};

    const int size_needed =
        MultiByteToWideChar(CP_UTF8,
                            0,
                            utf8_str.data(),
                            static_cast<int>(utf8_str.size()),
                            nullptr,
                            0);

    if(size_needed <= 0)
        return std::nullopt;

    std::wstring result(size_needed, 0);
    const int    converted = MultiByteToWideChar(CP_UTF8,
                                              0,
                                              utf8_str.data(),
                                              static_cast<int>(utf8_str.size()),
                                              result.data(),
                                              size_needed);

    return (converted > 0) ? std::optional<std::wstring>{std::move(result)}
                           : std::nullopt;
}

inline std::optional<std::string>
wide_to_utf8_impl(std::wstring_view wide_str) noexcept
{
    if(wide_str.empty())
        return std::string{};

    const int size_needed =
        WideCharToMultiByte(CP_UTF8,
                            0,
                            wide_str.data(),
                            static_cast<int>(wide_str.size()),
                            nullptr,
                            0,
                            nullptr,
                            nullptr);

    if(size_needed <= 0)
        return std::nullopt;

    std::string result(size_needed, 0);
    const int   converted = WideCharToMultiByte(CP_UTF8,
                                              0,
                                              wide_str.data(),
                                              static_cast<int>(wide_str.size()),
                                              result.data(),
                                              size_needed,
                                              nullptr,
                                              nullptr);

    return (converted > 0) ? std::optional<std::string>{std::move(result)}
                           : std::nullopt;
}

#else // POSIX
inline std::optional<std::wstring>
utf8_to_wide_impl(std::string_view utf8_str) noexcept
{
    if(utf8_str.empty())
        return std::wstring{};

    std::mbstate_t state{};
    const char    *src     = utf8_str.data();
    const char    *src_end = src + utf8_str.size();

    // Calculate required size
    size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
    if(len == static_cast<size_t>(-1))
        return std::nullopt;

    std::wstring result(len, 0);
    src   = utf8_str.data();  // Reset source pointer
    state = std::mbstate_t{}; // Reset state

    size_t converted = std::mbsrtowcs(result.data(), &src, len, &state);
    return (converted != static_cast<size_t>(-1))
               ? std::optional<std::wstring>{std::move(result)}
               : std::nullopt;
}

inline std::optional<std::string>
wide_to_utf8_impl(std::wstring_view wide_str) noexcept
{
    if(wide_str.empty())
        return std::string{};

    std::mbstate_t state{};
    const wchar_t *src = wide_str.data();

    // Calculate required size
    size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
    if(len == static_cast<size_t>(-1))
        return std::nullopt;

    std::string result(len, 0);
    src   = wide_str.data();  // Reset source pointer
    state = std::mbstate_t{}; // Reset state

    size_t converted = std::wcsrtombs(result.data(), &src, len, &state);
    return (converted != static_cast<size_t>(-1))
               ? std::optional<std::string>{std::move(result)}
               : std::nullopt;
}

#endif
} // namespace detail

class string_util
{
  public:
    enum class error_code
    {
        ok = 0,
        invalid_argument,
        conversion_error,
        regex_error,
        out_of_memory
    };

    string_util()                               = delete;
    ~string_util()                              = delete;
    string_util(const string_util &)            = delete;
    string_util &operator=(const string_util &) = delete;
    string_util(string_util &&)                 = delete;
    string_util &operator=(string_util &&)      = delete;

    template <typename StringType1, typename StringType2>
    static constexpr bool contains(const StringType1 &src,
                                   const StringType2 &sub) noexcept
    {
        static_assert(std::is_convertible_v<StringType1, std::string_view>);
        static_assert(std::is_convertible_v<StringType2, std::string_view>);

        std::string_view src_view = src;
        std::string_view sub_view = sub;
        return src_view.find(sub_view) != std::string_view::npos;
    }

    template <typename StringType1, typename StringType2>
    static constexpr bool starts_with(const StringType1 &src,
                                      const StringType2 &prefix) noexcept
    {
        static_assert(std::is_convertible_v<StringType1, std::string_view>);
        static_assert(std::is_convertible_v<StringType2, std::string_view>);

        std::string_view src_view    = src;
        std::string_view prefix_view = prefix;

        return src_view.size() >= prefix_view.size()
               && src_view.substr(0, prefix_view.size()) == prefix_view;
    }

    template <typename StringType1, typename StringType2>
    static constexpr bool ends_with(const StringType1 &src,
                                    const StringType2 &suffix) noexcept
    {
        static_assert(std::is_convertible_v<StringType1, std::string_view>);
        static_assert(std::is_convertible_v<StringType2, std::string_view>);

        std::string_view src_view    = src;
        std::string_view suffix_view = suffix;

        return src_view.size() >= suffix_view.size()
               && src_view.substr(src_view.size() - suffix_view.size())
                      == suffix_view;
    }

    static std::vector<std::string> split(std::string_view str,
                                          std::string_view delimiter)
    {
        if(str.empty() || delimiter.empty())
            return {std::string(str)};

        std::vector<std::string> result;
        result.reserve(8);
        size_t start = 0;
        size_t found = str.find(delimiter, start);
        while(found != std::string_view::npos)
        {
            if(found > start)
                result.emplace_back(str.substr(start, found - start));

            start = found + delimiter.length();
            found = str.find(delimiter, start);
        }

        if(start < str.length())
            result.emplace_back(str.substr(start));

        return result;
    }

    static std::string
    replace_all(std::string str, std::string_view from, std::string_view to)
    {
        if(from.empty())
            return str;

        size_t pos = 0;
        while((pos = str.find(from, pos)) != std::string::npos)
        {
            str.replace(pos, from.length(), to);
            pos += to.length();
        }
        return str;
    }

    static std::string &replace_all_inplace(std::string     &str,
                                            std::string_view from,
                                            std::string_view to)
    {
        if(from.empty())
            return str;

        size_t pos = 0;
        while((pos = str.find(from, pos)) != std::string::npos)
        {
            str.replace(pos, from.length(), to);
            pos += to.length();
        }
        return str;
    }

    // NOTE: this function returns the first match (match[0]).
    static std::optional<std::string>
    regex_search_first(std::string_view   src,
                       const std::string &pattern) noexcept
    {
        try
        {
            std::smatch match;
            const auto &regex =
                detail::regex_cache::instance().get_regex(pattern);
            std::string src_str(src); // Required for smatch

            if(std::regex_search(src_str, match, regex))
                return match[0].str();
        }
        catch(const std::regex_error &)
        {
            return std::nullopt;
        }
        return std::nullopt;
    }

    static std::vector<std::string>
    regex_search_all(std::string_view src, const std::string &pattern) noexcept
    {
        std::vector<std::string> results;
        try
        {
            const auto &regex =
                detail::regex_cache::instance().get_regex(pattern);
            std::string src_str(src);

            std::sregex_iterator begin(src_str.begin(), src_str.end(), regex),
                end;
            results.reserve(std::distance(begin, end));
            for(auto it = begin; it != end; ++it)
                results.emplace_back(it->str());
        }
        catch(const std::regex_error &)
        {
            // Log error in production code
        }
        return results;
    }

    static std::vector<std::string>
    regex_split(std::string_view str, const std::string &pattern) noexcept
    {
        try
        {
            const auto &regex =
                detail::regex_cache::instance().get_regex(pattern);
            std::string str_copy(str);

            std::sregex_token_iterator first{str_copy.begin(),
                                             str_copy.end(),
                                             regex,
                                             -1},
                last;
            return {first, last};
        }
        catch(const std::regex_error &)
        {
            return {std::string(str)};
        }
    }

    // Regex replace
    static std::string regex_replace(std::string        str,
                                     const std::string &pattern,
                                     std::string_view   replacement) noexcept
    {
        try
        {
            const auto &regex =
                detail::regex_cache::instance().get_regex(pattern);
            return std::regex_replace(str, regex, std::string(replacement));
        }
        catch(const std::regex_error &)
        {
            return str;
        }
    }

    static inline bool equal(const char *a, const char *b) noexcept
    {
        if(a == b)
            return true; // Same pointer (including both null)

        if(!a || !b)
            return false; // One is null, the other is not

        return std::strcmp(a, b) == 0;
    }

    static bool iequal(std::string_view a, std::string_view b) noexcept
    {
        return std::equal(a.begin(),
                          a.end(),
                          b.begin(),
                          b.end(),
                          [](char a, char b) {
                              return std::tolower(static_cast<unsigned char>(a))
                                     == std::tolower(
                                         static_cast<unsigned char>(b));
                          });
    }

    static inline std::optional<std::wstring>
    to_wstring(std::string_view src) noexcept
    {
        return detail::utf8_to_wide_impl(src);
    }

    static inline std::optional<std::wstring>
    to_wchar(std::string_view src) noexcept
    {
        return to_wstring(src);
    }

    static inline std::optional<std::string>
    from_wstring(std::wstring_view src) noexcept
    {
        return detail::wide_to_utf8_impl(src);
    }

    static inline std::optional<std::string>
    from_wchar_opt(const wchar_t *src) noexcept
    {
        if(!src)
            return std::string{};

        return from_wstring(std::wstring_view{src});
    }

    static inline std::wstring to_wstring_safe(std::string_view src) noexcept
    {
        auto result = to_wstring(src);
        return result ? std::move(*result) : std::wstring{};
    }

    static inline std::string from_wstring_safe(std::wstring_view src) noexcept
    {
        auto result = from_wstring(src);
        return result ? std::move(*result) : std::string{};
    }

    static inline std::string from_ptr_addr(const void *ptr,
                                            bool        is_hex = true) noexcept
    {
        if(!ptr)
            return is_hex ? "0x0" : "0";

        char       buffer[32]; // Enough for 64-bit pointer
        const auto addr = reinterpret_cast<std::uintptr_t>(ptr);

        if(is_hex)
            std::snprintf(buffer, sizeof(buffer), "0x%" PRIxPTR, addr);
        else
            std::snprintf(buffer, sizeof(buffer), "%" PRIuPTR, addr);

        return std::string{buffer};
    }

    static inline std::string_view
    trim_left(std::string_view str,
              std::string_view target = " \t\n\r\f\v") noexcept
    {
        const auto first_non_space = str.find_first_not_of(target);
        return (first_non_space == std::string_view::npos)
                   ? std::string_view{}
                   : str.substr(first_non_space);
    }

    static inline std::string_view
    trim_right(std::string_view str,
               std::string_view target = " \t\n\r\f\v") noexcept
    {
        const auto last_non_space = str.find_last_not_of(target);
        return (last_non_space == std::string_view::npos)
                   ? std::string_view{}
                   : str.substr(0, last_non_space + 1);
    }

    static inline std::string_view
    trim(std::string_view str, std::string_view target = " \t\n\r\f\v") noexcept
    {
        return trim_left(trim_right(str, target), target);
    }

    static inline std::string &
    trim_inplace(std::string &str, std::string_view target = " \t\n\r\f\v")
    {
        str.erase(str.find_last_not_of(target) + 1);
        str.erase(0, str.find_first_not_of(target));
        return str;
    }

    static inline std::string to_lower(std::string str)
    {
        std::transform(str.begin(),
                       str.end(),
                       str.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return str;
    }

    static inline std::string to_upper(std::string str)
    {
        std::transform(str.begin(),
                       str.end(),
                       str.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        return str;
    }

    static inline std::string search(const std::string &src,
                                     const std::string &pattern)
    {
        auto result = regex_search_first(src, pattern);
        return result ? *result : std::string{};
    }

    static inline std::vector<std::string> search_n(const std::string &src,
                                                    const std::string &pattern)
    {
        return regex_search_all(src, pattern);
    }

    static inline void search(const std::string &src,
                              std::smatch       &match,
                              const std::string &pattern) noexcept
    {
        try
        {
            const auto &regex =
                detail::regex_cache::instance().get_regex(pattern);
            std::regex_search(src, match, regex);
        }
        catch(const std::regex_error &)
        {
            match = std::smatch{};
        }
    }

    static inline std::vector<std::string>
    split_regex(const std::string &str, const std::string &pattern)
    {
        return regex_split(str, pattern);
    }

    static inline std::string &
    replace(std::string &str, const std::string &from, const std::string &to)
    {
        if(from.find_first_of("^$.*+?{}[]|()\\") == std::string::npos)
        {
            replace_all_inplace(str, from, to);
            return str;
        }

        str = regex_replace(str, from, to);
        return str;
    }

    static inline std::wstring to_wchar(const std::string &src)
    {
        return to_wstring_safe(src);
    }

    static inline std::wstring to_wstring(const std::string &src)
    {
        return to_wstring_safe(src);
    }

    static inline std::string from_wchar(const wchar_t *src)
    {
        if(!src)
            return std::string{};

        auto result = from_wchar_opt(src);
        return result ? *result : std::string{};
    }

    static inline std::string from_wstring(const std::wstring &src)
    {
        auto result = from_wstring(std::wstring_view{src});
        return result ? *result : std::string{};
    }

}; // class string

} // namespace hj

#endif // STRING_UTIL_HPP