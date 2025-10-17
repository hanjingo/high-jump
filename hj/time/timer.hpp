/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <iostream>

#include <boost/version.hpp>
#include <boost/asio.hpp>

namespace hj
{
namespace detail
{
class tm_runner
{
  public:
#if BOOST_VERSION < 108700
    using io_t         = boost::asio::io_service;
    using work_guard_t = boost::asio::io_service::work;
#else
    using io_t         = boost::asio::io_context;
    using work_guard_t = boost::asio::executor_work_guard<io_t::executor_type>;
#endif

    static void run_once()
    {
        std::call_once(s_once_flag(), []() {
            s_work_guard() =
                std::make_unique<work_guard_t>(io_global().get_executor());

            s_thread() =
                std::make_unique<std::thread>([]() { io_global().run(); });

            s_thread()->detach();
        });
    }

    // Accessors
    static io_t &io_global()
    {
        static io_t io_global_inst;
        return io_global_inst;
    }

  private:
    static std::once_flag &s_once_flag()
    {
        static std::once_flag flag;
        return flag;
    }

    static std::unique_ptr<work_guard_t> &s_work_guard()
    {
        static std::unique_ptr<work_guard_t> guard;
        return guard;
    }

    static std::unique_ptr<std::thread> &s_thread()
    {
        static std::unique_ptr<std::thread> thr;
        return thr;
    }
};
} // namespace detail

class timer : public std::enable_shared_from_this<timer>
{
  public:
    using callback_t     = std::function<void()>;
    using err_t          = boost::system::error_code;
    using steady_timer_t = boost::asio::steady_timer;

    timer(const timer &)            = delete;
    timer &operator=(const timer &) = delete;
    timer(timer &&)                 = delete;
    timer &operator=(timer &&)      = delete;

    explicit timer(unsigned long long us)
        : _tm(detail::tm_runner::io_global(), std::chrono::microseconds(us))
    {
        detail::tm_runner::run_once();
    }

    ~timer()
    {
        try
        {
            std::lock_guard<std::mutex> lk(_mu);
            _tm.cancel();
        }
        catch(...)
        {
            std::cerr << "Exception in timer destructor\n" << std::endl;
        }
    }

    template <typename F>
    void start(F &&f)
    {
        callback_t                  cb = std::forward<F>(f);
        std::lock_guard<std::mutex> lk(_mu);
        _tm.cancel();

        auto self = this->shared_from_this();
        _tm.expires_after(std::chrono::microseconds(_tm_exp_us()));
        _tm.async_wait([self, cb = std::move(cb)](const err_t &err) mutable {
            try
            {
                if(err)
                    return;

                if(cb)
                    cb();
            }
            catch(...)
            {
                std::cerr << "Exception in timer callback\n" << std::endl;
            }
        });
    }

    void reset(unsigned long long us)
    {
        std::lock_guard<std::mutex> lk(_mu);
        _tm_exp_us() = us;
        _tm.cancel();

        auto self = this->shared_from_this();
        _tm.expires_after(std::chrono::microseconds(_tm_exp_us()));
        _tm.async_wait([self](const err_t &err) { (void) err; });
    }

    template <typename F>
    void reset(unsigned long long us, F &&f)
    {
        callback_t                  cb = std::forward<F>(f);
        std::lock_guard<std::mutex> lk(_mu);
        _tm_exp_us() = us;
        _tm.cancel();

        auto self = this->shared_from_this();
        _tm.expires_after(std::chrono::microseconds(_tm_exp_us()));
        _tm.async_wait([self, cb = std::move(cb)](const err_t &err) mutable {
            try
            {
                if(err)
                    return;
                if(cb)
                    cb();
            }
            catch(...)
            {
            }
        });
    }

    void cancel()
    {
        std::lock_guard<std::mutex> lk(_mu);
        _tm.cancel();
    }

    template <typename F>
    static std::shared_ptr<timer> make(unsigned long long us, F &&f)
    {
        auto t = std::make_shared<timer>(us);
        t->start(std::forward<F>(f));
        return t;
    }

  private:
    unsigned long long &_tm_exp_us() { return _expiry_us; }

  private:
    steady_timer_t     _tm;
    std::mutex         _mu;
    unsigned long long _expiry_us{0};
};

} // namespace hj

#endif // TIMER_HPP