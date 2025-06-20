#ifndef STRIPED_MAP_HPP
#define STRIPED_MAP_HPP

#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

#if (__cplusplus >= 201703L)
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
namespace libcpp
{

template<typename Key, typename Value>
class striped_map
{
public:
    using range_handler_t     = std::function<bool(const Key&, const Value&)>;
    using strip_key_handler_t = std::function<int(const Key&)>;

public:
    explicit striped_map(std::size_t capa) 
        : striped_map([](const Key& k){ return std::hash<Key>{}(k) % capa; }, capa)
    {
    }
    striped_map(strip_key_handler_t fn, std::size_t capa) 
        : strip_fn_(std::move(fn))
    {
        buckets_.reserve(capa);
        locks_.reserve(capa);
        for (std::size_t bucket = 0; bucket < capa; bucket++)
        {
            buckets_.emplace_back();
            locks_.emplace_back(std::make_unique<shared_mutex_t>());
        }
    }

    ~striped_map() = default;

    void emplace(Key&& key, Value&& value)
    {
        int bucket = strip_fn_(key);
        if (bucket < 0 || static_cast<std::size_t>(bucket) >= buckets_.size())
            throw std::out_of_range("bucket index out of range");

        unique_lock_t lock(*locks_[bucket]);
        buckets_[bucket].emplace(std::forward<Key>(key), std::forward<Value>(value));
    }

    void replace(const Key& key, Value&& value)
    {
        int bucket = strip_fn_(key);
        if (bucket < 0 || static_cast<std::size_t>(bucket) >= buckets_.size())
            throw std::out_of_range("bucket index out of range");

        unique_lock_t lock(*locks_[bucket]);
        buckets_[bucket][key] = std::forward<Value>(value);
    }

    bool find(const Key& key, Value& value) const
    {
        int bucket = strip_fn_(key);
        if (bucket < 0 || static_cast<std::size_t>(bucket) >= buckets_.size())
            throw std::out_of_range("bucket index out of range");

        shared_lock_t lock(*locks_[bucket]);
        auto itr = buckets_[bucket].find(key);
        if (itr == buckets_[bucket].end()) 
            return false;

        value = itr->second;
        return true;
    }

    bool erase(const Key& key)
    {
        int bucket = strip_fn_(key);
        if (bucket < 0 || static_cast<std::size_t>(bucket) >= buckets_.size())
            throw std::out_of_range("bucket index out of range");

        unique_lock_t lock(*locks_[bucket]);
        bool ret = buckets_[bucket].erase(key) > 0;
        return ret;
    }

    void range(const range_handler_t& fn) const
    {
        for (std::size_t bucket = 0; bucket < buckets_.size(); ++bucket)
        {
            // lock begin
            {
                shared_lock_t lock(*locks_[bucket]);
                for (auto itr = buckets_[bucket].begin(); itr != buckets_[bucket].end(); ++itr)
                {
                    auto k = itr->first;
                    auto v = itr->second;
                    if (fn(k, v))
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
        for (std::size_t bucket = 0; bucket < buckets_.size(); ++bucket)
        {
            // lock begin
            {
                shared_lock_t lock(*locks_[bucket]);
                total += buckets_[bucket].size();
            }
            // lock end
        }
        return total;
    }

    void clear()
    {
        for (std::size_t bucket = 0; bucket < buckets_.size(); ++bucket)
        {
            // lock begin
            {
                unique_lock_t lock(*locks_[bucket]);
                buckets_[bucket].clear();
            }
            // lock end
        }
    }

private:
    striped_map(const striped_map&) = delete;
    striped_map& operator=(const striped_map&) = delete;
    striped_map(const striped_map&&) = delete;
    striped_map& operator=(const striped_map&&) = delete;

private:
    std::vector<std::unordered_map<Key, Value> >          buckets_;
    mutable std::vector<std::unique_ptr<shared_mutex_t> > locks_;
    strip_key_handler_t                                   strip_fn_;
};

}

#endif
