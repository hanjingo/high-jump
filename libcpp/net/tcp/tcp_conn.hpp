#ifndef TCP_CONN_HPP
#define TCP_CONN_HPP

#include <vector>

#include <libcpp/net/tcp/tcp_chan.hpp>
#include <libcpp/net/tcp/tcp_session.hpp>
#include <libcpp/net/proto/message.hpp>

namespace libcpp
{

class tcp_conn
{
public:
    typedef void(*conn_cb)();
    typedef void(*send_cb)(message*);
    typedef void(*recv_cb)(message*);
    typedef void(*close_cb)();

    enum event_t {
        event_start = 0,
        event_connecting,
        event_connected,
        event_closing,
        event_closed,
        event_end
    };

public:
    tcp_conn() {}
    ~tcp_conn() {}

    bool connect(const char* ip, const std::uint16_t port)
    {
        return sock_.connect(ip, port);
    }

    void disconnect()
    {
        if (w_closed_.load() || r_closed_.load())
            return;

        w_closed_.store(true);
        r_closed_.store(true);
    }

    void send(message* msg, const bool block = true)
    {
        if (w_closed_.load())
            return;

        w_ch_ << msg;
        if (!block)
            return;

        for (w_ch_ >> msg; msg != nullptr; w_ch_ >> msg) 
        {
            w_len_ = msg->encode(w_buf_, 1024);
            sock_.send(w_buf_, w_len_);
        }
    }

    void recv(message* msg, const bool block = true)
    {
        if (r_closed_.load())
            return;

        r_ch_ << msg;
        if (!block)
            return;
        
        for (r_ch_ >> msg; msg != nullptr; r_ch_ >> msg)
        {
            w_len_ = sock_.recv(r_buf_, 1024);
            msg->decode(w_buf_, w_len_);
        }
    }

    void close()
    {
        if (!w_closed_.load())
            w_closed_.store(true);
        message* msg = nullptr;
        for (w_ch_ >> msg; msg != nullptr; w_ch_ >> msg)
        {
            w_len_ = msg->encode(w_buf_, 1024);
            sock_.send(w_buf_, w_len_);
        }

        if (!r_closed_.load())
            r_closed_.store(true);
        for (r_ch_ >> msg; msg != nullptr; r_ch_ >> msg)
        {
            r_len_ = msg->decode(r_buf_, 1024);
            sock_.recv(r_buf_, r_len_);
        }
    }

    template<typename Fn>
    void add_event_handler(const event_t evt, Fn fn)
    {
        handlers_[evt] = fn;
    }

private:
    tcp_session sock_;
    void* handlers_[event_end];

    std::atomic<bool> w_closed_{true};
    std::atomic<bool> r_closed_{true};

    std::size_t w_len_;
    unsigned char w_buf_[1024];
    std::size_t r_len_;
    unsigned char r_buf_[1024];

    tcp_chan<message*> w_ch_;
    tcp_chan<message*> r_ch_;
};

}

#endif