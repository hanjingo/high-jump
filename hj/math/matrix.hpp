#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <cstdint>
#include <vector>
#include <functional>

#include <hj/math/matrix_iterator.hpp>
#include <hj/math/matrix_row_iterator.hpp>

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
        _buf = create_();
    }

    matrix(const int row_n, const int col_n)
        : _row_n{row_n}
        , _col_n{col_n}
    {
        _buf = create_();
    }

    matrix(const int row_n, const int col_n, T value)
        : _row_n{row_n}
        , _col_n{col_n}
    {
        _buf = create_();

        copy_n_(value);
    }

    matrix(const std::vector<std::vector<T> > &buf)
        : _row_n{int(buf.size())}
        , _col_n{_row_n > 0 ? int(buf[0].size()) : 0}
    {
        _buf = create_();

        copy_from_(buf);
    }

    matrix(const matrix &rhs)
        : _row_n{rhs._row_n}
        , _col_n{rhs._col_n}
        , _buf{rhs._buf}
    {
    }

    ~matrix() { clean_(_buf); }

    matrix &operator=(const matrix &rhs)
    {
        clean_(_buf);
        _buf = create_(rhs._row_n, rhs._col_n);
        copy_from_(rhs);

        _row_n = rhs._row_n;
        _col_n = rhs._col_n;
        return this;
    }

    inline T *operator[](const int row) { return _buf[row]; }

    inline friend bool operator==(const matrix &a, const matrix &b)
    {
        return a._col_n == b._col_n && a._row_n == b._row_n && a._buf == b._buf;
    }

    inline friend bool operator!=(const matrix &a, const matrix &b)
    {
        return !(a == b);
    }

    inline matrix_iterator<matrix<T>, T> begin() { return find(0, 0); }

    inline matrix_row_iterator<matrix<T>, T> vbegin() { return vfind(0, 0); }

    inline matrix_iterator<matrix<T>, T> end()
    {
        return matrix_iterator<matrix<T>, T>(this, _row_n, 0);
    }

    inline matrix_row_iterator<matrix<T>, T> vend()
    {
        return matrix_row_iterator<matrix<T>, T>(this, 0, _col_n);
    }

    inline matrix_iterator<matrix<T>, T> find(const int row, const int col)
    {
        return matrix_iterator<matrix<T>, T>(this, row, col);
    }

    inline matrix_row_iterator<matrix<T>, T> vfind(const int row, const int col)
    {
        return matrix_row_iterator<matrix<T>, T>(this, row, col);
    }

    inline T &at(const int row, const int col) { return _buf[row][col]; }

    inline std::pair<int, int> resize(int row_n, int col_n)
    {
        auto old = _buf;
        _buf     = create_(row_n, col_n);
        copy_from_(old, row_n, col_n);
        clean_(old);

        _row_n = row_n;
        _col_n = col_n;
        return std::make_pair(std::move(row_n), std::move(col_n));
    }

    inline int64_t size() { return _row_n * _col_n; }

    inline int row_n() { return _row_n; }

    inline int col_n() { return _col_n; }

    inline T **date() { return _buf; }

  private:
    void clean_(T **buf)
    {
        for(auto row = 0; row < _row_n; ++row)
        {
            delete[] buf[row];
        }

        delete[] buf;
    }

    T **create_() { return create_(_row_n, _col_n); }

    T **create_(const int row_n, const int col_n)
    {
        T **bak = new T *[col_n];
        for(auto row = 0; row < row_n; ++row)
        {
            bak[row] = new T[col_n];
        }
        return bak;
    }

    template <typename Container>
    void copy_from_(const Container &rhs)
    {
        copy_from_(rhs, _row_n, _col_n);
    }

    template <typename Container>
    void copy_from_(const Container &rhs, int row_n, int col_n)
    {
        row_n = _row_n < row_n ? _row_n : row_n;
        col_n = _col_n < col_n ? _col_n : col_n;

        for(int row = 0; row < row_n; ++row)
        {
            for(int col = 0; col < col_n; ++col)
            {
                _buf[row][col] = rhs[row][col];
            }
        }
    }

    void copy_n_(const T value, int n = -1)
    {
        n = n > -1 && n <= size() ? n : size();
        for(int row = 0; row < _row_n; ++row)
        {
            for(int col = 0; col < _col_n; ++col)
            {
                if(n == 0)
                {
                    return;
                }

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
