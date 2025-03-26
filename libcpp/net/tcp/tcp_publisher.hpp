#ifndef TCP_PUBLISHER_HPP
#define TCP_PUBLISHER_HPP

#include <string>
#include <libcpp/net/proto/message.hpp>

namespace libcpp
{

class tcp_publisher
{
public:
    using topic_t = std::string;

public:
    void pub(const topic_t& topic, message* msg)
    {
        
    }
};

}

#endif