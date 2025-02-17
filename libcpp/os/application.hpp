#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <signal.h>
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

#if defined(NSIG) && !defined(_NSIG)
#define _NSIG NSIG
#endif

namespace libcpp
{

class application
{
public:
    struct sig_info;

#ifdef HIGH_PERFORMANCE
    typedef void(*sig_action_t)(int, sig_info*, void*);
#else
    using sig_action_t = std::function<void(int, sig_info*, void*)>;
#endif

    struct sig_info
    {
        bool repeat;
        sig_action_t action;
    };

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

    #if defined(_WIN32)
    static inline long gettid() { return static_cast<long>(::GetCurrentThreadId()); };
    #elif __linux__
    static inline long gettid() { return static_cast<long>(syscall(SYS_gettid)); };
    #elif __APPLE__
    static inline long gettid() { pthread_t pt = pthread_self(); return static_cast<long>((pt == nullptr ? -1 : pt->__sig)); };
    #else
    // TODO
    #endif

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

    static void daemon()
    {
        // TODO
    };

    static inline int sig_raise(const int sig_no) { return raise(sig_no); };

    static sig_info* sig_reg(const int sig_no, sig_action_t&& act, bool repeat = false)
    {
        sig_info* pinfo = new sig_info();
        pinfo->action = std::move(act);
        pinfo->repeat = repeat;

        sig_info* ret = _sig_map[sig_no];
        _sig_map[sig_no] = pinfo;

        signal(sig_no, _on_sig);
        return ret;
    };

    static sig_info* sig_unreg(const int sig_no)
    {
        sig_info* ret = _sig_map[sig_no];
        _sig_map[sig_no] = nullptr;
        return ret;
    };

private:
    static void _on_sig(int sig_no)
    {
        auto pinfo = _sig_map[sig_no];
        if (pinfo != nullptr)
            pinfo->action(sig_no, pinfo, nullptr);
    }

private:
    int    _argc;
    char** _argv;
    static sig_info* _sig_map[_NSIG];
};

}

#endif