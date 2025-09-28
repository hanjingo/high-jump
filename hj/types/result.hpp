#ifndef RESULT_HPP
#define RESULT_HPP

#include <boost/leaf.hpp>

namespace hj
{

template <typename T>
using result = boost::leaf::result<T>;

}

#endif