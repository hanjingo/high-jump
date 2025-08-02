#include <gtest/gtest.h>
#include <libcpp/algo/skiplist.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>

using namespace libcpp;

class SkipListTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::cout << "\n=== Skip List Tests Setup ===" << std::endl;
    }
    
    void TearDown() override {
        std::cout << "=== Skip List Tests Teardown ===" << std::endl;
    }

    // Helper function to print skiplist for debugging
    template<typename T>
    void print_skiplist_debug(const skiplist<T>& sl) {
        std::cout << "SkipList size: " << sl.size() << std::endl;
        std::cout << "Elements: ";
        for (auto it = sl.begin(); it != sl.end(); ++it) {
            std::cout << "[" << it.score() << ":" << *it << "] ";
        }
        std::cout << std::endl;
    }

    // Helper function to verify skiplist order
    template<typename T>
    bool verify_order(const skiplist<T>& sl) {
        double prev_score = -std::numeric_limits<double>::infinity();
        for (auto it = sl.begin(); it != sl.end(); ++it) {
            if (it.score() < prev_score) {
                return false;
            }
            prev_score = it.score();
        }
        return true;
    }
};

// ================================ Basic Operations Tests ================================

TEST_F(SkipListTest, constructor_and_basic_properties) {
    SCOPED_TRACE("Testing skiplist constructor and basic properties");
    
    skiplist<std::string> sl;
    
    EXPECT_EQ(sl.size(), 0);
    EXPECT_TRUE(sl.empty());
    EXPECT_EQ(sl.first(), nullptr);
    EXPECT_EQ(sl.last(), nullptr);
    EXPECT_EQ(sl.begin(), sl.end());
}

TEST_F(SkipListTest, single_element_operations) {
    SCOPED_TRACE("Testing single element insert and access");
    
    skiplist<std::string> sl;
    
    // Insert single element
    auto* node = sl.insert(1.0, "apple");
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->score, 1.0);
    EXPECT_EQ(node->obj, "apple");
    
    // Test basic properties
    EXPECT_EQ(sl.size(), 1);
    EXPECT_FALSE(sl.empty());
    EXPECT_EQ(sl.first(), node);
    EXPECT_EQ(sl.last(), node);
    
    // Test iteration
    auto it = sl.begin();
    EXPECT_NE(it, sl.end());
    EXPECT_EQ(*it, "apple");
    EXPECT_EQ(it.score(), 1.0);
    
    ++it;
    EXPECT_EQ(it, sl.end());
}

TEST_F(SkipListTest, multiple_element_insertion) {
    SCOPED_TRACE("Testing multiple element insertion");
    
    skiplist<std::string> sl;
    
    // Insert elements in random order
    sl.insert(3.0, "cherry");
    sl.insert(1.0, "apple");
    sl.insert(5.0, "elderberry");
    sl.insert(2.0, "banana");
    sl.insert(4.0, "date");
    
    EXPECT_EQ(sl.size(), 5);
    EXPECT_TRUE(verify_order(sl));
    
    // Verify order by iteration
    std::vector<std::string> expected = {"apple", "banana", "cherry", "date", "elderberry"};
    std::vector<double> expected_scores = {1.0, 2.0, 3.0, 4.0, 5.0};
    
    size_t i = 0;
    for (auto it = sl.begin(); it != sl.end(); ++it, ++i) {
        EXPECT_EQ(*it, expected[i]);
        EXPECT_EQ(it.score(), expected_scores[i]);
    }
}

TEST_F(SkipListTest, duplicate_scores) {
    SCOPED_TRACE("Testing duplicate scores with different objects");
    
    skiplist<std::string> sl;
    
    // Insert elements with same score
    sl.insert(2.0, "banana");
    sl.insert(2.0, "blueberry");
    sl.insert(2.0, "blackberry");
    sl.insert(1.0, "apple");
    sl.insert(3.0, "cherry");
    
    EXPECT_EQ(sl.size(), 5);
    EXPECT_TRUE(verify_order(sl));
    
    // Elements with same score should be ordered by comparison function
    print_skiplist_debug(sl);
}

// ================================ Deletion Tests ================================

TEST_F(SkipListTest, delete_single_element) {
    SCOPED_TRACE("Testing deletion of single element");
    
    skiplist<std::string> sl;
    sl.insert(1.0, "apple");
    
    EXPECT_TRUE(sl.delete_node(1.0, "apple"));
    EXPECT_EQ(sl.size(), 0);
    EXPECT_TRUE(sl.empty());
    EXPECT_EQ(sl.first(), nullptr);
    EXPECT_EQ(sl.last(), nullptr);
}

