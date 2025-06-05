#ifndef RESULT_HPP
#define RESULT_HPP

#include <boost/leaf.hpp>

namespace libcpp
{

template<typename T>
using result = boost::leaf::result<T>;

}

#endif