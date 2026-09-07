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
#include <vector>
#include <type_traits>

namespace hj
{
namespace random
{

class engine
{
  public:
    using base_engine_type = std::mt19937_64;

    engine()
        : _engine{std::random_device{}()}
    {
    }

    explicit engine(unsigned long seed)
        : _engine{seed}
    {
    }

    ~engine() = default;

    engine(const engine &)            = delete;
    engine &operator=(const engine &) = delete;
    engine(engine &&)                 = delete;
    engine &operator=(engine &&)      = delete;

    static engine &instance()
    {
        thread_local engine _inst;
        return _inst;
    }

    template <typename T>
    T range(T min, T max)
    {
        static_assert(std::is_integral_v<T>,
                      "hj::random::range requires an integral type (e.g., int, "
                      "long, size_t)");
        std::uniform_int_distribution<T> dist(min, max);
        return dist(_engine);
    }

    template <typename T>
    T range_real(T min, T max)
    {
        static_assert(std::is_floating_point_v<T>,
                      "hj::random::range_real requires a floating-point type "
                      "(e.g., float, double)");
        std::uniform_real_distribution<T> dist(min, max);
        return dist(_engine);
    }

    template <typename T>
    std::vector<T> range_bulk(T min, T max, size_t n)
    {
        static_assert(std::is_integral_v<T>,
                      "hj::random::range_bulk requires an integral type");
        std::vector<T> out;
        out.reserve(n);
        std::uniform_int_distribution<T> dist(min, max);
        for(size_t i = 0; i < n; ++i)
        {
            out.push_back(dist(_engine));
        }

        return out;
    }

    template <typename T>
    T normal(T mean, T stddev)
    {
        static_assert(std::is_floating_point_v<T>,
                      "hj::random::normal requires a floating-point type");
        std::normal_distribution<T> dist(mean, stddev);
        return dist(_engine);
    }

  private:
    base_engine_type _engine;
};

template <typename T>
T range(T min, T max)
{
    return engine::instance().range<T>(min, max);
}

template <int Min, int Max>
int range()
{
    static_assert(Min <= Max, "Min must be less than or equal to Max");
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

#endif // RANDOM_HPP