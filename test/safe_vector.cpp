#include <gtest/gtest.h>
#include <libcpp/sync/safe_vector.hpp>

TEST (safe_vector, emplace)
{
    libcpp::safe_vector<int> vec;
    vec.emplace (1);
    vec.emplace (2);
    vec.emplace (3);
    vec.emplace (4);
    vec.emplace (5);
    vec.emplace (6);
    vec.emplace (7);
    vec.emplace (8);
    vec.emplace (9);
    vec.emplace (10);

    ASSERT_EQ (vec.size (), 10);
}

TEST (safe_vector, unsafe_clear)
{
    libcpp::safe_vector<int> vec;
    vec.emplace (1);
    vec.emplace (2);
    vec.emplace (3);
    vec.emplace (4);
    vec.emplace (5);
    vec.emplace (6);
    vec.emplace (7);
    vec.emplace (8);
    vec.emplace (9);
    vec.emplace (10);

    ASSERT_EQ (vec.size (), 10);

    vec.unsafe_clear ();
    ASSERT_EQ (vec.size (), 0);
}
TEST (safe_vector, size)
{
    libcpp::safe_vector<int> vec;
    vec.emplace (1);
    vec.emplace (2);
    vec.emplace (3);
    vec.emplace (4);
    vec.emplace (5);
    vec.emplace (6);
    vec.emplace (7);
    vec.emplace (8);
    vec.emplace (9);
    vec.emplace (10);

    ASSERT_EQ (vec.size (), 10);
}
TEST (safe_vector, empty)
{
    libcpp::safe_vector<int> vec;
    ASSERT_EQ (vec.empty (), true);
    vec.emplace (1);
    ASSERT_EQ (vec.empty (), false);
}
TEST (safe_vector, swap)
{
    libcpp::safe_vector<int> vec1;
    libcpp::safe_vector<int> vec2;

    vec1.emplace (1);
    vec1.emplace (2);
    vec1.emplace (3);
    vec1.emplace (4);
    vec1.emplace (5);
    vec1.emplace (6);
    vec1.emplace (7);
    vec1.emplace (8);
    vec1.emplace (9);
    vec1.emplace (10);

    ASSERT_EQ (vec1.size (), 10);

    vec2.emplace (11);
    vec2.emplace (12);
    vec2.emplace (13);
    vec2.emplace (14);
    vec2.emplace (15);
    vec2.emplace (16);
    vec2.emplace (17);
    vec2.emplace (18);
    vec2.emplace (19);
    vec2.emplace (20);

    ASSERT_EQ (vec2.size (), 10);
    vec1.swap (vec2);
    ASSERT_EQ (vec1.size (), 10);
    ASSERT_EQ (vec2.size (), 10);
    ASSERT_EQ (vec1.at (0), 11);
    ASSERT_EQ (vec2.at (0), 1);
}
TEST (safe_vector, range)
{
    libcpp::safe_vector<int> vec;
    vec.emplace (1);
    vec.emplace (2);
    vec.emplace (3);
    vec.emplace (4);
    vec.emplace (5);
    vec.emplace (6);
    vec.emplace (7);
    vec.emplace (8);
    vec.emplace (9);
    vec.emplace (10);

    ASSERT_EQ (vec.size (), 10);

    int sum = 0;
    auto fn = [&] (int &value) -> bool {
        sum += value;
        return true;
    };
    vec.range (fn);
    ASSERT_EQ (sum, 55);

    sum = 0;
    auto const_fn = [&] (const int &value) -> bool {
        sum += value;
        return true;
    };
    vec.range (const_fn);
    ASSERT_EQ (sum, 55);
}
TEST (safe_vector, sort)
{
    libcpp::safe_vector<int> vec;
    vec.emplace (10);
    vec.emplace (9);
    vec.emplace (8);
    vec.emplace (7);
    vec.emplace (6);
    vec.emplace (5);
    vec.emplace (4);
    vec.emplace (3);
    vec.emplace (2);
    vec.emplace (1);

    ASSERT_EQ (vec.size (), 10);

    auto fn = [] (int a, int b) -> bool { return a < b; };
    vec.sort (fn);

    ASSERT_EQ (vec.at (0), 1);
    ASSERT_EQ (vec.at (1), 2);
    ASSERT_EQ (vec.at (2), 3);
}
TEST (safe_vector, at)
{
    libcpp::safe_vector<int> vec;
    vec.emplace (1);
    vec.emplace (2);
    vec.emplace (3);
    vec.emplace (4);
    vec.emplace (5);
    vec.emplace (6);
    vec.emplace (7);
    vec.emplace (8);
    vec.emplace (9);
    vec.emplace (10);

    ASSERT_EQ (vec.size (), 10);

    ASSERT_EQ (vec.at (0), 1);
    ASSERT_EQ (vec.at (1), 2);
    ASSERT_EQ (vec.at (2), 3);
}
TEST (safe_vector, at_out_of_range)
{
    libcpp::safe_vector<int> vec;
    vec.emplace (1);
    vec.emplace (2);
    vec.emplace (3);
    vec.emplace (4);
    vec.emplace (5);
    vec.emplace (6);
    vec.emplace (7);
    vec.emplace (8);
    vec.emplace (9);
    vec.emplace (10);

    ASSERT_EQ (vec.size (), 10);

    ASSERT_THROW (vec.at (10), std::out_of_range);
}
TEST (safe_vector, at_negative_index)
{
    libcpp::safe_vector<int> vec;
    vec.emplace (1);
    vec.emplace (2);
    vec.emplace (3);
    vec.emplace (4);
    vec.emplace (5);
    vec.emplace (6);
    vec.emplace (7);
    vec.emplace (8);
    vec.emplace (9);
    vec.emplace (10);

    ASSERT_EQ (vec.size (), 10);

    ASSERT_THROW (vec.at (-1), std::out_of_range);
}
TEST (safe_vector, front)
{
    libcpp::safe_vector<int> vec;
    vec.emplace (1);
    vec.emplace (2);
    vec.emplace (3);
    vec.emplace (4);
    vec.emplace (5);
    vec.emplace (6);
    vec.emplace (7);
    vec.emplace (8);
    vec.emplace (9);
    vec.emplace (10);

    ASSERT_EQ (vec.size (), 10);

    ASSERT_EQ (vec.front (), 1);
}
TEST (safe_vector, back)
{
    libcpp::safe_vector<int> vec;
    vec.emplace (1);
    vec.emplace (2);
    vec.emplace (3);
    vec.emplace (4);
    vec.emplace (5);
    vec.emplace (6);
    vec.emplace (7);
    vec.emplace (8);
    vec.emplace (9);
    vec.emplace (10);

    ASSERT_EQ (vec.size (), 10);

    ASSERT_EQ (vec.back (), 10);
}
TEST (safe_vector, operator)
{
    libcpp::safe_vector<int> vec;
    vec.emplace (1);
    vec.emplace (2);
    vec.emplace (3);
    vec.emplace (4);
    vec.emplace (5);
    vec.emplace (6);
    vec.emplace (7);
    vec.emplace (8);
    vec.emplace (9);
    vec.emplace (10);

    ASSERT_EQ (vec.size (), 10);

    ASSERT_EQ (vec[0], 1);
    ASSERT_EQ (vec[1], 2);
    ASSERT_EQ (vec[2], 3);
}
TEST (safe_vector, operator_out_of_range)
{
    libcpp::safe_vector<int> vec;
    vec.emplace (1);
    vec.emplace (2);
    vec.emplace (3);
    vec.emplace (4);
    vec.emplace (5);
    vec.emplace (6);
    vec.emplace (7);
    vec.emplace (8);
    vec.emplace (9);
    vec.emplace (10);

    ASSERT_EQ (vec.size (), 10);
}
TEST (safe_vector, operator_negative_index)
{
    libcpp::safe_vector<int> vec;
    vec.emplace (1);
    vec.emplace (2);
    vec.emplace (3);
    vec.emplace (4);
    vec.emplace (5);
    vec.emplace (6);
    vec.emplace (7);
    vec.emplace (8);
    vec.emplace (9);
    vec.emplace (10);

    ASSERT_EQ (vec.size (), 10);

    // ASSERT_THROW(vec[-1], std::out_of_range);
}