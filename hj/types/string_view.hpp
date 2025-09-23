#ifndef STRING_VIEW_HPP
#define STRING_VIEW_HPP

#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
#include <string_view>

namespace hj
{
using string_view = std::string_view;
}

#else
#include <boost/utility/string_view.hpp>

namespace hj
{
using string_view = boost::string_view;
}
#endif

#endif