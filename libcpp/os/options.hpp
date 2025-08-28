#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <functional>

// c++ std::unary_function compatibility
#ifndef LIBCPP_UNARY_FUNCTION_DEFINED
    #if defined(_MSC_VER)
        #if (_MSC_VER >= 1910)
            #define LIBCPP_UNARY_FUNCTION_DEFINED 0
        #else
            #define LIBCPP_UNARY_FUNCTION_DEFINED 1
        #endif
    #elif (__cplusplus >= 201703L)
        #if defined(__GLIBCXX__)
            #define LIBCPP_UNARY_FUNCTION_DEFINED 1
        #elif defined(_LIBCPP_VERSION)
            #define LIBCPP_UNARY_FUNCTION_DEFINED 0
        #else
            #define LIBCPP_UNARY_FUNCTION_DEFINED 0
        #endif
    #else
        #define LIBCPP_UNARY_FUNCTION_DEFINED 1
    #endif
#endif

#if !LIBCPP_UNARY_FUNCTION_DEFINED
namespace std {
    template <class Arg, class Result>
    struct unary_function {
        typedef Arg argument_type;
        typedef Result result_type;
    };
}
#undef LIBCPP_UNARY_FUNCTION_DEFINED
#define LIBCPP_UNARY_FUNCTION_DEFINED 1
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

namespace libcpp
{
    
class options
{
public:
    template<typename T>
    void add(const char* key, T default_value, const char* memo = "")
    {
        _desc.add_options()(key, 
                            boost::program_options::value<T>()->default_value(default_value), 
                            memo);
    }

    template<typename T>
    T parse(int argc, char* argv[], const char* key)
    {
        T ret = T{};
        try {
            boost::program_options::variables_map vm;
            boost::program_options::store(
                boost::program_options::parse_command_line(argc, argv, _desc), vm);
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
            boost::program_options::store(
                boost::program_options::parse_command_line(argc, argv, _desc), vm);
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

private:
    boost::program_options::options_description _desc;
};

}

#endif // OPTIONS_HPP