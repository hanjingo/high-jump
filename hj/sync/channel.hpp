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

#include <concurrentqueue/blockingconcurrentqueue.h>

namespace hj
{

template <typename T>
struct channel
{
    channel(const std::size_t min_capa)
        : _q{min_capa * moodycamel::BlockingConcurrentQueue<T>::BLOCK_SIZE} {};
    ~channel() = default;

    channel(const channel &)            = delete;
    channel &operator=(const channel &) = delete;
    channel(channel &&)                 = delete;
    channel &operator=(channel &&)      = delete;

    inline channel &operator>>(T &t)
    {
        _q.try_dequeue(t); // noblock
        return *this;
    }

    inline channel &operator<<(const T &t)
    {
        _q.enqueue(t);
        return *this;
    }

    inline void wait_dequeue(T &t) { _q.wait_dequeue(t); }

    inline bool wait_dequeue_timeout(T &t, std::int64_t us)
    {
        return _q.wait_dequeue_timed(t, us);
    }

    inline bool try_dequeue(T &t) { return _q.try_dequeue(t); }

    inline void enqueue(const T &t) { _q.enqueue(t); }

  private:
    moodycamel::BlockingConcurrentQueue<T> _q;
};

}

#endif