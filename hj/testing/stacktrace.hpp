#ifndef STACKTRACE_HPP
#define STACKTRACE_HPP

#include <boost/core/demangle.hpp>
#include <boost/stacktrace.hpp>
#include <exception>
#include <string>

namespace hj
{

struct exception_info
{
    std::string type;
    std::string message;
    std::string stacktrace;
    bool        is_throw_site_stacktrace{false};
};

inline std::string current_stacktrace()
{
    return boost::stacktrace::to_string(boost::stacktrace::stacktrace());
}

inline std::string exception_stacktrace()
{
    auto st = boost::stacktrace::stacktrace::from_current_exception();
    if(st)
    {
        return boost::stacktrace::to_string(st);
    }
    return "[Warning: Exception throw-location stacktrace unavailable.\n"
           "Ensure Boost.Stacktrace exception hook is enabled]";
}

inline exception_info capture_exception(std::exception_ptr eptr)
{
    exception_info info;

    if(!eptr)
    {
        info.type                     = "[No active exception]";
        info.message                  = "[N/A]";
        info.stacktrace               = current_stacktrace();
        info.is_throw_site_stacktrace = false;
        return info;
    }

    try
    {
        std::rethrow_exception(eptr);
    }
    catch(const std::exception &e)
    {
        info.type    = boost::core::demangle(typeid(e).name());
        info.message = e.what() ? e.what() : "[null]";
    }
    catch(const char *e)
    {
        info.type    = "const char*";
        info.message = e ? e : "[null]";
    }
    catch(const std::string &e)
    {
        info.type    = "std::string";
        info.message = e;
    }
    catch(...)
    {
        info.type    = "[Unknown Exception Type]";
        info.message = "[N/A]";
    }

    auto st = boost::stacktrace::stacktrace::from_current_exception();
    if(st)
    {
        info.stacktrace               = boost::stacktrace::to_string(st);
        info.is_throw_site_stacktrace = true;
    } else
    {
        info.stacktrace               = current_stacktrace();
        info.is_throw_site_stacktrace = false;
    }

    return info;
}

inline exception_info capture_current_exception()
{
    return capture_exception(std::current_exception());
}

inline std::string format_exception(const exception_info &info)
{
    std::string result;
    result += "Exception:\n    " + info.type + "\n";
    result += "Message:\n    " + info.message + "\n";
    result += "Stacktrace:\n";
    if(!info.is_throw_site_stacktrace && info.type != "[No active exception]")
    {
        result += "[Catch-site Stacktrace]\n";
    }
    result += info.stacktrace;
    return result;
}

inline std::string current_exception_diagnostic()
{
    return format_exception(capture_current_exception());
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