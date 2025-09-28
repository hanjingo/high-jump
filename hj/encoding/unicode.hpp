#ifndef UNICODE_HPP
#define UNICODE_HPP

#include <string>
#include <codecvt>
#include <locale>

namespace hj
{
namespace unicode
{

std::wstring from_utf8(const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t> > cvt;
    return cvt.from_bytes(str);
}

std::string to_utf8(const std::wstring &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t> > cvt;
    return cvt.to_bytes(str);
}

} // namespace unicode
} // namespace hj

#endif