/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 */
#ifndef STOPWATCH_HPP
#define STOPWATCH_HPP

#include <chrono>

namespace hj
{
class stopwatch
{
  public:
    using clock_t      = std::chrono::steady_clock;
    using time_point_t = clock_t::time_point;
    using duration_t   = clock_t::duration;

  private:
    time_point_t _tp;

  public:
    stopwatch() noexcept
        : _tp(clock_t::now())
    {
    }

    ~stopwatch() = default;

    stopwatch(const stopwatch &)            = default;
    stopwatch &operator=(const stopwatch &) = default;

    stopwatch(stopwatch &&) noexcept            = default;
    stopwatch &operator=(stopwatch &&) noexcept = default;

    void reset() noexcept { _tp = clock_t::now(); }

    [[nodiscard]] duration_t elapsed() const noexcept
    {
        return clock_t::now() - _tp;
    }

    [[nodiscard]] long long elapsed_ns() const noexcept
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed())
            .count();
    }

    [[nodiscard]] long long elapsed_us() const noexcept
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(elapsed())
            .count();
    }

    [[nodiscard]] long long elapsed_ms() const noexcept
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed())
            .count();
    }

    [[nodiscard]] long long elapsed_secs() const noexcept
    {
        return std::chrono::duration_cast<std::chrono::seconds>(elapsed())
            .count();
    }
};
} // namespace hj

#define HJ_WATCH(cmd)                                                          \
    do                                                                         \
    {                                                                          \
        hj::stopwatch _hj_watch;                                               \
        cmd;                                                                   \
    } while(false);

#define HJ_WATCH_RESET() _hj_watch.reset()
#define HJ_WATCH_PASSED() _hj_watch.elapsed()
#define HJ_WATCH_PASSED_NS() _hj_watch.elapsed_ns()
#define HJ_WATCH_PASSED_US() _hj_watch.elapsed_us()
#define HJ_WATCH_PASSED_MS() _hj_watch.elapsed_ms()
#define HJ_WATCH_PASSED_SECS() _hj_watch.elapsed_secs()

#endif // STOPWATCH_HPP