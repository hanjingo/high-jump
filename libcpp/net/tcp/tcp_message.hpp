#ifndef TCP_MESSAGE_HPP
#define TCP_MESSAGE_HPP

#include <libcpp/net/tcp/tcp_interface.hpp>

namespace libcpp
{

class tcp_message : public message
{
public:
    enum flag
    {
        ACK     = 0,
        DATA    = 1 << 1,
        CTRL    = 1 << 2,
        VITAL   = 1 << 3,
        RESERVE = 1 << 4
    };

public:
    tcp_message()
    {

    }
    virtual ~tcp_message()
    {

    }

    virtual std::size_t size()
    {

    }

    virtual std::size_t encode(unsigned char* buf, std::size_t len)
    {

    }

    virtual bool decode(const unsigned char* buf, std::size_t len)
    {

    }

public:
    int copy(tcp_message& src)
    {

    }

    int move_from(tcp_message& src)
    {

    }

    void close()
    {

    }
}

}

#endif