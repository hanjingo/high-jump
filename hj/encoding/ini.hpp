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

// -------- detect compiler C++ version ------------
#ifndef HJ_CPP_VERSION
#if defined(_MSC_VER)
#if defined(_MSVC_LANG)
#define HJ_CPP_VERSION _MSVC_LANG
#else
#define HJ_CPP_VERSION __cplusplus
#endif
#else
#define HJ_CPP_VERSION __cplusplus
#endif
#endif // HJ_CPP_VERSION

// -------- detect whether std::unary_function exists ------------
#ifndef HJ_UNARY_FUNCTION_DEFINED
// Historically std::unary_function was provided by the standard
// library implementations but has been removed in C++17. Some
// libstdc++/libc++/MSVC versions may still provide a deprecated
// definition or a compatibility macro. We try to detect common
// cases conservatively and only provide a fallback when the symbol
// is truly absent.

// MSVC (Visual Studio) removed unary_function in recent toolsets
#if defined(_MSC_VER)
#if (_MSC_VER >= 1910 && HJ_CPP_VERSION >= 201703L)
#define HJ_UNARY_FUNCTION_DEFINED 0
#else
#define HJ_UNARY_FUNCTION_DEFINED 1
#endif

// libstdc++ (GCC's C++ library) historically provided unary_function
#elif defined(__GLIBCXX__)
#define HJ_UNARY_FUNCTION_DEFINED 1

// libc++ (LLVM's C++ library) removed many deprecated symbols; newer
// releases may define a feature-test macro to indicate removal.
#elif defined(_LIBCPP_VERSION)
#if defined(_LIBCPP_HAS_NO_DEPRECATED_UNARY_FUNCTION)
#define HJ_UNARY_FUNCTION_DEFINED 0
#else
#if (HJ_CPP_VERSION > 201703L)
#define HJ_UNARY_FUNCTION_DEFINED 0
#else
#define HJ_UNARY_FUNCTION_DEFINED 1
#endif
#endif

// Generic fallback: if compiling in C++17 or newer, assume unary_function
// is not present unless a known library macro indicates otherwise.
#else
#if (HJ_CPP_VERSION >= 201703L)
#define HJ_UNARY_FUNCTION_DEFINED 0
#else
#define HJ_UNARY_FUNCTION_DEFINED 1
#endif
#endif

#endif // HJ_UNARY_FUNCTION_DEFINED

// -------- provide fallback only when truly missing ------------
#if !HJ_UNARY_FUNCTION_DEFINED
// Note: injecting names into namespace std is undefined behavior in the
// C++ standard. In practice many implementations accept this for
// compatibility shims, but a safer approach is to provide the shim in
// a separate compatibility namespace and adapt callers. To minimize
// disruption we keep the original behavior here but document the risk.
namespace std
{
template <class Arg, class Result>
struct unary_function
{
    typedef Arg    argument_type;
    typedef Result result_type;
};
} // namespace std
#undef HJ_UNARY_FUNCTION_DEFINED
#define HJ_UNARY_FUNCTION_DEFINED 1
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