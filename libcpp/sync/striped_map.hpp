#ifndef STRIPED_MAP_HPP
#define STRIPED_MAP_HPP

#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

#if (__cplusplus >= 201703L)
#include <shared_mutex>
using shared_mutex_t = std::shared_mutex;

#else
#include <boost/thread.hpp>
using shared_mutex_t = boost::shared_mutex;
#endif

// NOTE: This container was implemented by stl and boost, suit to the scene of read more write less, 
//          if you need a higher performance thread safe map, find it here:
// folly: https://github.com/facebook/folly/blob/main/folly/concurrency/ConcurrentHashMap.h
// tbb: https://github.com/uxlfoundation/oneTBB/blob/master/include/tbb/concurrent_hash_map.h
namespace libcpp
{

template<typename Key, typename Value>
class striped_map
{
public:
    using range_handler_t = std::function<bool(const Key&, Value&)>;
    using strip_key_handler_t = std::function<int(const Key&)>;

public:
    explicit striped_map(strip_key_handler_t fn, std::size_t capa) 
        : _strip_fn(std::move(fn))
    {
        _buckets.reserve(capa);
        _locks.reserve(capa);
        for (std::size_t bucket = 0; bucket < capa; bucket++)
        {
            _buckets.emplace_back();
            _locks.emplace_back(std::make_unique<shared_mutex_t>());
        }
    }

    void emplace(Key&& key, Value&& value)
    {
        int bucket = _strip_fn(key);
        auto& mu = _locks[bucket];
        mu->lock();
        _buckets[bucket].emplace(std::forward<Key>(key), std::forward<Value>(value));
        mu->unlock();
    }

    void replace(Key&& key, Value&& value)
    {
        int bucket = _strip_fn(key);
        auto& mu = _locks[bucket];
        mu->lock();
        _buckets[bucket][std::forward<Key>(key)] = std::forward<Value>(value);
        mu->unlock();
    }

    bool find(const Key& key, Value& value) const
    {
        int bucket = _strip_fn(key);
        auto& mu = _locks[bucket];
        mu->lock_shared();
        auto itr = _buckets[bucket].find(key);
        if (itr == _buckets[bucket].end()) 
        {
            mu->unlock_shared();
            return false;
        }

        value = itr->second;
        mu->unlock_shared();
        return true;
    }

    bool erase(const Key& key)
    {
        int bucket = _strip_fn(key);
        auto& mu = _locks[bucket];
        mu->lock();
        bool ret = _buckets[bucket].erase(key) > 0;
        mu->unlock();
        return ret;
    }

    void range(const range_handler_t& fn) const
    {
        for (std::size_t bucket = 0; bucket < _buckets.size(); ++bucket)
        {
            auto& mu = _locks[bucket];
            mu->lock();
            for (auto itr = _buckets[bucket].begin(); itr != _buckets[bucket].end(); ++itr)
            {
                auto k = itr->first;
                auto v = itr->second;
                if (fn(k, v))
                    continue;

                mu->unlock();
                return;
            }
            // for (auto& pair : _buckets[bucket])
            // {
            //     auto k = pair.first;
            //     auto v = pair.second;
            //     if (fn(k, v)) 
            //         continue;
                
            //     mu->unlock();
            //     return;
            // }
            mu->unlock();
        }
    }

    std::size_t size() const
    {
        std::size_t total = 0;
        for (std::size_t bucket = 0; bucket < _buckets.size(); ++bucket)
        {
            auto& mu = _locks[bucket];
            mu->lock_shared();
            total += _buckets[bucket].size();
            mu->unlock_shared();
        }
        return total;
    }

private:
    std::vector<std::unordered_map<Key, Value> > _buckets;
    mutable std::vector<std::unique_ptr<shared_mutex_t> > _locks;
    strip_key_handler_t _strip_fn;
};

}

#endif
