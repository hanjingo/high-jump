#include <gtest/gtest.h>
#include <hj/algo/skiplist.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace hj;

namespace
{

template <typename T,
          typename Score        = double,
          typename Compare      = std::less<T>,
          typename ScoreCompare = std::less<Score>>
bool verify_order(const skiplist<T, Score, Compare, ScoreCompare> &sl)
{
    if(!sl.validate())
        return false;

    std::size_t count = 0;
    auto        curr  = sl.first();
    auto        end   = sl.end();

    if(sl.empty())
    {
        return curr == end && sl.last() == end;
    }

    auto prev       = curr;
    auto last_valid = curr;

    const auto &scmp = sl.score_comp();
    const auto &cmp  = sl.key_comp();

    for(; curr != end; ++curr)
    {
        count++;
        if(curr != sl.first())
        {
            if(scmp(curr.score(), prev.score()))
                return false;

            if(!scmp(prev.score(), curr.score())
               && !scmp(curr.score(), prev.score()))
            {
                if(cmp(*curr, *prev))
                    return false;
            }
        }
        last_valid = curr;
        prev       = curr;
    }

    if(sl.last() != last_valid)
        return false;
    if(sl.size() != count)
        return false;

    return true;
}

struct ThrowOnCopy
{
    int         id;
    static bool throw_flag;

    ThrowOnCopy(int i)
        : id(i)
    {
    }
    ThrowOnCopy(const ThrowOnCopy &other)
        : id(other.id)
    {
        if(throw_flag)
            throw std::runtime_error("Copy construction failed");
    }
    ThrowOnCopy(ThrowOnCopy &&) noexcept            = default;
    ThrowOnCopy &operator=(const ThrowOnCopy &)     = default;
    ThrowOnCopy &operator=(ThrowOnCopy &&) noexcept = default;

    bool operator<(const ThrowOnCopy &o) const { return id < o.id; }
};
bool ThrowOnCopy::throw_flag = false;

struct NonDefaultConstructible
{
    int val;
    explicit NonDefaultConstructible(int v)
        : val(v)
    {
    }
    bool operator<(const NonDefaultConstructible &o) const
    {
        return val < o.val;
    }
};

} // namespace

TEST(skiplist_test, constructor_and_basic_properties)
{
    SCOPED_TRACE("Testing skiplist constructor and basic properties");

    skiplist<std::string> sl;

    EXPECT_EQ(sl.size(), 0);
    EXPECT_TRUE(sl.empty());
    EXPECT_EQ(sl.first(), sl.end());
    EXPECT_EQ(sl.last(), sl.end());
    EXPECT_EQ(sl.begin(), sl.end());
    EXPECT_EQ(sl.cbegin(), sl.cend());
    EXPECT_TRUE(sl.validate());
}

TEST(skiplist_test, copy_constructor_and_copy_assignment)
{
    SCOPED_TRACE("Testing copy constructor and copy assignment independence");

    skiplist<std::string> a;
    a.insert(10.0, "apple");
    a.insert(20.0, "banana");
    a.insert(30.0, "cherry");

    // 1. Copy Constructor (skiplist b(a))
    skiplist<std::string> b(a);

    EXPECT_EQ(b.size(), 3);
    EXPECT_TRUE(verify_order(b));
    EXPECT_TRUE(b.validate());

    // Check deep copy independence
    b.insert(15.0, "avocado");
    b.erase(20.0, "banana");

    EXPECT_EQ(a.size(), 3);
    EXPECT_EQ(b.size(), 3);
    EXPECT_TRUE(a.contains(20.0, "banana"));
    EXPECT_FALSE(b.contains(20.0, "banana"));
    EXPECT_TRUE(b.contains(15.0, "avocado"));

    // 2. Copy Assignment (b = a)
    skiplist<std::string> c;
    c.insert(5.0, "date");

    c = a;

    EXPECT_EQ(c.size(), 3);
    EXPECT_TRUE(verify_order(c));
    EXPECT_TRUE(c.validate());
    EXPECT_TRUE(c.contains(20.0, "banana"));

    // Modify c to verify independence from a
    c.erase(10.0, "apple");
    EXPECT_EQ(c.size(), 2);
    EXPECT_EQ(a.size(), 3);

    // 3. Self Copy Assignment (a = a)
    a = a;
    EXPECT_EQ(a.size(), 3);
    EXPECT_TRUE(verify_order(a));
    EXPECT_TRUE(a.validate());

    // 4. Copy Assign Empty List
    skiplist<std::string> empty_sl;
    c = empty_sl;
    EXPECT_EQ(c.size(), 0);
    EXPECT_TRUE(c.empty());
    EXPECT_TRUE(c.validate());
}

