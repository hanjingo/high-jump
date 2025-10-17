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

#include <cstdlib>
#include <future>
#include <functional>
#include <chrono>
#include <memory>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <thread>
#include <iostream>

#include <boost/version.hpp>
#include <boost/asio.hpp>

namespace hj
{

class timer : public std::enable_shared_from_this<timer>
{
  public:
    using callback_t = std::function<void()>;
    using err_t      = boost::system::error_code;
    using io_t       = boost::asio::io_context;
#if BOOST_VERSION < 108700
    using io_work_t = boost::asio::io_service::work;
#endif

    timer(const timer &)            = delete;
    timer &operator=(const timer &) = delete;

    explicit timer(unsigned long long us)
        : _tm{io_global(), std::chrono::microseconds(us)}
    {
    }

    timer(timer &&rhs) noexcept
        : _tm{std::move(rhs._tm)}
        , _fn{std::move(rhs._fn)}
    {
    }

    ~timer() { cancel(); }
    inline void cancel()
    {
        _tm.cancel();
        _fn = [](const boost::system::error_code &) {};
    }

    template <typename F>
    void start(F &&f)
    {
        static std::once_flag io_run_once;
        callback_t            fn = std::forward<F>(f);
        _fn = [fn = std::move(fn)](const timer::err_t &err) {
            try
            {
                if(err.failed())
                    return;

                if(fn)
                    fn();
            }
            catch(...)
            {
                std::cerr << "timer callback exception" << std::endl;
            }
        };

        std::call_once(io_run_once, []() {
            (void) worker_global();
            std::thread([]() { io_global().run(); }).detach();
        });

        _tm.async_wait(_fn);
    }

    void reset(unsigned long long us)
    {
        _tm.expires_after(std::chrono::microseconds(us));
        _tm.async_wait(_fn);
    }

    template <typename F>
    void reset(unsigned long long us, F &&f)
    {
        callback_t fn = std::forward<F>(f);
        _fn = [fn = std::move(fn)](const boost::system::error_code &err) {
            try
            {
                if(err.failed())
                    return;

                if(fn)
                    fn();
            }
            catch(...)
            {
                std::cerr << "timer callback exception" << std::endl;
            }
        };
        reset(us);
    }

    template <typename F>
    static std::shared_ptr<timer> make(unsigned long long us, F &&f)
    {
        auto tm = std::make_shared<timer>(us);
        tm->start(std::move(f));
        return tm;
    }

  private:
#if BOOST_VERSION < 108700
    static boost::asio::io_service &io_global()
    {
        static boost::asio::io_service io_global_inst;
        return io_global_inst;
    }
    static io_work_t &worker_global()
    {
        static io_work_t worker_global_inst{io_global()};
        return worker_global_inst;
    }
#else
    static io_t &io_global()
    {
        static io_t io_global;
        return io_global;
    }
    static boost::asio::executor_work_guard<io_t::executor_type> &
    worker_global()
    {
        static boost::asio::executor_work_guard<io_t::executor_type>
            worker_global_inst{io_global().get_executor()};
        return worker_global_inst;
    }
#endif

    boost::asio::steady_timer                              _tm;
    std::function<void(const boost::system::error_code &)> _fn;
};


} // namespace hj

#endif