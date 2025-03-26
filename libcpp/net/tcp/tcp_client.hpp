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
    using io_t = boost::asio::io_context;

public:
    tcp_client() : _io{} {}
    ~tcp_client() { close(); }

    inline std::size_t size() { return _conns.size(); }
    inline tcp_conn* get(Key id) { return _get(id); }
    inline std::size_t add(Key id, tcp_conn* conn) { _add(id, conn); return _conns.size(); }

    tcp_conn* connect(const char* ip, 
                      const std::uint16_t port,
                      const std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
                      int retry_times = 1)
    {
        tcp_conn* conn = new tcp_conn(&_io);
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

    bool send(message* msg, Key id, bool noblock = false)
    {
        auto conn = _get(id);
        if (conn == nullptr)
            return false;

        if (noblock)
            return conn->async_send(msg);
        else 
            return conn->send(msg);
    }

    bool recv(message* msg, Key id, bool noblock = false)
    {
        auto conn = _get(id);
        if (conn == nullptr)
            return false;

        if (noblock)
            return conn->recv(msg);
        else
            return conn->async_recv(msg);
    }

    std::size_t broad_cast(message* msg)
    {
        std::size_t nsend = 0;
        for (auto itr = _conns.begin(); itr != _conns.end(); itr++)
            nsend = (itr->second->async_send(msg) ? nsend + 1 : nsend);
        return nsend;
    }

    std::size_t group_cast(message* msg, std::set<Key>& ids)
    {
        std::vector<Key> li{ids.begin(), ids.end()};
        return group_cast(msg, li);
    }

    std::size_t group_cast(message* msg, std::vector<Key> ids)
    {
        std::size_t nsend = 0;
        for (Key id : ids)
        {
            auto conn = _get(id);
            if (conn == nullptr)
                continue;

            if (conn->async_send(msg))
                nsend++;
        }
        return nsend;
    }

    bool req_resp(message* req, message* resp, const Key id)
    {
        auto conn = _get(id);
        if (conn == nullptr)
            return false;

        return conn->send(req) && conn->recv(resp);
    }

    void pub(const std::string& topic, message* msg)
    {
        if (topic == "" || topic == "*")
        {
            for (auto itr = _conns.begin(); itr != _conns.end(); itr++)
                itr->second->async_send(msg);
            return;
        }

        auto itr = _topics.find(topic);
        if (itr == _topics.end())
            return;

        group_cast(msg, itr->second);
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
        if (_closed.load())
            return;

        _closed.store(true);
        for (auto itr = _conns.begin(); itr != _conns.end(); itr++)
        {
            if (itr->second == nullptr)
                continue;

            itr->second->close();
            delete itr->second;
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
    io_t              _io;
    std::atomic<bool> _closed{false};
    std::unordered_map<Key, tcp_conn*> _conns;
    std::unordered_map<std::string, std::set<Key> > _topics;
};

}

#endif