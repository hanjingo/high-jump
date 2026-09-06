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

    matrix_vertical_iterator()
        : _mat(nullptr)
        , _offset(0)
        , _n_row(0)
        , _n_col(0)
    {
    }

    matrix_vertical_iterator(Mat *mat, int n_row, int n_col, int offset = 0)
        : _mat(mat)
        , _offset(offset)
        , _n_row(n_row)
        , _n_col(n_col)
    {
    }

    reference operator*()
    {
        int col = _offset / _n_row;
        int row = _offset % _n_row;
        return (*_mat)(row, col);
    }

    const T &operator*() const
    {
        int col = _offset / _n_row;
        int row = _offset % _n_row;
        return (*_mat)(row, col);
    }

    matrix_vertical_iterator &operator++()
    {
        ++_offset;
        return *this;
    }
    matrix_vertical_iterator operator++(int)
    {
        matrix_vertical_iterator tmp = *this;
        ++_offset;
        return tmp;
    }
    matrix_vertical_iterator &operator--()
    {
        --_offset;
        return *this;
    }
    matrix_vertical_iterator operator--(int)
    {
        matrix_vertical_iterator tmp = *this;
        --_offset;
        return tmp;
    }

    matrix_vertical_iterator operator+(difference_type n) const
    {
        return matrix_vertical_iterator(_mat, _n_row, _n_col, _offset + n);
    }
    matrix_vertical_iterator &operator+=(difference_type n)
    {
        _offset += n;
        return *this;
    }
    matrix_vertical_iterator operator-(difference_type n) const
    {
        return matrix_vertical_iterator(_mat, _n_row, _n_col, _offset - n);
    }
    matrix_vertical_iterator &operator-=(difference_type n)
    {
        _offset -= n;
        return *this;
    }
    difference_type operator-(const matrix_vertical_iterator &other) const
    {
        return _offset - other._offset;
    }

    bool operator==(const matrix_vertical_iterator &other) const
    {
        return _mat == other._mat && _offset == other._offset;
    }
    bool operator!=(const matrix_vertical_iterator &other) const
    {
        return !(*this == other);
    }
    bool operator<(const matrix_vertical_iterator &other) const
    {
        return _offset < other._offset;
    }
    bool operator>(const matrix_vertical_iterator &other) const
    {
        return _offset > other._offset;
    }
    bool operator<=(const matrix_vertical_iterator &other) const
    {
        return _offset <= other._offset;
    }
    bool operator>=(const matrix_vertical_iterator &other) const
    {
        return _offset >= other._offset;
    }

    int  row() const { return _offset % _n_row; }
    int  col() const { return _offset / _n_row; }
    Mat *matrix() const { return _mat; }
    int  offset() const { return _offset; }

  private:
    Mat *_mat;
    int  _offset;
    int  _n_row;
    int  _n_col;
};
} // namespace hj

#endif