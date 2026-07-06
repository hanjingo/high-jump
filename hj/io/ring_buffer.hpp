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

#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

#include <mutex>
#include <condition_variable>

#include <boost/circular_buffer.hpp>

namespace hj
{

template <typename T>
using ring_buffer = boost::circular_buffer<T>;

template <typename T>
class safe_ring_buffer
{
  public:
    safe_ring_buffer(size_t capacity)
        : _buffer(capacity)
    {
        if(capacity == 0)
            throw std::invalid_argument(
                "Ring buffer capacity must be greater than 0");
    }

    safe_ring_buffer(const safe_ring_buffer &)            = delete;
    safe_ring_buffer &operator=(const safe_ring_buffer &) = delete;

    safe_ring_buffer(safe_ring_buffer &&other) noexcept
        : _buffer(std::move(other._buffer))
    {
    }
    safe_ring_buffer &operator=(safe_ring_buffer &&other) noexcept
    {
        if(this != &other)
        {
            std::unique_lock<std::mutex> lock1(_mu, std::defer_lock);
            std::unique_lock<std::mutex> lock2(other._mu, std::defer_lock);
            std::lock(lock1, lock2);

            _buffer = std::move(other._buffer);
            if(!_buffer.empty())
                _not_empty.notify_all();
            if(!_buffer.full())
                _not_full.notify_all();
        }
        return *this;
    }

    ~safe_ring_buffer() = default;

    const T &operator[](size_t index) const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _buffer[index];
    }

