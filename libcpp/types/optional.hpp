#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP

#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
#include <optional>

namespace libcpp
{
template <class T>
using optional = std::optional<T>;
}

#else
#include <boost/optional.hpp>

namespace libcpp
{

template <class T>
using optional = boost::optional<T>;

}
#endif

#endif