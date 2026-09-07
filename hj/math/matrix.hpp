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

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <cstddef>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <utility>
#include <algorithm>

#include <hj/math/matrix_iterator.hpp>
#include <hj/math/matrix_vertical_iterator.hpp>

namespace hj
{

template <typename T>
class matrix
{
  public:
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = T &;
    using const_reference = const T &;
    using pointer         = T *;
    using const_pointer   = const T *;

    matrix() noexcept
        : _row_n(0)
        , _col_n(0)
    {
    }

    matrix(size_type row_n, size_type col_n)
        : _row_n(row_n)
        , _col_n(col_n)
        , _data(row_n * col_n)
    {
    }

    matrix(size_type row_n, size_type col_n, const T &value)
        : _row_n(row_n)
        , _col_n(col_n)
        , _data(row_n * col_n, value)
    {
    }

    matrix(const std::vector<std::vector<T>> &buf)
    {
        _row_n = buf.size();
        _col_n = _row_n > 0 ? buf[0].size() : 0;
        _data.reserve(_row_n * _col_n);
        for(const auto &row : buf)
        {
            if(row.size() != _col_n)
            {
                throw std::invalid_argument(
                    "Inconsistent row sizes in input vector.");
            }
            _data.insert(_data.end(), row.begin(), row.end());
        }
    }

    matrix(const matrix &)                = default;
    matrix(matrix &&) noexcept            = default;
    matrix &operator=(const matrix &)     = default;
    matrix &operator=(matrix &&) noexcept = default;
    ~matrix()                             = default;

    class row_proxy
    {
      public:
        row_proxy(pointer ptr) noexcept
            : _ptr(ptr)
        {
        }
        reference operator[](size_type col) noexcept { return _ptr[col]; }

      private:
        pointer _ptr;
    };

    class const_row_proxy
    {
      public:
        const_row_proxy(const_pointer ptr) noexcept
            : _ptr(ptr)
        {
        }
        const_reference operator[](size_type col) const noexcept
        {
            return _ptr[col];
        }

      private:
        const_pointer _ptr;
    };

    row_proxy operator[](size_type row) noexcept
    {
        return row_proxy(_data.data() + row * _col_n);
    }

    const_row_proxy operator[](size_type row) const noexcept
    {
        return const_row_proxy(_data.data() + row * _col_n);
    }

    reference operator()(size_type row, size_type col) noexcept
    {
        return _data[row * _col_n + col];
    }

    const_reference operator()(size_type row, size_type col) const noexcept
    {
        return _data[row * _col_n + col];
    }

    reference at(size_type row, size_type col)
    {
        if(row >= _row_n || col >= _col_n)
            throw std::out_of_range("Matrix index out of range.");
        return _data[row * _col_n + col];
    }

    const_reference at(size_type row, size_type col) const
    {
        if(row >= _row_n || col >= _col_n)
            throw std::out_of_range("Matrix index out of range.");
        return _data[row * _col_n + col];
    }

    size_type size() const noexcept { return _data.size(); }
    size_type row_n() const noexcept { return _row_n; }
    size_type col_n() const noexcept { return _col_n; }
    bool      empty() const noexcept { return _data.empty(); }

    pointer       data() noexcept { return _data.data(); }
    const_pointer data() const noexcept { return _data.data(); }

    std::pair<size_type, size_type>
    resize(size_type new_row, size_type new_col, const T &value = T())
    {
        std::vector<T> new_data(new_row * new_col, value);
        size_type      min_row = std::min(_row_n, new_row);
        size_type      min_col = std::min(_col_n, new_col);

        for(size_type r = 0; r < min_row; ++r)
        {
            for(size_type c = 0; c < min_col; ++c)
            {
                new_data[r * new_col + c] = std::move((*this)(r, c));
            }
        }

        _data  = std::move(new_data);
        _row_n = new_row;
        _col_n = new_col;
        return {new_row, new_col};
    }

    friend bool operator==(const matrix &a, const matrix &b) noexcept
    {
        return a._row_n == b._row_n && a._col_n == b._col_n
               && a._data == b._data;
    }

    friend bool operator!=(const matrix &a, const matrix &b) noexcept
    {
        return !(a == b);
    }

    using iterator       = matrix_iterator<matrix<T>, T>;
    using const_iterator = matrix_iterator<const matrix<T>, const T>;

    iterator begin() noexcept
    {
        return iterator(this,
                        static_cast<int>(_row_n),
                        static_cast<int>(_col_n),
                        0);
    }
    iterator end() noexcept
    {
        return iterator(this,
                        static_cast<int>(_row_n),
                        static_cast<int>(_col_n),
                        static_cast<int>(size()));
    }

    const_iterator begin() const noexcept { return cbegin(); }
    const_iterator end() const noexcept { return cend(); }

    const_iterator cbegin() const noexcept
    {
        return const_iterator(this,
                              static_cast<int>(_row_n),
                              static_cast<int>(_col_n),
                              0);
    }
    const_iterator cend() const noexcept
    {
        return const_iterator(this,
                              static_cast<int>(_row_n),
                              static_cast<int>(_col_n),
                              static_cast<int>(size()));
    }

    using vertical_iterator = matrix_vertical_iterator<matrix<T>, T>;
    using const_vertical_iterator =
        matrix_vertical_iterator<const matrix<T>, const T>;

    vertical_iterator vbegin() noexcept
    {
        return vertical_iterator(this,
                                 static_cast<int>(_row_n),
                                 static_cast<int>(_col_n),
                                 0);
    }
    vertical_iterator vend() noexcept
    {
        return vertical_iterator(this,
                                 static_cast<int>(_row_n),
                                 static_cast<int>(_col_n),
                                 static_cast<int>(size()));
    }

    const_vertical_iterator vbegin() const noexcept { return vcbegin(); }
    const_vertical_iterator vend() const noexcept { return vcend(); }

    const_vertical_iterator vcbegin() const noexcept
    {
        return const_vertical_iterator(this,
                                       static_cast<int>(_row_n),
                                       static_cast<int>(_col_n),
                                       0);
    }
    const_vertical_iterator vcend() const noexcept
    {
        return const_vertical_iterator(this,
                                       static_cast<int>(_row_n),
                                       static_cast<int>(_col_n),
                                       static_cast<int>(size()));
    }

    iterator find(size_type row, size_type col) noexcept
    {
        return iterator(this,
                        static_cast<int>(_row_n),
                        static_cast<int>(_col_n),
                        static_cast<int>(row * _col_n + col));
    }

    const_iterator find(size_type row, size_type col) const noexcept
    {
        return const_iterator(this,
                              static_cast<int>(_row_n),
                              static_cast<int>(_col_n),
                              static_cast<int>(row * _col_n + col));
    }

    vertical_iterator vfind(size_type row, size_type col) noexcept
    {
        return vertical_iterator(this,
                                 static_cast<int>(_row_n),
                                 static_cast<int>(_col_n),
                                 static_cast<int>(col * _row_n + row));
    }

    const_vertical_iterator vfind(size_type row, size_type col) const noexcept
    {
        return const_vertical_iterator(this,
                                       static_cast<int>(_row_n),
                                       static_cast<int>(_col_n),
                                       static_cast<int>(col * _row_n + row));
    }

  private:
    size_type      _row_n{0};
    size_type      _col_n{0};
    std::vector<T> _data;
};

} // namespace hj

#endif