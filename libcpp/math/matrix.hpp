#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <cstdint>
#include <vector>
#include <functional>

#include <libcpp/math/matrix_iterator.hpp>
#include <libcpp/math/matrix_row_iterator.hpp>

namespace libcpp
{

template <typename T>
class matrix
{
public:
    matrix() :
        row_n_{0},
        col_n_{0}
    {
        buf_ = create_();
    }

    matrix(const int row_n, const int col_n) :
        row_n_{row_n},
        col_n_{col_n}
    {
        buf_ = create_();
    }

    matrix(const int row_n, const int col_n, T value) :
        row_n_{row_n},
        col_n_{col_n}
    {
        buf_ = create_();

        copy_n_(value);
    }

    matrix(const std::vector<std::vector<T>>& buf) :
        row_n_{int(buf.size())},
        col_n_{row_n_ > 0 ? int(buf[0].size()) : 0}
    {
        buf_ = create_();

        copy_from_(buf);
    }

    matrix(const matrix& rhs) :
        row_n_{rhs.row_n_},
        col_n_{rhs.col_n_},
        buf_{rhs.buf_}
    {
    }

    ~matrix()
    {
        clean_(buf_);
    }

    matrix& operator=(const matrix& rhs)
    {
        clean_(buf_);
        buf_ = create_(rhs.row_n_, rhs.col_n_);
        copy_from_(rhs);

        row_n_ = rhs.row_n_;
        col_n_ = rhs.col_n_;
        return this;
    }

    inline T* operator[](const int row)
    {
        return buf_[row];
    }

    inline friend bool operator==(const matrix& a, const matrix& b)
    {
        return a.col_n_ == b.col_n_ && a.row_n_ == b.row_n_ && a.buf_ == b.buf_;
    }

    inline friend bool operator!=(const matrix& a, const matrix& b)
    {
        return !(a == b);
    }

    inline matrix_iterator<decltype<matrix<T>>, T> begin()
    {
        return find(0, 0);
    }

    inline MatrixRowIterator<T> vbegin()
    {
        return vfind(0, 0);
    }

    inline matrix_iterator<T> end()
    {
        return matrix_iterator<T>(this, row_n_, 0);
    }

    inline matrix_vertical_iterator<T> vend()
    {
        return matrix_vertical_iterator<T>(this, 0, col_n_);
    }

    inline matrix_iterator<T> find(const int row, const int col)
    {
        return matrix_iterator<T>(this, row, col);
    }

    inline matrix_vertical_iterator<T> vfind(const int row, const int col)
    {
        return matrix_vertical_iterator<T>(this, row, col);
    }

    inline T& at(const int row, const int col)
    {
        return buf_[row][col];
    }

    inline std::pair<int, int> resize(int row_n, int col_n)
    {
        auto old = buf_;
        buf_ = create_(row_n, col_n);
        copy_from_(old, row_n, col_n);
        clean_(old);

        row_n_ = row_n;
        col_n_ = col_n;
        return std::make_pair(std::move(row_n), std::move(col_n));
    }

    inline int64_t size()
    {
        return row_n_ * col_n_;
    }

    inline int row_n()
    {
        return row_n_;
    }

    inline int col_n()
    {
        return col_n_;
    }

    inline T** date()
    {
        return buf_;
    }

private:
    void clean_(T** buf)
    {
        for (auto row = 0; row < row_n_; ++row) {
            delete []buf[row];
        }

        delete []buf;
    }

    T** create_()
    {
        return create_(row_n_, col_n_);
    }

    T** create_(const int row_n, const int col_n)
    {
        T** bak = new T*[col_n];
        for (auto row = 0; row < row_n; ++row) {
            bak[row] = new T[col_n];
        }
        return bak;
    }

    template<typename Container>
    void copy_from_(const Container& rhs)
    {
        copy_from_(rhs, row_n_, col_n_);
    }

    template<typename Container>
    void copy_from_(const Container& rhs, int row_n, int col_n)
    {
        row_n = row_n_ < row_n ? row_n_ : row_n;
        col_n = col_n_ < col_n ? col_n_ : col_n;

        for (int row = 0; row < row_n; ++row) {
            for (int col = 0; col < col_n; ++col) {
                buf_[row][col] = rhs[row][col];
            }
        }
    }

    void copy_n_(const T value, int n = -1)
    {
        n = n > -1 && n <= size() ? n : size();
        for (int row = 0; row < row_n_; ++row) {
            for (int col = 0; col < col_n_; ++col) {
                if (n == 0) {
                    return;
                }

                buf_[row][col] = value;
                n--;
            }
        }
    }

private:
    T** buf_;
    int row_n_;
    int col_n_;
};

}

#endif
