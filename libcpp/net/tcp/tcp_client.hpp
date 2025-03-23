#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <set>
#include <unordered_map>

#include <libcpp/net/tcp/tcp_conn.hpp>
#include <libcpp/net/proto/message.hpp>

namespace libcpp
{

template<typename Key = std::uint64_t>
class tcp_client
{
public:
    typedef void(*pub_cb)();

public:
    tcp_client() {}
    ~tcp_client() {}

    inline tcp_conn* get(Key id)
    {
        auto itr = _conns.find(id);
        if (itr != _conns.end())
            return nullptr;

        return itr->second;
    }

    tcp_conn* connect(const char* ip, 
                      const std::uint16_t port,
                      Key id, 
                      const std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
                      int retry_times = 1)
    {
        tcp_conn* conn = new tcp_conn();
        if (!conn->connect(ip, port))
        {
            delete conn;
            conn = nullptr;
            return conn;
        }

        _add(id, conn);
        return conn;
    }

    void disconnect(const Key id)
    {
        auto conn = _get(id);
        if (conn == nullptr)
            return;

        conn->disconnect();
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

        conn->recv(msg, block);
        return true;
    }

    void broad_cast(message* msg, const bool block = false)
    {
        for (auto itr = _conns.begin(); itr != _conns.end(); itr++)
            itr->second->send(msg, block);
    }

    void group_cast(message* msg, const std::vector<Key>& ids, const bool block = false)
    {
        for (Key id : ids)
        {
            auto conn = _get(id);
            if (conn == nullptr)
                continue;

            conn->send(msg, block);
        }
    }

    void req_resp(message* req, message& resp, const Key target)
    {
        auto conn = _get(id);
        if (conn == nullptr)
            return;

        conn->send(req, true);
        conn->recv(resp, true);
    }

    void pub(const std::string& topic, message* msg, const pub_cb cb = nullptr, const bool block = false)
    {
        if (topic == "" || topic == "*")
        {
            for (auto itr = _conns.begin(); itr != _conns.end(); itr++)
                itr->second->send(msg, block);
            return;
        }

        auto topic = _topics.find(topic);
        if (topic != _topics.end())
            return;

        for (auto conn = topic->second.begin(); conn != topic->second.end(); ++conn)
            (*conn)->send(msg, block);
    }

    bool sub(const std::string& topic, tcp_conn* conn)
    {
        if (topic == "" || topic == "*")
            return false;

        _topics[topic].emplace(conn);
        return true;
    }

    bool sub(const std::string& topic, const unsigned char* ip, const std::uint16_t port)
    {
        auto conn = connect(ip, port);
        if (conn == nullptr)
            return false;

        return sub(topic, conn);
    }

    void unsub(const std::string& topic, const Key id)
    {
        auto conn = _get(id);
        if (conn == nullptr)
            return;

        if (topic != "")
        {
            auto topic = _topics.find(topic);
            if (topic == _topics.end())
                return;

            topic->second.erase(conn);
            return;
        }

        for (auto topic = _topics.begin(); topic != _topics.end(); topic++)
        {
            topic->second.erase(conn);
        }
    }

    void close()
    {
        for (auto itr = _conns.begin(); itr != _conns.end(); ++itr)
            itr->close();
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
    std::unordered_map<std::string, std::set<tcp_conn*>> _topics;
};

}

#endif