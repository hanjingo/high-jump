#ifndef TCP_MUXER_HPP
#define TCP_MUXER_HPP

#include <set>
#include <functional>
#include <unordered_map>
#include <libcpp/net/proto/message.hpp>
#include <libcpp/net/tcp/tcp_conn.hpp>

namespace libcpp
{

template<typename Key = std::uint64_t>
class tcp_muxer
{
public:
    using conn_ptr_t = tcp_conn*;
    using connlist_t = std::vector<conn_ptr_t>;
    using msg_ptr_t = libcpp::message*;
    using namelist_t = std::set<Key>;
    using namelist_generator_t = std::function<namelist_t(msg_ptr_t)>;

public:
    tcp_muxer()
    {
    }

    ~tcp_muxer()
    {
    }

    bool add(Key id, conn_ptr_t conn)
    {
        if (_conns.find(id) != _conns.end())
            return false;

        _conns.emplace(id, conn);
        return true;
    }

    conn_ptr_t del(Key id)
    {
        auto itr = _conns.find(id);
        if (itr == _conns.end())
            return nullptr;

        _conns.erase(itr);
        return itr->second;
    }

    conn_ptr_t get(Key id)
    {
        auto itr = _conns.find(id);
        if (itr == _conns.end())
            return nullptr;

        return itr->second;
    }

    std::size_t broad_cast(msg_ptr_t msg)
    {
        for (auto itr = _conns.begin(); itr != _conns.end(); itr++)
            itr->second->async_send(msg);

        return _conns.size();
    }

    std::size_t group_cast(msg_ptr_t msg, namelist_generator_t&& fn)
    {
        return group_cast(fn());
    }

    std::size_t group_cast(msg_ptr_t msg, const namelist_t& li)
    {
        std::size_t nsend = 0;
        conn_ptr_t conn = nullptr;
        for (auto id : ids)
        {
            conn = get(id);
            if (conn == nullptr)
                continue;

            conn->async_send(msg);
            nsend++;
        }
        return nsend;
    }

    bool route(msg_ptr_t msg, const Key id)
    {
        auto conn = get(id);
        if (conn == nullptr)
            return false;

        return conn->send(msg);
    }

private:
    std::unordered_map<Key, conn_ptr_t> _conns;
};

}

#endif