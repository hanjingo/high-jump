/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef INI_HPP
#define INI_HPP

#ifdef __cplusplus
#include <functional>

// Detect C++ standard level safely across compilers
#ifndef HJ_CPP_VERSION
#  if defined(_MSC_VER)
#    if defined(_MSVC_LANG)
#      define HJ_CPP_VERSION _MSVC_LANG
#    else
#      define HJ_CPP_VERSION __cplusplus
#    endif
#  else
#    define HJ_CPP_VERSION __cplusplus
#  endif
#endif

// Determine whether std::unary_function exists
#ifndef HJ_UNARY_FUNCTION_DEFINED
#  if defined(_MSC_VER)
#    if (_MSC_VER >= 1910 && HJ_CPP_VERSION >= 201703L)
#      define HJ_UNARY_FUNCTION_DEFINED 0
#    else
#      define HJ_UNARY_FUNCTION_DEFINED 1
#    endif
#  elif defined(__GLIBCXX__)
#    define HJ_UNARY_FUNCTION_DEFINED 1
#  elif defined(_LIBCPP_VERSION)
#    if defined(_LIBCPP_HAS_NO_DEPRECATED_UNARY_FUNCTION)
#      define HJ_UNARY_FUNCTION_DEFINED 0
#    elif defined(__has_include)
#      if __has_include(<__functional/unary_function.h>)
#        define HJ_UNARY_FUNCTION_DEFINED 1
#      else
#        define HJ_UNARY_FUNCTION_DEFINED 0
#      endif
#    else
#      define HJ_UNARY_FUNCTION_DEFINED 1
#    endif
#  elif (HJ_CPP_VERSION >= 201703L)
#    define HJ_UNARY_FUNCTION_DEFINED 0
#  else
#    define HJ_UNARY_FUNCTION_DEFINED 1
#  endif
#endif // HJ_UNARY_FUNCTION_DEFINED

// Provide fallback definition if missing
#if !HJ_UNARY_FUNCTION_DEFINED
#  define HJ_UNARY_FUNCTION_COMPAT_ACTIVE 1
#  if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wc++17-extensions"
#  elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#  endif

namespace std {
template <class Arg, class Result>
struct unary_function {
  typedef Arg    argument_type;
  typedef Result result_type;
};
} // namespace std

#  if defined(__clang__)
#    pragma clang diagnostic pop
#  elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#  endif

#  undef HJ_UNARY_FUNCTION_DEFINED
#  define HJ_UNARY_FUNCTION_DEFINED 1
#endif // !HJ_UNARY_FUNCTION_DEFINED

#endif // __cplusplus

#include <string>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace hj
{

class ini : public boost::property_tree::ptree
{
  public:
    ini() = default;
    ini(const ini &rhs)
        : boost::property_tree::ptree(rhs)
    {
    }
    ini(const boost::property_tree::ptree &tree)
        : boost::property_tree::ptree(tree)
    {
    }
    ~ini() = default;

    ini &operator=(const ini &rhs)
    {
        boost::property_tree::ptree::operator=(rhs);
        return *this;
    }

    ini &operator=(const boost::property_tree::ptree &rhs)
    {
        boost::property_tree::ptree::operator=(rhs);
        return *this;
    }

    static ini parse(const char *text) noexcept
    {
        std::stringstream           ss(text);
        boost::property_tree::ptree tree;
        try
        {
            boost::property_tree::ini_parser::read_ini(ss, tree);
        }
        catch(...)
        {
            return ini();
        }
        return ini(tree);
    }

    bool read_file(const char *filepath) noexcept
    {
        if(!boost::filesystem::exists(filepath))
            return false;

        boost::property_tree::ptree tree;
        try
        {
            boost::property_tree::ini_parser::read_ini(filepath, tree);
        }
        catch(...)
        {
            return false;
        }
        *this = tree;
        return true;
    }

    bool write_file(const char *filepath) noexcept
    {
        try
        {
            boost::property_tree::ini_parser::write_ini(filepath, *this);
        }
        catch(...)
        {
            return false;
        }
        return true;
    }

    std::string str() noexcept
    {
        std::ostringstream ss;
        try
        {
            boost::property_tree::write_ini(ss, *this);
        }
        catch(...)
        {
            return std::string();
        }
        return ss.str();
    }

    template <typename T>
    T get(const std::string &path, const T &default_value = T()) const noexcept
    {
        try
        {
            return boost::property_tree::ptree::get<T>(path, default_value);
        }
        catch(...)
        {
            return default_value;
        }
    }

    template <typename T>
    void set(const std::string &path, const T &value) noexcept
    {
        try
        {
            boost::property_tree::ptree::put(path, value);
        }
        catch(...)
        {
        }
    }
};

}

#endif