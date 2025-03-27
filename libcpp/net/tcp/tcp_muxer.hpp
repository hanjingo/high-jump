#ifndef TCP_MUXER_HPP
#define TCP_MUXER_HPP

#include <set>
#include <functional>

namespace libcpp
{

template<typename Key, typename Value>
class tcp_muxer
{
public:
    using namelist_t = std::set<Key>;
    using namelist_generator_t = std::function<namelist_t(msg_ptr_t)>;

public:
    tcp_muxer()
    {
    }

    ~tcp_muxer()
    {
    }

    std::size_t broad_cast(msg_ptr_t msg)
    {
        for (auto itr = _conns.begin(); itr != _conns.end(); itr++)
            itr->second->async_send(msg);

        return _conns.size();
    }

    bool group_combine()
    {

    }

    bool group_join()
    {

    }

    bool group_exit()
    {

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