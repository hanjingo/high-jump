#ifndef COROUTINE_HPP
#define COROUTINE_HPP

#include <boost/coroutine2/all.hpp>

namespace libcpp
{

// See Also: https://www.boost.org/doc/libs/1_82_0/libs/coroutine2/doc/html/index.html

template <typename T>
using coroutine = boost::coroutines2::coroutine<T>;

}

#endif