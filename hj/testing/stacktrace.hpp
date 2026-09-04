#ifndef STACKTRACE_HPP
#define STACKTRACE_HPP

#include <boost/core/demangle.hpp>
#include <boost/stacktrace.hpp>
#include <exception>
#include <iostream>
#include <mutex>
#include <string>

namespace hj
{

inline std::mutex &stacktrace_mutex()
{
    static std::mutex mtx;
    return mtx;
}

inline std::string current_stacktrace()
{
    std::lock_guard<std::mutex> lock(stacktrace_mutex());
    return boost::stacktrace::to_string(boost::stacktrace::stacktrace());
}

inline std::string exception_stacktrace()
{
    std::lock_guard<std::mutex> lock(stacktrace_mutex());
    auto st = boost::stacktrace::stacktrace::from_current_exception();
    if(st)
    {
        return boost::stacktrace::to_string(st);
    }
    return "[Warning: Exception throw-location stacktrace unavailable.\n"
           "Ensure Boost.Stacktrace exception hook is enabled]";
}

inline std::string current_exception_diagnostic()
{
    std::string        result;
    std::exception_ptr eptr = std::current_exception();

    if(!eptr)
    {
        return "[No active exception]\nStacktrace:\n" + current_stacktrace();
    }

    try
    {
        std::rethrow_exception(eptr);
    }
    catch(const std::exception &e)
    {
        result += "Exception:\n    ";
        result += boost::core::demangle(typeid(e).name());
        result += "\nMessage:\n    ";
        result += e.what();
    }
    catch(const char *e)
    {
        result += "Exception:\n    const char*\nMessage:\n    ";
        result += e;
    }
    catch(const std::string &e)
    {
        result += "Exception:\n    std::string\nMessage:\n    ";
        result += e;
    }
    catch(...)
    {
        result +=
            "Exception:\n    [Unknown Exception Type]\nMessage:\n    [N/A]";
    }

    result += "\nStacktrace:\n";

    std::lock_guard<std::mutex> lock(stacktrace_mutex());
    auto st = boost::stacktrace::stacktrace::from_current_exception();
    if(st)
    {
        result += boost::stacktrace::to_string(st);
    } else
    {
        result +=
            "[Catch-site Stacktrace]\n"
            + boost::stacktrace::to_string(boost::stacktrace::stacktrace());
    }

    return result;
}

inline std::string stacktrace()
{
    if(std::current_exception())
    {
        return current_exception_diagnostic();
    }
    return current_stacktrace();
}

} // namespace hj

#endif // STACKTRACE_HPP