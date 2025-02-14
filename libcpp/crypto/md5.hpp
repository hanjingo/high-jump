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
    explicit md5(bool upper_case) : _upper_case{upper_case} {};
    virtual ~md5() {};

    template<typename T>
    static inline friend std::ostream& operator<<(std::ostream& out, const T& src) 
    { 
        out << md5::calc(src);
        return out;
    }

    template<typename T>
    static inline friend std::istream& operator>>(std::istream& in, T& dst) 
    { 
        md5::calc(in, dst);
        return in;
    }

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
    };

    static std::string calc(const std::string& src)
    {
        std::string hash;
        hash.resize(16);
        MD5(reinterpret_cast<const unsigned char *>(&src[0]), 
            src.size(), 
            reinterpret_cast<unsigned char *>(&hash[0]));

        return hash;
    };

    static void calc(std::istream &in, std::string& out)
    {
        MD5_CTX ctx;
        MD5_Init(&ctx);
        std::streamsize sz;
        std::vector<char> buffer(buf_sz);
        while ((sz = in.read(&buffer[0], buf_sz).gcount()) > 0)
            MD5_Update(&ctx, buffer.data(), sz);

        out.resize(128 / 8);
        MD5_Final(reinterpret_cast<unsigned char*>(&out[0]), &context);
    };

    static const size_t buf_sz = 128 * 1024;
    static libcpp::md5 LE{true};
    static libcpp::md5 BE{true};

private:
    bool _upper_case;
};

}

#endif