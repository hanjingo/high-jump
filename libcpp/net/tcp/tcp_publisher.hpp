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
    bool bind(conn_ptr_t conn)
    {

    }

    bool detach(conn_ptr_t conn)
    {
        
    }

    void pub(const topic_t& topic, message* msg)
    {
        
    }
};

}

#endif