TEST(skiplist_test, move_constructor_and_assignment)
{
    SCOPED_TRACE("Testing move constructor and move assignment");

    skiplist<std::string> sl1;
    sl1.insert(1.0, "apple");
    sl1.insert(2.0, "banana");
    sl1.insert(3.0, "cherry");

    // Move construct
    skiplist<std::string> sl2(std::move(sl1));

    EXPECT_EQ(sl2.size(), 3);
    EXPECT_FALSE(sl2.empty());
    EXPECT_TRUE(verify_order(sl2));

    EXPECT_EQ(sl1.size(), 0);
    EXPECT_TRUE(sl1.empty());
    EXPECT_EQ(sl1.first(), sl1.end());
    EXPECT_EQ(sl1.last(), sl1.end());

    // Move assign
    skiplist<std::string> sl3;
    sl3.insert(10.0, "zebra");

    sl3 = std::move(sl2);

    EXPECT_EQ(sl3.size(), 3);
    EXPECT_TRUE(verify_order(sl3));
    EXPECT_EQ(*sl3.first(), "apple");
    EXPECT_EQ(*sl3.last(), "cherry");

    EXPECT_EQ(sl2.size(), 0);
    EXPECT_TRUE(sl2.empty());
    EXPECT_EQ(sl2.first(), sl2.end());

    // Self move assignment check
    sl3 = std::move(sl3);
    EXPECT_EQ(sl3.size(), 3);
    EXPECT_TRUE(verify_order(sl3));
}

TEST(skiplist_test, iterator_invalidation)
{
    SCOPED_TRACE("Testing iterator invalidation: node iterators must remain "
                 "valid during insertions and unrelated deletions");

    skiplist<std::string> sl;
    auto                  it_a = sl.insert(10.0, "Apple");
    auto                  it_m = sl.insert(50.0, "Mango");
    auto                  it_z = sl.insert(100.0, "Zebra");

    // Insert many nodes before, between, and after existing nodes
    for(int i = 0; i < 200; ++i)
    {
        sl.insert(static_cast<double>(i), "fill_" + std::to_string(i));
    }

    // Existing iterators must still point to the original nodes and retain correct data
    EXPECT_EQ(*it_a, "Apple");
    EXPECT_EQ(it_a.score(), 10.0);

    EXPECT_EQ(*it_m, "Mango");
    EXPECT_EQ(it_m.score(), 50.0);

    EXPECT_EQ(*it_z, "Zebra");
    EXPECT_EQ(it_z.score(), 100.0);

    // Iterating forward/backward from saved iterators should work on updated list topology
    auto it_m_next = it_m;
    ++it_m_next;
    EXPECT_NE(it_m_next, sl.end());
    EXPECT_GE(it_m_next.score(), 50.0);

    auto it_m_prev = it_m;
    --it_m_prev;
    EXPECT_LE(it_m_prev.score(), 50.0);

    // Erase nodes other than saved nodes
    sl.erase(10.0, "fill_10");
    sl.erase(90.0, "fill_90");

    EXPECT_EQ(*it_m, "Mango");
    EXPECT_EQ(it_m.score(), 50.0);
    EXPECT_TRUE(sl.validate());
}

