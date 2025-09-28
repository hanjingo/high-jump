#ifndef MATRIX_ITERATOR_HPP
#define MATRIX_ITERATOR_HPP

namespace hj
{

template <typename Mat, typename T>
struct matrix_iterator
{
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = ptrdiff_t;
    using difference_type   = T;
    using pointer           = const T *;
    using reference         = T *;

    matrix_iterator()
        : _mat{0}
        , _row{0}
        , _col{0}
        , _n_row{0}
        , _n_col{0}
    {
    }
    matrix_iterator(const matrix_iterator &rhs)
        : _mat{rhs._mat}
        , _row{rhs._row}
        , _col{rhs._col}
        , _n_row{rhs._n_row}
        , _n_col{rhs._n_col}
    {
    }
    matrix_iterator(Mat      *mat,
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

    inline matrix_iterator &operator=(matrix_iterator &itr)
    {
        _mat   = itr._mat;
        _row   = itr._row;
        _col   = itr._col;
        _n_row = itr._n_row;
        _n_col = itr._n_col;
        return *this;
    }

    inline T &operator*() { return at(_row, _col); }

    inline matrix_iterator &operator++() // ++itr
    {
        seek(1);
        return *this;
    }

    inline matrix_iterator &operator++(int offset) // itr++
    {
        seek(1);
        return *this;
    }

    inline matrix_iterator operator+(int offset) // itr + n
    {
        matrix_iterator ret(*this);
        ret.seek(offset);
        return ret;
    }

    inline matrix_iterator &operator+=(ptrdiff_t offset) // itr += n
    {
        seek(offset);
        return *this;
    }

    inline matrix_iterator &operator--() // --itr
    {
        seek(-1);
        return *this;
    }

    inline matrix_iterator &operator--(int offset) // itr--
    {
        seek(-1);
        return *this;
    }

    inline matrix_iterator operator-(int offset) // itr - n
    {
        matrix_iterator ret(*this);
        ret.seek(-offset);
        return ret;
    }

    inline matrix_iterator &operator-=(ptrdiff_t offset) // itr -= n
    {
        seek(-offset);
        return *this;
    }

    inline friend bool operator==(const matrix_iterator &a,
                                  const matrix_iterator &b)
    {
        return a._row == b._row && a._col == b._col;
    }

    inline friend bool operator!=(const matrix_iterator &a,
                                  const matrix_iterator &b)
    {
        return !(a == b);
    }

    inline friend bool operator<(const matrix_iterator &a,
                                 const matrix_iterator &b)
    {
        return (a._col < b._col) || (a._col == b._col && a._row < b._row);
    }

    inline friend bool operator<=(const matrix_iterator &a,
                                  const matrix_iterator &b)
    {
        return (a._col < b._col) || (a._col == b._col && a._row <= b._row);
    }

    inline friend bool operator>(const matrix_iterator &a,
                                 const matrix_iterator &b)
    {
        return (a._col > b._col) || (a._col == b._col && a._row > b._row);
    }

    inline friend bool operator>=(const matrix_iterator &a,
                                  const matrix_iterator &b)
    {
        return (a._col > b._col) || (a._col == b._col && a._row >= b._row);
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

    virtual T &at(const int row, const int col) { return (*_mat)(row, col); }

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
                if(_row + step > -1 && _row + step < _n_row)
                {
                    _row += step;
                    offset -= step;
                    if(offset == 0)
                    {
                        return;
                    }
                    continue;
                }
                break;
            }

            if(_col + step < 0 && _col + step >= _n_col)
                return;

            _col += step;
            _row = step > 0 ? 0 : _n_row - 1;
            offset -= step;
            if(offset == 0)
                return;
        }
    }

    ptrdiff_t distance(const matrix_iterator<Mat, T> &target)
    {
        auto offset = (abs(_col - target._col) + 1) * _n_row;
        if(*this < target)
            offset -= (_n_row - target._row);
        else
            offset = -(offset - (_n_row - _row));

        return offset;
    }

  protected:
    Mat *_mat;
    int  _row;
    int  _col;
    int  _n_row;
    int  _n_col;
};

}

#endif