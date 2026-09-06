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

#include <algorithm>
#include <array>
#include <atomic>
#include <csignal>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <vector>

class SignalTest;

namespace hj
{

using sig_t = int;

enum class sig_overflow_policy
{
    reject,
    overwrite
};

enum class sig_interrupt_policy
{
    restart,
    none
};

class sighandler
{
  public:
    static constexpr size_t   MAX_SIGNALS            = 64;
    static constexpr uint32_t MAX_PENDING_PER_SIGNAL = 100;

    sighandler()
        : _overflow_policy(sig_overflow_policy::overwrite)
        , _dropped_count(0)
    {
        _clear_queue_internal();
    }

    ~sighandler() = default;

    sighandler(const sighandler &)            = delete;
    sighandler &operator=(const sighandler &) = delete;
    sighandler(sighandler &&)                 = delete;
    sighandler &operator=(sighandler &&)      = delete;

    static sighandler &instance() noexcept
    {
        static sighandler *inst = new sighandler();
        return *inst;
    }

    void set_overflow_policy(sig_overflow_policy policy) noexcept
    {
        _overflow_policy.store(policy, std::memory_order_relaxed);
    }

    sig_overflow_policy overflow_policy() const noexcept
    {
        return _overflow_policy.load(std::memory_order_relaxed);
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
        if(sig >= 0 && static_cast<size_t>(sig) < MAX_SIGNALS)
            _pending_counts[sig].store(0, std::memory_order_relaxed);

        _restore_sys_handler(sig);
    }

    void sigunregister(const std::vector<sig_t> &sigs)
    {
        for(auto sig : sigs)
            sigunregister(sig);
    }

    void sigignore(sig_t sig)
    {
        std::lock_guard<std::mutex> lock(_mu);
        _callbacks.erase(sig);
        _one_shot.erase(sig);

        if(sig >= 0 && static_cast<size_t>(sig) < MAX_SIGNALS)
            _pending_counts[sig].store(0, std::memory_order_relaxed);

#if defined(_WIN32) || defined(_WIN64)
        ::signal(sig, SIG_IGN);
#else
        struct sigaction sa{};
        sa.sa_handler = SIG_IGN;
        ::sigaction(sig, &sa, nullptr);
#endif
    }

    void sigignore(const std::vector<sig_t> &sigs)
    {
        for(auto sig : sigs)
            sigignore(sig);
    }

    void sigrestore_default(sig_t sig)
    {
        std::lock_guard<std::mutex> lock(_mu);
        _callbacks.erase(sig);
        _one_shot.erase(sig);

        if(sig >= 0 && static_cast<size_t>(sig) < MAX_SIGNALS)
            _pending_counts[sig].store(0, std::memory_order_relaxed);

#if defined(_WIN32) || defined(_WIN64)
        ::signal(sig, SIG_DFL);
#else
        struct sigaction sa{};
        sa.sa_handler = SIG_DFL;
        ::sigaction(sig, &sa, nullptr);
#endif
    }

    void sigrestore_default(const std::vector<sig_t> &sigs)
    {
        for(auto sig : sigs)
            sigrestore_default(sig);
    }

    template <typename... Args>
    bool sigraise(Args... args)
    {
        static_assert((std::is_same_v<Args, sig_t> && ...),
                      "All arguments to sigraise must be of type sig_t");

        return ((::raise(args) == 0) && ...);
    }

    template <typename... Args>
    void signotify(Args... args) noexcept
    {
        static_assert((std::is_same_v<Args, sig_t> && ...),
                      "All arguments to signotify must be of type sig_t");

        (_handle(args), ...);
    }

    template <typename... Args>
    void sigdispatch(Args... args) noexcept
    {
        signotify(args...);
    }

    size_t poll(size_t max_events = std::numeric_limits<size_t>::max())
    {
        size_t total_processed = 0;

        for(size_t sig = 1; sig < MAX_SIGNALS; ++sig)
        {
            if(total_processed >= max_events)
                break;

            uint32_t count =
                _pending_counts[sig].exchange(0, std::memory_order_acq_rel);
            if(count == 0)
                continue;

            std::function<void(sig_t)> cb;
            bool                       is_one_shot_sig = false;

            {
                std::lock_guard<std::mutex> lock(_mu);
                auto it = _callbacks.find(static_cast<sig_t>(sig));
                if(it != _callbacks.end())
                {
                    cb              = it->second;
                    is_one_shot_sig = _one_shot[static_cast<sig_t>(sig)];
                    if(is_one_shot_sig)
                    {
                        _callbacks.erase(it);
                        _one_shot.erase(static_cast<sig_t>(sig));
                        _restore_sys_handler(static_cast<sig_t>(sig));
                    }
                }
            }

            if(cb)
            {
                uint32_t exec_times = is_one_shot_sig ? 1 : count;

                size_t budget = max_events - total_processed;
                if(exec_times > budget)
                {
                    uint32_t remaining =
                        exec_times - static_cast<uint32_t>(budget);
                    _pending_counts[sig].fetch_add(remaining,
                                                   std::memory_order_relaxed);
                    exec_times = static_cast<uint32_t>(budget);
                }

                for(uint32_t i = 0; i < exec_times; ++i)
                {
                    try
                    {
                        cb(static_cast<sig_t>(sig));
                        total_processed++;
                    }
                    catch(const std::exception &e)
                    {
                        std::cerr << "Signal callback exception for sig " << sig
                                  << ": " << e.what() << std::endl;
                    }
                    catch(...)
                    {
                        std::cerr
                            << "Unknown exception in signal callback for sig "
                            << sig << std::endl;
                    }
                }
            }
        }

        return total_processed;
    }