TEST(skiplist_test, rank_differential_test)
{
    SCOPED_TRACE("Differential test with std::vector oracle to catch span "
                 "calculation bugs");

    skiplist<int> sl;

    struct OracleEntry
    {
        double score;
        int    val;

        bool operator==(const OracleEntry &o) const
        {
            return score == o.score && val == o.val;
        }
    };

    std::vector<OracleEntry> oracle;

    auto oracle_cmp = [](const OracleEntry &a, const OracleEntry &b) {
        if(a.score != b.score)
            return a.score < b.score;
        return a.val < b.val;
    };

    std::mt19937                           gen(1337);
    std::uniform_int_distribution<int>     op_dist(0, 4);
    std::uniform_real_distribution<double> score_dist(0.0, 100.0);
    std::uniform_int_distribution<int>     val_dist(1, 50);

    const int num_ops = 1000;

    for(int op = 0; op < num_ops; ++op)
    {
        int type = op_dist(gen);

        if(type == 0 || oracle.empty()) // Insert
        {
            double score =
                std::round(score_dist(gen) * 10.0) / 10.0; // produce duplicates
            int val = val_dist(gen);

            sl.insert(score, val);

            OracleEntry entry{score, val};
            auto        it = std::lower_bound(oracle.begin(),
                                              oracle.end(),
                                              entry,
                                              oracle_cmp);
            oracle.insert(it, entry);
        } else if(type == 1) // Erase
        {
            std::uniform_int_distribution<std::size_t> idx_dist(0,
                                                                oracle.size()
                                                                    - 1);
            std::size_t                                idx   = idx_dist(gen);
            auto                                       entry = oracle[idx];

            bool sl_erased = sl.erase(entry.score, entry.val);
            EXPECT_TRUE(sl_erased);

            auto it = std::lower_bound(oracle.begin(),
                                       oracle.end(),
                                       entry,
                                       oracle_cmp);
            ASSERT_NE(it, oracle.end());
            oracle.erase(it);
        } else if(type == 2) // Rank queries
        {
            std::uniform_int_distribution<std::size_t> idx_dist(0,
                                                                oracle.size()
                                                                    - 1);
            std::size_t                                idx   = idx_dist(gen);
            auto                                       entry = oracle[idx];

            std::size_t sl_rank = sl.get_rank(entry.score, entry.val);
            EXPECT_NE(sl_rank, skiplist<int>::npos);

            auto elem_it = sl.get_element_by_rank(idx);
            ASSERT_NE(elem_it, sl.end());
            EXPECT_EQ(elem_it.score(), oracle[idx].score);
            EXPECT_EQ(*elem_it, oracle[idx].val);
        } else if(type == 3) // delete_range_by_rank
        {
            std::uniform_int_distribution<std::size_t> idx_dist(0,
                                                                oracle.size()
                                                                    - 1);
            std::size_t                                r1    = idx_dist(gen);
            std::size_t                                r2    = idx_dist(gen);
            std::size_t                                start = std::min(r1, r2);
            std::size_t                                end   = std::max(r1, r2);

            std::size_t sl_removed = sl.delete_range_by_rank(start, end);

            std::size_t expected_end = std::min(end, oracle.size() - 1);
            std::size_t oracle_removed =
                (start <= expected_end) ? (expected_end - start + 1) : 0;

            EXPECT_EQ(sl_removed, oracle_removed);
            if(oracle_removed > 0)
            {
                oracle.erase(oracle.begin() + start,
                             oracle.begin() + start + oracle_removed);
            }
        } else if(type == 4) // delete_range_by_score
        {
            double s1    = std::round(score_dist(gen) * 10.0) / 10.0;
            double s2    = std::round(score_dist(gen) * 10.0) / 10.0;
            double min_s = std::min(s1, s2);
            double max_s = std::max(s1, s2);

            std::size_t sl_removed = sl.delete_range_by_score(min_s, max_s);

            auto first_it = std::lower_bound(
                oracle.begin(),
                oracle.end(),
                OracleEntry{min_s, (std::numeric_limits<int>::min)()},
                oracle_cmp);
            auto last_it = std::upper_bound(
                oracle.begin(),
                oracle.end(),
                OracleEntry{max_s, (std::numeric_limits<int>::max)()},
                oracle_cmp);

            std::size_t oracle_removed = std::distance(first_it, last_it);
            EXPECT_EQ(sl_removed, oracle_removed);
            oracle.erase(first_it, last_it);
        }

        // Catch span and link bugs immediately via validate()
        ASSERT_EQ(sl.size(), oracle.size());
        ASSERT_TRUE(sl.validate())
            << "Validation failed at op iteration " << op;

        // Verify full snapshot match
        if(op % 50 == 0 || op == num_ops - 1)
        {
            std::size_t idx = 0;
            for(auto it = sl.begin(); it != sl.end(); ++it, ++idx)
            {
                ASSERT_EQ(it.score(), oracle[idx].score);
                ASSERT_EQ(*it, oracle[idx].val);
            }
        }
    }
}

