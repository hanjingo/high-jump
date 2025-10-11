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

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <cstdint>
#include <vector>
#include <functional>

#include <hj/math/matrix_iterator.hpp>
#include <hj/math/matrix_vertical_iterator.hpp>

namespace hj
{

template <typename T>
class matrix
{
  public:
    matrix()
        : _row_n{0}
        , _col_n{0}
    {
        _buf = _create();
    }

    matrix(const int row_n, const int col_n)
        : _row_n{row_n}
        , _col_n{col_n}
    {
        _buf = _create();
    }

    matrix(const int row_n, const int col_n, T value)
        : _row_n{row_n}
        , _col_n{col_n}
    {
        _buf = _create();

        _copy_n(value);
    }

    matrix(const std::vector<std::vector<T>> &buf)
        : _row_n{int(buf.size())}
        , _col_n{_row_n > 0 ? int(buf[0].size()) : 0}
    {
        _buf = _create();

        _copy_from(buf);
    }

    matrix(const matrix &rhs)
        : _row_n{rhs._row_n}
        , _col_n{rhs._col_n}
    {
        _buf = _create(_row_n, _col_n);
        _copy_from(rhs);
    }

    ~matrix() { _clean(_buf); }

    matrix &operator=(const matrix &rhs)
    {
        if(this == &rhs)
            return *this;

        _clean(_buf);
        _row_n = rhs._row_n;
        _col_n = rhs._col_n;
        _buf   = _create(_row_n, _col_n);
        _copy_from(rhs);
        return *this;
    }

    inline T *operator[](const int row) { return _buf[row]; }

    inline const T *operator[](const int row) const { return _buf[row]; }

    inline T &operator()(const int row, const int col)
    {
        return _buf[row][col];
    }

    inline const T &operator()(const int row, const int col) const
    {
        return _buf[row][col];
    }

    inline friend bool operator==(const matrix &a, const matrix &b)
    {
        if(a._row_n != b._row_n || a._col_n != b._col_n)
            return false;

        for(int row = 0; row < a._row_n; ++row)
            for(int col = 0; col < a._col_n; ++col)
                if(a._buf[row][col] != b._buf[row][col])
                    return false;

        return true;
    }

    inline friend bool operator!=(const matrix &a, const matrix &b)
    {
        return !(a == b);
    }

    inline T &at(const int row, const int col)
    {
        if(row < 0 || row >= _row_n || col < 0 || col >= _col_n)
            throw std::out_of_range("Index out of range");

        return _buf[row][col];
    }

    inline std::pair<int, int> resize(int row_n, int col_n)
    {
        auto old = _buf;
        _buf     = _create(row_n, col_n);
        _copy_from(old, row_n, col_n);
        _clean(old);

        _row_n = row_n;
        _col_n = col_n;
        return std::make_pair(std::move(row_n), std::move(col_n));
    }

    inline int64_t size() { return _row_n * _col_n; }

    inline int row_n() { return _row_n; }

    inline int col_n() { return _col_n; }

    inline T **date() { return _buf; }

    matrix_iterator<matrix<T>, T> begin()
    {
        return matrix_iterator<matrix<T>, T>(this, row_n(), col_n(), 0);
    }
    matrix_iterator<matrix<T>, T> end()
    {
        return matrix_iterator<matrix<T>, T>(this,
                                             row_n(),
                                             col_n(),
                                             row_n() * col_n());
    }

    matrix_vertical_iterator<matrix<T>, T> vbegin()
    {
        return matrix_vertical_iterator<matrix<T>, T>(this,
                                                      row_n(),
                                                      col_n(),
                                                      0);
    }
    matrix_vertical_iterator<matrix<T>, T> vend()
    {
        return matrix_vertical_iterator<matrix<T>, T>(this,
                                                      row_n(),
                                                      col_n(),
                                                      col_n() * row_n());
    }

    matrix_iterator<matrix<T>, T> find(const int row, const int col)
    {
        return matrix_iterator<matrix<T>, T>(this,
                                             row_n(),
                                             col_n(),
                                             row * col_n() + col);
    }

    matrix_vertical_iterator<matrix<T>, T> vfind(const int row, const int col)
    {
        return matrix_vertical_iterator<matrix<T>, T>(this,
                                                      row_n(),
                                                      col_n(),
                                                      col * row_n() + row);
    }

  private:
    void _clean(T **buf)
    {
        for(auto row = 0; row < _row_n; ++row)
        {
            delete[] buf[row];
        }

        delete[] buf;
    }

    T **_create() { return _create(_row_n, _col_n); }

    T **_create(const int row_n, const int col_n)
    {
        T **bak = new T *[col_n];
        for(auto row = 0; row < row_n; ++row)
            bak[row] = new T[col_n];

        return bak;
    }

    template <typename Container>
    void _copy_from(const Container &rhs)
    {
        _copy_from(rhs, _row_n, _col_n);
    }

    template <typename Container>
    void _copy_from(const Container &rhs, int row_n, int col_n)
    {
        row_n = _row_n < row_n ? _row_n : row_n;
        col_n = _col_n < col_n ? _col_n : col_n;
        for(int row = 0; row < row_n; ++row)
        {
            for(int col = 0; col < col_n; ++col)
                _buf[row][col] = rhs[row][col];
        }
    }

    void _copy_from(const matrix &rhs)
    {
        for(int row = 0; row < _row_n; ++row)
            for(int col = 0; col < _col_n; ++col)
                _buf[row][col] = rhs._buf[row][col];
    }

    void _copy_n(const T value, int n = -1)
    {
        n = n > -1 && n <= size() ? n : size();
        for(int row = 0; row < _row_n; ++row)
        {
            for(int col = 0; col < _col_n; ++col)
            {
                if(n == 0)
                    return;

                _buf[row][col] = value;
                n--;
            }
        }
    }

  private:
    T **_buf;
    int _row_n;
    int _col_n;
};

}

#endif
