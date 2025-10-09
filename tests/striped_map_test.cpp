#include <thread>
#include <atomic>
#include <vector>
#include <gtest/gtest.h>
#include <hj/types/striped_map.hpp>
#include <string>

TEST(striped_map, emplace)
{
    hj::striped_map<int, std::string> m{
        [](const int &key) -> int { return key % 10; },
        10};
    m.emplace(1, "e1");
    m.emplace(11, "e11");
    m.emplace(123, "e123");
    std::string value;
    ASSERT_EQ(m.find(1, value), true);
    ASSERT_EQ(value == std::string("e1"), true);

    m.emplace(1, "new e1");
    ASSERT_EQ(m.find(1, value), true);
    ASSERT_EQ(value == std::string("e1"), true);
}

TEST(striped_map, replace)
{
    hj::striped_map<int, std::string> m{
        [](const int &key) -> int { return key % 10; },
        10};
    m.emplace(1, "e1");
    m.emplace(11, "e11");
    m.emplace(123, "e123");
    std::string value;
    ASSERT_EQ(m.find(1, value), true);
    ASSERT_EQ(value == std::string("e1"), true);

    m.replace(1, "new e1");
    std::string new_value;
    ASSERT_EQ(m.find(1, new_value), true);
    ASSERT_EQ(new_value == std::string("new e1"), true);
}

TEST(striped_map, find)
{
    hj::striped_map<int, std::string> m{
        [](const int &key) -> int { return key % 10; },
        10};
    m.emplace(1, "e1");
    m.emplace(11, "e11");
    m.emplace(123, "e123");
    std::string value;
    ASSERT_EQ(m.find(1, value), true);
    ASSERT_EQ(value == std::string("e1"), true);
    ASSERT_EQ(m.find(11, value), true);
    ASSERT_EQ(value == std::string("e11"), true);
    ASSERT_EQ(m.find(123, value), true);
    ASSERT_EQ(value == std::string("e123"), true);
}

TEST(striped_map, erase)
{
    hj::striped_map<int, std::string> m{
        [](const int &key) -> int { return key % 10; },
        10};
    m.emplace(1, "e1");
    m.emplace(11, "e11");
    m.emplace(123, "e123");
    ASSERT_EQ(m.erase(11), true);
    std::string value;
    ASSERT_EQ(m.find(11, value), false);
}

TEST(striped_map, range)
{
    hj::striped_map<int, std::string> m{
        [](const int &key) -> int { return key % 10; },
        10};
    m.emplace(1, "e1");
    m.emplace(11, "e11");
    m.emplace(123, "e123");
    bool equal = false;
    m.range([&](const int &key, const std::string &value) -> bool {
        if(key == 1)
            equal = (value == std::string("e1"));
        if(key == 11)
            equal = (value == std::string("e11"));
        if(key == 123)
            equal = (value == std::string("e123"));
        return equal;
    });
    ASSERT_EQ(equal, true);
}

TEST(striped_map, size)
{
    hj::striped_map<int, std::string> m{
        [](const int &key) -> int { return key % 10; },
        10};
    m.emplace(1, "e1");
    m.emplace(11, "e11");
    m.emplace(123, "e123");
    ASSERT_EQ(m.size(), 3);
}

TEST(striped_map, iterator_traversal)
{
    hj::striped_map<int, std::string> m{
        [](const int &key) -> int { return key % 4; },
        4};
    m.emplace(1, "a");
    m.emplace(2, "b");
    m.emplace(3, "c");
    m.emplace(4, "d");
    m.emplace(5, "e");
    std::vector<std::pair<int, std::string>> result;
    for(auto it = m.begin(); it != m.end(); ++it)
        result.emplace_back(it->first, it->second);

    ASSERT_EQ(result.size(), 5);
    ASSERT_TRUE(std::find(result.begin(),
                          result.end(),
                          std::make_pair(1, std::string("a")))
                != result.end());
    ASSERT_TRUE(std::find(result.begin(),
                          result.end(),
                          std::make_pair(2, std::string("b")))
                != result.end());
    ASSERT_TRUE(std::find(result.begin(),
                          result.end(),
                          std::make_pair(3, std::string("c")))
                != result.end());
    ASSERT_TRUE(std::find(result.begin(),
                          result.end(),
                          std::make_pair(4, std::string("d")))
                != result.end());
    ASSERT_TRUE(std::find(result.begin(),
                          result.end(),
                          std::make_pair(5, std::string("e")))
                != result.end());
}

TEST(striped_map, iterator_empty_map)
{
    hj::striped_map<int, std::string> m{
        [](const int &key) -> int { return key % 4; },
        4};

    ASSERT_EQ(m.begin(), m.end());
    int count = 0;
    for(auto it = m.begin(); it != m.end(); ++it)
        ++count;
    ASSERT_EQ(count, 0);
}

TEST(striped_map, iterator_single_element)
{
    hj::striped_map<int, std::string> m{
        [](const int &key) -> int { return key % 4; },
        4};

    m.emplace(42, "single");
    auto it = m.begin();
    ASSERT_NE(it, m.end());
    ASSERT_EQ(it->first, 42);
    ASSERT_EQ(it->second, "single");
    ++it;
    ASSERT_EQ(it, m.end());
}

TEST(striped_map, iterator_all_buckets_empty)
{
    hj::striped_map<int, std::string> m{
        [](const int &key) -> int { return key % 4; },
        4};

    m.emplace(1, "a");
    m.emplace(2, "b");
    m.erase(1);
    m.erase(2);
    ASSERT_EQ(m.size(), 0);
    ASSERT_EQ(m.begin(), m.end());
    int count = 0;
    for(auto it = m.begin(); it != m.end(); ++it)
        ++count;
    ASSERT_EQ(count, 0);
}

TEST(striped_map, allocator_compatibility)
{
    using my_alloc = std::allocator<std::pair<const int, std::string>>;
    hj::striped_map<int, std::string, my_alloc> m(
        [](const int &key) { return key % 8; },
        8,
        my_alloc{});

    m.emplace(42, "alloc");
    std::string value;
    ASSERT_TRUE(m.find(42, value));
    ASSERT_EQ(value, "alloc");
}

TEST(striped_map, multithreaded_read_write)
{
    hj::striped_map<int64_t, int64_t> m(8);
    constexpr int64_t                 thread_count   = 2;
    constexpr int64_t                 ops_per_thread = 1000000;
    std::atomic<int64_t>              write_sum{0};
    std::atomic<int64_t>              read_sum{0};
    std::vector<std::thread>          write_threads;
    std::vector<std::thread>          read_threads;

    for(int64_t t = 0; t < thread_count; ++t)
    {
        write_threads.emplace_back([&, t]() {
            for(int64_t i = 0; i < ops_per_thread; ++i)
            {
                int64_t key = t * ops_per_thread + i;
                m.emplace(key, key);
                write_sum += key;
            }
        });
    }

    for(auto &th : write_threads)
        th.join();

    for(int64_t t = 0; t < thread_count; ++t)
    {
        read_threads.emplace_back([&, t]() {
            int64_t local_sum = 0;
            for(int64_t i = 0; i < ops_per_thread; ++i)
            {
                int64_t key   = t * ops_per_thread + i;
                int64_t value = 0;
                if(m.find(key, value))
                    local_sum += value;
            }
            read_sum += local_sum;
        });
    }

    for(auto &th : read_threads)
        th.join();

    ASSERT_EQ(m.size(), thread_count * ops_per_thread);
    ASSERT_EQ(write_sum.load(), read_sum.load());
}