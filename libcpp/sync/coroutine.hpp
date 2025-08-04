#ifndef COROUTINE_HPP
#define COROUTINE_HPP

#include <boost/coroutine2/all.hpp>

namespace libcpp
{

// See Also:
// https://www.boost.org/doc/libs/1_75_0/libs/coroutine2/doc/html/index.html

template <typename T> using coroutine = boost::coroutines2::coroutine<T>;

using stack_alloc = boost::context::fixedsize_stack;

} // namespace libcpp

#define __coroutine_cat(a, b) a##b
#define _coroutine_cat(a, b) __coroutine_cat (a, b)

#define COROUTINE(cmd)                                                         \
    libcpp::coroutine<void>::pull_type __coroutine_cat (__coroutine__,         \
                                                        __COUNTER__) (         \
      [&] (libcpp::coroutine<void>::push_type &) { cmd; });

#endif