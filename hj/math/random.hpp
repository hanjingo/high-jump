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

#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>
#include <mutex>

namespace hj
{

class random
{
  public:
    random()
        : _engine{std::random_device{}()}
    {
    }

    explicit random(unsigned int seed)
        : _engine{seed}
    {
    }

    ~random() = default;

    template <typename T>
    T range(T min, T max)
    {
        std::lock_guard<std::mutex>      lock(_mu);
        std::uniform_int_distribution<T> dist(min, max);
        return dist(_engine);
    }

    template <typename T>
    T range_real(T min, T max)
    {
        std::lock_guard<std::mutex>       lock(_mu);
        std::uniform_real_distribution<T> dist(min, max);
        return dist(_engine);
    }

    template <typename T>
    std::vector<T> range_bulk(T min, T max, size_t n)
    {
        std::lock_guard<std::mutex> lock(_mu);
        std::vector<T>              out;
        out.reserve(n);
        std::uniform_int_distribution<T> dist(min, max);
        for(size_t i = 0; i < n; ++i)
            out.push_back(dist(_engine));

        return out;
    }

    template <typename T>
    T normal(T mean, T stddev)
    {
        std::lock_guard<std::mutex> lock(_mu);
        std::normal_distribution<T> dist(mean, stddev);
        return dist(_engine);
    }

  private:
    std::default_random_engine _engine;
    std::mutex                 _mu;
};

}

#endif