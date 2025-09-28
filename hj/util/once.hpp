#ifndef ONCE_HPP
#define ONCE_HPP

#include <mutex>

namespace hj
{

#define __once_cat(a, b) a##b
#define _once_cat(a, b) __once_cat(a, b)

#define ONCE(cmd)                                                              \
    static std::once_flag _once_cat(do_once_, __LINE__);                       \
    std::call_once(_once_cat(do_once_, __LINE__), [&]() { cmd });

}

#endif