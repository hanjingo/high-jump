#ifndef ZMQ_EVENT_HPP
#define ZMQ_EVENT_HPP

#include <string>
#include <sstream>
#include <functional>
#include <unordered_map>

#include <zmq.h>
#include <zmqpp/zmqpp.hpp>

namespace libcpp
{

namespace event = zmqpp::event;

struct zmq_event {
    uint16_t    Event;
    uint32_t    Value;
    std::string Address;

    zmq_event(const zmqpp::message& msg)
    {
        if (msg.parts() < 2 || msg.get(0).size() < 6) {
            return;
        }

        uint8_t data[6];
        memcpy(data, msg.get(0).c_str(), 6);
        Event   = *(uint16_t*)(data);
        Value   = *(uint32_t*)(data + 2);
        Address = msg.get(1);
    }
    ~zmq_event() {}
};

class zmq_monitor
{
public:
    zmq_monitor(zmqpp::context& ctx, zmqpp::socket& sock)
        : loop_{}
        , sock_{ctx, zmqpp::socket_type::pair}
    {
        std::ostringstream ss;
        ss << std::hex << this;
        std::string id = "inproc://" + ss.str();

        sock.monitor(id, zmqpp::event::all);
        sock_.connect(id);

        loop_.add(sock_, std::bind(&zmq_monitor::on_sock_event, this));
    }
    ~zmq_monitor() {}

    virtual bool on_sock_event()
    {
        while (true) {
            zmqpp::message msg;
            if (!sock_.receive(msg, true)) {
                return true;
            }

            zmq_event event{msg};

            auto itr = m_event_.find(event.Event);
            if (itr == m_event_.end()) {
                return true;
            }

            return itr->second(event);
        }
    }

    inline void loop_start()
    {
        loop_.start();
    }

    void add_event_handler(uint16_t key, std::function<bool(libcpp::zmq_event&)>&& fn)
    {
        m_event_[key] = std::move(fn);
    }

private:
    zmqpp::loop                                                           loop_;
    zmqpp::socket                                                         sock_;
    std::unordered_map<uint16_t, std::function<bool(libcpp::zmq_event&)>> m_event_;
};

}

#endif