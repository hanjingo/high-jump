#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <string>
#include <libcpp/net/proto/message.hpp>
#include <libcpp/net/tcp/tcp_listener.hpp>
#include <libcpp/net/tcp/tcp_conn.hpp>

namespace libcpp
{

template<typename Key = std::uint64_t>
class tcp_server
{
public:
    tcp_server()
        : _listener{}
    {
    }
    
    ~tcp_server()
    {
        _listener.close();

        auto ids = _conns.keys();
        for (Key id : ids)
            _del(id);
    }

    void listen(
        const char* ip, 
        const uint16_t port, 
        libcpp::tcp_listener::accept_handler_t&& handler)
    {
        _listener.async_accept(ip, port, std::move(handler));
    }

    void close()
    {
        auto keys = _conns.keys();
        for (Key id : keys)
            _del(id);
    }

    bool send(message* msg, const Key id, const bool block = true)
    {
        auto conn = _get(id);
        if (conn == nullptr)
            return false;

        conn->send(msg, block);
        return true;
    }

    bool recv(message* msg, const Key id, const bool block = true)
    {
        auto conn = _get(id);
        if (conn == nullptr)
            return false;

        conn->send(msg, block);
        return true;
    }

    void broad_cast(message* msg, const bool block = false)
    {
        auto ids = _conns.keys();
        for (Key id : ids)
            send(id);
    }

    void group_cast(message* msg, const std::vector<Key>& ids, const bool block = false)
    {
        for (auto id : ids)
            send(id);
    }

    void kick_off(const std::vector<Key>& ids)
    {
        for (Key id : ids)
            _del(id);
    }

private:
    bool _add(const Key id, tcp_conn* conn)
    {
        if (conn == nullptr || _conns.find(id) != _conns.end())
            return false;

        _conns.emplace(id, conn);
        return true;
    }

    void _del(const Key id)
    {
        auto itr = _conns.remove(id);
        if (itr == _conns.end())
            return;

        itr->second->disconnect();
        delete itr->second;
    }

    tcp_conn* _get(const Key id)
    {
        auto itr = _conns.find(id);
        if (itr == _conns.end())
            return nullptr;

        return itr->second;
    }

private:
    libcpp::tcp_listener _listener;
    std::unordered_map<Key, tcp_conn*> _conns;
};

}

#endif