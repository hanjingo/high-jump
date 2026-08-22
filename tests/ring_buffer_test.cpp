#include <gtest/gtest.h>
#include <hj/io/ring_buffer.hpp>

#include <vector>

TEST(ring_buffer, push_back_overwrite)
{
    hj::ring_buffer<int> buf{3};
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    buf.push_back(4);
    ASSERT_EQ(buf.size(), 3);
    EXPECT_EQ(buf[0], 2);
    EXPECT_EQ(buf[1], 3);
    EXPECT_EQ(buf[2], 4);
}

TEST(ring_buffer, front)
{
    hj::ring_buffer<int> buf{3};
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    EXPECT_EQ(buf.front(), 1);
    buf.push_back(4);
    EXPECT_EQ(buf.front(), 2);
}

TEST(ring_buffer, back)
{
    hj::ring_buffer<int> buf{3};
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    EXPECT_EQ(buf.back(), 3);
    buf.push_back(4);
    EXPECT_EQ(buf.back(), 4);
}

TEST(ring_buffer, at_and_operator)
{
    hj::ring_buffer<int> buf{3};
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    EXPECT_EQ(buf.at(0), 1);
    EXPECT_EQ(buf[0], 1);
    buf.push_back(4);
    EXPECT_EQ(buf.at(0), 2);
    EXPECT_EQ(buf[0], 2);

    EXPECT_THROW(buf.at(3), std::out_of_range);
}

TEST(ring_buffer, full_and_empty)
{
    hj::ring_buffer<int> buf{3};
    EXPECT_TRUE(buf.empty());
    EXPECT_FALSE(buf.full());
    buf.push_back(1);
    buf.push_back(2);
    EXPECT_FALSE(buf.full());
    buf.push_back(3);
    EXPECT_TRUE(buf.full());
    buf.clear();
    EXPECT_TRUE(buf.empty());
    EXPECT_FALSE(buf.full());
}

TEST(ring_buffer, size_and_clear)
{
    hj::ring_buffer<int> buf{3};
    EXPECT_EQ(buf.size(), 0);
    buf.push_back(1);
    buf.push_back(2);
    EXPECT_EQ(buf.size(), 2);
    buf.push_back(3);
    EXPECT_EQ(buf.size(), 3);
    buf.clear();
    EXPECT_EQ(buf.size(), 0);
}

TEST(ring_buffer, capacity)
{
    hj::ring_buffer<int> buf{3};
    EXPECT_EQ(buf.capacity(), 3);
    buf.push_back(1);
    buf.push_back(2);
    EXPECT_EQ(buf.capacity(), 3);
}

TEST(ring_buffer, pop_front_and_pop_back)
{
    hj::ring_buffer<int> buf{3};
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    buf.pop_front();
    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf.front(), 2);
    buf.pop_back();
    EXPECT_EQ(buf.size(), 1);
    EXPECT_EQ(buf.back(), 2);
}

TEST(ring_buffer, iterator)
{
    hj::ring_buffer<int> buf{3};
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    int sum = 0;
    for(auto v : buf)
        sum += v;
    EXPECT_EQ(sum, 6);
    buf.push_back(4); // now 2,3,4
    std::vector<int> v(buf.begin(), buf.end());
    EXPECT_EQ((v == std::vector<int>({2, 3, 4})), true);
}

TEST(ring_buffer, reject_policy_and_try_push)
{
    hj::ring_buffer<int, hj::ring_buffer_policy::reject> buf{3};

    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    EXPECT_THROW(buf.push_back(4), std::overflow_error);

    EXPECT_EQ(buf.size(), 3);
    EXPECT_EQ(buf[0], 1);
    EXPECT_EQ(buf[1], 2);
    EXPECT_EQ(buf[2], 3);

    EXPECT_FALSE(buf.try_push_back(4));
    EXPECT_FALSE(buf.try_push_back(int{5}));

    buf.pop_front();
    EXPECT_TRUE(buf.try_push_back(4));
    EXPECT_EQ(buf.back(), 4);
}

struct move_only_type
{
    int value;
    explicit move_only_type(int v)
        : value(v)
    {
    }

    move_only_type(const move_only_type &)            = delete;
    move_only_type &operator=(const move_only_type &) = delete;

    move_only_type(move_only_type &&) noexcept            = default;
    move_only_type &operator=(move_only_type &&) noexcept = default;
};

