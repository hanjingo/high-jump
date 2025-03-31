#ifndef STACKTRACE_HPP
#define STACKTRACE_HPP

#include <iostream>
#include <boost/stacktrace.hpp>

namespace libcpp
{

inline auto stacktrace()
{
    return boost::stacktrace::stacktrace();
}

}

#define RECOVER(cmd) \
    try { \
        cmd \
    } catch(...) { \
        std::cerr << libcpp::stacktrace() << std::endl; \
    } \

#endif