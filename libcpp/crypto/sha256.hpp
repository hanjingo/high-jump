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

    static const std::string& encode(const std::string& src)
    {
        std::string hash;
        hash.resize(256 / 8);
        SHA256(reinterpret_cast<const unsigned char *>(&src[0]), 
               src.size(), 
               reinterpret_cast<unsigned char *>(&hash[0]));
        return hash;
    };

    static const std::string& encode(std::istream& in)
    {
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        std::streamsize sz;
        std::vector<char> buf(1024 * 128);
        while ((sz = in.read(&buf[0], sz).gcount()) > 0)
            SHA256_Update(&ctx, buf.data(), sz);

        std::string hash;
        hash.resize(256 / 8);
        SHA256_Final(reinterpret_cast<unsigned char *>(&hash[0]), &ctx);
        return hash;
    };
};

}

#endif