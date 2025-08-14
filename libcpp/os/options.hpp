#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <thread>
#include <chrono>
#include <functional>

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