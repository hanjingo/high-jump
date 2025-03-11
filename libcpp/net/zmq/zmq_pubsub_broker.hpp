#ifndef ZMQ_PUBSUB_BROKER_HPP
#define ZMQ_PUBSUB_BROKER_HPP

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <iostream>

#include <libcpp/net/zmq/zmq_chan.hpp>

#ifndef MAX_PUBSUB_BROKER_BUF_SZ 
#define MAX_PUBSUB_BROKER_BUF_SZ 4096
#endif

namespace libcpp
{

static const std::string UNFILT(1, 0x01);

class zmq_pubsub_broker
{
public:
    using callable_t = std::function<bool(void)>;

public:
    zmq_pubsub_broker(void* ctx)
        : _ctx{ctx}
        , _xpub_sock{zmq_socket(ctx, ZMQ_XPUB)}
        , _xsub_sock{zmq_socket(ctx, ZMQ_XSUB)}
    {
        int opt = 1;
        zmq_setsockopt(_xpub_sock, ZMQ_XPUB_VERBOSE, &opt, sizeof(opt)); // SET ALL PASS
        zmq_send(_xpub_sock, UNFILT.data(), UNFILT.size(), 0);
    }
    virtual ~zmq_pubsub_broker()
    {
        if (_xpub_sock != nullptr)
        {
            zmq_close(_xpub_sock);
            _xpub_sock = nullptr;
        }
        if (_xpub_sock != nullptr)
        {
            zmq_close(_xsub_sock);
            _xsub_sock = nullptr;
        }
    }

    inline int bind(const std::string& xpub_addr, const std::string& xsub_addr)
    {
        int ret = zmq_bind(_xpub_sock, xpub_addr.c_str());
        if (ret != 0)
            return ret;

        ret = zmq_bind(_xsub_sock, xsub_addr.c_str());
        if (ret != 0)
            return ret;

        const zmq_pollitem_t xsub_event { _xsub_sock, 0, ZMQ_POLLIN, 0 };
        callable_t xsub_cb = std::bind(&zmq_pubsub_broker::on_xsub, this, _xpub_sock, _xsub_sock);
        add_poll_item(xsub_event, std::move(xsub_cb));

        const zmq_pollitem_t xpub_event { _xpub_sock, 0, ZMQ_POLLIN, 0 };
        callable_t xpub_cb = std::bind(&zmq_pubsub_broker::on_xpub, this, _xpub_sock, _xsub_sock);
        add_poll_item(xpub_event, std::move(xpub_cb));
    }

    inline void add_poll_item(zmq_pollitem_t item, callable_t&& fn)
    {
        _callbacks[item.socket] = std::move(fn);
        _items.push_back(item);
    }

    inline void poll(long timeout_ms = -1)
    {
        callable_t cb;
        while (zmq_poll(_items.data(), _items.size(), timeout_ms) > 0) 
        {
            for (auto item : _items)
            {
                if (item.revents & ZMQ_POLLIN)
                {
                    cb = _callbacks[item.socket];
                    cb();
                    continue;
                }
            }
        };
    }

public:
    virtual bool on_xsub(void* xpub_sock, void* xsub_sock)
    {
        std::cout << "on_xsub start" << std::endl;
        zmq_msg_t msg;
        while (true) 
        {
            if (zmq_msg_recv(&msg, xsub_sock, ZMQ_NOBLOCK) < 0)
                break;

            if (zmq_msg_send(&msg, xpub_sock, ZMQ_NOBLOCK) < 0)
                break;
        }
        std::cout << "on_xsub end" << std::endl;
        return true;
    }
    virtual bool on_xpub(void* xpub_sock, void* xsub_sock)
    {
        std::cout << "on_xpub start" << std::endl;
        int nbytes;
        while (true)
        {
            nbytes = zmq_recv(xpub_sock, _buf, MAX_PUBSUB_BROKER_BUF_SZ, ZMQ_NOBLOCK);
            if (nbytes < 0 || nbytes > MAX_PUBSUB_BROKER_BUF_SZ - 2)
                break;

            for (int i = nbytes; i > 0; --i)
                _buf[i] = _buf[i - 1];
            _buf[0] = 0x01;
            zmq_send(xsub_sock, _buf, nbytes + 1, ZMQ_NOBLOCK);
        }
        std::cout << "on_xpub end" << std::endl;
        return true;
    }

private:
    void*     _ctx;
    void*     _xpub_sock;
    void*     _xsub_sock;
    char      _buf[MAX_PUBSUB_BROKER_BUF_SZ];

    std::vector<zmq_pollitem_t> _items;
    std::unordered_map<void*, callable_t> _callbacks;
};

}

#endif