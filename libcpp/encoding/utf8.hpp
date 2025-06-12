#ifndef UTF8_HPP
#define UTF8_HPP

#include <string>
#include <codecvt>
#include <locale>

namespace libcpp
{
namespace utf8
{

inline bool is_valid(const std::string& str)
{
    int c, i, ix, n, j;
    for (i = 0, ix = str.length(); i < ix; i++)
    {
        c = (unsigned char)str[i];
        if (0x00 <= c && c <= 0x7F) n = 0;
        else if ((c & 0xE0) == 0xC0) n = 1;
        else if ((c & 0xF0) == 0xE0) n = 2;
        else if ((c & 0xF8) == 0xF0) n = 3;
        else return false;
        for (j = 0; j < n && i < ix; j++)
            if ((++i == ix) || ((str[i] & 0xC0) != 0x80))
                return false;
    }
    return true;
}

std::string from_unicode(const std::wstring& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    return cvt.to_bytes(str);
}

std::wstring to_unicode(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    return cvt.from_bytes(str);
}

}
}

#endif