#include <gtest/gtest.h>
#include <hj/types/optional.hpp>
#include <string>
#include <memory>
#include <utility>
#include <type_traits>

struct Tracker
{
    static int constructed;
    static int destroyed;

    Tracker() { ++constructed; }
    Tracker(int val)
        : value(val)
    {
        ++constructed;
    }
    ~Tracker() { ++destroyed; }

    Tracker(const Tracker &other)
        : value(other.value)
    {
        ++constructed;
    }
    Tracker(Tracker &&other) noexcept
        : value(other.value)
    {
        ++constructed;
    }

    Tracker &operator=(const Tracker &other)
    {
        value = other.value;
        return *this;
    }
    Tracker &operator=(Tracker &&other) noexcept
    {
        value = other.value;
        return *this;
    }

    int value = 0;

    static void reset_counters()
    {
        constructed = 0;
        destroyed   = 0;
    }
};

int Tracker::constructed = 0;
int Tracker::destroyed   = 0;

static_assert(std::is_default_constructible_v<hj::optional<int>>,
              "Should be default constructible");
static_assert(std::is_copy_constructible_v<hj::optional<int>>,
              "Should be copy constructible");
static_assert(std::is_move_constructible_v<hj::optional<int>>,
              "Should be move constructible");

TEST(optional, construction_assignment)
{
    hj::optional<int> o1;
    ASSERT_FALSE(o1.has_value());

    hj::optional<int> o2(hj::nullopt);
    ASSERT_FALSE(o2.has_value());

    hj::optional<int> o3(42);
    ASSERT_TRUE(o3.has_value());
    ASSERT_EQ(*o3, 42);

    hj::optional<int> o4;
    o4 = 100;
    ASSERT_TRUE(o4.has_value());
    ASSERT_EQ(o4.value(), 100);

    o4 = hj::nullopt;
    ASSERT_FALSE(o4.has_value());
}

TEST(optional, state_transition)
{
    hj::optional<int> o;
    ASSERT_FALSE(o);

    o.emplace(10);
    ASSERT_TRUE(o);
    ASSERT_EQ(*o, 10);

    o.emplace(20);
    ASSERT_TRUE(o);
    ASSERT_EQ(*o, 20);

    o.reset();
    ASSERT_FALSE(o);
}

TEST(optional, access_and_operators)
{
    hj::optional<std::string> o("hello");
    ASSERT_TRUE(o.has_value());

    EXPECT_EQ(*o, "hello");
    EXPECT_EQ(o->size(), 5);

    const hj::optional<std::string> co = o;
    EXPECT_EQ(*co, "hello");
    EXPECT_EQ(co->size(), 5);
    EXPECT_EQ(co.value(), "hello");
}

TEST(optional, move_only_support)
{
    hj::optional<std::unique_ptr<int>> opt_ptr;
    opt_ptr.emplace(std::make_unique<int>(42));

    ASSERT_TRUE(opt_ptr.has_value());
    ASSERT_EQ(**opt_ptr, 42);

    hj::optional<std::unique_ptr<int>> moved_opt = std::move(opt_ptr);
    ASSERT_TRUE(opt_ptr.has_value());
    ASSERT_TRUE(moved_opt.has_value());
    ASSERT_EQ(**moved_opt, 42);

    auto def_val = std::make_unique<int>(99);
    auto res     = std::move(moved_opt).value_or(std::move(def_val));
    ASSERT_EQ(*res, 42);
}

TEST(optional, lifecycle_tracker)
{
    Tracker::reset_counters();

    {
        hj::optional<Tracker> opt;
        EXPECT_FALSE(opt.has_value());
        EXPECT_EQ(Tracker::constructed, 0);

        opt.emplace(5);
        EXPECT_TRUE(opt.has_value());
        EXPECT_EQ(Tracker::constructed, 1);
        EXPECT_EQ(Tracker::destroyed, 0);

        opt.emplace(10);
        EXPECT_EQ(Tracker::constructed, 2);
        EXPECT_EQ(Tracker::destroyed, 1);

        opt.reset();
        EXPECT_FALSE(opt.has_value());
        EXPECT_EQ(Tracker::destroyed, 2);
    }
    EXPECT_EQ(Tracker::constructed, Tracker::destroyed);
}

TEST(optional, swap_test)
{
    hj::optional<int> a(1);
    hj::optional<int> b(2);

    a.swap(b);
    EXPECT_EQ(a.value(), 2);
    EXPECT_EQ(b.value(), 1);

    hj::optional<int> empty_opt;
    a.swap(empty_opt);
    EXPECT_FALSE(a.has_value());
    EXPECT_TRUE(empty_opt.has_value());
    EXPECT_EQ(empty_opt.value(), 2);
}

TEST(optional, exception_safety)
{
    hj::optional<int> o;
#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
    EXPECT_THROW(o.value(), std::bad_optional_access);
#else
    EXPECT_THROW(o.value(), boost::bad_optional_access);
#endif
}

TEST(optional, value_returns_reference)
{
    hj::optional<int> opt(42);
    opt.value() = 100;
    ASSERT_TRUE(opt.has_value());
    ASSERT_EQ(opt.value(), 100);
}

TEST(optional, const_value_returns_const_reference)
{
    const hj::optional<int> opt(42);
    static_assert(
        std::is_same_v<decltype(std::declval<hj::optional<int> &>().value()),
                       int &>,
        "hj::optional<T>::value() & must return T&");

    static_assert(std::is_same_v<decltype(opt.value()), const int &>,
                  "hj::optional<T>::value() const & must return const T&");
}

// TEST(optional, reference_type_unsupported)
// {
//     static_assert(!std::is_default_constructible_v<hj::optional<int &>>,
//                   "hj::optional<int&> should not be constructible");
// }

struct Throwing
{
    Throwing() { throw std::runtime_error("error"); }
};

TEST(optional, exception_safety_emplace)
{
    hj::optional<int> opt(42);
}

struct ThrowingTracker
{
    ThrowingTracker(int) { throw std::runtime_error("ctor failed"); }
};

TEST(optional, throwing_emplace_safety)
{
    hj::optional<ThrowingTracker> opt;
    EXPECT_THROW(opt.emplace(42), std::runtime_error);
    EXPECT_FALSE(opt.has_value());
}

static_assert(std::is_default_constructible_v<hj::optional<int>>,
              "Should be default constructible");
static_assert(std::is_nothrow_default_constructible_v<hj::optional<int>>,
              "Should be nothrow default constructible");
static_assert(std::is_copy_constructible_v<hj::optional<int>>,
              "Should be copy constructible");
static_assert(std::is_move_constructible_v<hj::optional<int>>,
              "Should be move constructible");
static_assert(std::is_nothrow_move_constructible_v<hj::optional<int>>,
              "Should be nothrow move constructible");

static_assert(std::is_copy_assignable_v<hj::optional<int>>,
              "Should be copy assignable");
static_assert(std::is_move_assignable_v<hj::optional<int>>,
              "Should be move assignable");

static_assert(!std::is_copy_constructible_v<hj::optional<std::unique_ptr<int>>>,
              "optional of move-only type must not be copy constructible");

static_assert(std::is_move_constructible_v<hj::optional<std::unique_ptr<int>>>,
              "optional of move-only type must be move constructible");