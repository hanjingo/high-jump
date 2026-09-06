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

#ifndef STR_REPLACE_HPP
#define STR_REPLACE_HPP

#include <string>
#include <string_view>
#include <regex>
#include <mutex>
#include <list>
#include <unordered_map>
#include <memory>
#include <utility>

namespace hj::str::replace_detail
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
} // namespace hj::str::replace_detail

namespace hj::str
{
/**
 * @brief High-performance replace_all using a two-pass scan algorithm (O(n) complexity).
 * Avoids O(n^2) memory shifting and reallocations when from.length() != to.length().
 */
inline std::string
replace_all(std::string_view str, std::string_view from, std::string_view to)
{
    if(from.empty())
        return std::string(str);

    size_t count = 0;
    size_t pos   = 0;
    while((pos = str.find(from, pos)) != std::string::npos)
    {
        ++count;
        pos += from.length();
    }

    if(count == 0)
        return std::string(str);

    std::string result;
    size_t      new_size = str.size() + count * (to.length() - from.length());
    result.reserve(new_size);
    size_t last_pos = 0;
    pos             = 0;
    while((pos = str.find(from, last_pos)) != std::string::npos)
    {
        result.append(str.data() + last_pos, pos - last_pos);
        result.append(to);
        last_pos = pos + from.length();
    }
    result.append(str.data() + last_pos, str.size() - last_pos);

    return result;
}

inline std::string &replace_all_inplace(std::string     &str,
                                        std::string_view from,
                                        std::string_view to)
{
    if(from.empty() || str.empty())
        return str;

    // Utilize the optimized two-pass builder via move assignment
    str = replace_all(str, from, to);
    return str;
}

// Regex replace with optimized cache usage
inline std::string regex_replace(const std::string &str,
                                 const std::string &pattern,
                                 std::string_view   replacement) noexcept
{
    try
    {
        const auto &regex =
            replace_detail::regex_cache::instance().get_regex(pattern);
        return std::regex_replace(str, *regex, std::string(replacement));
    }
    catch(const std::regex_error &)
    {
        return str;
    }
}

// Unified interface with smart fallback
inline std::string &
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
} // namespace hj::str

#endif // STR_REPLACE_HPP