  private:
    friend class ::SignalTest;

    void _clear_queue_internal() noexcept
    {
        std::lock_guard<std::mutex> lock(_mu);
        _dropped_count.store(0, std::memory_order_relaxed);
        for(size_t i = 0; i < MAX_SIGNALS; ++i)
            _pending_counts[i].store(0, std::memory_order_relaxed);
    }

    static_assert(std::atomic<uint32_t>::is_always_lock_free,
                  "std::atomic<uint32_t> MUST be hardware lock-free!");
    static_assert(std::atomic<uint64_t>::is_always_lock_free,
                  "std::atomic<uint64_t> MUST be hardware lock-free!");

    void _install_sys_handler(sig_t sig, sig_interrupt_policy int_policy)
    {
#if defined(_WIN32) || defined(_WIN64)
        (void) int_policy;
        auto prev = ::signal(sig, &sighandler::_handle);
        if(_has_saved_action.find(sig) == _has_saved_action.end())
        {
            _saved_win_handlers[sig] = prev;
            _has_saved_action[sig]   = true;
        }
#else
        struct sigaction sa{};
        sa.sa_handler = &sighandler::_handle;
        sigemptyset(&sa.sa_mask);

        if(int_policy == sig_interrupt_policy::restart)
            sa.sa_flags = SA_RESTART;
        else
            sa.sa_flags = 0;

        struct sigaction old_sa{};
        if(::sigaction(sig, &sa, &old_sa) == 0)
        {
            if(_has_saved_action.find(sig) == _has_saved_action.end())
            {
                _saved_posix_actions[sig] = old_sa;
                _has_saved_action[sig]    = true;
            }
        }
#endif
    }

    void _restore_sys_handler(sig_t sig)
    {
        auto it = _has_saved_action.find(sig);
        if(it == _has_saved_action.end())
        {
#if defined(_WIN32) || defined(_WIN64)
            ::signal(sig, SIG_DFL);
#else
            struct sigaction sa{};
            sa.sa_handler = SIG_DFL;
            ::sigaction(sig, &sa, nullptr);
#endif
            return;
        }

#if defined(_WIN32) || defined(_WIN64)
        ::signal(sig, _saved_win_handlers[sig]);
        _saved_win_handlers.erase(sig);
#else
        ::sigaction(sig, &_saved_posix_actions[sig], nullptr);
        _saved_posix_actions.erase(sig);
#endif
        _has_saved_action.erase(it);
    }

    static void _handle(int sig) noexcept
    {
#if defined(_WIN32) || defined(_WIN64)
        ::signal(sig, &sighandler::_handle);
#endif

        sighandler &inst = instance();
        if(sig >= 0 && static_cast<size_t>(sig) < MAX_SIGNALS)
        {
            sig_overflow_policy policy =
                inst._overflow_policy.load(std::memory_order_relaxed);
            uint32_t current =
                inst._pending_counts[sig].load(std::memory_order_relaxed);

            if(current >= MAX_PENDING_PER_SIGNAL)
            {
                inst._dropped_count.fetch_add(1, std::memory_order_relaxed);
                if(policy == sig_overflow_policy::reject)
                    return;

            } else
            {
                inst._pending_counts[sig].fetch_add(1,
                                                    std::memory_order_relaxed);
            }
        }
    }

    std::mutex                                            _mu;
    std::unordered_map<sig_t, std::function<void(sig_t)>> _callbacks;
    std::unordered_map<sig_t, bool>                       _one_shot;

#if defined(_WIN32) || defined(_WIN64)
    using win_handler_t = void (*)(int);
    std::unordered_map<sig_t, win_handler_t> _saved_win_handlers;
#else
    std::unordered_map<sig_t, struct sigaction> _saved_posix_actions;
#endif
    std::unordered_map<sig_t, bool> _has_saved_action;

    std::atomic<sig_overflow_policy> _overflow_policy;
    std::atomic<uint64_t>            _dropped_count;

    std::array<std::atomic<uint32_t>, MAX_SIGNALS> _pending_counts;
};

} // namespace hj

#endif // SIGNAL_HPP