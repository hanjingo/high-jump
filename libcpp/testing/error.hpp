#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>
#include <sstream>

#ifndef ERR_MASK
#define ERR_MASK 0
#endif

namespace libcpp
{

template <typename T>
static int err_to_int(const T err) 
{ 
    return static_cast<int>(err); 
}

template <typename T>
static std::string err_to_hex(const T err, bool upper_case = true) 
{
    std::ostringstream ss;
    ss << (upper_case ? std::uppercase : std::nouppercase) << std::hex << static_cast<int>(err);
    return "0x" + ss.str(); 
}

}

#endif