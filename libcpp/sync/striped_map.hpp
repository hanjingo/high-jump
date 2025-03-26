#ifndef STRIPED_MAP_HPP
#define STRIPED_MAP_HPP

#include <vector>
#include <unordered_map>
#include <functional>
#include <boost/thread.hpp>

namespace libcpp
{

template<typename Key, typename Value>
class striped_map
{
public:
    using range_handler_t = std::function<bool(const Key&, const Value&)>;
    using strip_key_handler_t = std::function<int(const Key&)>;

public:
    explicit striped_map(strip_key_handler_t fn, std::size_t capa) 
        : _strip_fn(std::move(fn))
    {
        _buckets.reserve(capa);
        _locks.reserve(capa);
        for (std::size_t bucket = 0; bucket < capa; bucket++)
        {
            _buckets.emplace_back(std::unordered_map<Key, Value>{});
            _locks.emplace_back(boost::shared_mutex{});
        }
    }

    void insert(Key&& key, Value&& value)
    {
        int bucket = _strip_fn(key);
        std::unique_lock<boost::shared_mutex> lock(_locks[bucket]);
        _buckets[bucket].emplace(std::forward<Key>(key), std::forward<Value>(value));
    }

    bool find(const Key& key, Value& value) const
    {
        int bucket = _strip_fn(key);
        std::shared_lock<boost::shared_mutex> lock(_locks[bucket]);
        auto it = _buckets[bucket].find(key);
        if (it == _buckets[bucket].end()) return false;

        value = it->second;
        return true;
    }

    bool erase(const Key& key)
    {
        int bucket = _strip_fn(key);
        std::unique_lock<boost::shared_mutex> lock(_locks[bucket]);
        return _buckets[bucket].erase(key) > 0;
    }

    void range(const range_handler_t& fn) const
    {
        for (std::size_t bucket = 0; bucket < _buckets.size(); ++bucket)
        {
            std::shared_lock<boost::shared_mutex> lock(_locks[bucket]);
            for (const auto& [k, v] : _buckets[bucket])
            {
                if (!fn(k, v)) 
                    return;
            }
        }
    }

    std::size_t size() const
    {
        std::size_t total = 0;
        for (std::size_t bucket = 0; bucket < _buckets.size(); ++bucket)
        {
            std::shared_lock<boost::shared_mutex> lock(_locks[bucket]);
            total += _buckets[bucket].size();
        }
        return total;
    }

private:
    std::vector<std::unordered_map<Key, Value>> _buckets;
    std::vector<boost::shared_mutex> _locks;
    strip_key_handler_t _strip_fn;
};

}

#endif
