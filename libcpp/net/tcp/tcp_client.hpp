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
        auto itr = conns_.find(id);
        if (itr != conns_.end())
            return nullptr;

        return itr->second;
    }

    tcp_conn* connect(const char* ip, 
                      const std::uint16_t port,
                      Key id, 
                      const std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
                      int retry_times = 1)
    {
        tcp_conn* conn = new tcp_conn();
        if (!conn->connect(ip, port))
        {
            delete conn;
            conn = nullptr;
            return conn;
        }

        add_(id, conn);
        return conn;
    }

    void disconnect(Key id)
    {
        auto itr = conns_.find(id);
        if (itr != conns_.end())
            return;

        itr->second->disconnect();
    }

    bool send(message* msg, const Key id, const bool block = true)
    {
        auto itr = conns_.find(id);
        if (itr != conns_.end())
            return false;

        itr->second->send(msg, block);
        return true;
    }

    bool recv(message* msg, const Key id, const bool block = true)
    {
        auto itr = conns_.find(id);
        if (itr != conns_.end())
            return false;

        itr->second->recv(msg, block);
        return true;
    }

    void broad_cast(message* msg, const bool block = false)
    {
        for (auto itr = conns_.begin(); itr != conns_.end(); itr++)
            itr->second->send(msg, block);
    }

    void group_cast(message* msg, const std::vector<Key>& ids, const bool block = false)
    {
        for (auto itr = ids.begin(); itr != ids.end(); itr++)
        {
            auto conn = conns_.find(*itr);
            if (conn == conns_.end())
                continue;
            conn->second->send(msg, block);
        }
    }

    void req_resp(message* req, message& resp, const Key target)
    {
        auto itr = conns_.find(target);
        if (itr != conns_.end())
            return;

        itr->second->send(req, true);
        itr->second->recv(resp, true);
    }

    void pub(const std::string& topic, message* msg, const pub_cb cb = nullptr, const bool block = false)
    {
        if (topic == "" || topic == "*")
        {
            for (auto itr = conns_.begin(); itr != conns_.end(); itr++)
                itr->second->send(msg, block);
            return;
        }

        auto itr = topics_.find(topic);
        if (itr != topics_.end())
            return;

        for (auto conn = itr->second.begin(); conn != itr->second.end(); ++conn)
            (*conn)->send(msg, block);
    }

    bool sub(const std::string& topic, tcp_conn* conn)
    {
        if (topic == "" || topic == "*")
        {
            return false;
        }

        topics_[topic].insert(conn);
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
        auto itr = conns_.find(id);
        if (itr == conns_.end())
            return;

        if (topic != "")
        {
            auto itr1 = topics_.find(topic);
            if (itr1 == topics_.end())
                return;

            itr1->second.erase(itr->second);
            return;
        }

        for (auto itr1 = topics_.begin(); itr1 != topics_.end(); itr1++)
        {
            itr1->second.erase(itr->second);
        }
    }

    void close()
    {
        for (auto itr = conns_.begin(); itr != conns_.end(); ++itr)
        {
            itr->close();
        }
    }

private:
    bool add_(const Key id, tcp_conn* conn)
    {
        if (conn == nullptr || conns_.find(id) != conns_.end())
            return false;

        conns_.insert(id, conn);
        return true;
    }

    void del_(const Key id)
    {
        auto itr = conns_.remove(id);
        if (itr == conns_.end())
            return;

        unsub("", id);
    }

private:
    std::unordered_map<Key, tcp_conn*> conns_;
    std::unordered_map<std::string, std::set<tcp_conn*>> topics_;
};

}

#endif