#ifndef SAFE_MAP_HPP
#define SAFE_MAP_HPP

#include <oneapi/tbb/concurrent_hash_map.h>

namespace libcpp {
// NOTE: This container was implemented by onetbb
template <typename Key, typename Value>
class safe_map
{
  public:
    using hash_map_t = tbb::concurrent_hash_map<Key, Value>;
    using accessor_t = typename hash_map_t::accessor;
    using const_accessor_t = typename hash_map_t::const_accessor;

    using range_handler_t = std::function<bool(const Key&, Value&)>;
    using const_range_handler_t = std::function<bool(const Key&, const Value&)>;

  public:
    safe_map() : map_{} {}

    virtual ~safe_map() {}

    inline bool insert(Key&& key, Value&& value)
    {
        return map_.insert(std::make_pair(std::move(key), std::move(value)));
    }

    inline bool emplace(Key&& key, Value&& value)
    {
        return map_.emplace(std::make_pair(std::move(key), std::move(value)));
    }

    void replace(Key&& key, Value&& value)
    {
        accessor_t acc;
        map_.insert(acc, std::move(key));
        acc->second = std::move(value);
    }

    void replace(Key&& key, Value&& value, Value& old)
    {
        accessor_t acc;
        map_.insert(acc, std::move(key));
        old = acc->second;
        acc->second = std::move(value);
    }

    bool find(Key&& key, Value& value) const
    {
        const_accessor_t acc;
        bool ret = map_.find(acc, key);
        value = acc->second;
        return ret;
    }

    inline bool erase(Key&& key) { return map_.erase(key); }

    void range(const range_handler_t& fn)
    {
        for (auto itr = map_.begin(); itr != map_.end(); ++itr)
            if (!fn(itr->first, itr->second))
                return;
    }
    void range(const const_range_handler_t& fn) const
    {
        for (auto itr = map_.begin(); itr != map_.end(); ++itr)
            if (!fn(itr->first, itr->second))
                return;
    }
    inline std::size_t count(Key&& key) const { return map_.count(key); }
    inline std::size_t size() const { return map_.size(); }
    inline bool empty() const { return map_.empty(); }
    inline void clear() { map_.clear(); }
    inline void swap(safe_map& other) noexcept { map_.swap(other.map_); }
    inline void swap(safe_map&& other) noexcept { map_.swap(other.map_); }

  private:
    oneapi::tbb::concurrent_hash_map<Key, Value> map_;
};
}  // namespace libcpp

#endif