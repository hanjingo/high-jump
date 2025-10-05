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

#ifndef INIT_HPP
#define INIT_HPP

#include <list>
#include <functional>

namespace hj
{

class init final
{
  public:
    explicit init(std::function<void()> &&cb) noexcept
    {
        _cb = std::move(cb);

        if(_cb)
            _cb();
    }
    ~init() noexcept {}

    init(const init &other)       = delete;
    init &operator=(const init &) = delete;
    init(init &&other)            = delete;
    init &operator=(init &&other) = delete;

  private:
    std::function<void()> _cb;
};

}

#define __init_cat(a, b) a##b
#define _init_cat(a, b) __init_cat(a, b)

#define INIT(cmd)                                                              \
    namespace                                                                  \
    {                                                                          \
    ::hj::init _init_cat(__simulate_go_init__, __COUNTER__)([]() noexcept {    \
        try                                                                    \
        {                                                                      \
            cmd;                                                               \
        }                                                                      \
        catch(...)                                                             \
        {                                                                      \
        }                                                                      \
    });                                                                        \
    }

#endif