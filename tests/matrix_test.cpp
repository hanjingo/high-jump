#include <gtest/gtest.h>
#include <hj/math/matrix.hpp>

using hj::matrix;

// TEST(MatrixTest, DefaultConstructor)
// {
//     matrix<int> m;
//     EXPECT_EQ(m.row_n(), 0);
//     EXPECT_EQ(m.col_n(), 0);
//     EXPECT_EQ(m.size(), 0);
// }

// TEST(MatrixTest, SizeConstructor)
// {
//     matrix<int> m(3, 4);
//     EXPECT_EQ(m.row_n(), 3);
//     EXPECT_EQ(m.col_n(), 4);
//     EXPECT_EQ(m.size(), 12);
// }

// TEST(MatrixTest, ValueConstructor)
// {
//     matrix<int> m(2, 2, 7);
//     EXPECT_EQ(m.at(0, 0), 7);
//     EXPECT_EQ(m.at(1, 1), 7);
// }

// TEST(MatrixTest, VectorConstructor)
// {
//     std::vector<std::vector<int>> buf = {{1, 2}, {3, 4}};
//     matrix<int>                   m(buf);
//     EXPECT_EQ(m.row_n(), 2);
//     EXPECT_EQ(m.col_n(), 2);
//     EXPECT_EQ(m.at(0, 0), 1);
//     EXPECT_EQ(m.at(1, 1), 4);
// }

// TEST(MatrixTest, CopyConstructorAndAssignment)
// {
//     matrix<int> m1(2, 2, 5);
//     matrix<int> m2(m1);
//     EXPECT_EQ(m2.at(0, 0), 5);
//     matrix<int> m3;
//     m3 = m1;
//     EXPECT_EQ(m3.at(1, 1), 5);
// }

// TEST(MatrixTest, ElementAccess)
// {
//     matrix<int> m(2, 3, 0);
//     m.at(1, 2) = 42;
//     EXPECT_EQ(m.at(1, 2), 42);
//     m[0][1] = 99;
//     EXPECT_EQ(m.at(0, 1), 99);
// }

// TEST(MatrixTest, EqualityOperators)
// {
//     matrix<int> m1(2, 2, 1);
//     matrix<int> m2(2, 2, 1);
//     matrix<int> m3(2, 2, 2);
//     EXPECT_TRUE(m1 == m2);
//     EXPECT_FALSE(m1 == m3);
//     EXPECT_TRUE(m1 != m3);
// }

// TEST(MatrixTest, Resize)
// {
//     matrix<int> m(2, 2, 3);
//     auto        old_size = m.size();
//     m.resize(3, 4);
//     EXPECT_EQ(m.row_n(), 3);
//     EXPECT_EQ(m.col_n(), 4);
//     EXPECT_EQ(m.size(), 12);
// }

// TEST(MatrixTest, IteratorBasic)
// {
//     matrix<int> m(2, 2, 1);
//     auto        it = m.begin();
//     EXPECT_EQ(*it, 1);
//     ++it;
//     EXPECT_EQ(*it, 1);
//     auto end   = m.end();
//     int  count = 0;
//     for(auto itr = m.begin(); itr != end; ++itr)
//     {
//         ++count;
//     }
//     EXPECT_EQ(count, 4);
// }

// TEST(MatrixTest, RowIteratorBasic)
// {
//     matrix<int> m(2, 2, 2);
//     auto        it = m.vbegin();
//     EXPECT_EQ(*it, 2);
//     auto end   = m.vend();
//     int  count = 0;
//     for(auto itr = m.vbegin(); itr != end; ++itr)
//     {
//         ++count;
//     }
//     EXPECT_EQ(count, 2);
// }
