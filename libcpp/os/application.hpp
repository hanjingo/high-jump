#ifndef APPLICATION_HPP
#define APPLICATION_HPP

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
    application() : _argc{0}, _argv{NULL} {};
    virtual ~application();

    virtual bool init(int argc, char* argv[])
    {
        _argc = argc;
        _argv = argv;
        return true;
    };

    virtual void run()
    {
        // Implement It
    };

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

    static inline int getpid() { return ::getpid(); };

    static inline int getppid() { return 0; };

    static unsigned int sleep(unsigned int sec)
    {
    #ifdef _MSC_VER
        Sleep(sec);
        return 0;
    #else
        return ::sleep(sec);
    #endif
    };

private:
    int    _argc;
    char** _argv;
};

}

#endif