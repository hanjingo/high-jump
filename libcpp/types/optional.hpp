#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP

#if (__cplusplus >= 201703L)

#include <optional>

namespace libcpp
{

template <class T>
using optional = std::optional<T>;

}

#endif

#if (__cplusplus < 201703L)
#include <boost/optional.hpp>

namespace libcpp
{

template <class T>
using optional = boost::optional<T>;

}
#endif

#endif