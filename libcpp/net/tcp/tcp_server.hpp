#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <atomic>
#include <initializer_list>

#include <libcpp/net/proto/message.hpp>
#include <libcpp/net/tcp/tcp_listener.hpp>
#include <libcpp/net/tcp/tcp_conn.hpp>
#include <libcpp/net/tcp/tcp_muxer.hpp>

namespace libcpp
{

template<typename Key = std::uint64_t>
class tcp_server
{
public:
    using io_t                 = tcp_conn::io_t;
    using io_work_t            = tcp_conn::io_work_t;
    using conn_ptr_t           = tcp_conn*;
    using server_ptr_t         = tcp_server*;
    using listen_handler_t     = std::function<void(server_ptr_t, conn_ptr_t)>;

public:
    tcp_server()
        : _io{}
        , _listener{_io}
    {
    }
    
    ~tcp_server()
    {
        close();
    }

    inline std::size_t size() { return _conns.size(); }
    inline std::size_t add(const Key id, conn_ptr_t conn) { _add(id, conn); return size(); }
    inline conn_ptr_t get(const Key id) { return _get(id); }
    inline void poll() { _io.run(); }
    inline void loop_start() { io_work_t work{_io}; _io.run(); }
    inline void loop_end() { _io.stop(); }

    conn_ptr_t listen(const char* ip, const uint16_t port)
    {
        auto sock_ptr = _listener.accept(ip, port);
        if (sock_ptr == nullptr)
            return nullptr;

        auto ret = new tcp_conn(_io, sock_ptr);
        return ret;
    }

    void async_listen(const char* ip, const uint16_t port, listen_handler_t fn)
    {
        _listener.async_accept(ip, port, [this, fn](const tcp_listener::err_t& err, tcp_socket* sock) {
            if (err.failed())
            {
                fn(this, nullptr);
                return;
            }
            
            conn_ptr_t conn = new tcp_conn(_io, sock);
            fn(this, conn);
        });
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
        _listener.close();
    }

    bool send(message* msg, const Key id, bool noblock = false)
    {
        auto conn = _get(id);
        if (conn == nullptr)
            return false;

        if (noblock)
            return conn->async_send(msg);
        else
            return conn->send(msg);
    }

    bool recv(message* msg, const Key id, bool noblock = false)
    {
        auto conn = _get(id);
        if (conn == nullptr)
            return false;

        if (noblock)
            return conn->async_recv(msg);
        else
            return conn->recv(msg);
    }

    bool recv(message* msg)
    {
        for (auto itr = _conns.begin(); itr != _conns.end(); itr++)
        {
            if (itr->second == nullptr || !itr->second->is_connected())
                continue;

            if (!itr->second->async_recv(msg))
                return false;
        }

        return true;
    }

    void broad_cast(message* msg)
    {
        for (auto itr = _conns.begin(); itr != _conns.end(); itr++)
            itr->second->async_send(msg);            
    }

    void group_cast(message* msg, std::initializer_list<Key> ids)
    {
        for (auto id : ids)
            async_send(msg, id);
    }

    void kick_off(std::initializer_list<Key> ids)
    {
        for (Key id : ids)
            _del(id);
    }

private:
    bool _add(const Key id, conn_ptr_t conn)
    {
        if (conn == nullptr || _conns.find(id) != _conns.end())
            return false;

        _conns.emplace(id, conn);
        return true;
    }

    void _del(const Key id)
    {
        auto itr = _conns.find(id);
        if (itr == _conns.end())
            return;

        if (itr->second != nullptr)
        {
            itr->second->disconnect();
            delete itr->second;
            itr->second = nullptr;
        }
        _conns.erase(itr);
    }

    conn_ptr_t _get(const Key id)
    {
        auto itr = _conns.find(id);
        if (itr == _conns.end())
            return nullptr;

        return itr->second;
    }

private:
    io_t                               _io;
    std::atomic<bool>                  _closed{false};
    libcpp::tcp_listener               _listener;
    std::unordered_map<Key, conn_ptr_t> _conns;
};

}

#endif