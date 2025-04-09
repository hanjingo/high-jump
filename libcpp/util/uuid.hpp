#ifndef UUID_HPP
#define UUID_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace libcpp
{

static boost::uuids::random_generator gen_rand;

class uuid
{

public:
    uuid() {}
    ~uuid() {}

    static std::string gen()
    {
        return boost::uuids::to_string(gen_rand());
    }

    static unsigned long long gen_u64(bool big_endian = true)
    {
        auto bytes = gen_rand().data;

        unsigned long long ull = 0;
        if (big_endian) {
            ull  = bytes[7] & 0xFF;
            ull |= ((bytes[6] << 8) & 0xFF00);
            ull |= ((bytes[5] << 16) & 0xFF0000);
            ull |= ((bytes[4] << 24) & 0xFF000000);
            ull |= ((((long long) bytes[3]) << 32) & 0xFF00000000);
            ull |= ((((long long) bytes[2]) << 40) & 0xFF0000000000);
            ull |= ((((long long) bytes[1]) << 48) & 0xFF000000000000);
            ull |= ((((long long) bytes[0]) << 56) & 0xFF00000000000000);
        } else {
            ull  = bytes[0] & 0xFF;
            ull |= ((bytes[1] << 8) & 0xFF00);
            ull |= ((bytes[2] << 16) & 0xFF0000);
            ull |= ((bytes[3] << 24) & 0xFF000000);
            ull |= ((((long long) bytes[4]) << 32) & 0xFF00000000);
            ull |= ((((long long) bytes[5]) << 40) & 0xFF0000000000);
            ull |= ((((long long) bytes[6]) << 48) & 0xFF000000000000);
            ull |= ((((long long) bytes[7]) << 56) & 0xFF00000000000000);
        }
        return ull;
    }
};

}

#endif