TEST_F(SkipListTest, delete_multiple_elements) {
    SCOPED_TRACE("Testing deletion of multiple elements");
    
    skiplist<std::string> sl;
    
    // Insert test data
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    sl.insert(3.0, "cherry");
    sl.insert(4.0, "date");
    sl.insert(5.0, "elderberry");
    
    // Delete middle element
    EXPECT_TRUE(sl.delete_node(3.0, "cherry"));
    EXPECT_EQ(sl.size(), 4);
    
    // Delete first element
    EXPECT_TRUE(sl.delete_node(1.0, "apple"));
    EXPECT_EQ(sl.size(), 3);
    
    // Delete last element
    EXPECT_TRUE(sl.delete_node(5.0, "elderberry"));
    EXPECT_EQ(sl.size(), 2);
    
    // Verify remaining elements
    std::vector<std::string> expected = {"banana", "date"};
    std::vector<double> expected_scores = {2.0, 4.0};
    
    size_t i = 0;
    for (auto it = sl.begin(); it != sl.end(); ++it, ++i) {
        EXPECT_EQ(*it, expected[i]);
        EXPECT_EQ(it.score(), expected_scores[i]);
    }
}

TEST_F(SkipListTest, delete_nonexistent_element) {
    SCOPED_TRACE("Testing deletion of non-existent element");
    
    skiplist<std::string> sl;
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    
    // Try to delete non-existent elements
    EXPECT_FALSE(sl.delete_node(3.0, "cherry"));
    EXPECT_FALSE(sl.delete_node(1.0, "orange"));
    EXPECT_FALSE(sl.delete_node(2.5, "banana"));
    
    EXPECT_EQ(sl.size(), 2);
}

// ================================ Rank Operations Tests ================================

TEST_F(SkipListTest, get_element_by_rank) {
    SCOPED_TRACE("Testing get element by rank");
    
    skiplist<std::string> sl;
    
    // Insert test data
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    sl.insert(3.0, "cherry");
    sl.insert(4.0, "date");
    sl.insert(5.0, "elderberry");
    
    // Test valid ranks (0-based)
    auto* node0 = sl.get_element_by_rank(0);
    ASSERT_NE(node0, nullptr);
    EXPECT_EQ(node0->obj, "apple");
    EXPECT_EQ(node0->score, 1.0);
    
    auto* node2 = sl.get_element_by_rank(2);
    ASSERT_NE(node2, nullptr);
    EXPECT_EQ(node2->obj, "cherry");
    EXPECT_EQ(node2->score, 3.0);
    
    auto* node4 = sl.get_element_by_rank(4);
    ASSERT_NE(node4, nullptr);
    EXPECT_EQ(node4->obj, "elderberry");
    EXPECT_EQ(node4->score, 5.0);
    
    // Test invalid ranks
    EXPECT_EQ(sl.get_element_by_rank(5), nullptr);
    EXPECT_EQ(sl.get_element_by_rank(100), nullptr);
}

TEST_F(SkipListTest, get_rank) {
    SCOPED_TRACE("Testing get rank of elements");
    
    skiplist<std::string> sl;
    
    // Insert test data
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    sl.insert(3.0, "cherry");
    sl.insert(4.0, "date");
    sl.insert(5.0, "elderberry");
    
    // Test ranks of existing elements
    EXPECT_EQ(sl.get_rank(1.0, "apple"), 0);
    EXPECT_EQ(sl.get_rank(2.0, "banana"), 1);
    EXPECT_EQ(sl.get_rank(3.0, "cherry"), 2);
    EXPECT_EQ(sl.get_rank(4.0, "date"), 3);
    EXPECT_EQ(sl.get_rank(5.0, "elderberry"), 4);
    
    // Test ranks of non-existent elements
    EXPECT_EQ(sl.get_rank(6.0, "fig"), sl.size());
    EXPECT_EQ(sl.get_rank(1.0, "orange"), sl.size());
}

// ================================ Range Operations Tests ================================

TEST_F(SkipListTest, first_and_last_in_range) {
    SCOPED_TRACE("Testing first and last in range");
    
    skiplist<std::string> sl;
    
    // Insert test data
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    sl.insert(3.0, "cherry");
    sl.insert(4.0, "date");
    sl.insert(5.0, "elderberry");
    
    // Test valid ranges
    auto* first = sl.first_in_range(2.0, 4.0);
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first->obj, "banana");
    EXPECT_EQ(first->score, 2.0);
    
    auto* last = sl.last_in_range(2.0, 4.0);
    ASSERT_NE(last, nullptr);
    EXPECT_EQ(last->obj, "date");
    EXPECT_EQ(last->score, 4.0);
    
    // Test edge cases
    auto* first_all = sl.first_in_range(0.0, 10.0);
    EXPECT_EQ(first_all, sl.first());
    
    auto* last_all = sl.last_in_range(0.0, 10.0);
    EXPECT_EQ(last_all, sl.last());
    
    // Test invalid ranges
    auto* invalid = sl.first_in_range(6.0, 10.0);
    EXPECT_EQ(invalid, nullptr);
    
    auto* invalid2 = sl.first_in_range(10.0, 5.0);  // min > max
    EXPECT_EQ(invalid2, nullptr);
}

