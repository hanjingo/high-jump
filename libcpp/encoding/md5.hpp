#ifndef MD5_HPP
#define MD5_HPP

#include <string.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

#include <openssl/md5.h>

#ifndef MD5_BUF_SZ
#define MD5_BUF_SZ 128 * 1024
#endif

namespace libcpp
{

class md5
{
public:
    enum out_case {
        lower_case,
        upper_case
    };

public:
    explicit md5() {};
    virtual ~md5() {};

    static std::string calc(const std::string& src, 
                            libcpp::md5::out_case ocase = libcpp::md5::lower_case)
    {
        unsigned char hash[MD5_DIGEST_LENGTH];
        MD5_CTX ctx;
        MD5_Init(&ctx);
        MD5_Update(&ctx, src.c_str(), src.size());
        MD5_Final(hash, &ctx);;

        std::stringstream ss;
        for (size_t i = 0; i < MD5_DIGEST_LENGTH; ++i) {
            if (ocase == libcpp::md5::upper_case) {
                ss << std::setiosflags(std::ios::uppercase) << std::hex << std::setw(2)
                   << std::setfill('0') << static_cast<int>(hash[i]);
            } else {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
            }
        }

        return ss.str();
    };

    static void calc(std::istream &in, std::string& out)
    {
        MD5_CTX ctx;
        MD5_Init(&ctx);
        std::streamsize sz;
        std::vector<char> buffer(MD5_BUF_SZ);
        while ((sz = in.read(&buffer[0], MD5_BUF_SZ).gcount()) > 0)
            MD5_Update(&ctx, buffer.data(), sz);

        out.resize(128 / 8);
        MD5_Final(reinterpret_cast<unsigned char*>(&out[0]), &ctx);
    };
};

}

#endif