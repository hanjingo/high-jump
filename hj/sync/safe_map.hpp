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
#ifndef SAFE_MAP_HPP
#define SAFE_MAP_HPP

#include <oneapi/tbb/concurrent_hash_map.h>
#include <vector>
#include <functional>

namespace hj
{
// NOTE: This container was implemented by onetbb
template <typename Key, typename Value>
class safe_map
{
  public:
    using hash_map_t       = tbb::concurrent_hash_map<Key, Value>;
    using accessor_t       = typename hash_map_t::accessor;
    using const_accessor_t = typename hash_map_t::const_accessor;

    using range_handler_t = std::function<bool(const Key &, Value &)>;
    using const_range_handler_t =
        std::function<bool(const Key &, const Value &)>;

  public:
    safe_map() = default;
    virtual ~safe_map() {}

    inline bool insert(const Key &key, const Value &value)
    {
        return _map.insert(std::make_pair(key, value));
    }

    inline bool insert(Key &&key, Value &&value)
    {
        return _map.insert(std::make_pair(std::move(key), std::move(value)));
    }

    void insert_bulk(const std::vector<std::pair<Key, Value>> &items)
    {
        for(const auto &kv : items)
            _map.insert(kv);
    }

    inline bool emplace(Key &&key, Value &&value)
    {
        return _map.emplace(std::make_pair(std::move(key), std::move(value)));
    }

    void replace(const Key &key, const Value &value)
    {
        accessor_t acc;
        _map.insert(acc, key);
        acc->second = value;
    }

    void replace(Key &&key, Value &&value)
    {
        accessor_t acc;
        _map.insert(acc, std::move(key));
        acc->second = std::move(value);
    }

    void replace(Key &&key, Value &&value, Value &old)
    {
        accessor_t acc;
        _map.insert(acc, std::move(key));
        old         = acc->second;
        acc->second = std::move(value);
    }

    std::optional<Value> find(const Key &key) const
    {
        const_accessor_t acc;
        if(_map.find(acc, key))
            return acc->second;

        return std::nullopt;
    }

    bool find(Key &&key, Value &value) const
    {
        const_accessor_t acc;
        bool             ret = _map.find(acc, key);
        if(ret)
            value = acc->second;

        return ret;
    }

    void range(const range_handler_t &fn)
    {
        for(auto itr = _map.begin(); itr != _map.end(); ++itr)
            if(!fn(itr->first, itr->second))
                return;
    }

    void range(const const_range_handler_t &fn) const
    {
        for(auto itr = _map.begin(); itr != _map.end(); ++itr)
            if(!fn(itr->first, itr->second))
                return;
    }

    inline bool        erase(Key &&key) { return _map.erase(key); }
    inline std::size_t count(Key &&key) const { return _map.count(key); }
    inline std::size_t size() const { return _map.size(); }
    inline bool        empty() const { return _map.empty(); }
    inline void        clear() { _map.clear(); }
    inline void        swap(safe_map &other) noexcept { _map.swap(other._map); }
    inline void swap(safe_map &&other) noexcept { _map.swap(other._map); }

  private:
    oneapi::tbb::concurrent_hash_map<Key, Value> _map;
};
}

#endif