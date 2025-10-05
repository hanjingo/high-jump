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
        if(src == 0)
            return sizeof(T) * 8;

#if defined(__GNUC__) || defined(__clang__)
        if constexpr(sizeof(T) == 4)
            return __builtin_clz(static_cast<uint32_t>(src));
        else if constexpr(sizeof(T) == 8)
            return __builtin_clzll(static_cast<uint64_t>(src));

#elif defined(_MSC_VER)
        unsigned long index;
        if constexpr(sizeof(T) == 4)
        {
            _BitScanReverse(&index, static_cast<uint32_t>(src));
            return 31 - index;
        } else if constexpr(sizeof(T) == 8)
        {
            unsigned long index64;
            _BitScanReverse64(&index64, static_cast<uint64_t>(src));
            return 63 - index64;
        }

#else
        int count = 0;
        int bits  = sizeof(T) * 8;
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