#ifndef PROCESS_HPP
#define PROCESS_HPP

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


#include <vector>
#include <algorithm>
#include <boost/process.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace libcpp
{

namespace process
{
using pid_t         = boost::process::pid_t;
using exit_code_t   = int;
using error_code_t  = std::error_code;
using child_t       = boost::process::child;
using group_t       = boost::process::group;
using path_t        = boost::filesystem::path;

using pstream_t     = boost::process::pstream;
using ipstream_t    = boost::process::ipstream;
using opstream_t    = boost::process::opstream;

// pair: [pid, process name]
using list_match_cb = std::function<bool(const std::vector<std::string>)>;

static constexpr boost::process::detail::std_in_<void> std_in = boost::process::std_in;
static constexpr boost::process::detail::std_out_<1> std_out  = boost::process::std_out;
static constexpr boost::process::detail::std_out_<2> std_err  = boost::process::std_err;

#if defined(_WIN32)
static constexpr char cmd_list_pid[] = "tasklist /FO CSV /NH";
static constexpr char cmd_kill[]     = "taskkill /PID ";
#else
static constexpr char cmd_list_pid[] = "ps -eo comm,pid";
static constexpr char cmd_kill[]     = "kill -9 ";
#endif

inline pid_t getpid() { return pid_t(::getpid()); }

#if defined(_WIN32)
inline pid_t getppid() { return pid_t(0); }
#else
inline pid_t getppid() { return pid_t(::getppid()); }
#endif

template <typename... Args>
inline int system(Args&&... args) 
{ 
    return boost::process::system(std::forward<Args>(args)...); 
}

template <typename... Args>
inline child_t child(Args&&... args) 
{ 
    return boost::process::child(std::forward<Args>(args)...);
}

template <typename... Args>
inline void spawn(Args&&... args) 
{ 
    boost::process::spawn(std::forward<Args>(args)...);
}

template <typename... Args>
inline void daemon(Args&&... args)
{
    // Step1. ignore signal
    // Step2. fork child and set work directory to pwd/root
    // Step3. close file
    // Step4. io_in, io_out, io_err redirect io null
    boost::process::group gp{};
    boost::process::spawn(
        std::forward<Args>(args)..., 
        boost::process::start_dir(boost::filesystem::current_path()), // pwd/root
        boost::process::std_in.null(),
        boost::process::std_out.null(),
        boost::process::std_out.null(),
        gp
    );
    gp.detach();
}

inline group_t group()
{
    return boost::process::group();
}

inline void list(std::vector<pid_t>& result, 
                 list_match_cb match = [](const std::vector<std::string>) -> bool{return true;})
{
        boost::process::ipstream stream;
        std::string line;
        std::vector<std::string> vec;
        bool is_first_line = true;
    try{
        boost::process::child proc(cmd_list_pid, boost::process::std_out > stream);
        while (std::getline(stream, line)) 
        {
#if defined(_WIN32)
            boost::algorithm::split(vec, line, [](char c) { return c == ','; });
            for (auto itr = vec.begin(); itr != vec.end(); ++itr)
                boost::erase_all(*itr, "\"");
#else
            if (is_first_line) 
            { 
                is_first_line = false; 
                continue; 
            }
            boost::algorithm::split(vec, line, boost::algorithm::is_space(), boost::token_compress_on);
#endif
            if (!match(vec) || vec.size() < 2)
                continue;

            result.push_back(pid_t(std::stoi(vec[1])));
        }
    } catch(const std::exception& e) {
        return;
    }
}

inline void kill(const pid_t pid)
{
    try {
        std::string cmd = std::string(cmd_kill).append(std::to_string(pid));
        boost::process::child proc(cmd);
        proc.wait();
    } catch(const std::exception& e) {
        return;
    }

    return;
}

};

}

#endif