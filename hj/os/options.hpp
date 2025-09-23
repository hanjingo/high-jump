#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <functional>

// c++ std::unary_function compatibility
#ifndef HJ_UNARY_FUNCTION_DEFINED
    #if defined(_MSC_VER)
        #if (_MSC_VER >= 1910)
            #define HJ_UNARY_FUNCTION_DEFINED 0
        #else
            #define HJ_UNARY_FUNCTION_DEFINED 1
        #endif
    #elif (__cplusplus >= 201703L)
        #if defined(__GLIBCXX__)
            #define HJ_UNARY_FUNCTION_DEFINED 1
        #elif defined(_HJ_VERSION)
            #define HJ_UNARY_FUNCTION_DEFINED 0
        #else
            #define HJ_UNARY_FUNCTION_DEFINED 0
        #endif
    #else
        #define HJ_UNARY_FUNCTION_DEFINED 1
    #endif
#endif

#if !HJ_UNARY_FUNCTION_DEFINED
namespace std {
    template <class Arg, class Result>
    struct unary_function {
        typedef Arg argument_type;
        typedef Result result_type;
    };
}
#undef HJ_UNARY_FUNCTION_DEFINED
#define HJ_UNARY_FUNCTION_DEFINED 1
#endif

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
    options() = default;
    ~options() = default;

    template<typename T>
    void add(const char* key, T default_value, const char* memo = "")
    {
        _add_impl(key, default_value, memo, std::is_same<T, std::vector<std::string>>{});
    }

    void add_positional(const char* key, int max_count = 1)
    {
        _pos.add(key, max_count);
    }

    template<typename T>
    T parse(int argc, char* argv[], const char* key)
    {
        T ret = T{};
        try {
            boost::program_options::variables_map vm;
            if (_pos.max_total_count() > 0) 
            {
                boost::program_options::store(
                    boost::program_options::command_line_parser(argc, argv)
                        .options(_desc)
                        .positional(_pos)
                        .run(),
                    vm);
            } 
            else 
            {
                boost::program_options::store(
                    boost::program_options::parse_command_line(argc, argv, _desc), vm);
            }
            boost::program_options::notify(vm);
            if (vm.count(key))
                ret = vm[key].as<T>();
        } catch (...) {}
        return ret;
    }

    template<typename T>
    T parse(int argc, char* argv[], const char* key, const T& default_value)
    {
        try {
            boost::program_options::variables_map vm;
            if (_pos.max_total_count() > 0) 
            {
                boost::program_options::store(
                    boost::program_options::command_line_parser(argc, argv)
                        .options(_desc)
                        .positional(_pos)
                        .run(),
                    vm);
            } 
            else 
            {
                boost::program_options::store(
                    boost::program_options::parse_command_line(argc, argv, _desc), vm);
            }
            boost::program_options::notify(vm);
            if (!vm.count(key)) 
                return default_value;

            auto& variable_value = vm[key];
            if (!variable_value.defaulted()) 
                return vm[key].as<T>();
            else
                return default_value;
        } catch (...) {
            return default_value;
        }
    }

    template<typename T = std::vector<std::string>>
    T parse_positional(int argc, char* argv[], const char* key)
    {
        T ret{};
        try {
            boost::program_options::variables_map vm;
            boost::program_options::store(
                boost::program_options::command_line_parser(argc, argv)
                    .options(_desc)
                    .positional(_pos)
                    .run(),
                vm);
            boost::program_options::notify(vm);
            if (vm.count(key))
                ret = vm[key].as<T>();
        } catch (...) {}
        return ret;
    }

private:
    template<typename T>
    void _add_impl(const char* key, T default_value, const char* memo, std::false_type)
    {
        _desc.add_options()(
            key,
            boost::program_options::value<T>()->default_value(default_value),
            memo);
    }

    template<typename T>
    void _add_impl(const char* key, T, const char* memo, std::true_type)
    {
        _desc.add_options()(
            key,
            boost::program_options::value<T>(),
            memo);
    }

private:
    boost::program_options::options_description _desc;
    boost::program_options::positional_options_description _pos;
};

}

#endif // OPTIONS_HPP