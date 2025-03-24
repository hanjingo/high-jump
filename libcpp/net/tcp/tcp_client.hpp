#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <set>
#include <unordered_map>
#include <functional>

#include <libcpp/net/tcp/tcp_conn.hpp>
#include <libcpp/net/proto/message.hpp>

namespace libcpp
{

template<typename Key>
class tcp_client
{
public:

public:
    tcp_client() {}
    ~tcp_client() { close(); }

    inline std::size_t size() { return _conns.size(); }
    inline tcp_conn* get(Key id) { return _get(id); }
    inline std::size_t add(Key id, tcp_conn* conn) { _add(id, conn); return _conns.size(); }

    tcp_conn* connect(const char* ip, 
                      const std::uint16_t port,
                      const std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
                      int retry_times = 1)
    {
        tcp_conn* conn = new tcp_conn();
        if (!conn->connect(ip, port, timeout, retry_times))
        {
            delete conn;
            conn = nullptr;
        }

        return conn;
    }

    void disconnect(const Key id)
    {
        auto conn = _get(id);
        if (conn == nullptr)
            return;

        conn->disconnect();
    }

    bool send(message* msg, Key id, bool block = true)
    {
        auto conn = _get(id);
        if (conn == nullptr)
            return false;

        conn->send(msg, block);
        return true;
    }

    bool recv(message* msg, Key id, bool block = true)
    {
        auto conn = _get(id);
        if (conn == nullptr)
            return false;

        conn->recv(msg, block);
        return true;
    }

    void broad_cast(message* msg, bool block = false)
    {
        for (auto itr = _conns.begin(); itr != _conns.end(); itr++)
            itr->second->send(msg, block);
    }

    void group_cast(message* msg, std::set<Key>& ids, bool block = false)
    {
        group_cast(msg, std::initializer_list<Key>(ids.begin(), ids.end()), block);
    }

    void group_cast(message* msg, std::initializer_list<Key> ids, bool block = false)
    {
        for (Key id : ids)
        {
            auto conn = _get(id);
            if (conn == nullptr)
                continue;

            conn->send(msg, block);
        }
    }

    void req_resp(message* req, message& resp, const Key id)
    {
        auto conn = _get(id);
        if (conn == nullptr)
            return;

        conn->send(req, true);
        conn->recv(resp, true);
    }

    void pub(const std::string& topic, message* msg, const bool block = false)
    {
        if (topic == "" || topic == "*")
        {
            for (auto itr = _conns.begin(); itr != _conns.end(); itr++)
                itr->second->send(msg, block);
            return;
        }

        auto itr = _topics.find(topic);
        if (itr == _topics.end())
            return;

        group_cast(msg, itr->second, block);
    }

    bool sub(const std::string& topic, Key id)
    {
        if (topic == "" || topic == "*" || _get(id) == nullptr)
            return false;

        _topics[topic].emplace(id);
        return true;
    }

    bool sub(const std::string& topic, Key id, const unsigned char* ip, const std::uint16_t port)
    {
        auto itr = _topics.find(topic);
        if (itr != _topics.end())
        {
            auto conn_set = itr->second;
            if (conn_set.find(id) != conn_set.end())
                return true;
        }

        auto conn = _get(id);
        if (conn = nullptr)
            conn = connect(ip, port);
        if (conn == nullptr)
            return false;

        _add(id, conn);
        return sub(topic, id);
    }

    void unsub(const std::string& topic, const Key id)
    {
        if (topic == "")
        {
            for (auto itr = _topics.begin(); itr != _topics.end(); itr++)
                itr->second.erase(id);
        }
       
        auto itr = _topics.find(topic);
        if (itr == _topics.end())
            return;

        itr->second.erase(id); 
    }

    void close()
    {
        for (auto itr = _conns.begin(); itr != _conns.end(); ++itr)
        {
            itr->second->close();
            itr->second = nullptr;
        }
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
        unsub("", id);
        _conns.remove(id);
    }

    tcp_conn* _get(const Key id)
    {
        auto itr = _conns.find(id);
        if (itr == _conns.end())
            return nullptr;

        return itr->second;
    }

private:
    std::unordered_map<Key, tcp_conn*> _conns;
    std::unordered_map<std::string, std::set<Key> > _topics;
};

}

#endif