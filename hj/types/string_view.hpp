#ifndef STRING_VIEW_HPP
#define STRING_VIEW_HPP

#if __has_include(<string_view>)
#include <string_view>
namespace hj
{
using string_view = std::string_view;
}

#elif __has_include(<boost/utility/string_view.hpp>)
#include <boost/utility/string_view.hpp>
namespace hj
{
using string_view = boost::string_view;
}

#else
#error                                                                         \
    "No suitable string_view implementation found (need C++17 std::string_view or boost::string_view)"

#endif

#endif // STRING_VIEW_HPP