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

#ifndef OPTIONS_HPP
#define OPTIONS_HPP

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

#include <thread>
#include <chrono>

#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#include <windows.h>

#elif __linux__
#include <syscall.h>
#include <unistd.h>

#else
#include <unistd.h>

#endif

#include <boost/program_options.hpp>

namespace hj
{

class options
{
  public:
    options()  = default;
    ~options() = default;

    template <typename T>
    void add(const char *key, T default_value, const char *memo = "")
    {
        _add_impl(key,
                  default_value,
                  memo,
                  std::is_same<T, std::vector<std::string>>{});
    }

    void add_positional(const char *key, int max_count = 1)
    {
        _pos.add(key, max_count);
    }

    template <typename T>
    T parse(int argc, char *argv[], const char *key)
    {
        T ret = T{};
        try
        {
            boost::program_options::variables_map vm;
            if(_pos.max_total_count() > 0)
            {
                boost::program_options::store(
                    boost::program_options::command_line_parser(argc, argv)
                        .options(_desc)
                        .positional(_pos)
                        .run(),
                    vm);
            } else
            {
                boost::program_options::store(
                    boost::program_options::parse_command_line(argc,
                                                               argv,
                                                               _desc),
                    vm);
            }
            boost::program_options::notify(vm);
            if(vm.count(key))
                ret = vm[key].as<T>();
        }
        catch(...)
        {
        }
        return ret;
    }

    template <typename T>
    T parse(int argc, char *argv[], const char *key, const T &default_value)
    {
        try
        {
            boost::program_options::variables_map vm;
            if(_pos.max_total_count() > 0)
            {
                boost::program_options::store(
                    boost::program_options::command_line_parser(argc, argv)
                        .options(_desc)
                        .positional(_pos)
                        .run(),
                    vm);
            } else
            {
                boost::program_options::store(
                    boost::program_options::parse_command_line(argc,
                                                               argv,
                                                               _desc),
                    vm);
            }
            boost::program_options::notify(vm);
            if(!vm.count(key))
                return default_value;

            auto &variable_value = vm[key];
            if(!variable_value.defaulted())
                return vm[key].as<T>();
            else
                return default_value;
        }
        catch(...)
        {
            return default_value;
        }
    }

    template <typename T = std::vector<std::string>>
    T parse_positional(int argc, char *argv[], const char *key)
    {
        T ret{};
        try
        {
            boost::program_options::variables_map vm;
            boost::program_options::store(
                boost::program_options::command_line_parser(argc, argv)
                    .options(_desc)
                    .positional(_pos)
                    .run(),
                vm);
            boost::program_options::notify(vm);
            if(vm.count(key))
                ret = vm[key].as<T>();
        }
        catch(...)
        {
        }
        return ret;
    }

  private:
    template <typename T>
    void _add_impl(const char *key,
                   T           default_value,
                   const char *memo,
                   std::false_type)
    {
        _desc.add_options()(
            key,
            boost::program_options::value<T>()->default_value(default_value),
            memo);
    }

    template <typename T>
    void _add_impl(const char *key, T, const char *memo, std::true_type)
    {
        _desc.add_options()(key, boost::program_options::value<T>(), memo);
    }

  private:
    boost::program_options::options_description            _desc;
    boost::program_options::positional_options_description _pos;
};

}

#endif // OPTIONS_HPP