TEST(skiplist_test, nan_score_handling)
{
    SCOPED_TRACE(
        "Testing NaN comparisons (std::less<double> behavior with NaN)");

    double            nan_val = (std::numeric_limits<double>::quiet_NaN)();
    std::less<double> score_cmp;

    // Standard C++ std::less<double> behavior verification with IEEE 754 NaN:
    // NaN < x is false; x < NaN is false.
    EXPECT_FALSE(score_cmp(nan_val, 10.0));
    EXPECT_FALSE(score_cmp(10.0, nan_val));
    EXPECT_FALSE(score_cmp(nan_val, nan_val));

    skiplist<std::string> sl;

    sl.insert(10.0, "apple");
    sl.insert(20.0, "banana");

    // Inserting NaN score items
    auto it_nan1 = sl.insert(nan_val, "nan_item1");
    auto it_nan2 = sl.insert(nan_val, "nan_item2");

    EXPECT_TRUE(std::isnan(it_nan1.score()));
    EXPECT_TRUE(std::isnan(it_nan2.score()));
    EXPECT_EQ(sl.size(), 4);

    // Structural validation after NaN insertion
    EXPECT_TRUE(sl.validate());

    // Search and retrieval operations
    EXPECT_TRUE(sl.contains(nan_val, "nan_item1"));
    EXPECT_TRUE(sl.contains(nan_val, "nan_item2"));

    auto find_it = sl.find(nan_val, "nan_item1");
    ASSERT_NE(find_it, sl.end());
    EXPECT_EQ(*find_it, "nan_item1");

    // Erase NaN items
    EXPECT_TRUE(sl.erase(nan_val, "nan_item1"));
    EXPECT_EQ(sl.size(), 3);
    EXPECT_FALSE(sl.contains(nan_val, "nan_item1"));
    EXPECT_TRUE(sl.validate());
}

TEST(skiplist_test, single_element_operations)
{
    SCOPED_TRACE("Testing single element insert and access");

    skiplist<std::string> sl;

    auto it = sl.insert(1.0, "apple");
    ASSERT_NE(it, sl.end());
    EXPECT_EQ(it.score(), 1.0);
    EXPECT_EQ(*it, "apple");

    EXPECT_EQ(sl.size(), 1);
    EXPECT_FALSE(sl.empty());
    EXPECT_EQ(sl.first(), it);
    EXPECT_EQ(sl.last(), it);

    auto iter = sl.begin();
    EXPECT_NE(iter, sl.end());
    EXPECT_EQ(*iter, "apple");
    EXPECT_EQ(iter.score(), 1.0);
    EXPECT_EQ(iter, it);

    ++iter;
    EXPECT_EQ(iter, sl.end());
    EXPECT_TRUE(verify_order(sl));
}

TEST(skiplist_test, emplace_and_rvalue_insert)
{
    SCOPED_TRACE("Testing emplace and rvalue insert");

    struct Point
    {
        int x, y;

        Point(int x_, int y_)
            : x(x_)
            , y(y_)
        {
        }

        bool operator<(const Point &o) const
        {
            return std::tie(x, y) < std::tie(o.x, o.y);
        }
        bool operator==(const Point &o) const { return x == o.x && y == o.y; }
    };

    skiplist<Point> sl;

    // Emplace constructing in-place
    auto it1 = sl.emplace(1.5, 10, 20);
    ASSERT_NE(it1, sl.end());
    EXPECT_EQ(it1->x, 10);
    EXPECT_EQ(it1->y, 20);

    // Rvalue insert
    Point p{30, 40};
    auto  it2 = sl.insert(2.5, std::move(p));
    ASSERT_NE(it2, sl.end());
    EXPECT_EQ(it2->x, 30);
    EXPECT_EQ(it2->y, 40);

    EXPECT_EQ(sl.size(), 2);
    EXPECT_TRUE(verify_order(sl));
}

