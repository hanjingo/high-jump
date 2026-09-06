/*
 * This file is part of high-jump(hj).
 *
 * Copyright 2025 hanjingo <hehehunanchina@live.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>
#include <mutex>

namespace hj
{
namespace random
{

class engine
{
  public:
    engine()
        : _engine{std::random_device{}()}
    {
    }

    explicit engine(unsigned int seed)
        : _engine{seed}
    {
    }

    ~engine() = default;

    static engine &instance()
    {
        static engine _inst;
        return _inst;
    }

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

template <typename T>
T range(T min, T max)
{
    return engine::instance().range<T>(min, max);
}

template <int Min, int Max>
int range()
{
    return engine::instance().range<int>(Min, Max);
}

template <typename T>
T range_real(T min, T max)
{
    return engine::instance().range_real<T>(min, max);
}

template <typename T>
std::vector<T> range_bulk(T min, T max, size_t n)
{
    return engine::instance().range_bulk<T>(min, max, n);
}

template <typename T>
T normal(T mean, T stddev)
{
    return engine::instance().normal<T>(mean, stddev);
}

} // namespace random
} // namespace hj

#endif