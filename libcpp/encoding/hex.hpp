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

#include <algorithm>
#include <vector>
#include <type_traits>

#ifndef HEX_BUF_SZ
#define HEX_BUF_SZ 4096
#endif

namespace libcpp
{
class hex
{
public:
    template<typename T = std::string>
    static T decode(const std::string& str)
    {   
        std::string hex_str;
        hex_str.reserve(str.length());
        
        for (char c : str) 
        {
            if (hex_char_to_value_(c) >= 0)
                hex_str += c;
        }

        if (hex_str.length() % 2 != 0) 
            hex_str = "0" + hex_str;
        
        if constexpr (std::is_same_v<T, std::string>) 
        {
            T result;
            result.reserve(hex_str.length() / 2);
            
            for (std::size_t i = 0; i < hex_str.length(); i += 2) 
            {
                int high = hex_char_to_value_(hex_str[i]);
                int low = hex_char_to_value_(hex_str[i + 1]);
                
                if (high >= 0 && low >= 0) 
                {
                    unsigned char byte_value = static_cast<unsigned char>((high << 4) | low);
                    result.push_back(static_cast<char>(byte_value));
                }
            }
            return result;
        }
        else if constexpr (std::is_same_v<T, std::vector<unsigned char>>) 
        {
            T result;
            result.reserve(hex_str.length() / 2);
            
            for (std::size_t i = 0; i < hex_str.length(); i += 2) 
            {
                int high = hex_char_to_value_(hex_str[i]);
                int low = hex_char_to_value_(hex_str[i + 1]);
                
                if (high >= 0 && low >= 0) 
                {
                    unsigned char byte_value = static_cast<unsigned char>((high << 4) | low);
                    result.push_back(byte_value);
                }
            }
            return result;
        }
        else if constexpr (std::is_integral_v<T>) 
        {
            T result = 0;
            int value = 0;
            for (char c : hex_str) 
            {
                value = hex_char_to_value_(c);
                if (value >= 0) 
                    result = (result << 4) | value;
            }
            return result;
        }
        else 
        {
            T result;
            int high = 0, low = 0;;
            for (std::size_t i = 0; i < hex_str.length(); i += 2) 
            {
                high = hex_char_to_value_(hex_str[i]);
                low = hex_char_to_value_(hex_str[i + 1]);
                if (high >= 0 && low >= 0) 
                {
                    unsigned char byte_value = static_cast<unsigned char>((high << 4) | low);
                    result.push_back(static_cast<typename T::value_type>(byte_value));
                }
            }
            return result;
        }
    }

    template<typename T = std::string>
    static T decode(const char* buf)
    {
        return decode<T>(std::string(buf));
    }

    static void decode(std::ostream& out, std::istream& in)
    {
        std::ostringstream oss;
        oss << in.rdbuf();
        std::string hex_content = oss.str();
        std::string binary_data = decode<std::string>(hex_content);
        out << binary_data;
    }

    template<typename T>
    static std::string encode(const T& value, bool upper_case = true)
    {
        std::ostringstream oss;
        oss << (upper_case ? std::uppercase : std::nouppercase) << std::hex;
        
        if constexpr (std::is_integral_v<T>) 
        {
            oss << value;
        }
        else if constexpr (std::is_same_v<T, std::string>) 
        {
            for (unsigned char c : value)
                oss << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(c);
        }
        else if constexpr (std::is_same_v<T, std::vector<unsigned char>>) 
        {
            for (unsigned char c : value) 
                oss << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(c);
        }
        else 
        {
            for (auto c : value) 
                oss << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(c);
        }
        
        return oss.str();
    }

    static void encode(std::ostream& out, std::istream& in, bool upper_case = true)
    {
        out << (upper_case ? std::uppercase : std::nouppercase) << std::hex;
        
        char ch;
        while (in.read(&ch, 1)) 
            out << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(static_cast<unsigned char>(ch));
    }

private:
    static int hex_char_to_value_(char c)
    {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return -1;
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