TEST(skiplist_test, duplicate_scores_and_objects)
{
    SCOPED_TRACE(
        "Testing duplicate scores with different and identical objects");

    skiplist<std::string> sl;

    // Same score, different objects
    sl.insert(2.0, "banana");
    sl.insert(2.0, "blueberry");
    sl.insert(2.0, "blackberry");
    sl.insert(1.0, "apple");
    sl.insert(3.0, "cherry");

    EXPECT_EQ(sl.size(), 5);
    EXPECT_TRUE(verify_order(sl));

    // Ordered by score, then by string comparison
    std::vector<std::string> expected = {"apple",
                                         "banana",
                                         "blackberry",
                                         "blueberry",
                                         "cherry"};
    size_t                   idx      = 0;
    for(const auto &val : sl)
    {
        EXPECT_EQ(val, expected[idx++]);
    }

    // Insert identical score and object
    sl.insert(2.0, "banana");
    EXPECT_EQ(sl.size(), 6);
    EXPECT_TRUE(verify_order(sl));
}

TEST(skiplist_test, erase)
{
    SCOPED_TRACE("Testing erase with various scenarios");

    skiplist<std::string> sl;
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    sl.insert(3.0, "cherry");
    sl.insert(4.0, "date");
    sl.insert(5.0, "elderberry");

    // Delete middle node
    EXPECT_TRUE(sl.erase(3.0, "cherry"));
    EXPECT_EQ(sl.size(), 4);
    EXPECT_TRUE(verify_order(sl));

    // Delete head node
    EXPECT_TRUE(sl.erase(1.0, "apple"));
    EXPECT_EQ(sl.size(), 3);
    EXPECT_EQ(*sl.first(), "banana");
    EXPECT_TRUE(verify_order(sl));

    // Delete tail node
    EXPECT_TRUE(sl.erase(5.0, "elderberry"));
    EXPECT_EQ(sl.size(), 2);
    EXPECT_EQ(*sl.last(), "date");
    EXPECT_TRUE(verify_order(sl));

    // Non-existent score
    EXPECT_FALSE(sl.erase(10.0, "banana"));
    // Correct score, wrong object
    EXPECT_FALSE(sl.erase(2.0, "orange"));
    // Wrong score, correct object
    EXPECT_FALSE(sl.erase(9.0, "banana"));

    EXPECT_EQ(sl.size(), 2);

    // Clear remaining
    EXPECT_TRUE(sl.erase(2.0, "banana"));
    EXPECT_TRUE(sl.erase(4.0, "date"));
    EXPECT_EQ(sl.size(), 0);
    EXPECT_TRUE(sl.empty());
    EXPECT_EQ(sl.first(), sl.end());
    EXPECT_EQ(sl.last(), sl.end());
    EXPECT_TRUE(verify_order(sl));
}

TEST(skiplist_test, rank_and_element_access)
{
    SCOPED_TRACE("Testing get_element_by_rank, get_rank, and get_node_rank");

    skiplist<std::string> sl;

    sl.insert(10.0, "A");
    sl.insert(20.0, "B");
    sl.insert(20.0, "C");
    sl.insert(30.0, "D");
    sl.insert(40.0, "E");

    // 0-indexed rank verification
    auto it0 = sl.get_element_by_rank(0);
    ASSERT_NE(it0, sl.end());
    EXPECT_EQ(*it0, "A");
    EXPECT_EQ(sl.get_rank(10.0, "A"), 0);
    EXPECT_EQ(sl.get_node_rank(it0), 0);

    auto it2 = sl.get_element_by_rank(2);
    ASSERT_NE(it2, sl.end());
    EXPECT_EQ(*it2, "C");
    EXPECT_EQ(sl.get_rank(20.0, "C"), 2);
    EXPECT_EQ(sl.get_node_rank(it2), 2);

    auto it4 = sl.get_element_by_rank(4);
    ASSERT_NE(it4, sl.end());
    EXPECT_EQ(*it4, "E");
    EXPECT_EQ(sl.get_rank(40.0, "E"), 4);
    EXPECT_EQ(sl.get_node_rank(it4), 4);

    // Out of bounds rank
    EXPECT_EQ(sl.get_element_by_rank(5), sl.end());
    EXPECT_EQ(sl.get_element_by_rank(100), sl.end());

    // Non-existent elements rank
    EXPECT_EQ(sl.get_rank(20.0, "X"), skiplist<std::string>::npos);
    EXPECT_EQ(sl.get_rank(99.0, "E"), skiplist<std::string>::npos);
    EXPECT_EQ(sl.get_node_rank(sl.end()), skiplist<std::string>::npos);
}

