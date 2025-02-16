#ifndef HEX_HPP
#define HEX_HPP

#include <string.h>

#include <iostream>
#include <sstream>

namespace libcpp
{
class hex
{
public:
    template<typename T>
    static T from_str(const std::string& str)
    {
        return libcpp::hex::from_str<T>(str.c_str());
    }

    template<typename T>
    static T from_str(const char* buf)
    {
        T t;
        std::stringstream ss;
        ss << std::hex << buf;
        ss >> t;
        return t;
    }

    template<typename T>
    static std::string to_str(const T& t, bool upper_case = true)
    {
        std::ostringstream ss;
        ss << (upper_case ? std::uppercase : std::nouppercase) << std::hex << t;
        return ss.str();
    }

    template<typename T>
    static char* to_str(const T& t, char* buf, bool upper_case = true)
    {
        auto str = libcpp::hex::to_str(t, upper_case);
        memcpy(buf, str.c_str(), str.length());
        return buf;
    }
};

}

#endif