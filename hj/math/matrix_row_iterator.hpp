#ifndef MATRIX_ROW_ITERATOR_HPP
#define MATRIX_ROW_ITERATOR_HPP

#include <iostream>
#include <stdlib.h>

namespace hj
{

template <typename Mat, typename T, bool square_style = false>
struct matrix_row_iterator
{
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = ptrdiff_t;
    using difference_type   = T;
    using pointer           = const T *;
    using reference         = T *;

    matrix_row_iterator()
        : _mat{nullptr}
        , _row{0}
        , _col{0}
        , _n_row{0}
        , _n_col{0}
    {
    }
    matrix_row_iterator(const matrix_row_iterator &itr)
        : _mat{itr._mat}
        , _row{itr._row}
        , _col{itr._col}
        , _n_row{itr._n_row}
        , _n_col{itr._n_col}
    {
    }
    matrix_row_iterator(Mat      *mat,
                        const int row,
                        const int col,
                        const int n_row,
                        const int n_col)
        : _mat{mat}
        , _row{row}
        , _col{col}
        , _n_row{n_row}
        , _n_col{n_col}
    {
    }

    inline matrix_row_iterator &operator=(matrix_row_iterator &itr)
    {
        _mat   = itr._mat;
        _row   = itr._row;
        _col   = itr._col;
        _n_row = itr._n_row;
        _n_col = itr._n_col;
        return *this;
    }

    inline T &operator*() // *itr
    {
        return at(_row, _col);
    }

    inline matrix_row_iterator &operator++() // ++itr
    {
        seek(1);
        return *this;
    }

    inline matrix_row_iterator &operator++(int offset) // itr++
    {
        seek(1);
        return *this;
    }

    inline matrix_row_iterator operator+(int offset) // itr + n
    {
        matrix_row_iterator ret(*this);
        ret.seek(offset);
        return ret;
    }

    inline matrix_row_iterator &operator+=(ptrdiff_t offset) // itr += n
    {
        seek(offset);
        return *this;
    }

    inline matrix_row_iterator &operator--() // --itr
    {
        seek(-1);
        return *this;
    }

    inline matrix_row_iterator &operator--(int offset) // itr--
    {
        seek(-1);
        return *this;
    }

    inline matrix_row_iterator operator-(int offset) // itr + n
    {
        matrix_row_iterator ret(*this);
        ret.seek(-offset);
        return ret;
    }

    inline matrix_row_iterator &operator-=(ptrdiff_t offset) // itr -= n
    {
        seek(-offset);
        return *this;
    }

    inline friend bool operator==(const matrix_row_iterator &a,
                                  const matrix_row_iterator &b) // itr1 == itr2
    {
        return a._row == b._row && a._col == b._col;
    }

    inline friend bool operator!=(const matrix_row_iterator &a,
                                  const matrix_row_iterator &b) // itr1 != itr2
    {
        return !(a == b);
    }

    inline friend bool operator<(const matrix_row_iterator &a,
                                 const matrix_row_iterator &b)
    {
        return (a._row < b._row) || (a._row == b._row && a._col < b._col);
    }

    inline friend bool operator<=(const matrix_row_iterator &a,
                                  const matrix_row_iterator &b)
    {
        return (a._row < b._row) || (a._row == b._row && a._col <= b._col);
    }

    inline friend bool operator>(const matrix_row_iterator &a,
                                 const matrix_row_iterator &b)
    {
        return (a._row > b._row) || (a._row == b._row && a._col > b._col);
    }

    inline friend bool operator>=(const matrix_row_iterator &a,
                                  const matrix_row_iterator &b)
    {
        return (a._row > b._row) || (a._row == b._row && a._col >= b._col);
    }

    inline std::pair<int, int> pos()
    {
        int row = _row;
        int col = _col;
        return std::make_pair<int, int>(std::move(row), std::move(col));
    }

    inline int row() { return _row; }

    inline int col() { return _col; }

    inline Mat *matrix() { return _mat; }

    T &at(const int row, const int col) { return (*_mat)(row, col); }

    void seek(const int row, const int col)
    {
        _row = row;
        _col = col;
    }

    void seek(ptrdiff_t offset)
    {
        int step = offset < 0 ? -1 : 1;
        while(true)
        {
            while(true)
            {
                if(_col + step > -1 && _col + step < _n_col)
                {
                    _col += step;
                    offset -= step;
                    if(offset == 0)
                    {
                        return;
                    }
                    continue;
                }
                break;
            }

            if(_row + step < 0 && _row + step >= _n_row)
            {
                return;
            }
            _row += step;
            _col = step > 0 ? 0 : _n_col - 1;
            offset -= step;
            if(offset == 0)
            {
                return;
            }
        }
    }

    ptrdiff_t distance(const matrix_row_iterator<Mat, T> &target)
    {
        auto offset = (abs(_row - target._row) + 1) * _n_col;
        if(*this < target)
        {
            offset -= (_n_col - target._col);
        } else
        {
            offset = -(offset - (_n_col - _col));
        }

        return offset;
    }

  private:
    Mat *_mat;
    int  _row;
    int  _col;
    int  _n_row;
    int  _n_col;
};

}

#endif