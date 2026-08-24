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
 *  other than warranties of merchantability or fitness for a particular purpose.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef OBJECT_POOL_HPP
#define OBJECT_POOL_HPP

#include <atomic>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef NDEBUG
#include <unordered_set>
#endif

#include <boost/pool/pool.hpp>

#include <concurrentqueue/moodycamel/concurrentqueue.h>

namespace hj
{

template <typename T>
class object_pool
{
    static_assert(std::is_object_v<T>,
                  "object_pool requires T to be a non-reference object type.");
    static_assert(!std::is_array_v<T>,
                  "object_pool does not support array types for T.");
    static_assert(std::is_nothrow_destructible_v<T>,
                  "object_pool requires T to be nothrow destructible.");

    struct alignas(std::max(
        {alignof(T), alignof(std::max_align_t), alignof(void *)})) node_wrapper
    {
        const object_pool *owner_pool;

#ifndef NDEBUG
        std::atomic<bool> in_pool{false};
#endif

        alignas(T) char storage[sizeof(T)];

        T *get_object() noexcept { return reinterpret_cast<T *>(storage); }
        const T *get_object() const noexcept
        {
            return reinterpret_cast<const T *>(storage);
        }

        static node_wrapper *from_object(T *ptr) noexcept
        {
            return reinterpret_cast<node_wrapper *>(
                reinterpret_cast<char *>(ptr)
                - offsetof(node_wrapper, storage));
        }
    };

    struct aligned_user_allocator
    {
        using size_type       = std::size_t;
        using difference_type = std::ptrdiff_t;

        static char *malloc(size_type bytes)
        {
            constexpr std::size_t alignment = alignof(node_wrapper);
            if constexpr(alignment <= alignof(std::max_align_t))
            {
                return static_cast<char *>(std::malloc(bytes));
            } else
            {
                std::size_t aligned_bytes =
                    ((bytes + alignment - 1) / alignment) * alignment;
#if defined(_MSC_VER)
                return static_cast<char *>(
                    _aligned_malloc(aligned_bytes, alignment));
#else
                void *ptr = nullptr;
                if(posix_memalign(&ptr, alignment, aligned_bytes) != 0)
                    return nullptr;
                return static_cast<char *>(ptr);
#endif
            }
        }

        static void free(char *block)
        {
            if(!block)
                return;
            constexpr std::size_t alignment = alignof(node_wrapper);
            if constexpr(alignment <= alignof(std::max_align_t))
            {
                std::free(block);
            } else
            {
#if defined(_MSC_VER)
                _aligned_free(block);
#else
                std::free(block);
#endif
            }
        }
    };

  public:
    struct deleter
    {
        object_pool *pool{nullptr};

        void operator()(T *ptr) const noexcept
        {
            if(pool && ptr)
                pool->release(ptr);
        }
    };

    using ptr_type = std::unique_ptr<T, deleter>;

    object_pool()
        : _pool(sizeof(node_wrapper))
    {
    }

    ~object_pool() noexcept { clear(); }

    object_pool(const object_pool &)            = delete;
    object_pool &operator=(const object_pool &) = delete;
    object_pool(object_pool &&)                 = delete;
    object_pool &operator=(object_pool &&)      = delete;

    inline std::size_t size_approx() const noexcept
    {
        return _container.size_approx();
    }

    T *acquire() noexcept
    {
        node_wrapper *wrapper = nullptr;
        if(_container.try_dequeue(wrapper))
        {
            if(wrapper)
            {
#ifndef NDEBUG
                bool expected = true;
                bool exchanged =
                    wrapper->in_pool.compare_exchange_strong(expected, false);
                assert(exchanged);
#endif
                return wrapper->get_object();
            }
        }
        return nullptr;
    }

    template <typename... Args>
    T *acquire_or_create(Args &&...args)
    {
        T *ptr = acquire();
        if(ptr != nullptr)
            return ptr;

        return create_raw(std::forward<Args>(args)...);
    }

    ptr_type acquire_smart() noexcept
    {
        T *ptr = acquire();
        if(ptr == nullptr)
            return ptr_type(nullptr, deleter{this});

        return ptr_type(ptr, deleter{this});
    }

    template <typename... Args>
    ptr_type acquire_or_create_smart(Args &&...args)
    {
        T *ptr = acquire_or_create(std::forward<Args>(args)...);
        return ptr_type(ptr, deleter{this});
    }

    void acquire_bulk(std::vector<T *> &out, std::size_t n)
    {
        out.clear();
        if(n == 0)
            return;

        std::vector<node_wrapper *> wrappers(n, nullptr);
        std::size_t actual = _container.try_dequeue_bulk(wrappers.data(), n);
        wrappers.resize(actual);

        out.reserve(actual);
        for(auto *w : wrappers)
        {
            if(w)
            {
#ifndef NDEBUG
                bool expected = true;
                bool exchanged =
                    w->in_pool.compare_exchange_strong(expected, false);
                assert(exchanged);
#endif
                out.push_back(w->get_object());
            }
        }
    }