TEST(skiplist_test, range_by_score_and_first_last_in_range)
{
    SCOPED_TRACE("Testing first_in_range, last_in_range, and range_by_score");

    skiplist<std::string> sl;
    sl.insert(10.0, "A");
    sl.insert(20.0, "B");
    sl.insert(30.0, "C");
    sl.insert(40.0, "D");
    sl.insert(50.0, "E");

    // Valid range
    auto first = sl.first_in_range(20.0, 40.0);
    auto last  = sl.last_in_range(20.0, 40.0);
    ASSERT_NE(first, sl.end());
    ASSERT_NE(last, sl.end());
    EXPECT_EQ(*first, "B");
    EXPECT_EQ(*last, "D");

    // Out of bounds / non-overlapping range
    EXPECT_EQ(sl.first_in_range(60.0, 70.0), sl.end());
    EXPECT_EQ(sl.last_in_range(60.0, 70.0), sl.end());
    EXPECT_EQ(sl.first_in_range(0.0, 5.0), sl.end());

    // Invalid min > max
    EXPECT_EQ(sl.first_in_range(40.0, 20.0), sl.end());
    EXPECT_EQ(sl.last_in_range(40.0, 20.0), sl.end());

    // range_by_score standalone helper
    auto res = range_by_score(sl, 20.0, 40.0);
    EXPECT_EQ(res.total_in_range, 3);
    EXPECT_EQ(res.iterators.size(), 3);
    EXPECT_EQ(*res.iterators[0], "B");
    EXPECT_EQ(*res.iterators[1], "C");
    EXPECT_EQ(*res.iterators[2], "D");

    // With offset and limit
    auto res_opt = range_by_score(sl, 10.0, 50.0, 1, 2);
    EXPECT_EQ(res_opt.total_in_range, 5);
    EXPECT_EQ(res_opt.iterators.size(), 2);
    EXPECT_EQ(*res_opt.iterators[0], "B");
    EXPECT_EQ(*res_opt.iterators[1], "C");
}

TEST(skiplist_test, range_by_rank_helper)
{
    SCOPED_TRACE("Testing range_by_rank standalone helper function");

    skiplist<std::string> sl;
    sl.insert(1.0, "A");
    sl.insert(2.0, "B");
    sl.insert(3.0, "C");
    sl.insert(4.0, "D");

    // Forward range [1, 2] -> "B", "C"
    auto res_fwd = range_by_rank(sl, 1, 2, false);
    EXPECT_EQ(res_fwd.total_in_range, 2);
    ASSERT_EQ(res_fwd.iterators.size(), 2);
    EXPECT_EQ(*res_fwd.iterators[0], "B");
    EXPECT_EQ(*res_fwd.iterators[1], "C");

    // Reverse range [1, 3] -> "D", "C", "B"
    auto res_rev = range_by_rank(sl, 1, 3, true);
    EXPECT_EQ(res_rev.total_in_range, 3);
    ASSERT_EQ(res_rev.iterators.size(), 3);
    EXPECT_EQ(*res_rev.iterators[0], "D");
    EXPECT_EQ(*res_rev.iterators[1], "C");
    EXPECT_EQ(*res_rev.iterators[2], "B");

    // Out of bound / invalid ranks
    auto res_inv1 = range_by_rank(sl, 5, 10);
    EXPECT_EQ(res_inv1.total_in_range, 0);
    EXPECT_TRUE(res_inv1.iterators.empty());

    auto res_inv2 = range_by_rank(sl, 3, 1);
    EXPECT_EQ(res_inv2.total_in_range, 0);

    // End parameter clamp
    auto res_clamp = range_by_rank(sl, 2, 100);
    EXPECT_EQ(res_clamp.total_in_range, 2); // indices 2, 3
    EXPECT_EQ(res_clamp.iterators.size(), 2);
}

