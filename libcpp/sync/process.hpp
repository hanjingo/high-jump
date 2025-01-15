#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <functional>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#define __WINDOWS__
#endif

#if defined(__WINDOWS__)
#include <windows.h>
#elif __APPLE__
#include <sys/param.h>
#include <sys/sysctl.h>
#elif __linux__
#include <unistd.h>
#include <pthread.h>
#else
#pragma warning unknown OS, some function will be disabled
#endif

namespace libcpp
{

class process
{
public:
    enum event_t {
        event_quit,
        event_quit_finish,
        event_quit_error,

        event_terminate,
        event_terminate_finish,
        event_terminate_error,

        event_restart,
        event_restart_finish,
        event_restart_error,

        event_destroy,
    };

    enum state_t {
        state_unknow,
        state_starting,
        state_running,
    };

    using error_t = int;
    using pid_t = int;
    using exit_code_t = int;
    using event_filter_t = std::function<bool(event_t, void*)>;

    static const event_filter_t default_process_event_filter = [](event_t ev, void* arg) -> bool
    {
        libcpp::process* proc = static_cast<process*>(arg);
        switch (ev) 
        {
        case event_quit:
        {
            proc->quit();
            break;
        }
        case event_terminate:
        {
            proc->terminate();
            break;
        }
        default: break;
        }
    };

public:
    process(const event_filter_t fn) : _fn{fn}
    {
    };

    process(const pid_t pid, const event_filter_t fn) : _fn{fn}
    {
    };

    process(const pid_t pid, const char* program, const event_filter_t fn) : _fn{fn}
    {
    };

    ~process()
    {
        notify(event_destroy, this);
    };

    static libcpp::process* create_process(const char* exe)
    {
        libcpp::process proc{default_process_event_filter};
        return &proc;
    };

    static int nprocess(const char* exe)
    {
        return 0;
    };

    static error_t terminate_process(pid_t pid)
    {
        return 0;
    };

    static error_t terminate_process(const char* pid)
    {
        return 0;
    };

    void exe_file(char* out)
    {
    };

    void set_exe_file(const char* file)
    {
    };

    pid_t id()
    {
        return 0;
    };

    pid_t parent_id()
    {
        return 0;
    };

    bool bind(const libcpp::process* rhs)
    {

    };

    exit_code_t quit()
    {
        return notify(event_quit, this);
    };

    exit_code_t wait(const int64_t dur = -1)
    {

    };

    error_t daemon(pid_t pid)
    {
        return 0;
    };

    error_t terminate()
    {
        return notify(event_terminate, this);
    };

    u_int64_t time_passed()
    {
        return 0;
    };

    state_t state()
    {
        return state_unknow;
    };

    inline bool is_running()
    {
        return state() == state_running;
    };

    error_t notify(const event_t e, void* arg)
    {
        return 0;
    };

    void set_event_filter(event_filter_t&& filter)
    {
        _fn = std::move(filter);
    };

protected:
    virtual void on_event(event_t ev)
    {
        _fn(ev, this);
    }

private:
    event_filter_t _fn;
};

}

#endif