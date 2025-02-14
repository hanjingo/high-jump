#ifndef VECTOR2D_ITERATOR_HPP
#define VECTOR2D_ITERATOR_HPP

namespace libcpp
{

template <typename Vec, typename T>
struct vector2d_iterator {
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = ptrdiff_t;
    using difference_type   = T;
    using pointer           = const T*;
    using reference         = T*;

    vector2d_iterator()
        : vec_{0}, row_{0}, col_{0}, n_row_{0}, n_col_{0}
    {}
    vector2d_iterator(const vector2d_iterator& rhs)
        : vec_{rhs.vec_}, row_{rhs.row_}, col_{rhs.col_}, n_row_{rhs.n_row_}, n_col_{rhs.n_col_}
    {}
    vector2d_iterator(Vec* vec, const int row, const int col, const int n_row, const int n_col)
        : vec_{vec}, row_{row}, col_{col}, n_row_{n_row}, n_col_{n_col}
    {}

    inline vector2d_iterator& operator=(vector2d_iterator& itr)
    {
        vec_   = itr.vec_;
        row_   = itr.row_;
        col_   = itr.col_;
        n_row_ = itr.n_row_;
        n_col_ = itr.n_col_;
        return *this;
    }

    inline T& operator*()
    {
        return at(row_, col_);
    }

    inline vector2d_iterator& operator++() // ++itr
    {
        seek(1);
        return *this;
    }

    inline vector2d_iterator& operator++(int offset) // itr++
    {
        seek(1);
        return *this;
    }

    inline vector2d_iterator operator+(int offset) // itr + n
    {
        vector2d_iterator ret(*this);
        ret.seek(offset);
        return ret;
    }

    inline vector2d_iterator& operator+=(ptrdiff_t offset) // itr += n
    {
        seek(offset);
        return *this;
    }

    inline vector2d_iterator& operator--() // --itr
    {
        seek(-1);
        return *this;
    }

    inline vector2d_iterator& operator--(int offset) // itr--
    {
        seek(-1);
        return *this;
    }

    inline vector2d_iterator operator-(int offset) // itr - n
    {
        vector2d_iterator ret(*this);
        ret.seek(-offset);
        return ret;
    }

    inline vector2d_iterator& operator-=(ptrdiff_t offset) // itr -= n
    {
        seek(-offset);
        return *this;
    }

    inline friend bool operator==(const vector2d_iterator& a, const vector2d_iterator& b)
    {
        return a.row_ == b.row_ && a.col_ == b.col_;
    }

    inline friend bool operator!=(const vector2d_iterator& a, const vector2d_iterator& b)
    {
        return !(a == b);
    }

    inline friend bool operator<(const vector2d_iterator& a, const vector2d_iterator& b)
    {
        return (a.col_ < b.col_) || (a.col_ == b.col_ && a.row_ < b.row_);
    }

    inline friend bool operator<=(const vector2d_iterator& a, const vector2d_iterator& b)
    {
        return (a.col_ < b.col_) || (a.col_ == b.col_ && a.row_ <= b.row_);
    }

    inline friend bool operator>(const vector2d_iterator& a, const vector2d_iterator& b)
    {
        return (a.col_ > b.col_) || (a.col_ == b.col_ && a.row_ > b.row_);
    }

    inline friend bool operator>=(const vector2d_iterator& a, const vector2d_iterator& b)
    {
        return (a.col_ > b.col_) || (a.col_ == b.col_ && a.row_ >= b.row_);
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

    inline Vec* vector()
    {
        return vec_;
    }

    virtual T& at(const int row, const int col)
    {
        return (*vec_)[row][col];
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
                if (row_ + step > -1 && row_ + step < n_row_) {
                    row_   += step;
                    offset -= step;
                    if (offset == 0) {
                        return;
                    }
                    continue;
                }
                break;
            }

            if (col_ + step < 0 && col_ + step >= n_col_) {
                return;
            }
            col_   += step;
            row_    = step > 0 ? 0 : n_row_ - 1;
            offset -= step;
            if (offset == 0) {
                return;
            }
        }
    }

    ptrdiff_t distance(const vector2d_iterator<Vec, T>& target)
    {
        auto offset = (abs(col_ - target.col_) + 1) * n_row_;
        if (*this < target) {
            offset -= (n_row_ - target.row_);
        } else {
            offset = -(offset - (n_row_ - row_));
        }

        return offset;
    }

protected:
    Vec* vec_;
    int row_;
    int col_;
    int n_row_;
    int n_col_;
};

}

#endif