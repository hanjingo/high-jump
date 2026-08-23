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
#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>

#include <concurrentqueue/moodycamel/blockingconcurrentqueue.h>

namespace hj
{

template <typename T>
class channel
{
  public:
    explicit channel(const std::size_t initial_capacity)
        : _q{validate_capacity(initial_capacity)}
        , _closed(false)
    {
    }

    ~channel() { close(); }

    channel(const channel &)            = delete;
    channel &operator=(const channel &) = delete;
    channel(channel &&)                 = delete;
    channel &operator=(channel &&)      = delete;

    void close() noexcept
    {
        bool expected = false;
        if(_closed.compare_exchange_strong(expected,
                                           true,
                                           std::memory_order_seq_cst))
        {
        }
    }

    bool is_closed() const noexcept
    {
        return _closed.load(std::memory_order_acquire);
    }

    bool wait_dequeue(T &t)
    {
        while(true)
        {
            if(_q.try_dequeue(t))
                return true;

            if(_closed.load(std::memory_order_acquire))
            {
                if(_q.try_dequeue(t))
                    return true;

                return false;
            }
            if(_q.wait_dequeue_timed(t, 10000))
                return true;

            if(_closed.load(std::memory_order_acquire) && _q.size_approx() == 0)
                return false;
        }
    }

    bool wait_dequeue_timeout(T &t, int64_t timeout_ms)
    {
        return wait_dequeue_for(t, std::chrono::milliseconds(timeout_ms));
    }

    template <typename Rep, typename Period>
    bool wait_dequeue_for(T &t, std::chrono::duration<Rep, Period> timeout)
    {
        auto deadline = std::chrono::steady_clock::now() + timeout;
        return wait_dequeue_until(t, deadline);
    }

    bool wait_dequeue_until(T                                    &t,
                            std::chrono::steady_clock::time_point deadline)
    {
        while(true)
        {
            if(_q.try_dequeue(t))
                return true;

            if(_closed.load(std::memory_order_acquire) && _q.size_approx() == 0)
                return false;

            auto now = std::chrono::steady_clock::now();
            if(deadline <= now)
                return _q.try_dequeue(t);

            auto duration =
                std::chrono::duration_cast<std::chrono::microseconds>(deadline
                                                                      - now);
            auto wait_us =
                std::min(duration.count(), static_cast<int64_t>(10000));
            if(_q.wait_dequeue_timed(t, wait_us))
                return true;
        }
    }

    bool try_dequeue(T &t) { return _q.try_dequeue(t); }

    bool operator>>(T &t) { return wait_dequeue(t); }

    template <typename U>
    inline channel &operator<<(U &&value)
    {
        enqueue(std::forward<U>(value));
        return *this;
    }

    template <typename U>
    inline bool enqueue(U &&value)
    {
        if(_closed.load(std::memory_order_relaxed))
            return false;

        return _q.enqueue(std::forward<U>(value));
    }

    template <typename... Args>
    inline bool emplace(Args &&...args)
    {
        if(_closed.load(std::memory_order_relaxed))
            return false;

        return _q.enqueue(T(std::forward<Args>(args)...));
    }

    inline std::size_t size() const noexcept { return _q.size_approx(); }
    inline std::size_t size_approx() const noexcept { return _q.size_approx(); }
    inline bool        empty() const noexcept { return _q.size_approx() == 0; }

  private:
    static std::size_t validate_capacity(std::size_t capacity)
    {
        if(capacity == 0)
        {
            throw std::invalid_argument(
                "hj::channel initial_capacity must be greater than 0.");
        }
        return capacity;
    }

    moodycamel::BlockingConcurrentQueue<T> _q;
    std::atomic<bool>                      _closed;
};

} // namespace hj

#endif