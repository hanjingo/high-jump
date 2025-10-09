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
#ifndef SAFE_VECTOR_HPP
#define SAFE_VECTOR_HPP

#include <oneapi/tbb/concurrent_vector.h>
#include <vector>
#include <functional>
#include <stdexcept>
#include <algorithm>

namespace hj
{
// NOTE: This container was implemented by onetbb
template <typename T>
class safe_vector
{
  public:
    using vector_t              = tbb::concurrent_vector<T>;
    using iterator_t            = typename vector_t::iterator;
    using const_iterator_t      = typename vector_t::const_iterator;
    using range_handler_t       = std::function<bool(T &)>;
    using const_range_handler_t = std::function<bool(const T &)>;
    using sort_handler_t        = std::function<bool(const T &, const T &)>;

  public:
    safe_vector() = default;
    virtual ~safe_vector() {}

    inline T &operator[](std::size_t index) { return _vector[index]; }

    inline void emplace(T &&value) { _vector.emplace_back(std::move(value)); }

    inline void emplace(const T &value) { _vector.emplace_back(value); }

    void emplace_bulk(const std::vector<T> &values)
    {
        for(const auto &v : values)
            _vector.emplace_back(v);
    }

    inline void unsafe_clear() { _vector.clear(); }

    inline std::size_t size() const { return _vector.size(); }

    inline bool empty() const { return _vector.empty(); }

    inline void swap(safe_vector &other) { _vector.swap(other._vector); }

    inline void range(const range_handler_t &fn)
    {
        for(auto itr = _vector.begin(); itr != _vector.end(); ++itr)
            if(!fn(*itr))
                return;
    }

    inline void range(const const_range_handler_t &fn) const
    {
        for(auto itr = _vector.begin(); itr != _vector.end(); ++itr)
            if(!fn(*itr))
                return;
    }

    inline void sort(const sort_handler_t &fn)
    {
        std::sort(_vector.begin(), _vector.end(), fn);
    }

    inline T &at(std::size_t index)
    {
        if(index >= _vector.size())
            throw std::out_of_range("safe_vector::at out of range");

        return _vector.at(index);
    }

    inline T &front()
    {
        if(_vector.empty())
            throw std::out_of_range("safe_vector::front empty");

        return _vector.front();
    }

    inline T &back()
    {
        if(_vector.empty())
            throw std::out_of_range("safe_vector::back empty");

        return _vector.back();
    }

  private:
    vector_t _vector;
};

}

#endif