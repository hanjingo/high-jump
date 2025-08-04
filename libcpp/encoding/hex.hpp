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

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <vector>

#ifndef HEX_BUF_SZ
#define HEX_BUF_SZ 4096
#endif

namespace libcpp
{

namespace hex_detail
{
template <typename T, typename U> struct is_same
{
    static const bool value = false;
};

template <typename T> struct is_same<T, T>
{
    static const bool value = true;
};

template <bool B, typename T = void> struct enable_if
{
};

template <typename T> struct enable_if<true, T>
{
    typedef T type;
};

struct string_tag
{
};
struct vector_tag
{
};
struct integral_tag
{
};
struct container_tag
{
};

template <typename T>
typename enable_if<is_same<T, std::string>::value, string_tag>::type
get_type_tag ()
{
    return string_tag{};
}

template <typename T>
typename enable_if<is_same<T, std::vector<unsigned char> >::value,
                   vector_tag>::type
get_type_tag ()
{
    return vector_tag{};
}

template <typename T>
typename enable_if<std::is_integral<T>::value && !is_same<T, std::string>::value
                     && !is_same<T, std::vector<unsigned char> >::value,
                   integral_tag>::type
get_type_tag ()
{
    return integral_tag{};
}

template <typename T>
typename enable_if<!std::is_integral<T>::value
                     && !is_same<T, std::string>::value
                     && !is_same<T, std::vector<unsigned char> >::value,
                   container_tag>::type
get_type_tag ()
{
    return container_tag{};
}
} // namespace hex_detail

class hex
{
  public:
    template <typename T = std::string> static T decode (const std::string &str)
    {
        std::string hex_str;
        hex_str.reserve (str.length ());

        for (std::size_t i = 0; i < str.length (); ++i) {
            char c = str[i];
            if (hex_char_to_value_ (c) >= 0)
                hex_str += c;
        }

        if (hex_str.length () % 2 != 0)
            hex_str = "0" + hex_str;

        return decode_impl_<T> (hex_str, hex_detail::get_type_tag<T> ());
    }

    template <typename T = std::string> static T decode (const char *buf)
    {
        return decode<T> (std::string (buf));
    }

    static void decode (std::ostream &out, std::istream &in)
    {
        std::ostringstream oss;
        oss << in.rdbuf ();
        std::string hex_content = oss.str ();
        std::string binary_data = decode<std::string> (hex_content);
        out << binary_data;
    }

    template <typename T>
    static std::string encode (const T &value, bool upper_case = true)
    {
        std::ostringstream oss;
        oss << (upper_case ? std::uppercase : std::nouppercase) << std::hex;

        encode_impl_ (oss, value, hex_detail::get_type_tag<T> ());

        return oss.str ();
    }

    static void
    encode (std::ostream &out, std::istream &in, bool upper_case = true)
    {
        out << (upper_case ? std::uppercase : std::nouppercase) << std::hex;

        char ch;
        while (in.read (&ch, 1))
            out << std::setw (2) << std::setfill ('0')
                << static_cast<unsigned int> (static_cast<unsigned char> (ch));
    }

  private:
    template <typename T>
    static T decode_impl_ (const std::string &hex_str, hex_detail::string_tag)
    {
        T result;
        result.reserve (hex_str.length () / 2);

        for (std::size_t i = 0; i < hex_str.length (); i += 2) {
            int high = hex_char_to_value_ (hex_str[i]);
            int low = hex_char_to_value_ (hex_str[i + 1]);

            if (high >= 0 && low >= 0) {
                unsigned char byte_value =
                  static_cast<unsigned char> ((high << 4) | low);
                result.push_back (static_cast<char> (byte_value));
            }
        }
        return result;
    }

    template <typename T>
    static T decode_impl_ (const std::string &hex_str, hex_detail::vector_tag)
    {
        T result;
        result.reserve (hex_str.length () / 2);

        for (std::size_t i = 0; i < hex_str.length (); i += 2) {
            int high = hex_char_to_value_ (hex_str[i]);
            int low = hex_char_to_value_ (hex_str[i + 1]);

            if (high >= 0 && low >= 0) {
                unsigned char byte_value =
                  static_cast<unsigned char> ((high << 4) | low);
                result.push_back (byte_value);
            }
        }
        return result;
    }

    template <typename T>
    static T decode_impl_ (const std::string &hex_str, hex_detail::integral_tag)
    {
        T result = 0;
        int value = 0;
        for (std::size_t i = 0; i < hex_str.length (); ++i) {
            char c = hex_str[i];
            value = hex_char_to_value_ (c);
            if (value >= 0)
                result = (result << 4) | value;
        }
        return result;
    }

    template <typename T>
    static T decode_impl_ (const std::string &hex_str,
                           hex_detail::container_tag)
    {
        T result;
        int high = 0, low = 0;
        for (std::size_t i = 0; i < hex_str.length (); i += 2) {
            high = hex_char_to_value_ (hex_str[i]);
            low = hex_char_to_value_ (hex_str[i + 1]);
            if (high >= 0 && low >= 0) {
                unsigned char byte_value =
                  static_cast<unsigned char> ((high << 4) | low);
                result.push_back (
                  static_cast<typename T::value_type> (byte_value));
            }
        }
        return result;
    }

    template <typename T>
    static void encode_impl_ (std::ostringstream &oss,
                              const T &value,
                              hex_detail::integral_tag)
    {
        oss << value;
    }

    template <typename T>
    static void encode_impl_ (std::ostringstream &oss,
                              const T &value,
                              hex_detail::string_tag)
    {
        for (std::size_t i = 0; i < value.length (); ++i) {
            unsigned char c = static_cast<unsigned char> (value[i]);
            oss << std::setw (2) << std::setfill ('0')
                << static_cast<unsigned int> (c);
        }
    }

    template <typename T>
    static void encode_impl_ (std::ostringstream &oss,
                              const T &value,
                              hex_detail::vector_tag)
    {
        for (std::size_t i = 0; i < value.size (); ++i) {
            unsigned char c = value[i];
            oss << std::setw (2) << std::setfill ('0')
                << static_cast<unsigned int> (c);
        }
    }

    template <typename T>
    static void encode_impl_ (std::ostringstream &oss,
                              const T &value,
                              hex_detail::container_tag)
    {
        for (typename T::const_iterator it = value.begin (); it != value.end ();
             ++it) {
            unsigned char c = static_cast<unsigned char> (*it);
            oss << std::setw (2) << std::setfill ('0')
                << static_cast<unsigned int> (c);
        }
    }

    static int hex_char_to_value_ (char c)
    {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        return -1;
    }

  private:
    hex () = default;
    ~hex () = default;
    hex (const hex &) = delete;
    hex &operator= (const hex &) = delete;
    hex (hex &&) = delete;
    hex &operator= (hex &&) = delete;
};

} // namespace libcpp

#endif