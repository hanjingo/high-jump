#ifndef MD5_HPP
#define MD5_HPP

#include <string.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

#include <openssl/md5.h>

namespace libcpp
{

class md5
{
public:
    static std::string calc(const std::string& src, bool to_upper_case = false)
    {
        unsigned char hash[MD5_DIGEST_LENGTH];
        MD5_CTX ctx;
        MD5_Init(&ctx);
        MD5_Update(&ctx, src.c_str(), src.size());
        MD5_Final(hash, &ctx);;

        std::stringstream ss;
        for (size_t i = 0; i < MD5_DIGEST_LENGTH; ++i) {
            if (to_upper_case) {
                ss << std::setiosflags(std::ios::uppercase) << std::hex << std::setw(2)
                   << std::setfill('0') << static_cast<int>(hash[i]);
            } else {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
            }
        }

        return ss.str();
    }
};

}

#endif