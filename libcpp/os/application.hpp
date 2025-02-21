#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <thread>
#include <chrono>

#include <functional>

#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#elif __linux__
#include <syscall.h>
#include <unistd.h>
#else
#include <unistd.h>
#endif

#include <boost/program_options.hpp>

namespace libcpp
{

class application
{
public:
    application() {};
    virtual ~application();

    static long gettid() 
    {
#if defined(_WIN32)
        return static_cast<long>(::GetCurrentThreadId());
#elif __linux__
        return static_cast<long>(syscall(SYS_gettid));
#elif __APPLE__
        pthread_t pt = pthread_self(); 
        return static_cast<long>((pt == nullptr ? -1 : pt->__sig));
#else
        // TODO
        return 0;
#endif
    }

    static void sleep(unsigned int sec)
    {
        std::this_thread::sleep_for(std::chrono::seconds(sec));
    };

    static void msleep(unsigned long long ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    };
};

class options
{
public:
    template<typename T>
    void add(const char* key, T default_value, const char* memo = "")
    {
        _desc.add_options()(key, boost::program_options::value<T>()->default_value(default_value), memo);
    }

    template<typename T>
    T parse(int argc, char* argv[], const char* key)
    {
        T ret;
        try {
            boost::program_options::variables_map vm;
            boost::program_options::store(
                boost::program_options::parse_command_line(argc, argv, _desc), vm);
            boost::program_options::notify(vm);
            if (vm.count(key))
                ret = vm[key].as<T>();
            return ret;
        } catch (const std::exception& e) {
            return ret;
        }
    }

private:
    boost::program_options::options_description _desc;
};

}

#endif