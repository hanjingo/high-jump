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
#ifndef OBJECT_POOL_HPP
#define OBJECT_POOL_HPP

#include <cassert>
#include <boost/pool/object_pool.hpp>
#include <concurrentqueue/blockingconcurrentqueue.h>

namespace hj
{

template <typename T>
class object_pool
{
  public:
    object_pool() = default;
    ~object_pool() { clear(); }

    object_pool(const object_pool &)            = delete;
    object_pool &operator=(const object_pool &) = delete;
    object_pool(object_pool &&)                 = delete;
    object_pool &operator=(object_pool &&)      = delete;

    inline std::size_t size() const noexcept
    {
        return _container.size_approx();
    }

    template <typename... Args>
    T *create(Args &&...args)
    {
        T *ptr = _pool.malloc();
        if(!ptr)
            throw std::bad_alloc();

        new(ptr) T(std::forward<Args>(args)...);
        release(ptr);
        return ptr;
    }

    T *allocate()
    {
        T *ptr = _pool.malloc();
        if(!ptr)
            throw std::bad_alloc();

        return ptr;
    }

    T *acquire()
    {
        T *ret = nullptr;
        if(!_container.try_dequeue(ret))
            return nullptr;
        return ret;
    }

    void acquire_bulk(std::vector<T *> &out, size_t n)
    {
        out.clear();
        if(n == 0)
            return;

        out.resize(n, nullptr);
        size_t actual = _container.try_dequeue_bulk(out.data(), n);
        out.resize(actual);
    }

    void release(T *obj)
    {
        assert(obj != nullptr);
        _container.enqueue(obj);
    }

    void release_bulk(const std::vector<T *> &in)
    {
        if(in.empty())
            return;

        _container.enqueue_bulk(in.data(), in.size());
    }

    void clear() noexcept
    {
        T *obj = nullptr;
        while(_container.try_dequeue(obj))
        {
        }
    }

  private:
    boost::object_pool<T>                    _pool;
    moodycamel::BlockingConcurrentQueue<T *> _container;
};

}

#endif