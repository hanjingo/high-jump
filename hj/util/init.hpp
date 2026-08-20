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

#ifndef HJ_INIT_HPP
#define HJ_INIT_HPP

#include <vector>
#include <algorithm>
#include <functional>
#include <mutex>
#include <iostream>
#include <exception>

namespace hj
{

// Define priority levels
enum class priority : int
{
    HIGHEST = 1000,
    HIGH    = 500,
    NORMAL  = 0,
    LOW     = -500,
    LOWEST  = -1000
};

/**
 * @brief Internal registry for ordered, priority-based initialization.
 * 
 * Execution Model (Deferred Registry-based):
 * Callbacks registered here do not run upon construction; instead, they are 
 * collected, sorted by priority, and executed in a batch during explicit bootstrap.
 */
class registry
{
  public:
    struct Item
    {
        int                   priority;
        std::function<void()> cb;
    };

    static registry &instance()
    {
        static registry reg;
        return reg;
    }

    // Renamed parameter 'priority' to 'prio' to prevent shadowing the enum type hj::priority[cite: 24, 25]
    void add(int prio, std::function<void()> cb)
    {
        std::lock_guard<std::mutex> lock(mtx);
        items.push_back({prio, std::move(cb)});
        sorted = false;
    }

    /**
     * @brief Bootstraps and executes all registered initialization callbacks.
     * 
     * @warning EXPLICIT CONTRACT: 
     * To eliminate cross-TU SIOF (Static Initialization Order Fiasco) and partial 
     * registration hazards, automatic pre-main runners have been completely removed. 
     * Users MUST explicitly invoke hj::registry::instance().bootstrap() at the very 
     * beginning of main() after all static translation units have safely constructed.
     */
    void bootstrap()
    {
        std::lock_guard<std::mutex> lock(mtx);
        if(executed)
            return;

        if(items.empty())
            return;

        if(!sorted)
        {
            // Use std::stable_sort to preserve the relative registration order
            // for items sharing the exact same priority level.
            std::stable_sort(items.begin(),
                             items.end(),
                             [](const Item &a, const Item &b) {
                                 return a.priority > b.priority;
                             });
            sorted = true;
        }

        for(auto &item : items)
        {
            try
            {
                item.cb();
            }
            catch(...)
            {
                std::cerr << "Critical: Exception during HJ_INIT execution."
                          << std::endl;
                std::terminate();
            }
        }
        executed = true;
    }

  private:
    std::vector<Item> items;
    std::mutex        mtx;
    bool              sorted   = false;
    bool              executed = false;
};

/**
 * @brief Helper class to execute logic during global initialization.
 * Non-copyable and non-movable to ensure lifecycle uniqueness.
 * 
 * Execution Model Note:
 * - Single-argument constructor: Executes the callback immediately inline.
 * - Two-argument constructor (used by HJ_INIT/HJ_INIT_PRIORITY): Defers execution 
 *   by registering the callback into the global registry for explicit ordered execution.
 */
class init final
{
  public:
    // Immediate execution overload
    explicit init(std::function<void()> &&cb) noexcept
    {
        if(cb)
        {
            try
            {
                cb();
            }
            catch(...)
            {
                std::cerr << "Critical: Exception in HJ_INIT." << std::endl;
                std::terminate();
            }
        }
    }

    // Deferred registry-based execution overload (Priority-aware)
    // Renamed parameter 'priority' to 'prio' to prevent shadowing the enum type hj::priority
    init(int prio, std::function<void()> &&cb) noexcept
    {
        if(cb)
        {
            try
            {
                registry::instance().add(prio, std::move(cb));
            }
            catch(...)
            {
                std::cerr << "Critical: Exception during HJ_INIT registration."
                          << std::endl;
                std::terminate();
            }
        }
    }

    init(const init &)            = delete;
    init &operator=(const init &) = delete;
    init(init &&)                 = delete;
    init &operator=(init &&)      = delete;
};

class init_once final
{
  public:
    explicit init_once(std::function<void()> &&cb) noexcept
    {
        std::call_once(flag_, [cb = std::move(cb)]() {
            try
            {
                if(cb)
                    cb();
            }
            catch(...)
            {
                std::cerr << "Critical: Exception in HJ_INIT_ONCE."
                          << std::endl;
                std::terminate();
            }
        });
    }

    init_once(const init_once &)            = delete;
    init_once &operator=(const init_once &) = delete;
    init_once(init_once &&)                 = delete;
    init_once &operator=(init_once &&)      = delete;

  private:
    std::once_flag flag_;
};

} // namespace hj

// Internal macro concatenation helper
#define HJ_INIT_CAT_IMPL_(a, b) a##b
#define HJ_INIT_CAT_(a, b) HJ_INIT_CAT_IMPL_(a, b)

/**
 * @brief Executes a block of code exactly once globally during static initialization.
 * @warning DEPRECATED MODEL: HJ_INIT_ONCE executes immediately inline and does not 
 * participate in the ordered, priority-based registry execution model used by HJ_INIT.
 */
#define HJ_INIT_ONCE(...)                                                      \
    namespace                                                                  \
    {                                                                          \
    ::hj::init_once HJ_INIT_CAT_(hj_sim_init_once_,                            \
                                 __COUNTER__)([]() { __VA_ARGS__; });          \
    }

/**
 * @brief Registers a block of code for priority-ordered execution before main().
 */
#define HJ_INIT_PRIORITY(prio, ...)                                            \
    namespace                                                                  \
    {                                                                          \
    ::hj::init HJ_INIT_CAT_(hj_reg_, __COUNTER__)(static_cast<int>(prio),      \
                                                  []() { __VA_ARGS__; });      \
    }

// Convenience wrapper for normal priority (deferred registry-based execution)
#define HJ_INIT(...) HJ_INIT_PRIORITY(::hj::priority::NORMAL, __VA_ARGS__)

#endif // HJ_INIT_HPP