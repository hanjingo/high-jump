#include <gtest/gtest.h>
#include <hj/math/matrix_iterator.hpp>
#include <vector>
#include <Eigen/Dense>

using hj::matrix_iterator;

TEST(matrix_iterator, std_vector2d_array)
{
    std::vector<std::vector<int>> arr = {{1, 2, 3}, {4, 5, 6}};
    struct Wrapper
    {
        int &operator()(int row, int col) const { return arr[row][col]; }
        std::vector<std::vector<int>> &arr;
        Wrapper(std::vector<std::vector<int>> &a)
            : arr(a)
        {
        }
    } mat(arr);
    int                           n_row = 2, n_col = 3;
    matrix_iterator<Wrapper, int> it(&mat, n_row, n_col, 0);
    matrix_iterator<Wrapper, int> end(&mat, n_row, n_col, n_row * n_col);
    std::vector<int>              result;
    for(; it != end; ++it)
    {
        result.push_back(*it);
    }
    EXPECT_EQ(result, std::vector<int>({1, 2, 3, 4, 5, 6}));
}

TEST(matrix_iterator, eigen_matrix)
{
    Eigen::Matrix<int, 2, 3> emat;
    emat << 1, 2, 3, 4, 5, 6;
    struct Wrapper
    {
        int &operator()(int row, int col) const { return mat(row, col); }
        Eigen::Matrix<int, 2, 3> &mat;
        Wrapper(Eigen::Matrix<int, 2, 3> &m)
            : mat(m)
        {
        }
    } mat(emat);
    int                           n_row = 2, n_col = 3;
    matrix_iterator<Wrapper, int> it(&mat, n_row, n_col, 0);
    matrix_iterator<Wrapper, int> end(&mat, n_row, n_col, n_row * n_col);
    std::vector<int>              result;
    for(; it != end; ++it)
    {
        result.push_back(*it);
    }
    EXPECT_EQ(result, std::vector<int>({1, 2, 3, 4, 5, 6}));
}
