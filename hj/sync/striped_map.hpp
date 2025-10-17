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
#ifndef STRIPED_MAP_HPP
#define STRIPED_MAP_HPP

#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
#include <shared_mutex>

using shared_mutex_t = std::shared_mutex;
using unique_lock_t  = std::unique_lock<shared_mutex_t>;
using shared_lock_t  = std::shared_lock<shared_mutex_t>;
#else
#include <boost/thread.hpp>

using shared_mutex_t = boost::shared_mutex;
using unique_lock_t  = boost::unique_lock<shared_mutex_t>;
using shared_lock_t  = boost::shared_lock<shared_mutex_t>;
#endif

// NOTE: This container was implemented by stl and boost, suit to the scene of read more write less,
//          if you need a higher performance thread safe map, find it here:
// folly: https://github.com/facebook/folly/blob/main/folly/concurrency/ConcurrentHashMap.h
// tbb: https://github.com/uxlfoundation/oneTBB/blob/master/include/tbb/concurrent_hash_map.h
namespace hj
{

template <typename Key,
          typename Value,
          typename Alloc = std::allocator<std::pair<const Key, Value>>>
class striped_map
{
  public:
    using range_handler_t     = std::function<bool(const Key &, const Value &)>;
    using strip_key_handler_t = std::function<int(const Key &)>;
    using allocator_type      = Alloc;
    using value_type          = std::pair<const Key, Value>;
    using bucket_type         = std::unordered_map<Key,
                                                   Value,
                                                   std::hash<Key>,
                                                   std::equal_to<Key>,
                                                   allocator_type>;

  public:
    class const_iterator
    {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = std::pair<const Key, Value>;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const value_type *;
        using reference         = const value_type &;

        const_iterator(const striped_map                   *map,
                       std::size_t                          bucket,
                       typename bucket_type::const_iterator it)
            : _map(map)
            , _bucket(bucket)
            , _it(it)
        {
            _advance_to_valid();
        }

        const_iterator &operator++()
        {
            ++_it;
            _advance_to_valid();
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        reference operator*() const { return *_it; }
        pointer   operator->() const { return &(*_it); }

        bool operator==(const const_iterator &other) const
        {
            return _map == other._map && _bucket == other._bucket
                   && _it == other._it;
        }
        bool operator!=(const const_iterator &other) const
        {
            return !(*this == other);
        }

      private:
        void _advance_to_valid()
        {
            if(_map->_buckets.empty())
            {
                _bucket = 0;
                _it     = typename bucket_type::const_iterator();
                _bucket = _map->_buckets.size();
                return;
            }

            while(_bucket < _map->_buckets.size()
                  && _it == _map->_buckets[_bucket].end())
            {
                ++_bucket;
                if(_bucket < _map->_buckets.size())
                    _it = _map->_buckets[_bucket].begin();
            }

            if(_bucket >= _map->_buckets.size())
                _it = typename bucket_type::const_iterator();
        }

        const striped_map                   *_map;
        std::size_t                          _bucket;
        typename bucket_type::const_iterator _it;
    };

    const_iterator begin() const
    {
        if(_buckets.empty())
            return end();

        return const_iterator(this, 0, _buckets[0].begin());
    }

    const_iterator end() const
    {
        return const_iterator(this,
                              _buckets.size(),
                              typename bucket_type::const_iterator());
    }

  public:
    explicit striped_map(std::size_t    capa,
                         allocator_type alloc = allocator_type())
        : striped_map(
              [capa](const Key &k) -> int {
                  return static_cast<int>(std::hash<Key>{}(k) % capa);
              },
              capa,
              alloc)
    {
    }
    striped_map(strip_key_handler_t fn,
                std::size_t         capa,
                allocator_type      alloc = allocator_type())
        : _strip_fn(std::move(fn))
        , _alloc(alloc)
    {
        _buckets.reserve(capa);
        _locks.reserve(capa);
        for(std::size_t bucket = 0; bucket < capa; bucket++)
        {
            _buckets.emplace_back(_alloc);
            _locks.emplace_back(std::make_unique<shared_mutex_t>());
        }
    }

    ~striped_map()                               = default;
    striped_map(const striped_map &)             = delete;
    striped_map &operator=(const striped_map &)  = delete;
    striped_map(const striped_map &&)            = delete;
    striped_map &operator=(const striped_map &&) = delete;

    void emplace(const Key &key, const Value &value)
    {
        int bucket = _strip_fn(key);
        if(bucket < 0 || static_cast<std::size_t>(bucket) >= _buckets.size())
            throw std::out_of_range("bucket index out of range");

        unique_lock_t lock(*_locks[bucket]);
        _buckets[bucket].emplace(key, value);
    }

    void replace(const Key &key, Value &&value)
    {
        int bucket = _strip_fn(key);
        if(bucket < 0 || static_cast<std::size_t>(bucket) >= _buckets.size())
            throw std::out_of_range("bucket index out of range");

        unique_lock_t lock(*_locks[bucket]);
        _buckets[bucket][key] = std::forward<Value>(value);
    }

    bool find(const Key &key, Value &value) const
    {
        int bucket = _strip_fn(key);
        if(bucket < 0 || static_cast<std::size_t>(bucket) >= _buckets.size())
            throw std::out_of_range("bucket index out of range");

        shared_lock_t lock(*_locks[bucket]);
        auto          itr = _buckets[bucket].find(key);
        if(itr == _buckets[bucket].end())
            return false;

        value = itr->second;
        return true;
    }

    bool erase(const Key &key)
    {
        int bucket = _strip_fn(key);
        if(bucket < 0 || static_cast<std::size_t>(bucket) >= _buckets.size())
            throw std::out_of_range("bucket index out of range");

        unique_lock_t lock(*_locks[bucket]);
        bool          ret = _buckets[bucket].erase(key) > 0;
        return ret;
    }

    void range(const range_handler_t &fn) const
    {
        for(std::size_t bucket = 0; bucket < _buckets.size(); ++bucket)
        {
            // lock begin
            {
                shared_lock_t lock(*_locks[bucket]);
                for(auto itr = _buckets[bucket].begin();
                    itr != _buckets[bucket].end();
                    ++itr)
                {
                    auto k = itr->first;
                    auto v = itr->second;
                    if(fn(k, v))
                        continue;

                    return;
                }
            }
            // lock end
        }
    }

    std::size_t size() const
    {
        std::size_t total = 0;
        for(std::size_t bucket = 0; bucket < _buckets.size(); ++bucket)
        {
            // lock begin
            {
                shared_lock_t lock(*_locks[bucket]);
                total += _buckets[bucket].size();
            }
            // lock end
        }
        return total;
    }

    void clear()
    {
        for(std::size_t bucket = 0; bucket < _buckets.size(); ++bucket)
        {
            // lock begin
            {
                unique_lock_t lock(*_locks[bucket]);
                _buckets[bucket].clear();
            }
            // lock end
        }
    }

  private:
    std::vector<bucket_type>                             _buckets;
    mutable std::vector<std::unique_ptr<shared_mutex_t>> _locks;
    strip_key_handler_t                                  _strip_fn;
    allocator_type                                       _alloc;
};

}

#endif
