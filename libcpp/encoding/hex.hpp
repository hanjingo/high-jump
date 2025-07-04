/*
 *  This file is part of libcpp.
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HEX_HPP
#define HEX_HPP

#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace libcpp
{
class hex
{
public:
    template<typename T>
    static T from(const std::string& str)
    {
        return libcpp::hex::from_str<T>(str.c_str());
    }

    template<typename T>
    static T from(const char* buf)
    {
        T t;
        std::stringstream ss;
        ss << std::hex << buf;
        ss >> t;
        return t;
    }

    static std::ostream& from(std::ostream& out, std::istream& in, bool upper_case = true)
    {
        if (!in.good())
            return out;

        out << (upper_case ? std::uppercase : std::nouppercase) << std::hex;
        char c1, c2;
        while (true) 
        {
            if (!in.get(c1)) 
                break;

            if (!in.get(c2)) 
            {
                // padding 0
                unsigned int byte = 0;
                std::istringstream iss(std::string("0") + c1);
                iss >> std::hex >> byte;
                out.put(static_cast<char>(byte));
                break;
            }
            unsigned int byte = 0;
            std::istringstream iss(std::string() + c1 + c2);
            iss >> std::hex >> byte;
            out.put(static_cast<char>(byte));
        }
        
        out.flush();
        return out;
    }

    template<typename RET, typename T>
    static RET to(const T& t, bool upper_case = true)
    {
        std::ostringstream ss;
        ss << (upper_case ? std::uppercase : std::nouppercase) << std::hex << t;
        return ss.str();
    }

    template<typename RET, typename T>
    static RET to(const T& t, char* buf, bool upper_case = true)
    {
        auto str = libcpp::hex::to_str(t, upper_case);
        memcpy(buf, str.c_str(), str.length());
        return buf;
    }

    static std::ostream& to(std::ostream& out, std::istream& in, bool upper_case = true)
    {
        if (!in.good())
            return out;

        out << (upper_case ? std::uppercase : std::nouppercase) << std::hex;
        unsigned char byte;
        while (in.read(reinterpret_cast<char*>(&byte), 1)) 
        {
            char buf[3];
            snprintf(buf, sizeof(buf), "%02X", byte);
            out.write(buf, 2);
        }
        
        out.flush();
        return out;
    }

private:
    hex() = default;
    ~hex() = default;
    hex(const hex&) = delete;
    hex& operator=(const hex&) = delete;
    hex(hex&&) = delete;
    hex& operator=(hex&&) = delete;
};

}

#endif