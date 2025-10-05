/*
 *  This file is part of hj.
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

#ifndef DEFER_HPP
#define DEFER_HPP

#include <functional>

namespace hj
{

class defer final
{
  public:
    defer() noexcept {}
    ~defer() noexcept
    {
        if(_cb)
            _cb();
    }

    defer(const defer &other)       = delete;
    defer &operator=(const defer &) = delete;
    defer(defer &&other)            = delete;
    defer &operator=(defer &&other) = delete;

    explicit defer(std::function<void()> &&cb) { _cb = std::move(cb); }

  private:
    std::function<void()> _cb;
};

}

#define __defer_cat(a, b) a##b
#define _defer_cat(a, b) __defer_cat(a, b)

#define DEFER(cmd)                                                             \
    ::hj::defer _defer_cat(__simulate_go_defer__,                              \
                           __COUNTER__)([&]() noexcept {                       \
        try                                                                    \
        {                                                                      \
            cmd;                                                               \
        }                                                                      \
        catch(...)                                                             \
        {                                                                      \
        }                                                                      \
    });
#define DEFER_CLASS(cmd)                                                       \
    ::hj::defer _defer_cat(__simulate_go_defer_class__,                        \
                           __COUNTER__)([&, this]() noexcept {                 \
        try                                                                    \
        {                                                                      \
            cmd;                                                               \
        }                                                                      \
        catch(...)                                                             \
        {                                                                      \
        }                                                                      \
    });

#endif // DEFER_HPP