TEST_F(SkipListTest, get_range_by_score) {
    SCOPED_TRACE("Testing get range by score");
    
    skiplist<std::string> sl;
    
    // Insert test data
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    sl.insert(3.0, "cherry");
    sl.insert(4.0, "date");
    sl.insert(5.0, "elderberry");
    sl.insert(6.0, "fig");
    sl.insert(7.0, "grape");
    
    // Test basic range
    auto result = get_range_by_score(sl, 2.0, 5.0);
    EXPECT_EQ(result.nodes.size(), 4);
    EXPECT_EQ(result.total_in_range, 4);
    
    std::vector<std::string> expected = {"banana", "cherry", "date", "elderberry"};
    for (size_t i = 0; i < result.nodes.size(); ++i) {
        EXPECT_EQ(result.nodes[i]->obj, expected[i]);
    }
    
    // Test with offset and limit
    auto result_limited = get_range_by_score(sl, 1.0, 7.0, 2, 3);
    EXPECT_EQ(result_limited.nodes.size(), 3);
    EXPECT_EQ(result_limited.total_in_range, 7);  // Total in range should count all
    
    std::vector<std::string> expected_limited = {"cherry", "date", "elderberry"};
    for (size_t i = 0; i < result_limited.nodes.size(); ++i) {
        EXPECT_EQ(result_limited.nodes[i]->obj, expected_limited[i]);
    }
}

TEST_F(SkipListTest, get_range_by_rank) {
    SCOPED_TRACE("Testing get range by rank");
    
    skiplist<std::string> sl;
    
    // Insert test data
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    sl.insert(3.0, "cherry");
    sl.insert(4.0, "date");
    sl.insert(5.0, "elderberry");
    
    // Test forward range
    auto result = get_range_by_rank(sl, 1, 3, false);
    EXPECT_EQ(result.nodes.size(), 3);
    EXPECT_EQ(result.total_in_range, 3);
    
    std::vector<std::string> expected = {"banana", "cherry", "date"};
    for (size_t i = 0; i < result.nodes.size(); ++i) {
        EXPECT_EQ(result.nodes[i]->obj, expected[i]);
    }
    
    // Test reverse range
    auto result_reverse = get_range_by_rank(sl, 1, 3, true);
    EXPECT_EQ(result_reverse.nodes.size(), 3);
    EXPECT_EQ(result_reverse.total_in_range, 3);
    
    std::vector<std::string> expected_reverse = {"date", "cherry", "banana"};
    for (size_t i = 0; i < result_reverse.nodes.size(); ++i) {
        EXPECT_EQ(result_reverse.nodes[i]->obj, expected_reverse[i]);
    }
}

TEST_F(SkipListTest, delete_range_by_score) {
    SCOPED_TRACE("Testing delete range by score");
    
    skiplist<std::string> sl;
    
    // Insert test data
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    sl.insert(3.0, "cherry");
    sl.insert(4.0, "date");
    sl.insert(5.0, "elderberry");
    sl.insert(6.0, "fig");
    
    // Delete range [2.0, 4.0]
    unsigned long deleted = sl.delete_range_by_score(2.0, 4.0);
    EXPECT_EQ(deleted, 3);
    EXPECT_EQ(sl.size(), 3);
    
    // Verify remaining elements
    std::vector<std::string> expected = {"apple", "elderberry", "fig"};
    std::vector<double> expected_scores = {1.0, 5.0, 6.0};
    
    size_t i = 0;
    for (auto it = sl.begin(); it != sl.end(); ++it, ++i) {
        EXPECT_EQ(*it, expected[i]);
        EXPECT_EQ(it.score(), expected_scores[i]);
    }
}

