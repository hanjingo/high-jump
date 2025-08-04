#ifndef HELLO_HPP
#define HELLO_HPP

#include <string>

namespace libcpp {

struct Hello
{
    std::string Name() { return "Hello"; }
};

}  // namespace libcpp

#endif