    const T &at(size_t index) const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _buffer.at(index);
    }

    const T &front() const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _buffer.front();
    }

    const T &back() const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _buffer.back();
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _buffer.size();
    }

    size_t capacity() const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _buffer.capacity();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _buffer.empty();
    }

    bool full() const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _buffer.full();
    }

    size_t available() const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _buffer.capacity() - _buffer.size();
    }

    bool try_push_back(const T &x)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(_buffer.full())
            return false;

        _buffer.push_back(x);
        _not_empty.notify_one();
        return true;
    }

    bool push_back(const T &x, int timeout_ms = -1)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(timeout_ms < 0)
        {
            _not_full.wait(lock, [this]() { return !_buffer.full(); });
        } else
        {
            if(!_not_full.wait_for(lock,
                                   std::chrono::milliseconds(timeout_ms),
                                   [this]() { return !_buffer.full(); }))
                return false;
        }

        _buffer.push_back(x);
        _not_empty.notify_one();
        return true;
    }

    bool try_push_back(T &&x)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(_buffer.full())
            return false;

        _buffer.push_back(std::move(x));
        _not_empty.notify_one();
        return true;
    }

    bool push_back(T &&x, int timeout_ms = -1)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(timeout_ms < 0)
        {
            _not_full.wait(lock, [this]() { return !_buffer.full(); });
        } else
        {
            if(!_not_full.wait_for(lock,
                                   std::chrono::milliseconds(timeout_ms),
                                   [this]() { return !_buffer.full(); }))
                return false;
        }

        _buffer.push_back(std::move(x));

        _not_empty.notify_one();
        return true;
    }

    template <typename Container>
    size_t
    push_back_batch(const Container &data, size_t data_sz, int timeout_ms = -1)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(timeout_ms < 0)
        {
            _not_full.wait(lock, [this]() { return !_buffer.full(); });
        } else
        {
            if(!_not_full.wait_for(lock,
                                   std::chrono::milliseconds(timeout_ms),
                                   [this]() { return !_buffer.full(); }))
                return 0;
        }

        size_t count = std::min(data_sz, _buffer.capacity() - _buffer.size());
        for(size_t i = 0; i < count; ++i)
            _buffer.push_back(data[i]);

        if(count > 1)
            _not_empty.notify_all();
        else if(count == 1)
            _not_empty.notify_one();
        return count;
    }

    bool try_push_front(const T &x)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(_buffer.full())
            return false;

        _buffer.push_front(x);
        _not_empty.notify_one();
        return true;
    }

    bool push_front(const T &x, int timeout_ms = -1)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(timeout_ms < 0)
        {
            _not_full.wait(lock, [this]() { return !_buffer.full(); });
        } else
        {
            if(!_not_full.wait_for(lock,
                                   std::chrono::milliseconds(timeout_ms),
                                   [this]() { return !_buffer.full(); }))
                return false;
        }

        _buffer.push_front(x);

        _not_empty.notify_one();
        return true;
    }

    bool try_push_front(T &&x)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(_buffer.full())
            return false;

        _buffer.push_front(std::move(x));
        _not_empty.notify_one();
        return true;
    }

    bool push_front(T &&x, int timeout_ms = -1)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(timeout_ms < 0)
        {
            _not_full.wait(lock, [this]() { return !_buffer.full(); });
        } else
        {
            if(!_not_full.wait_for(lock,
                                   std::chrono::milliseconds(timeout_ms),
                                   [this]() { return !_buffer.full(); }))
                return false;
        }

        _buffer.push_front(std::move(x));

        _not_empty.notify_one();
        return true;
    }

    template <typename Container>
    size_t
    push_front_batch(const Container &data, size_t data_sz, int timeout_ms = -1)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(timeout_ms < 0)
        {
            _not_full.wait(lock, [this]() { return !_buffer.full(); });
        } else
        {
            if(!_not_full.wait_for(lock,
                                   std::chrono::milliseconds(timeout_ms),
                                   [this]() { return !_buffer.full(); }))
                return 0;
        }

        size_t count = std::min(data_sz, _buffer.capacity() - _buffer.size());
        for(size_t i = 0; i < count; ++i)
            _buffer.push_front(data[i]);

        if(count > 1)
            _not_empty.notify_all();
        else if(count == 1)
            _not_empty.notify_one();
        return count;
    }

    bool try_insert(size_t index, const T &x)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(index > _buffer.size() || _buffer.full())
            return false;

        _buffer.insert(_buffer.begin() + index, x);
        _not_empty.notify_one();
        return true;
    }

    bool try_insert(size_t index, T &&x)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(index > _buffer.size() || _buffer.full())
            return false;

        _buffer.insert(_buffer.begin() + index, std::move(x));
        _not_empty.notify_one();
        return true;
    }

    std::optional<T> try_pop_front()
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(_buffer.empty())
            return std::nullopt;

        T x = std::move(_buffer.front());
        _buffer.pop_front();
        _not_full.notify_one();
        return x;
    }

    std::optional<T> pop_front(int timeout_ms = -1)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(timeout_ms < 0)
        {
            _not_empty.wait(lock, [this]() { return !_buffer.empty(); });
        } else
        {
            if(!_not_empty.wait_for(lock,
                                    std::chrono::milliseconds(timeout_ms),
                                    [this]() { return !_buffer.empty(); }))
                return std::nullopt;
        }

        T x = std::move(_buffer.front());
        _buffer.pop_front();
        _not_full.notify_one();
        return x;
    }

    template <typename Container>
    size_t pop_front_batch(Container &data, size_t data_sz, int timeout_ms = -1)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(timeout_ms < 0)
        {
            _not_empty.wait(lock, [this]() { return !_buffer.empty(); });
        } else
        {
            if(!_not_empty.wait_for(lock,
                                    std::chrono::milliseconds(timeout_ms),
                                    [this]() { return !_buffer.empty(); }))
                return 0;
        }

        size_t count = std::min(data_sz, _buffer.size());
        for(size_t i = 0; i < count; ++i)
        {
            data[i] = std::move(_buffer.front());
            _buffer.pop_front();
        }

        if(count > 1)
            _not_full.notify_all();
        else if(count == 1)
            _not_full.notify_one();
        return count;
    }

    std::optional<T> try_pop_back()
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(_buffer.empty())
            return std::nullopt;

        T x = std::move(_buffer.back());
        _buffer.pop_back();
        _not_full.notify_one();
        return x;
    }

    std::optional<T> pop_back(int timeout_ms = -1)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(timeout_ms < 0)
        {
            _not_empty.wait(lock, [this]() { return !_buffer.empty(); });
        } else
        {
            if(!_not_empty.wait_for(lock,
                                    std::chrono::milliseconds(timeout_ms),
                                    [this]() { return !_buffer.empty(); }))
                return std::nullopt;
        }

        T x = std::move(_buffer.back());
        _buffer.pop_back();
        _not_full.notify_one();
        return x;
    }

    template <typename Container>
    size_t pop_back_batch(Container &data, size_t data_sz, int timeout_ms = -1)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(timeout_ms < 0)
        {
            _not_empty.wait(lock, [this]() { return !_buffer.empty(); });
        } else
        {
            if(!_not_empty.wait_for(lock,
                                    std::chrono::milliseconds(timeout_ms),
                                    [this]() { return !_buffer.empty(); }))
                return 0;
        }

        size_t count = std::min(data_sz, _buffer.size());
        for(size_t i = 0; i < count; ++i)
        {
            data[i] = std::move(_buffer.back());
            _buffer.pop_back();
        }

        if(count > 1)
            _not_full.notify_all();
        else if(count == 1)
            _not_full.notify_one();
        return count;
    }

    void erase(size_t index)
    {
        std::unique_lock<std::mutex> lock(_mu);
        if(index < _buffer.size())
        {
            _buffer.erase(_buffer.begin() + index);
            _not_full.notify_one();
        }
    }

    void clear()
    {
        std::unique_lock<std::mutex> lock(_mu);
        _buffer.clear();
        _not_full.notify_all();
    }

    void swap(safe_ring_buffer &other)
    {
        if(this != &other)
        {
            std::unique_lock<std::mutex> lock1(_mu, std::defer_lock);
            std::unique_lock<std::mutex> lock2(other._mu, std::defer_lock);
            std::lock(lock1, lock2);

            _buffer.swap(other._buffer);

            if(!_buffer.empty())
                _not_empty.notify_all();
            if(!_buffer.full())
                _not_full.notify_all();

            if(!other._buffer.empty())
                other._not_empty.notify_all();
            if(!other._buffer.full())
                other._not_full.notify_all();
        }
    }

  private:
    mutable std::mutex        _mu;
    std::condition_variable   _not_empty;
    std::condition_variable   _not_full;
    boost::circular_buffer<T> _buffer;
};

} // namespace hj

#endif