#ifndef TCP_MGR_HPP
#define TCP_MGR_HPP

#include <oneapi/tbb/concurrent_hash_map.h>
#include <libcpp/net/tcp/tcp_conn.hpp>

namespace libcpp
{

template<typename Key, typename Value>
class tcp_mgr
{
public:
    using hash_map_t = tbb::concurrent_hash_map<Key, Value>;
    using accessor_t = typename hash_map_t::accessor;
    using const_accessor_t = typename hash_map_t::const_accessor;

    using range_handler_t = std::function<bool(const Key&, Value&)>;

public:
    tcp_mgr() 
        : map_{}
    {
    }

    virtual ~tcp_mgr()
    {
    }

    inline void emplace(Key&& key, Value&& value)
    {
        map_.insert(std::make_pair(std::move(key), std::move(value)));
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

    inline bool erase(Key&& key)
    {
        return map_.erase(key);
    }

    void range(const range_handler_t& fn)
    {
        for (auto itr = map_.begin(); itr != map_.end(); ++itr) 
            if (!fn(itr->first, itr->second))
                return;
    }

    inline std::size_t count(Key&& key) const
    {
        return map_.count(key);
    }

private:
    oneapi::tbb::concurrent_hash_map<Key, Value> map_;
};

}

#endif