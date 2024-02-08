#ifndef TCP_PEER_HPP
#define TCP_PEER_HPP

#include <vector>
#include <map>

#include <libcpp/net/tcp/tcp_session.hpp>
#include <libcpp/net/tcp/tcp_interface.hpp>

namespace libcpp
{

template<typename Key = std::unit64_t>
class tcp_peer
{
public:
    using loop_query_cb   = std::function<bool(message*, message*, std::int64_t)>;

public:
    tcp_peer() {}
    ~tcp_peer() {}

    bool add(const Key id, session* sess)
    {
        if (sessions_.find(id) != sessions_.end())
            return false;

        sessions_.insert(id, sess);
    }

    void del(const Key id)
    {
        sessions_.remove(id);
    }

    bool send(const Key id, message& msg)
    {
        std::size_t len = msgs.size();
        unsigned char buf[len];
        encode(buf, len);
        std::copy(w_buf_.end(), buf, buf+len);
    }

    bool recv(const Key id, message& msg)
    {
        auto itr = sessions_.find(id);
        if (itr != sessions_.end())
            return false;

        std::size_t len;
        unsigned char buf[1024];
        for (r_buf_.size() < msg.size())
        {
            len = itr->recv(buf, 1024);
            std::copy(w_buf_.end(), buf, buf+len);
        }
        msg.decode(r_buf_.data(), r_buf_.size());
    }

    void req_resp(const Key id, message& req, message& resp)
    {
        send(id, req);
        recv(id, resp);
    }

    void loop_query(message& req, message& resp, loop_query_cb cb, std::int64_t interval_ms = -1)
    {
        std::int64_t dur = 0;

        send(id, req);
        recv(id, resp);
        while (!cb(req, resp, dur))
        {
            send(id, req);
            recv(id, resp);
        }
    }

    void close()
    {
        for (auto itr = sessions_.begin(); itr != sessions_.end(); ++itr)
        {
            itr->close();
        }
    }

private:
    std::map<Key, session*> sessions_;
    std::vector<unsigned char> w_buf_;
    std::vector<unsigned char> r_buf_;
};

}

#endif