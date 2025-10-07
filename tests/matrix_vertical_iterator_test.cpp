#include <gtest/gtest.h>
#include <hj/math/matrix_vertical_iterator.hpp>
#include <vector>
#include <Eigen/Dense>

using hj::matrix_vertical_iterator;

TEST(matrix_vertical_iterator, StdVector2DArray)
{
    std::vector<std::vector<int>> arr = {{1, 4}, {2, 5}, {3, 6}};
    struct Wrapper
    {
        int &operator()(int row, int col) const
        {
            return const_cast<int &>(arr[row][col]);
        }
        const std::vector<std::vector<int>> &arr;
        Wrapper(const std::vector<std::vector<int>> &a)
            : arr(a)
        {
        }
    } mat(arr);
    int                                    n_row = 3, n_col = 2;
    matrix_vertical_iterator<Wrapper, int> it(&mat, n_row, n_col, 0);
    matrix_vertical_iterator<Wrapper, int> end(&mat,
                                               n_row,
                                               n_col,
                                               n_row * n_col);
    std::vector<int>                       result;
    for(; it != end; ++it)
    {
        result.push_back(*it);
    }
    EXPECT_EQ(result, std::vector<int>({1, 2, 3, 4, 5, 6}));
}

TEST(matrix_vertical_iterator, EigenMatrix)
{
    Eigen::Matrix<int, 3, 2> emat;
    emat << 1, 2, 3, 4, 5, 6;
    struct Wrapper
    {
        int &operator()(int row, int col) const
        {
            return const_cast<int &>(mat(row, col));
        }
        const Eigen::Matrix<int, 3, 2> &mat;
        Wrapper(const Eigen::Matrix<int, 3, 2> &m)
            : mat(m)
        {
        }
    } mat(emat);
    int                                    n_row = 3, n_col = 2;
    matrix_vertical_iterator<Wrapper, int> it(&mat, n_row, n_col, 0);
    matrix_vertical_iterator<Wrapper, int> end(&mat,
                                               n_row,
                                               n_col,
                                               n_row * n_col);
    std::vector<int>                       result;
    for(; it != end; ++it)
    {
        result.push_back(*it);
    }
    EXPECT_EQ(result, std::vector<int>({1, 3, 5, 2, 4, 6}));
}