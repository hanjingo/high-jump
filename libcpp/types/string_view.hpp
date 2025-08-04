#ifndef STRING_VIEW_HPP
#define STRING_VIEW_HPP

#if (__cplusplus >= 201703L)
#include <string_view>

namespace libcpp {
using string_view = std::string_view;
}
#endif


#if (__cplusplus < 201703L)
#include <boost/utility/string_view.hpp>

namespace libcpp {
using string_view = boost::string_view;
}
#endif

#endif