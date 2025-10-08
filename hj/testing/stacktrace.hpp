#ifndef STACKTRACE_HPP
#define STACKTRACE_HPP

#include <boost/stacktrace.hpp>
#include <iostream>

namespace hj
{

inline auto stacktrace()
{
    return boost::stacktrace::stacktrace();
}

} // namespace hj

#define RECOVER(cmd)                                                           \
    try                                                                        \
    {                                                                          \
        cmd                                                                    \
    }                                                                          \
    catch(...)                                                                 \
    {                                                                          \
        std::cerr << hj::stacktrace() << std::endl;                            \
    }

#endif