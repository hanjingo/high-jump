#ifndef SHA256_HPP
#define SHA256_HPP

#include <iostream>
#include <vector>
#include <openssl/sha.h>

namespace libcpp
{

class sha256
{
public:
    sha256() {};
    ~sha256() {};

    static void encode(const std::string& src, std::string& dst)
    {
        dst.resize(256 / 8);
        SHA256(reinterpret_cast<const unsigned char *>(&src[0]), 
               src.size(), 
               reinterpret_cast<unsigned char *>(&dst[0]));
    };

    static void encode(std::istream& in, std::string& dst)
    {
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        std::streamsize sz;
        std::vector<char> buf(1024 * 128);
        while ((sz = in.read(&buf[0], sz).gcount()) > 0)
            SHA256_Update(&ctx, buf.data(), sz);

        dst.resize(256 / 8);
        SHA256_Final(reinterpret_cast<unsigned char *>(&dst[0]), &ctx);
    };
};

}

#endif