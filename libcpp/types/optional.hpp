#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP

#include <boost/optional.hpp>

namespace libcpp
{

template <class T>
using optional = boost::optional<T>;

}

#endif