TEST(skiplist_test, delete_range_by_score_and_rank)
{
    SCOPED_TRACE("Testing delete_range_by_score and delete_range_by_rank");

    skiplist<int> sl;
    for(int i = 1; i <= 10; ++i)
    {
        sl.insert(static_cast<double>(i * 10), i);
    }

    // Delete range by score [30, 60] -> removes 30, 40, 50, 60 (4 elements)
    std::size_t removed_score = sl.delete_range_by_score(30.0, 60.0);
    EXPECT_EQ(removed_score, 4);
    EXPECT_EQ(sl.size(), 6);
    EXPECT_TRUE(verify_order(sl));

    // Remaining scores: 10, 20, 70, 80, 90, 100
    // Delete range by rank [1, 3] -> removes rank 1 (20), 2 (70), 3 (80)
    std::size_t removed_rank = sl.delete_range_by_rank(1, 3);
    EXPECT_EQ(removed_rank, 3);
    EXPECT_EQ(sl.size(), 3);
    EXPECT_TRUE(verify_order(sl));

    std::vector<int> expected = {1, 9, 10};
    size_t           idx      = 0;
    for(int val : sl)
    {
        EXPECT_EQ(val, expected[idx++]);
    }

    // Invalid delete rank ranges
    EXPECT_EQ(sl.delete_range_by_rank(5, 10), 0);
    EXPECT_EQ(sl.delete_range_by_rank(2, 1), 0);
    EXPECT_EQ(sl.size(), 3);
}

TEST(skiplist_test, iterator_mechanics)
{
    SCOPED_TRACE("Testing iterator operators and bidirectional traversal");

    skiplist<std::string> sl;
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    sl.insert(3.0, "cherry");

    auto it = sl.begin();
    EXPECT_EQ(*it, "apple");
    EXPECT_EQ(it->length(), 5); // operator->

    auto it_end = sl.end();
    --it_end;
    EXPECT_EQ(*it_end, "cherry");

    // Pre-increment and Post-increment
    auto it_copy = it++;
    EXPECT_EQ(*it_copy, "apple");
    EXPECT_EQ(*it, "banana");

    ++it;
    EXPECT_EQ(*it, "cherry");

    // Pre-decrement and Post-decrement
    auto it_last = it--;
    EXPECT_EQ(*it_last, "cherry");
    EXPECT_EQ(*it, "banana");

    --it;
    EXPECT_EQ(*it, "apple");

    // Equality operators
    EXPECT_TRUE(sl.begin() == sl.cbegin());
    EXPECT_FALSE(sl.begin() == sl.end());
}

TEST(skiplist_test, custom_comparator_and_types)
{
    SCOPED_TRACE("Testing custom comparator and integral score type");

    struct DescendingCompare
    {
        bool operator()(const std::string &a, const std::string &b) const
        {
            return a > b;
        }
    };

    skiplist<std::string, int, DescendingCompare> sl;

    sl.insert(10, "apple");
    sl.insert(10, "cherry");
    sl.insert(10, "banana");

    std::vector<std::string> expected = {"cherry", "banana", "apple"};
    size_t                   idx      = 0;
    for(const auto &val : sl)
    {
        EXPECT_EQ(val, expected[idx++]);
    }
    EXPECT_TRUE(verify_order(sl));
}