TEST_F(SkipListTest, delete_range_by_rank) {
    SCOPED_TRACE("Testing delete range by rank");
    
    skiplist<std::string> sl;
    
    // Insert test data
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    sl.insert(3.0, "cherry");
    sl.insert(4.0, "date");
    sl.insert(5.0, "elderberry");
    sl.insert(6.0, "fig");
    
    // Delete range [1, 3] (0-based, inclusive)
    unsigned long deleted = sl.delete_range_by_rank(1, 3);
    EXPECT_EQ(deleted, 3);
    EXPECT_EQ(sl.size(), 3);
    
    // Verify remaining elements
    std::vector<std::string> expected = {"apple", "elderberry", "fig"};
    std::vector<double> expected_scores = {1.0, 5.0, 6.0};
    
    size_t i = 0;
    for (auto it = sl.begin(); it != sl.end(); ++it, ++i) {
        EXPECT_EQ(*it, expected[i]);
        EXPECT_EQ(it.score(), expected_scores[i]);
    }
}

// ================================ Iterator Tests ================================

TEST_F(SkipListTest, forward_iteration) {
    SCOPED_TRACE("Testing forward iteration");
    
    skiplist<std::string> sl;
    
    // Insert test data
    sl.insert(3.0, "cherry");
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    
    std::vector<std::string> expected = {"apple", "banana", "cherry"};
    std::vector<double> expected_scores = {1.0, 2.0, 3.0};
    
    size_t i = 0;
    for (auto it = sl.begin(); it != sl.end(); ++it, ++i) {
        EXPECT_LT(i, expected.size());
        EXPECT_EQ(*it, expected[i]);
        EXPECT_EQ(it.score(), expected_scores[i]);
    }
    
    EXPECT_EQ(i, expected.size());
}

TEST_F(SkipListTest, backward_iteration) {
    SCOPED_TRACE("Testing backward iteration");
    
    skiplist<std::string> sl;
    
    // Insert test data
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    sl.insert(3.0, "cherry");
    
    std::vector<std::string> expected = {"cherry", "banana", "apple"};
    std::vector<double> expected_scores = {3.0, 2.0, 1.0};
    
    // Start from last element and go backward
    auto it = sl.begin();
    ++it; ++it;  // Move to last element
    
    size_t i = 0;
    do {
        EXPECT_LT(i, expected.size());
        EXPECT_EQ(*it, expected[i]);
        EXPECT_EQ(it.score(), expected_scores[i]);
        ++i;
        if (i < expected.size()) --it;
    } while (i < expected.size());
}

TEST_F(SkipListTest, iterator_operators) {
    SCOPED_TRACE("Testing iterator operators");
    
    skiplist<std::string> sl;
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    
    auto it1 = sl.begin();
    auto it2 = sl.begin();
    auto it3 = sl.end();
    
    // Test equality
    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 == it3);
    EXPECT_FALSE(it1 != it2);
    EXPECT_TRUE(it1 != it3);
    
    // Test increment
    ++it1;
    EXPECT_FALSE(it1 == it2);
    EXPECT_EQ(*it1, "banana");
    
    auto it4 = it2++;
    EXPECT_EQ(*it4, "apple");
    EXPECT_EQ(*it2, "banana");
}

// ================================ Custom Comparison Tests ================================

TEST_F(SkipListTest, custom_comparison_function) {
    SCOPED_TRACE("Testing custom comparison function");
    
    // Use reverse comparison for strings (Z to A)
    auto reverse_cmp = [](const std::string& a, const std::string& b) {
        return a > b;  // Reverse order
    };
    
    skiplist<std::string> sl(reverse_cmp);
    
    // Insert elements with same score
    sl.insert(1.0, "apple");
    sl.insert(1.0, "banana");
    sl.insert(1.0, "cherry");
    
    // Should be ordered: cherry, banana, apple (reverse alphabetical)
    std::vector<std::string> expected = {"cherry", "banana", "apple"};
    
    size_t i = 0;
    for (auto it = sl.begin(); it != sl.end(); ++it, ++i) {
        EXPECT_EQ(*it, expected[i]);
        EXPECT_EQ(it.score(), 1.0);
    }
}

// ================================ Edge Cases Tests ================================

TEST_F(SkipListTest, empty_skiplist_operations) {
    SCOPED_TRACE("Testing operations on empty skiplist");
    
    skiplist<std::string> sl;
    
    // Test operations on empty skiplist
    EXPECT_FALSE(sl.delete_node(1.0, "apple"));
    EXPECT_EQ(sl.get_element_by_rank(0), nullptr);
    EXPECT_EQ(sl.get_rank(1.0, "apple"), 0);
    EXPECT_EQ(sl.first_in_range(0.0, 10.0), nullptr);
    EXPECT_EQ(sl.last_in_range(0.0, 10.0), nullptr);
    EXPECT_EQ(sl.delete_range_by_score(0.0, 10.0), 0);
    EXPECT_EQ(sl.delete_range_by_rank(0, 10), 0);
    
    auto range_result = get_range_by_score(sl, 0.0, 10.0);
    EXPECT_EQ(range_result.nodes.size(), 0);
    EXPECT_EQ(range_result.total_in_range, 0);
}

