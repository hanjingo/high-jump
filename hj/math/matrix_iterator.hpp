#ifndef MATRIX_ITERATOR_HPP
#define MATRIX_ITERATOR_HPP

namespace hj
{
template <typename Mat, typename T>
struct matrix_iterator
{
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = T *;
    using reference         = T &;

    matrix_iterator()
        : _mat(nullptr)
        , _offset(0)
        , _n_row(0)
        , _n_col(0)
    {
    }

    matrix_iterator(Mat *mat, int n_row, int n_col, int offset = 0)
        : _mat(mat)
        , _offset(offset)
        , _n_row(n_row)
        , _n_col(n_col)
    {
    }

    reference operator*()
    {
        int row = _offset / _n_col;
        int col = _offset % _n_col;
        return (*_mat)(row, col);
    }

    const T &operator*() const
    {
        int row = _offset / _n_col;
        int col = _offset % _n_col;
        return (*_mat)(row, col);
    }

    matrix_iterator &operator++()
    {
        ++_offset;
        return *this;
    }
    matrix_iterator operator++(int)
    {
        matrix_iterator tmp = *this;
        ++_offset;
        return tmp;
    }
    matrix_iterator &operator--()
    {
        --_offset;
        return *this;
    }
    matrix_iterator operator--(int)
    {
        matrix_iterator tmp = *this;
        --_offset;
        return tmp;
    }

    matrix_iterator operator+(difference_type n) const
    {
        return matrix_iterator(_mat, _n_row, _n_col, _offset + n);
    }
    matrix_iterator &operator+=(difference_type n)
    {
        _offset += n;
        return *this;
    }
    matrix_iterator operator-(difference_type n) const
    {
        return matrix_iterator(_mat, _n_row, _n_col, _offset - n);
    }
    matrix_iterator &operator-=(difference_type n)
    {
        _offset -= n;
        return *this;
    }
    difference_type operator-(const matrix_iterator &other) const
    {
        return _offset - other._offset;
    }

    bool operator==(const matrix_iterator &other) const
    {
        return _mat == other._mat && _offset == other._offset;
    }
    bool operator!=(const matrix_iterator &other) const
    {
        return !(*this == other);
    }
    bool operator<(const matrix_iterator &other) const
    {
        return _offset < other._offset;
    }
    bool operator>(const matrix_iterator &other) const
    {
        return _offset > other._offset;
    }
    bool operator<=(const matrix_iterator &other) const
    {
        return _offset <= other._offset;
    }
    bool operator>=(const matrix_iterator &other) const
    {
        return _offset >= other._offset;
    }

    int  row() const { return _offset / _n_col; }
    int  col() const { return _offset % _n_col; }
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