TEST(skiplist_test, exception_safety_and_resource_leak)
{
    SCOPED_TRACE("Testing exception safety during element insertion");

    skiplist<ThrowOnCopy> sl;
    ThrowOnCopy           val(42);

    ThrowOnCopy::throw_flag = false;
    sl.insert(1.0, val);
    EXPECT_EQ(sl.size(), 1);

    ThrowOnCopy::throw_flag = true;
    EXPECT_THROW(sl.insert(2.0, val), std::runtime_error);

    // Skiplist should retain its valid state and size 1
    EXPECT_EQ(sl.size(), 1);
    EXPECT_TRUE(verify_order(sl));

    ThrowOnCopy::throw_flag = false; // Reset flag for cleanup
}

TEST(skiplist_test, non_default_constructible_type)
{
    SCOPED_TRACE("Testing non-default constructible type");

    skiplist<NonDefaultConstructible> sl;
    sl.emplace(1.0, 100);
    sl.emplace(2.0, 200);

    EXPECT_EQ(sl.size(), 2);
    EXPECT_EQ(sl.first()->val, 100);
    EXPECT_EQ(sl.last()->val, 200);
}

TEST(skiplist_test, boundary_score_values)
{
    SCOPED_TRACE("Testing infinity and extreme score values");

    skiplist<std::string> sl;
    double                inf = (std::numeric_limits<double>::infinity)();

    sl.insert(-inf, "neg_inf");
    sl.insert(inf, "pos_inf");
    sl.insert(0.0, "zero");

    EXPECT_EQ(sl.size(), 3);
    EXPECT_TRUE(verify_order(sl));

    auto it = sl.begin();
    EXPECT_EQ(*it++, "neg_inf");
    EXPECT_EQ(*it++, "zero");
    EXPECT_EQ(*it++, "pos_inf");
}

TEST(skiplist_test, stress_random_operations)
{
    SCOPED_TRACE("Testing stress random operations");

    skiplist<int> sl;
    std::mt19937  gen(12345); // Fixed seed for deterministic testing
    std::uniform_real_distribution<double> score_dist(-1000.0, 1000.0);
    std::uniform_int_distribution<int>     val_dist(1, 500);

    const int                           N = 2000;
    std::vector<std::pair<double, int>> elements;

    for(int i = 0; i < N; ++i)
    {
        double score = score_dist(gen);
        int    val   = val_dist(gen);
        sl.insert(score, val);
        elements.emplace_back(score, val);
    }

    EXPECT_EQ(sl.size(), N);
    EXPECT_TRUE(verify_order(sl));

    // Delete half elements randomly
    std::shuffle(elements.begin(), elements.end(), gen);
    for(size_t i = 0; i < N / 2; ++i)
    {
        sl.erase(elements[i].first, elements[i].second);
    }

    EXPECT_EQ(sl.size(), N - N / 2);
    EXPECT_TRUE(verify_order(sl));
}

TEST(skiplist_test, custom_score_comparator_descending)
{
    SCOPED_TRACE(
        "Testing skiplist with descending score comparator (std::greater)");

    skiplist<std::string, double, std::less<std::string>, std::greater<double>>
        leaderboard;

    leaderboard.insert(100.0, "Player_A");
    leaderboard.insert(500.0, "Player_B");
    leaderboard.insert(250.0, "Player_C");

    EXPECT_EQ(leaderboard.size(), 3);
    EXPECT_TRUE(verify_order(leaderboard));

    auto it = leaderboard.begin();
    EXPECT_EQ(*it, "Player_B");
    EXPECT_EQ(it.score(), 500.0);

    ++it;
    EXPECT_EQ(*it, "Player_C");
    EXPECT_EQ(it.score(), 250.0);

    ++it;
    EXPECT_EQ(*it, "Player_A");
    EXPECT_EQ(it.score(), 100.0);
}

TEST(skiplist_test, copy_constructor_exception_safety)
{
    skiplist<ThrowOnCopy> src;

    ThrowOnCopy::throw_flag = false;

    src.insert(1.0, ThrowOnCopy{1});
    src.insert(2.0, ThrowOnCopy{2});
    src.insert(3.0, ThrowOnCopy{3});

    ThrowOnCopy::throw_flag = true;

    EXPECT_THROW(skiplist<ThrowOnCopy> copy(src), std::runtime_error);

    ThrowOnCopy::throw_flag = false;

    EXPECT_EQ(src.size(), 3);
    EXPECT_TRUE(src.validate());
}