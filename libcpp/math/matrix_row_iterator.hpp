#ifndef MATRIX_ROW_ITERATOR_HPP
#define MATRIX_ROW_ITERATOR_HPP

#include <iostream>
#include <stdlib.h>

namespace libcpp
{

template <typename Mat, typename T, bool square_style = false>
struct matrix_row_iterator {
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = ptrdiff_t;
    using difference_type   = T;
    using pointer           = const T*;
    using reference         = T*;

    matrix_row_iterator()
        : mat_{nullptr}, row_{0}, col_{0}, n_row_{0}, n_col_{0}
    {}
    matrix_row_iterator(const matrix_row_iterator& itr)
        : mat_{itr.mat_}, row_{itr.row_}, col_{itr.col_}, n_row_{itr.n_row_}, n_col_{itr.n_col_}
    {}
    matrix_row_iterator(Mat* mat, const int row, const int col, const int n_row, const int n_col)
        : mat_{mat}, row_{row}, col_{col}, n_row_{n_row}, n_col_{n_col}
    {}

    inline matrix_row_iterator& operator=(matrix_row_iterator& itr)
    {
        mat_   = itr.mat_;
        row_   = itr.row_;
        col_   = itr.col_;
        n_row_ = itr.n_row_;
        n_col_ = itr.n_col_;
        return *this;
    }

    inline T& operator*() // *itr
    {
        return at(row_, col_);
    }

    inline matrix_row_iterator& operator++() // ++itr
    {
        seek(1);
        return *this;
    }

    inline matrix_row_iterator& operator++(int offset) // itr++
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

    inline matrix_row_iterator& operator+=(ptrdiff_t offset) // itr += n
    {
        seek(offset);
        return *this;
    }

    inline matrix_row_iterator& operator--() // --itr
    {
        seek(-1);
        return *this;
    }

    inline matrix_row_iterator& operator--(int offset) // itr--
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

    inline matrix_row_iterator& operator-=(ptrdiff_t offset) // itr -= n
    {
        seek(-offset);
        return *this;
    }

    inline friend bool operator==(const matrix_row_iterator& a, const matrix_row_iterator& b) // itr1 == itr2
    {
        return a.row_ == b.row_ && a.col_ == b.col_;
    }

    inline friend bool operator!=(const matrix_row_iterator& a, const matrix_row_iterator& b) // itr1 != itr2
    {
        return !(a == b);
    }

    inline friend bool operator<(const matrix_row_iterator& a, const matrix_row_iterator& b)
    {
        return (a.row_ < b.row_) || (a.row_ == b.row_ && a.col_ < b.col_);
    }

    inline friend bool operator<=(const matrix_row_iterator& a, const matrix_row_iterator& b)
    {
        return (a.row_ < b.row_) || (a.row_ == b.row_ && a.col_ <= b.col_);
    }

    inline friend bool operator>(const matrix_row_iterator& a, const matrix_row_iterator& b)
    {
        return (a.row_ > b.row_) || (a.row_ == b.row_ && a.col_ > b.col_);
    }

    inline friend bool operator>=(const matrix_row_iterator& a, const matrix_row_iterator& b)
    {
        return (a.row_ > b.row_) || (a.row_ == b.row_ && a.col_ >= b.col_);
    }

    inline std::pair<int, int> pos()
    {
        int row = row_;
        int col = col_;
        return std::make_pair<int, int>(std::move(row), std::move(col));
    }

    inline int row()
    {
        return row_;
    }

    inline int col()
    {
        return col_;
    }

    inline Mat* matrix()
    {
        return mat_;
    }

    T& at(const int row, const int col)
    {
        return (*mat_)(row, col);
    }

    void seek(const int row, const int col)
    {
        row_   = row;
        col_   = col;
    }

    void seek(ptrdiff_t offset)
    {
        int step = offset < 0 ? -1 : 1;
        while (true) {
            while (true) {
                if (col_ + step > -1 && col_ + step < n_col_) {
                    col_   += step;
                    offset -= step;
                    if (offset == 0) {
                        return;
                    }
                    continue;
                }
                break;
            }

            if (row_ + step < 0 && row_ + step >= n_row_) {
                return;
            }
            row_   += step;
            col_    = step > 0 ? 0 : n_col_ - 1;
            offset -= step;
            if (offset == 0) {
                return;
            }
        }
    }

    ptrdiff_t distance(const matrix_row_iterator<Mat, T>& target)
    {
        auto offset = (abs(row_ - target.row_) + 1) * n_col_;
        if (*this < target) {
            offset -= (n_col_ - target.col_);
        } else {
            offset = -(offset - (n_col_ - col_));
        }

        return offset;
    }

private:
    Mat* mat_;
    int row_;
    int col_;
    int n_row_;
    int n_col_;
};

}

#endif