TEST(ring_buffer, move_only_type_support)
{
    hj::ring_buffer<std::unique_ptr<int>, hj::ring_buffer_policy::reject> buf{
        2};

    buf.push_back(std::make_unique<int>(10));
    buf.push_back(std::make_unique<int>(20));

    EXPECT_TRUE(buf.full());
    EXPECT_EQ(*(buf.front()), 10);
    EXPECT_EQ(*(buf.back()), 20);

    hj::ring_buffer<move_only_type> overwrite_buf{2};
    overwrite_buf.push_back(move_only_type{100});
    overwrite_buf.push_back(move_only_type{200});

    overwrite_buf.push_back(move_only_type{300});
    EXPECT_EQ(overwrite_buf.size(), 2);
    EXPECT_EQ(overwrite_buf[0].value, 200);
    EXPECT_EQ(overwrite_buf[1].value, 300);
}

TEST(ring_buffer, zero_capacity)
{
    hj::ring_buffer<int, hj::ring_buffer_policy::reject> reject_buf{0};
    EXPECT_EQ(reject_buf.capacity(), 0);
    EXPECT_TRUE(reject_buf.empty());
    EXPECT_TRUE(reject_buf.full());

    EXPECT_THROW(reject_buf.push_back(1), std::overflow_error);
    EXPECT_FALSE(reject_buf.try_push_back(1));

    hj::ring_buffer<int, hj::ring_buffer_policy::overwrite> overwrite_buf{0};
    EXPECT_EQ(overwrite_buf.capacity(), 0);

    overwrite_buf.push_back(1);
    EXPECT_EQ(overwrite_buf.size(), 0);
}

struct throwing_type
{
    int                value;
    inline static bool throw_on_copy = false;

    explicit throwing_type(int v)
        : value(v)
    {
    }

    throwing_type(const throwing_type &other)
        : value(other.value)
    {
        if(throw_on_copy)
        {
            throw std::runtime_error("Copy constructor failed");
        }
    }

    throwing_type &operator=(const throwing_type &)     = default;
    throwing_type(throwing_type &&) noexcept            = default;
    throwing_type &operator=(throwing_type &&) noexcept = default;
};

TEST(ring_buffer, exception_safety)
{
    hj::ring_buffer<throwing_type> buf{3};
    buf.push_back(throwing_type{1});
    buf.push_back(throwing_type{2});

    throwing_type::throw_on_copy = true;
    throwing_type item{3};

    EXPECT_THROW(buf.push_back(item), std::runtime_error);
    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf[0].value, 1);
    EXPECT_EQ(buf[1].value, 2);

    throwing_type::throw_on_copy = false;
}

namespace
{
struct tracker
{
    inline static int constructions = 0;
    inline static int destructions  = 0;

    int value;

    explicit tracker(int v)
        : value(v)
    {
        constructions++;
    }

    tracker(const tracker &other)
        : value(other.value)
    {
        constructions++;
    }

    tracker(tracker &&other) noexcept
        : value(other.value)
    {
        constructions++;
    }

    tracker &operator=(const tracker &)     = default;
    tracker &operator=(tracker &&) noexcept = default;

    ~tracker() { destructions++; }

    static void reset()
    {
        constructions = 0;
        destructions  = 0;
    }
};

TEST(ring_buffer, lifecycle_and_overwrite_destruction)
{
    tracker::reset();

    {
        hj::ring_buffer<tracker, hj::ring_buffer_policy::overwrite> buf{2};

        buf.push_back(tracker{1});
        buf.push_back(tracker{2});
        EXPECT_EQ(tracker::constructions, tracker::destructions + 2);

        buf.push_back(tracker{3});

        EXPECT_EQ(buf.size(), 2);
        EXPECT_EQ(buf[0].value, 2);
        EXPECT_EQ(buf[1].value, 3);

        buf.pop_front();

        buf.clear();
        EXPECT_EQ(buf.size(), 0);
    }

    EXPECT_EQ(tracker::constructions, tracker::destructions);
}
}

namespace
{
struct complex_type
{
    int         a;
    double      b;
    std::string c;

    complex_type(int a, double b, std::string c)
        : a(a)
        , b(b)
        , c(std::move(c))
    {
    }
};

TEST(ring_buffer, emplace_back)
{
    hj::ring_buffer<complex_type, hj::ring_buffer_policy::reject> buf{2};

    buf.emplace_back(1, 2.5, "hello");
    buf.emplace_back(2, 3.5, "world");

    EXPECT_EQ(buf.size(), 2);
    EXPECT_EQ(buf[0].a, 1);
    EXPECT_EQ(buf[1].c, "world");

    EXPECT_THROW(buf.emplace_back(3, 4.5, "fail"), std::overflow_error);
}

TEST(ring_buffer, try_emplace_back)
{
    hj::ring_buffer<complex_type, hj::ring_buffer_policy::reject> buf{1};

    EXPECT_TRUE(buf.try_emplace_back(10, 1.1, "test"));

    EXPECT_FALSE(buf.try_emplace_back(20, 2.2, "fail"));

    EXPECT_EQ(buf.size(), 1);
    EXPECT_EQ(buf.back().a, 10);
}
}