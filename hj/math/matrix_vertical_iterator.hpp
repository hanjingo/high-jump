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

#ifndef MATRIX_VERTICAL_ITERATOR_HPP
#define MATRIX_VERTICAL_ITERATOR_HPP

#include <cstddef>
#include <iterator>

namespace hj
{

template <typename Mat, typename T>
struct matrix_vertical_iterator
{
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = T *;
    using reference         = T &;

    matrix_vertical_iterator() noexcept
        : _mat(nullptr)
        , _offset(0)
        , _n_row(0)
        , _n_col(0)
        , _row(0)
        , _col(0)
    {
    }

    matrix_vertical_iterator(Mat *mat,
                             int  n_row,
                             int  n_col,
                             int  offset = 0) noexcept
        : _mat(mat)
        , _offset(offset)
        , _n_row(n_row)
        , _n_col(n_col)
    {
        _sync_coords();
    }

    reference operator*() const { return (*_mat)(_row, _col); }

    pointer operator->() const { return &(*(*this)); }

    reference operator[](difference_type n) const { return *(*this + n); }

    matrix_vertical_iterator &operator++() noexcept
    {
        ++_offset;
        if(_n_row > 0)
        {
            if(++_row >= _n_row)
            {
                _row = 0;
                ++_col;
            }
        }
        return *this;
    }

    matrix_vertical_iterator operator++(int) noexcept
    {
        matrix_vertical_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    matrix_vertical_iterator &operator--() noexcept
    {
        --_offset;
        if(_n_row > 0)
        {
            if(_row == 0)
            {
                _row = _n_row - 1;
                --_col;
            } else
            {
                --_row;
            }
        }
        return *this;
    }

    matrix_vertical_iterator operator--(int) noexcept
    {
        matrix_vertical_iterator tmp = *this;
        --(*this);
        return tmp;
    }

    matrix_vertical_iterator operator+(difference_type n) const noexcept
    {
        return matrix_vertical_iterator(_mat,
                                        _n_row,
                                        _n_col,
                                        _offset + static_cast<int>(n));
    }

    matrix_vertical_iterator &operator+=(difference_type n) noexcept
    {
        _offset += static_cast<int>(n);
        _sync_coords();
        return *this;
    }

    matrix_vertical_iterator operator-(difference_type n) const noexcept
    {
        return matrix_vertical_iterator(_mat,
                                        _n_row,
                                        _n_col,
                                        _offset - static_cast<int>(n));
    }

    matrix_vertical_iterator &operator-=(difference_type n) noexcept
    {
        _offset -= static_cast<int>(n);
        _sync_coords();
        return *this;
    }

    difference_type
    operator-(const matrix_vertical_iterator &other) const noexcept
    {
        return _offset - other._offset;
    }

    bool operator==(const matrix_vertical_iterator &other) const noexcept
    {
        return _mat == other._mat && _offset == other._offset;
    }
    bool operator!=(const matrix_vertical_iterator &other) const noexcept
    {
        return !(*this == other);
    }
    bool operator<(const matrix_vertical_iterator &other) const noexcept
    {
        return _offset < other._offset;
    }
    bool operator>(const matrix_vertical_iterator &other) const noexcept
    {
        return _offset > other._offset;
    }
    bool operator<=(const matrix_vertical_iterator &other) const noexcept
    {
        return _offset <= other._offset;
    }
    bool operator>=(const matrix_vertical_iterator &other) const noexcept
    {
        return _offset >= other._offset;
    }

    int  row() const noexcept { return _row; }
    int  col() const noexcept { return _col; }
    Mat *matrix() const noexcept { return _mat; }
    int  offset() const noexcept { return _offset; }

  private:
    void _sync_coords() noexcept
    {
        if(_n_row > 0)
        {
            _col = _offset / _n_row;
            _row = _offset % _n_row;
        } else
        {
            _row = 0;
            _col = 0;
        }
    }

  private:
    Mat *_mat;
    int  _offset;
    int  _n_row;
    int  _n_col;
    int  _row{0};
    int  _col{0};
};

template <typename Mat, typename T>
inline matrix_vertical_iterator<Mat, T>
operator+(typename matrix_vertical_iterator<Mat, T>::difference_type n,
          const matrix_vertical_iterator<Mat, T> &it) noexcept
{
    return it + n;
}

} // namespace hj

#endif