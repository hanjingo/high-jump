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

}

#endif