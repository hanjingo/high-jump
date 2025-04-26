#ifndef UTF8_HPP
#define UTF8_HPP

#include <string>
#include <codecvt>
#include <locale>

namespace libcpp
{
namespace utf8
{

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