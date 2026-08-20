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

#ifndef STR_CHUNK_HPP
#define STR_CHUNK_HPP

#include <string>
#include <string_view>
#include <vector>
#include <mutex>
#include <regex>
#include <algorithm>
#include <memory>

namespace hj::str::chunk_detail
{

class regex_cache
{
  private:
    static constexpr size_t MAX_CACHE_SIZE = 32;
    struct cache_entry
    {
        std::string                 pattern;
        std::shared_ptr<std::regex> compiled;
        size_t                      access_count = 0;
    };

    mutable std::vector<cache_entry> _cache;
    mutable std::mutex               _mu;

  public:
    static regex_cache &instance()
    {
        static regex_cache inst;
        return inst;
    }

    std::shared_ptr<std::regex> get_regex(const std::string &pattern)
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

        auto reg = std::make_shared<std::regex>(pattern);
        _cache.emplace_back(cache_entry{pattern, reg, 1});
        return reg;
    }
};

inline std::string_view
trim_left(std::string_view str,
          std::string_view target = " \t\n\r\f\v") noexcept
{
    const auto first_non_space = str.find_first_not_of(target);
    return (first_non_space == std::string_view::npos)
               ? std::string_view{}
               : str.substr(first_non_space);
}

inline std::string_view
trim_right(std::string_view str,
           std::string_view target = " \t\n\r\f\v") noexcept
{
    const auto last_non_space = str.find_last_not_of(target);
    return (last_non_space == std::string_view::npos)
               ? std::string_view{}
               : str.substr(0, last_non_space + 1);
}

inline std::string_view trim(std::string_view str,
                             std::string_view target = " \t\n\r\f\v") noexcept
{
    return trim_left(trim_right(str, target), target);
}

inline std::string &trim_inplace(std::string     &str,
                                 std::string_view target = " \t\n\r\f\v")
{
    auto last = str.find_last_not_of(target);
    if(last == std::string::npos)
        return str.erase();
    str.erase(last + 1);
    auto first = str.find_first_not_of(target);
    if(first == std::string::npos)
        return str.erase();
    str.erase(0, first);
    return str;
}

inline std::vector<std::string> regex_split(std::string_view   str,
                                            const std::string &pattern)
{
    auto        regex_ptr = regex_cache::instance().get_regex(pattern);
    std::string str_copy(str);
    std::sregex_token_iterator first{str_copy.begin(),
                                     str_copy.end(),
                                     *regex_ptr,
                                     -1},
        last;
    return {first, last};
}

inline std::vector<std::string> split(std::string_view str,
                                      std::string_view delimiter)
{
    if(str.empty() || delimiter.empty())
        return {std::string(str)};

    std::vector<std::string> result;
    size_t                   start = 0;
    size_t                   found = str.find(delimiter, start);
    while(found != std::string_view::npos)
    {
        result.emplace_back(str.substr(start, found - start));
        start = found + delimiter.length();
        found = str.find(delimiter, start);
    }
    result.emplace_back(str.substr(start));
    return result;
}
} // namespace hj::str::chunk_detail

// ---------------------chunk api----------------------------
namespace hj::str
{
inline std::string_view
trim_left(std::string_view str,
          std::string_view target = " \t\n\r\f\v") noexcept
{
    return chunk_detail::trim_left(str, target);
}
inline std::string_view
trim_right(std::string_view str,
           std::string_view target = " \t\n\r\f\v") noexcept
{
    return chunk_detail::trim_right(str, target);
}
inline std::string_view trim(std::string_view str,
                             std::string_view target = " \t\n\r\f\v") noexcept
{
    return chunk_detail::trim(str, target);
}
inline std::string &trim_inplace(std::string     &str,
                                 std::string_view target = " \t\n\r\f\v")
{
    return chunk_detail::trim_inplace(str, target);
}
inline std::vector<std::string> regex_split(std::string_view   str,
                                            const std::string &pattern)
{
    return chunk_detail::regex_split(str, pattern);
}
inline std::vector<std::string> split(std::string_view str,
                                      std::string_view delimiter)
{
    return chunk_detail::split(str, delimiter);
}

} // namespace hj::str

#endif // STR_CHUNK_HPP