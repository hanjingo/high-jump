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
#ifndef DBUFFER_HPP
#define DBUFFER_HPP

#include <atomic>
#include <mutex>
#include <iostream>
#include <functional>

namespace hj
{

template <typename Container>
class dbuffer
{
  public:
    using copy_fn = std::function<bool(Container &src, Container &dst)>;

  public:
    dbuffer(copy_fn &&copy = nullptr)
        : _back{&_back_data}
        , _front{&_front_data}
        , _copy{copy ? std::move(copy)
                     : std::bind(&_default_copy,
                                 std::placeholders::_1,
                                 std::placeholders::_2)}
    {
    }

    dbuffer(const dbuffer &)            = delete;
    dbuffer &operator=(const dbuffer &) = delete;
    dbuffer(dbuffer &&)                 = default;
    dbuffer &operator=(dbuffer &&)      = default;

    ~dbuffer() {}

    bool write(Container &value)
    {
        Container *back = _back.load(std::memory_order_acquire);
        if(!_copy(value, *back))
            return false;

        swap();
        return true;
    }

    bool write(Container &&value)
    {
        Container *back = _back.load(std::memory_order_acquire);
        *back           = std::move(value);
        swap();
        return true;
    }

    bool read(Container &value)
    {
        Container *front = _front.load(std::memory_order_acquire);
        return _copy(*front, value);
    }

    void swap()
    {
        auto tmp = _front.load(std::memory_order_acquire);
        _front.store(_back.load(std::memory_order_relaxed),
                     std::memory_order_release);
        _back.store(tmp, std::memory_order_release);
    }

  private:
    static bool _default_copy(Container &src, Container &dst) noexcept
    {
        try
        {
            dst = src;
        }
        catch(...)
        {
            return false;
        }
        return true;
    }

  private:
    Container                _back_data;
    Container                _front_data;
    std::atomic<Container *> _back;
    std::atomic<Container *> _front;
    copy_fn                  _copy;
};

}

#endif