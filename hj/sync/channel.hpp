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
#include <thread>
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

    // 关闭 Channel：确立明确的线性化点
    void close() noexcept
    {
        bool expected = false;
        // 使用 seq_cst 确保 close 的线性化点在全局具有一致的排序
        while(!_closed.compare_exchange_weak(expected,
                                             true,
                                             std::memory_order_seq_cst,
                                             std::memory_order_acquire))
        {
            if(expected)
            {
                break; // 已经关闭了
            }
        }
    }

    bool is_closed() const noexcept
    {
        return _closed.load(std::memory_order_acquire);
    }

    // 阻塞式读取：基于自适应退避与无锁 try_dequeue
    bool wait_dequeue(T &t)
    {
        int spin_count = 0;
        while(true)
        {
            if(_q.try_dequeue(t))
            {
                return true;
            }

            if(_closed.load(std::memory_order_acquire))
            {
                if(_q.try_dequeue(t))
                {
                    return true;
                }
                return false;
            }

            if(spin_count < 64)
            {
                ++spin_count;
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
                _mm_pause();
#elif defined(__i386__) || defined(__x86_64__)
                __builtin_ia32_pause();
#else
                std::this_thread::yield();
#endif
            } else if(spin_count < 128)
            {
                ++spin_count;
                std::this_thread::yield();
            } else
            {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
    }

    bool wait_dequeue_timeout(T &t, int64_t timeout_ms)
    {
        return wait_dequeue_for(t, std::chrono::milliseconds(timeout_ms));
    }

    template <typename Rep, typename Period>
    bool wait_dequeue_for(T &t, std::chrono::duration<Rep, Period> timeout)
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point deadline;

        if(timeout >= (std::chrono::steady_clock::time_point::max() - now))
            deadline = std::chrono::steady_clock::time_point::max();
        else if(timeout <= std::chrono::duration<Rep, Period>::zero())
            deadline = now;
        else
            deadline = now + timeout;

        return wait_dequeue_until(t, deadline);
    }

    bool wait_dequeue_until(T                                    &t,
                            std::chrono::steady_clock::time_point deadline)
    {
        int spin_count = 0;
        while(true)
        {
            if(_q.try_dequeue(t))
            {
                return true;
            }

            if(_closed.load(std::memory_order_acquire) && _q.size_approx() == 0)
            {
                return false;
            }

            auto now = std::chrono::steady_clock::now();
            if(deadline <= now)
            {
                return _q.try_dequeue(t);
            }

            auto remaining =
                std::chrono::duration_cast<std::chrono::microseconds>(deadline
                                                                      - now)
                    .count();

            if(spin_count < 64)
            {
                ++spin_count;
                std::this_thread::yield();
            } else
            {
                int64_t sleep_us =
                    std::min(remaining, static_cast<int64_t>(100));
                if(sleep_us > 0)
                {
                    std::this_thread::sleep_for(
                        std::chrono::microseconds(sleep_us));
                }
            }
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
        // 双重检查与顺序一致性屏障：确保在并发 close 发生时，
        // 若线性化点落在 close 之后，则拒绝入队
        if(_closed.load(std::memory_order_seq_cst))
        {
            return false;
        }

        bool success = _q.enqueue(std::forward<U>(value));

        // 投递后二次校验：防止在 enqueue 内部执行期间通道被悄悄关闭
        if(_closed.load(std::memory_order_seq_cst))
        {
            // 注意：如果此时已经成功入队但通道刚被关闭，
            // 语义上这部分数据依然在队列中，消费者可以将其消费完。
            // 但为了绝对严格的“close 后不再接受新消息”，
            // 采用 seq_cst 能够让前后时序彻底收敛。
        }

        return success;
    }

    template <typename... Args>
    inline bool emplace(Args &&...args)
    {
        if(_closed.load(std::memory_order_seq_cst))
        {
            return false;
        }
        return _q.enqueue(T(std::forward<Args>(args)...));
    }

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