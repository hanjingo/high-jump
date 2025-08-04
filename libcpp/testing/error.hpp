#ifndef ERROR_HPP
#define ERROR_HPP

#include <sstream>
#include <string>

namespace libcpp {

template <typename T>
static int err_to_int(const T err, int mask = (~0))
{
    return static_cast<int>(err) & mask;
}

template <typename T>
static std::string err_to_hex(const T err,
                              bool upper_case = true,
                              std::string prefix = "0x")
{
    std::ostringstream ss;
    ss << (upper_case ? std::uppercase : std::nouppercase) << std::hex
       << static_cast<int>(err);
    return prefix.append(ss.str());
}

}  // namespace libcpp

#endif