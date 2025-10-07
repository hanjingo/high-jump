/*
 *  This file is part of high-jump(hj).
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

#ifndef BITS_HPP
#define BITS_HPP

#include <string>
#include <bitset>
#include <type_traits>
#include <cstring>

namespace hj
{

class bits
{
  public:
    template <typename T>
    static bool get(const T src, const unsigned int pos)
    {
        static_assert(std::is_integral<T>::value, "T must be integral");
        return (src >> (pos - 1)) & 1;
    }

    template <typename T>
    static T &put(T &src, const unsigned int pos)
    {
        static_assert(std::is_integral<T>::value, "T must be integral");
        return src |= (1 << (pos - 1));
    }

    template <typename T>
    static T &put(T &src, const unsigned int pos, const bool bit)
    {
        static_assert(std::is_integral<T>::value, "T must be integral");
        return bit ? src |= (1 << (pos - 1)) : src &= (~(1 << (pos - 1)));
    }

    template <typename T>
    static T &reset(T &src, const bool bit)
    {
        static_assert(std::is_integral<T>::value, "T must be integral");
        src = bit ? (~T(0)) : (0);
        return src;
    }

    template <typename T>
    static T &flip(T &src)
    {
        static_assert(std::is_integral<T>::value, "T must be integral");
        src = ~(src);
        return src;
    }

    template <typename T>
    static void to_string(const T &src, char *buf)
    {
        static_assert(std::is_integral<T>::value, "T must be integral");
        unsigned int sz = sizeof(src) * 8;
        memset(buf, '0', sz);
        for(int pos = sz - 1; pos >= 0; pos--)
        {
            buf[sz - pos - 1] += ((src >> pos) & 1);
        }

        buf[sz] = '\0';
    }

    template <typename T>
    static std::string to_string(const T &src)
    {
        static_assert(std::is_integral<T>::value, "T must be integral");
        constexpr size_t N = sizeof(T) * 8;
        std::bitset<N>   bits(
            static_cast<typename std::make_unsigned<T>::type>(src));
        return bits.to_string();
    }

    template <typename T>
    static int count_leading_zeros(const T &src)
    {
        static_assert(std::is_integral<T>::value, "T must be integral");
        using UT           = typename std::make_unsigned<T>::type;
        constexpr int bits = sizeof(T) * 8;
        if(src == 0)
            return bits;

#if defined(__GNUC__) || defined(__clang__)
        if constexpr(sizeof(T) == 8)
            return __builtin_clzll(static_cast<uint64_t>(src));
        else if constexpr(sizeof(T) == 4)
            return __builtin_clz(static_cast<uint32_t>(src));
        else if constexpr(sizeof(T) == 2)
            return __builtin_clz(static_cast<uint32_t>(static_cast<UT>(src)))
                   - 16;
        else if constexpr(sizeof(T) == 1)
            return __builtin_clz(static_cast<uint32_t>(static_cast<UT>(src)))
                   - 24;
        else
        {
            // fallback
            int count = 0;
            for(int i = bits - 1; i >= 0; --i)
            {
                if((src >> i) & 1)
                    break;

                ++count;
            }
            return count;
        }
#elif defined(_MSC_VER)
        unsigned long index;
        if constexpr(sizeof(T) == 8)
        {
            unsigned long index64;
            _BitScanReverse64(&index64, static_cast<uint64_t>(src));
            return 63 - index64;
        } else if constexpr(sizeof(T) == 4)
        {
            _BitScanReverse(&index, static_cast<uint32_t>(src));
            return 31 - index;
        } else
        {
            // fallback for 8/16 bit
            int count = 0;
            for(int i = bits - 1; i >= 0; --i)
            {
                if((src >> i) & 1)
                    break;

                ++count;
            }
            return count;
        }
#else
        int count = 0;
        for(int i = bits - 1; i >= 0; --i)
        {
            if((src >> i) & 1)
                break;

            ++count;
        }
        return count;
#endif
    }
};

}

#endif