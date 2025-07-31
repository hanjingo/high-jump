#ifndef BITS_HPP
#define BITS_HPP

#include <string.h>
#include <string>
// #include <bitset>

namespace libcpp
{

class bits
{
public:
    template<typename T>
    static bool get(const T src, const unsigned int pos)
    {
        return (src >> (pos - 1)) & 1;
    }

    template<typename T>
    static T& put(T& src, const unsigned int pos)
    {
        return src |= (1 << (pos - 1));
    }

    template<typename T>
    static T& put(T& src, const unsigned int pos, const bool bit)
    {
        return bit ? src |= (1 << (pos - 1)) : src &= (~(1 << (pos - 1)));
    }

    template<typename T>
    static T& reset(T& src, const bool bit)
    {
        src = bit ? (~T(0)) : (0);
        return src;
    }

    template<typename T>
    static T& flip(T& src)
    {
        src = ~(src);
        return src;
    }

    template<typename T>
    static void to_string(const T& src, char* buf)
    {
        unsigned int sz = sizeof(src) * 8;
        memset(buf, '0', sz);
        for (int pos = sz - 1; pos >= 0; pos--) {
            buf[sz - pos - 1] += ((src >> pos) & 1);
        }

        buf[sz] = '\0';
    }

    template<typename T>
    static std::string to_string(const T& src)
    {
        char buf[sizeof(src) * 8 + 1];
        libcpp::bits::to_string(src, buf);
        return std::string(buf);
    }

    // template<typename T>
    // static std::string to_string(const T& src)
    // {
    //     std::bitset<sizeof(src) * 8> buf;
    //     return buf.to_string();
    // }

    template<typename T>
    static int count_leading_zeros(const T& src)
    {
        // TODO
    }
};

}

#endif