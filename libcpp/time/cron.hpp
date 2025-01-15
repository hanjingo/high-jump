#ifndef CRON_HPP
#define CRON_HPP

#include <ctime>
#include <memory>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace libcpp
{
class cron
{
public:
    cron(const std::string& fmt)
    {

    }
    ~cron() {}

    // template <typename F>
    // void after(unsigned long ms, F&& f)
    // {

    // }

    // std::time_t time()
    // {

    // }

    // void cancel()
    // {

    // }

    // template <typename F>
    // void add(const std::string& fmt, F&& f)
    // {

    // }

    // static std::shared_ptr<cron> make(const std::string& fmt)
    // {

    // }
};
}

#endif