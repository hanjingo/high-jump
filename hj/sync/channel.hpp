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

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
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
    {
    }

    ~channel() { close(); }

    channel(const channel &)            = delete;
    channel &operator=(const channel &) = delete;
    channel(channel &&)                 = delete;
    channel &operator=(channel &&)      = delete;

    void close() noexcept
    {
        std::unique_lock<std::mutex> lock(_state_mutex);

        if(_state == state::closed)
            return;

        if(_state == state::closing)
        {
            _state_cv.wait(lock, [this] { return _state == state::closed; });
            return;
        }

        // Linearization point for close(): OPEN -> CLOSING.
        _state = state::closing;
        _state_cv.wait(lock, [this] { return _active_enqueuers == 0; });

        // No producer admitted before close can still modify the queue.
        _state = state::closed;
        lock.unlock();

        // Wake all blocked consumers so they can observe CLOSED + empty.
        _state_cv.notify_all();
    }

    bool is_closed() const noexcept
    {
        std::lock_guard<std::mutex> lock(_state_mutex);
        return _state == state::closed;
    }

    bool wait_dequeue(T &t)
    {
        std::unique_lock<std::mutex> lock(_state_mutex);

        for(;;)
        {
            if(_q.try_dequeue(t))
                return true;

            if(_state == state::closed)
                return false;

            _state_cv.wait(lock);
        }
    }

    bool wait_dequeue_timeout(T &t, std::int64_t timeout_ms)
    {
        if(timeout_ms <= 0)
            return try_dequeue(t);

        return wait_dequeue_for(t, std::chrono::milliseconds(timeout_ms));
    }

    template <typename Rep, typename Period>
    bool wait_dequeue_for(T &t, std::chrono::duration<Rep, Period> timeout)
    {
        const auto now = std::chrono::steady_clock::now();

        if(timeout <= std::chrono::duration<Rep, Period>::zero())
            return try_dequeue(t);

        const auto max_duration =
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::steady_clock::time_point::max() - now);

        const auto timeout_duration =
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                timeout);

        const auto deadline = timeout_duration >= max_duration
                                  ? std::chrono::steady_clock::time_point::max()
                                  : now + timeout_duration;

        return wait_dequeue_until(t, deadline);
    }

    bool wait_dequeue_until(T                                    &t,
                            std::chrono::steady_clock::time_point deadline)
    {
        std::unique_lock<std::mutex> lock(_state_mutex);

        for(;;)
        {
            if(_q.try_dequeue(t))
                return true;

            if(_state == state::closed)
                return false;

            if(std::chrono::steady_clock::now() >= deadline)
                return _q.try_dequeue(t);

            if(_state_cv.wait_until(lock, deadline) == std::cv_status::timeout)
                return _q.try_dequeue(t);
        }
    }

    bool try_dequeue(T &t) { return _q.try_dequeue(t); }
    bool operator>>(T &t) { return wait_dequeue(t); }
    template <typename U>
    channel &operator<<(U &&value)
    {
        (void) enqueue(std::forward<U>(value));
        return *this;
    }

    template <typename U>
    bool enqueue(U &&value)
    {
        if(!begin_enqueue())
            return false;

        try
        {
            const bool success = _q.enqueue(std::forward<U>(value));
            end_enqueue();
            return success;
        }
        catch(...)
        {
            end_enqueue();
            throw;
        }
    }

    template <typename... Args>
    bool emplace(Args &&...args)
    {
        if(!begin_enqueue())
            return false;

        try
        {
            const bool success = _q.enqueue(T(std::forward<Args>(args)...));
            end_enqueue();
            return success;
        }
        catch(...)
        {
            end_enqueue();
            throw;
        }
    }

    std::size_t size_approx() const noexcept { return _q.size_approx(); }

    bool empty() const noexcept { return size_approx() == 0; }

  private:
    enum class state : unsigned char
    {
        open,
        closing,
        closed
    };

    static std::size_t validate_capacity(std::size_t capacity)
    {
        if(capacity == 0)
        {
            throw std::invalid_argument(
                "hj::channel initial_capacity must be greater than 0.");
        }

        return capacity;
    }

    bool begin_enqueue()
    {
        std::lock_guard<std::mutex> lock(_state_mutex);

        if(_state != state::open)
            return false;

        ++_active_enqueuers;
        return true;
    }

    void end_enqueue() noexcept
    {
        {
            std::lock_guard<std::mutex> lock(_state_mutex);

            --_active_enqueuers;

            if(_state == state::closing && _active_enqueuers == 0)
                _state = state::closed;
        }

        // Notify both waiting consumers and a concurrent close().
        _state_cv.notify_all();
    }

    moodycamel::BlockingConcurrentQueue<T> _q;

    mutable std::mutex      _state_mutex;
    std::condition_variable _state_cv;
    state                   _state{state::open};
    std::size_t             _active_enqueuers{0};
};

} // namespace hj

#endif