TEST_F(SkipListTest, large_score_values) {
    SCOPED_TRACE("Testing large score values");
    
    skiplist<std::string> sl;
    
    // Test with very large and very small scores
    sl.insert(1e9, "large");
    sl.insert(-1e9, "small");
    sl.insert(0.0, "zero");
    sl.insert(std::numeric_limits<double>::infinity(), "infinity");
    
    EXPECT_EQ(sl.size(), 4);
    EXPECT_TRUE(verify_order(sl));
    
    // Verify order: -inf, zero, large, +inf
    auto it = sl.begin();
    EXPECT_EQ(*it, "small");
    ++it;
    EXPECT_EQ(*it, "zero");
    ++it;
    EXPECT_EQ(*it, "large");
    ++it;
    EXPECT_EQ(*it, "infinity");
}

TEST_F(SkipListTest, stress_test_random_operations) {
    SCOPED_TRACE("Testing stress test with random operations");
    
    skiplist<int> sl;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> score_dist(0.0, 100.0);
    std::uniform_int_distribution<int> value_dist(1, 1000);
    
    const int num_operations = 1000;
    std::vector<std::pair<double, int>> inserted;
    
    // Insert random elements
    for (int i = 0; i < num_operations; ++i) {
        double score = score_dist(gen);
        int value = value_dist(gen);
        
        sl.insert(score, value);
        inserted.emplace_back(score, value);
    }
    
    EXPECT_EQ(sl.size(), num_operations);
    EXPECT_TRUE(verify_order(sl));
    EXPECT_TRUE(sl.validate());
    
    // Verify some random access operations
    for (int i = 0; i < 10; ++i) {
        unsigned long rank = gen() % sl.size();
        auto* node = sl.get_element_by_rank(rank);
        EXPECT_NE(node, nullptr);
        
        unsigned long found_rank = sl.get_rank(node->score, node->obj);
        EXPECT_EQ(found_rank, rank);
    }
}

// ================================ Performance Tests ================================

TEST_F(SkipListTest, performance_test) {
    SCOPED_TRACE("Testing performance with large dataset");
    
    skiplist<int> sl;
    const int num_elements = 10000;
    
    // Measure insertion time
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_elements; ++i) {
        sl.insert(static_cast<double>(i), i);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Inserted " << num_elements << " elements in " << duration.count() << " ms" << std::endl;
    
    EXPECT_EQ(sl.size(), num_elements);
    EXPECT_TRUE(sl.validate());
    
    // Measure search time
    start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        auto* node = sl.get_element_by_rank(i * (num_elements / 1000));
        EXPECT_NE(node, nullptr);
    }
    
    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "1000 rank-based searches took " << duration.count() << " μs" << std::endl;
}

// ================================ Validation Tests ================================

TEST_F(SkipListTest, validate_structure) {
    SCOPED_TRACE("Testing skiplist structure validation");
    
    skiplist<std::string> sl;
    
    // Test validation on empty list
    EXPECT_TRUE(sl.validate());
    
    // Test validation with elements
    sl.insert(1.0, "apple");
    sl.insert(2.0, "banana");
    sl.insert(3.0, "cherry");
    
    EXPECT_TRUE(sl.validate());
    
    // Test validation after deletions
    sl.delete_node(2.0, "banana");
    EXPECT_TRUE(sl.validate());
    
    sl.delete_node(1.0, "apple");
    EXPECT_TRUE(sl.validate());
    
    sl.delete_node(3.0, "cherry");
    EXPECT_TRUE(sl.validate());
}

TEST_F(SkipListTest, integer_type_operations) {
    SCOPED_TRACE("Testing skiplist with integer type");
    
    skiplist<int> sl;
    
    // Insert integers
    sl.insert(3.5, 100);
    sl.insert(1.2, 50);
    sl.insert(2.8, 75);
    sl.insert(4.1, 125);
    
    EXPECT_EQ(sl.size(), 4);
    
    // Verify order
    std::vector<int> expected_values = {50, 75, 100, 125};
    std::vector<double> expected_scores = {1.2, 2.8, 3.5, 4.1};
    
    size_t i = 0;
    for (auto it = sl.begin(); it != sl.end(); ++it, ++i) {
        EXPECT_EQ(*it, expected_values[i]);
        EXPECT_EQ(it.score(), expected_scores[i]);
    }
}