    void release(T *obj) noexcept
    {
        if(obj == nullptr)
            return;

        node_wrapper *wrapper = node_wrapper::from_object(obj);

        assert(wrapper->owner_pool == this);

#ifndef NDEBUG
        {
            std::lock_guard<std::mutex> registry_lock(_registry_mutex);
            assert(_allocated_wrappers.find(obj) != _allocated_wrappers.end());
        }
#endif

#ifndef NDEBUG
        bool expected = false;
        bool exchanged =
            wrapper->in_pool.compare_exchange_strong(expected, true);
        assert(exchanged);
#endif

        _container.enqueue(wrapper);
    }

    void release_bulk(const std::vector<T *> &in)
    {
        if(in.empty())
            return;

        std::vector<node_wrapper *> wrappers;
        wrappers.reserve(in.size());
        for(auto *obj : in)
        {
            if(!obj)
                continue;

            node_wrapper *wrapper = node_wrapper::from_object(obj);
            assert(wrapper->owner_pool == this);

#ifndef NDEBUG
            {
                std::lock_guard<std::mutex> registry_lock(_registry_mutex);
                assert(_allocated_wrappers.find(obj)
                       != _allocated_wrappers.end());
            }
#endif

#ifndef NDEBUG
            bool expected = false;
            bool exchanged =
                wrapper->in_pool.compare_exchange_strong(expected, true);
            assert(exchanged);
#endif
            wrappers.push_back(wrapper);
        }

        if(!wrappers.empty())
            _container.enqueue_bulk(wrappers.data(), wrappers.size());
    }

    void trim(std::size_t max_keep) noexcept
    {
        while(true)
        {
            std::size_t current_size = _container.size_approx();
            if(current_size <= max_keep)
                break;

            node_wrapper *wrapper = nullptr;
            if(!_container.try_dequeue(wrapper))
                break;

            if(wrapper)
            {
#ifndef NDEBUG
                bool expected = true;
                wrapper->in_pool.compare_exchange_strong(expected, false);

                {
                    std::lock_guard<std::mutex> registry_lock(_registry_mutex);
                    _allocated_wrappers.erase(wrapper->get_object());
                }
#endif
                T *obj = wrapper->get_object();
                obj->~T();

                std::lock_guard<std::mutex> lock(_pool_mutex);
                _pool.free(wrapper);
            }
        }
    }

    void clear() noexcept
    {
        node_wrapper *wrapper = nullptr;
        while(_container.try_dequeue(wrapper))
        {
            if(!wrapper)
                continue;

#ifndef NDEBUG
            bool expected = true;
            wrapper->in_pool.compare_exchange_strong(expected, false);

            {
                std::lock_guard<std::mutex> registry_lock(_registry_mutex);
                _allocated_wrappers.erase(wrapper->get_object());
            }
#endif
            T *obj = wrapper->get_object();
            obj->~T();

            std::lock_guard<std::mutex> lock(_pool_mutex);
            _pool.free(wrapper);
        }

#ifndef NDEBUG
        std::lock_guard<std::mutex> registry_lock(_registry_mutex);
        _allocated_wrappers.clear();
#endif

        std::lock_guard<std::mutex> lock(_pool_mutex);
        _pool.purge_memory();
    }

  private:
    template <typename... Args>
    T *create_raw(Args &&...args)
    {
        void *raw_ptr = nullptr;
        {
            std::lock_guard<std::mutex> lock(_pool_mutex);
            raw_ptr = _pool.malloc();
        }

        if(!raw_ptr)
            throw std::bad_alloc();

        node_wrapper *wrapper = static_cast<node_wrapper *>(raw_ptr);
        wrapper->owner_pool   = this;

#ifndef NDEBUG
        wrapper->in_pool.store(false);
        {
            std::lock_guard<std::mutex> registry_lock(_registry_mutex);
            _allocated_wrappers.insert(wrapper->get_object());
        }
#endif

        T *ptr = wrapper->get_object();

        try
        {
            new(ptr) T(std::forward<Args>(args)...);
        }
        catch(...)
        {
#ifndef NDEBUG
            {
                std::lock_guard<std::mutex> registry_lock(_registry_mutex);
                _allocated_wrappers.erase(ptr);
            }
#endif
            std::lock_guard<std::mutex> lock(_pool_mutex);
            _pool.free(wrapper);
            throw;
        }

        return ptr;
    }

  private:
    std::mutex                                  _pool_mutex;
    boost::pool<aligned_user_allocator>         _pool;
    moodycamel::ConcurrentQueue<node_wrapper *> _container;

#ifndef NDEBUG
    std::mutex                 _registry_mutex;
    std::unordered_set<void *> _allocated_wrappers;
#endif
};

} // namespace hj

#endif // OBJECT_POOL_HPP