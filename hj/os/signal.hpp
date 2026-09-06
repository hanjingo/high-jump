/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025-2026 hanjingo <hehehunanchina@live.com>
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

#ifndef SIGNAL_HPP
#define SIGNAL_HPP

#include <array>
#include <atomic>
#include <csignal>
#include <cstdint>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace hj
{

using sig_t = int;

// 队列满时的丢包处理策略
enum class sig_overflow_policy
{
    reject,   // 拒绝新信号，保持旧信号
    overwrite // 覆盖最旧的未处理信号
};

// 被信号打断的阻塞系统调用处理策略 (POSIX sigaction SA_RESTART 语义)
enum class sig_interrupt_policy
{
    restart, // 自动重启被打断的系统调用 (SA_RESTART)
    none     // 不自动重启，使阻塞系统调用返回 -1 且 errno = EINTR
};

class sighandler
{
  public:
    static constexpr size_t RING_BUFFER_SIZE = 128; // 必须是 2 的幂

    sighandler()
        : _head(0)
        , _tail(0)
        , _overflow_policy(sig_overflow_policy::reject)
        , _dropped_count(0)
    {
        clear_queue();
    }

    ~sighandler() { _g_inst.store(nullptr, std::memory_order_release); }

    sighandler(const sighandler &)            = delete;
    sighandler &operator=(const sighandler &) = delete;
    sighandler(sighandler &&)                 = delete;
    sighandler &operator=(sighandler &&)      = delete;

    static sighandler &instance() noexcept
    {
        static sighandler inst;
        _g_inst.store(&inst, std::memory_order_release);
        return inst;
    }

    // 清空内部环形队列与丢包统计（用于测试隔离和系统重置）
    void clear_queue() noexcept
    {
        std::lock_guard<std::mutex> lock(_mu);
        _head.store(0, std::memory_order_relaxed);
        _tail.store(0, std::memory_order_relaxed);
        _dropped_count.store(0, std::memory_order_relaxed);

        for(size_t i = 0; i < RING_BUFFER_SIZE; ++i)
        {
            _ring_buffer[i].sig.store(0, std::memory_order_relaxed);
            _ring_buffer[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    // 设置与获取队列溢出策略
    void set_overflow_policy(sig_overflow_policy policy) noexcept
    {
        std::lock_guard<std::mutex> lock(_mu);
        _overflow_policy = policy;
    }

    sig_overflow_policy overflow_policy() const noexcept
    {
        return _overflow_policy;
    }

    uint64_t dropped_count() const noexcept
    {
        return _dropped_count.load(std::memory_order_relaxed);
    }

    void reset_dropped_count() noexcept
    {
        _dropped_count.store(0, std::memory_order_relaxed);
    }

    bool is_registered(sig_t sig) noexcept
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _callbacks.find(sig) != _callbacks.end();
    }

    bool is_one_shot(sig_t sig) noexcept
    {
        std::lock_guard<std::mutex> lock(_mu);
        auto                        it = _one_shot.find(sig);
        return (it != _one_shot.end()) && it->second;
    }

    [[deprecated("Use is_one_shot instead")]]
    bool is_one_shoot(sig_t sig) noexcept
    {
        return is_one_shot(sig);
    }

    // 注册信号捕获，可指定是否为 one_shot 及 interrupt_policy (SA_RESTART)
    void
    sigcatch(sig_t                      sig,
             std::function<void(sig_t)> cb,
             bool                       one_shot = false,
             sig_interrupt_policy int_policy = sig_interrupt_policy::restart)
    {
        std::lock_guard<std::mutex> lock(_mu);
        _callbacks[sig] = std::move(cb);
        _one_shot[sig]  = one_shot;

        _install_sys_handler(sig, int_policy);
    }

    void
    sigcatch(const std::vector<sig_t>  &sigs,
             std::function<void(sig_t)> cb,
             bool                       one_shot = false,
             sig_interrupt_policy int_policy = sig_interrupt_policy::restart)
    {
        for(auto sig : sigs)
        {
            sigcatch(sig, cb, one_shot, int_policy);
        }
    }

    void sigunregister(sig_t sig)
    {
        std::lock_guard<std::mutex> lock(_mu);
        _callbacks.erase(sig);
        _one_shot.erase(sig);

#if defined(_WIN32) || defined(_WIN64)
        ::signal(sig, SIG_IGN);
#else
        struct sigaction sa{};
        sa.sa_handler = SIG_IGN;
        ::sigaction(sig, &sa, nullptr);
#endif
    }

    void sigunregister(const std::vector<sig_t> &sigs)
    {
        for(auto sig : sigs)
            sigunregister(sig);
    }

    void sigignore(const std::vector<sig_t> &sigs)
    {
        std::lock_guard<std::mutex> lock(_mu);
        for(auto sig : sigs)
        {
            _callbacks.erase(sig);
            _one_shot.erase(sig);
#if defined(_WIN32) || defined(_WIN64)
            ::signal(sig, SIG_IGN);
#else
            struct sigaction sa{};
            sa.sa_handler = SIG_IGN;
            ::sigaction(sig, &sa, nullptr);
#endif
        }
    }

    template <typename... Args>
    bool sigraise(Args... args)
    {
        static_assert((std::is_same_v<Args, sig_t> && ...),
                      "All arguments to sigraise must be of type sig_t");

#if defined(_WIN32) || defined(_WIN64)
        (_handle(args), ...);
        return true;
#else
        return ((::raise(args) == 0) && ...);
#endif
    }

    void poll()
    {
        while(true)
        {
            sig_t sig = _pop_signal();
            if(sig == 0)
                break;

            std::function<void(sig_t)> cb;
            bool                       is_one_shot_sig = false;

            {
                std::lock_guard<std::mutex> lock(_mu);
                auto                        it = _callbacks.find(sig);
                if(it != _callbacks.end())
                {
                    cb              = it->second;
                    is_one_shot_sig = _one_shot[sig];
                    if(is_one_shot_sig)
                    {
                        _callbacks.erase(it);
                        _one_shot.erase(sig);

#if defined(_WIN32) || defined(_WIN64)
                        ::signal(sig, SIG_IGN);
#else
                        struct sigaction sa{};
                        sa.sa_handler = SIG_IGN;
                        ::sigaction(sig, &sa, nullptr);
#endif
                    }
                }
            }

            if(cb)
            {
                try
                {
                    cb(sig);
                }
                catch(const std::exception &e)
                {
                    std::cerr << "Signal callback exception for sig " << sig
                              << ": " << e.what() << std::endl;
                }
                catch(...)
                {
                    std::cerr << "Unknown exception in signal callback for sig "
                              << sig << std::endl;
                }
            }
        }
    }

  private:
    struct alignas(64) Slot
    {
        std::atomic<sig_t>  sig{0};
        std::atomic<size_t> sequence{0};
    };

    static void _install_sys_handler(sig_t sig, sig_interrupt_policy int_policy)
    {
#if defined(_WIN32) || defined(_WIN64)
        (void) int_policy;
        ::signal(sig, &sighandler::_handle);
#else
        struct sigaction sa{};
        sa.sa_handler = &sighandler::_handle;
        sigemptyset(&sa.sa_mask);

        if(int_policy == sig_interrupt_policy::restart)
        {
            sa.sa_flags = SA_RESTART;
        } else
        {
            sa.sa_flags = 0; // 不带 SA_RESTART，慢速系统调用被打断时返回 EINTR
        }
        ::sigaction(sig, &sa, nullptr);
#endif
    }

    static void _handle(int sig) noexcept
    {
#if defined(_WIN32) || defined(_WIN64)
        ::signal(sig, &sighandler::_handle);
#endif

        sighandler *inst = _g_inst.load(std::memory_order_acquire);
        if(inst)
            inst->_push_signal(sig);
    }

    void _push_signal(sig_t sig) noexcept
    {
        size_t pos = _tail.load(std::memory_order_relaxed);

        while(true)
        {
            Slot    &slot = _ring_buffer[pos % RING_BUFFER_SIZE];
            size_t   seq  = slot.sequence.load(std::memory_order_acquire);
            intptr_t diff =
                static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);

            if(diff == 0)
            {
                if(_tail.compare_exchange_weak(pos,
                                               pos + 1,
                                               std::memory_order_relaxed))
                {
                    slot.sig.store(sig, std::memory_order_relaxed);
                    slot.sequence.store(pos + 1, std::memory_order_release);
                    return;
                }
            } else if(diff < 0)
            {
                _dropped_count.fetch_add(1, std::memory_order_relaxed);
                return;
            } else
            {
                pos = _tail.load(std::memory_order_relaxed);
            }
        }
    }

    sig_t _pop_signal() noexcept
    {
        size_t pos = _head.load(std::memory_order_relaxed);

        while(true)
        {
            size_t tail = _tail.load(std::memory_order_acquire);
            if(pos == tail)
            {
                return 0;
            }

            Slot    &slot = _ring_buffer[pos % RING_BUFFER_SIZE];
            size_t   seq  = slot.sequence.load(std::memory_order_acquire);
            intptr_t diff =
                static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);

            if(diff == 0)
            {
                if(_head.compare_exchange_weak(pos,
                                               pos + 1,
                                               std::memory_order_relaxed))
                {
                    sig_t sig = slot.sig.load(std::memory_order_relaxed);
                    slot.sequence.store(pos + RING_BUFFER_SIZE,
                                        std::memory_order_release);
                    return sig;
                }
            } else
            {
                std::this_thread::yield();
            }
        }
    }

    inline static std::atomic<sighandler *> _g_inst{nullptr};

    std::mutex                                            _mu;
    std::unordered_map<sig_t, std::function<void(sig_t)>> _callbacks;
    std::unordered_map<sig_t, bool>                       _one_shot;

    sig_overflow_policy   _overflow_policy;
    std::atomic<uint64_t> _dropped_count;

    std::array<Slot, RING_BUFFER_SIZE> _ring_buffer;
    std::atomic<size_t>                _head;
    std::atomic<size_t>                _tail;
};

} // namespace hj

#endif // SIGNAL_HPP