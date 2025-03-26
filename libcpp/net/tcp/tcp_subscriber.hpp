#ifndef TCP_SUBSCRIBER_HPP
#define TCP_SUBSCRIBER_HPP

#include <string>
#include <libcpp/net/proto/message.hpp>

namespace libcpp
{

class tcp_subscriber
{
public:
    using topic_t = std::string;

public:
    void sub(const topic_t& topic, message* msg)
    {
        
    }

    void unsub(const topic_t& topic)
    {
        
    }
};

}

#endif
#endif