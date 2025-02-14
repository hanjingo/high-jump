#ifndef PROTO_FIX_HPP
#define PROTO_FIX_HPP

#define PROTO_FIX_VERSION 4_2

// implement fix protocol v4.2: https://www.borsaitaliana.it/borsaitaliana/gestione-mercati/migrazioneidem/fix42specification.pdf

#include <libcpp/net/proto/message.hpp>

namespace libcpp
{

class fix_proto : public message
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

#pragma pack(push,1)
    struct head {
        uint8_t  start_flag     = 0xAA;
        uint16_t len            = 0;
        uint8_t  ctl            = 0;
    };

    struct data {

    };
#pragma pack(pop)

public:
    fix_proto()
    {

    }
    virtual ~fix_proto()
    {

    }

    virtual std::size_t size()
    {

    }

    virtual std::size_t encode(unsigned char* buf, const std::size_t len)
    {

    }

    virtual bool decode(const unsigned char* buf, const std::size_t len)
    {

    }

public:
    int copy_from(const fix_proto& src)
    {

    }

    int move_from(const fix_proto& src)
    {

    }

    void close()
    {

    }

    fix_proto next()
    {

    }

    fix_proto front()
    {
        
    }

};

}

#endif