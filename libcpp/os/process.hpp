#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <boost/process.hpp>

namespace libcpp
{
class process
{
public:
    using pid_t = boost::process::pid_t;
    using exit_code_t = int;
    using error_code_t = std::error_code;
    using child_t = boost::process::child;

    enum state_t {
        state_create,
        state_ready,
        state_running,
        state_block,
        state_stop,
    };

public:
    explicit process() {}
    ~process() { }

    template <typename... Args>
    static child_t child(Args&&... args) 
    { 
        return boost::process::child(std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void spawn(Args&&... args) 
    { 
        boost::process::spawn(std::forward<Args>(args)...);
    }

    template <typename... Args>
    static int system(Args&&... args) 
    { 
        return boost::process::system(std::forward<Args>(args)...); 
    }
    
    static pid_t getpid() { return ::getpid(); }

    static pid_t getppid() { return ::getppid(); }

    static int64_t time_stamp() 
    { 
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
};

}

#endif