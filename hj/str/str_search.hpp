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

#ifndef STR_SEARCH_HPP
#define STR_SEARCH_HPP

#include <string>
#include <string_view>
#include <optional>
#include <regex>
#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <cstring>
#include <cctype>
#include <memory>
#include <list>
#include <unordered_map>

namespace hj::str::search_detail
{

class regex_cache
{
  private:
    static constexpr size_t MAX_CACHE_SIZE = 32;

    // LRU structure
    using ListType =
        std::list<std::pair<std::string, std::shared_ptr<std::regex>>>;
    ListType _lru_list;
    // Map for O(1) lookup
    std::unordered_map<std::string, ListType::iterator> _cache_map;

    mutable std::mutex _mu;

  public:
    static regex_cache &instance()
    {
        static regex_cache inst;
        return inst;
    }

    std::shared_ptr<std::regex> get_regex(const std::string &pattern)
    {
        std::lock_guard<std::mutex> lock(_mu);

        auto it = _cache_map.find(pattern);
        if(it != _cache_map.end())
        {
            // Move to front (mark as recently used)
            _lru_list.splice(_lru_list.begin(), _lru_list, it->second);
            return it->second->second;
        }

        // Cache miss
        if(_cache_map.size() >= MAX_CACHE_SIZE)
        {
            // Evict least recently used (back of list)
            const auto &last = _lru_list.back();
            _cache_map.erase(last.first);
            _lru_list.pop_back();
        }

        auto reg = std::make_shared<std::regex>(pattern);
        _lru_list.emplace_front(pattern, reg);
        _cache_map[pattern] = _lru_list.begin();

        return reg;
    }
};

template <typename StringType1, typename StringType2>
constexpr bool contains(const StringType1 &src, const StringType2 &sub) noexcept
{
    std::string_view src_view = src;
    std::string_view sub_view = sub;
    return src_view.find(sub_view) != std::string_view::npos;
}

template <typename StringType1, typename StringType2>
constexpr bool starts_with(const StringType1 &src,
                           const StringType2 &prefix) noexcept
{
    std::string_view src_view    = src;
    std::string_view prefix_view = prefix;
    return src_view.size() >= prefix_view.size()
           && src_view.substr(0, prefix_view.size()) == prefix_view;
}

template <typename StringType1, typename StringType2>
constexpr bool ends_with(const StringType1 &src,
                         const StringType2 &suffix) noexcept
{
    std::string_view src_view    = src;
    std::string_view suffix_view = suffix;
    return src_view.size() >= suffix_view.size()
           && src_view.substr(src_view.size() - suffix_view.size())
                  == suffix_view;
}

inline void regex_search(std::string_view   src,
                         std::smatch       &match,
                         const std::string &pattern) noexcept
{
    try
    {
        const auto &regex = regex_cache::instance().get_regex(pattern);
        std::string src_str(src);
        std::regex_search(src_str, match, *regex);
    }
    catch(const std::regex_error &)
    {
        match = std::smatch{};
    }
}

inline std::optional<std::string> regex_search_first(std::string_view   src,
                                                     const std::string &pattern)
{
    try
    {
        const auto &regex_ptr = regex_cache::instance().get_regex(pattern);
        std::cmatch match;
        if(std::regex_search(src.data(),
                             src.data() + src.size(),
                             match,
                             *regex_ptr))
            return match[0].str();
    }
    catch(const std::regex_error &)
    {
    }
    return std::nullopt;
}

inline std::vector<std::string> regex_search_all(std::string_view   src,
                                                 const std::string &pattern)
{
    std::vector<std::string> results;
    try
    {
        const auto &regex_ptr = regex_cache::instance().get_regex(pattern);
        std::cregex_iterator begin(src.data(),
                                   src.data() + src.size(),
                                   *regex_ptr);
        std::cregex_iterator end;

        for(auto it = begin; it != end; ++it)
            results.emplace_back(it->str());
    }
    catch(const std::regex_error &)
    {
    }
    return results;
}

inline std::vector<std::string> regex_search_n(std::string_view   src,
                                               const std::string &pattern,
                                               const std::size_t  required)
{
    std::vector<std::string> results;
    try
    {
        const auto &regex_ptr = regex_cache::instance().get_regex(pattern);
        std::cregex_iterator begin(src.data(),
                                   src.data() + src.size(),
                                   *regex_ptr);
        std::cregex_iterator end;
        std::size_t          i = 0;
        for(auto it = begin; it != end && i < required; ++it, ++i)
            results.emplace_back(it->str());
    }
    catch(const std::regex_error &)
    {
    }
    return results;
}

inline bool cequal(const char *a, const char *b) noexcept
{
    if(a == b)
        return true;
    if(!a || !b)
        return false;
    return std::strcmp(a, b) == 0;
}

inline bool equal(std::string_view a, std::string_view b) noexcept
{
    return a.size() == b.size()
           && std::equal(a.begin(),
                         a.end(),
                         b.begin(),
                         b.end(),
                         [](char a, char b) {
                             return std::tolower(static_cast<unsigned char>(a))
                                    == std::tolower(
                                        static_cast<unsigned char>(b));
                         });
}

} // namespace hj::str::search_detail

// ---------------------search api----------------------------
namespace hj::str
{
template <typename StringType1, typename StringType2>
inline constexpr bool contains(const StringType1 &src,
                               const StringType2 &sub) noexcept
{
    return search_detail::contains(src, sub);
}

template <typename StringType1, typename StringType2>
inline constexpr bool starts_with(const StringType1 &src,
                                  const StringType2 &prefix) noexcept
{
    return search_detail::starts_with(src, prefix);
}

template <typename StringType1, typename StringType2>
inline constexpr bool ends_with(const StringType1 &src,
                                const StringType2 &suffix) noexcept
{
    return search_detail::ends_with(src, suffix);
}

inline std::optional<std::string> regex_search(std::string_view   src,
                                               const std::string &pattern)
{
    return search_detail::regex_search_first(src, pattern);
}

inline std::vector<std::string> regex_search_n(std::string_view   src,
                                               const std::string &pattern,
                                               const std::size_t  required)
{
    return search_detail::regex_search_n(src, pattern, required);
}

inline void regex_search(std::string_view   src,
                         std::smatch       &match,
                         const std::string &pattern) noexcept
{
    search_detail::regex_search(src, match, pattern);
}

inline std::optional<std::string> regex_search_first(std::string_view   src,
                                                     const std::string &pattern)
{
    return search_detail::regex_search_first(src, pattern);
}

inline std::vector<std::string> regex_search_all(std::string_view   src,
                                                 const std::string &pattern)
{
    return search_detail::regex_search_all(src, pattern);
}

inline bool cequal(const char *a, const char *b) noexcept
{
    return search_detail::cequal(a, b);
}

inline bool equal(std::string_view a, std::string_view b) noexcept
{
    return search_detail::equal(a, b);
}

} // namespace hj::str

#endif // STR_SEARCH_HPP