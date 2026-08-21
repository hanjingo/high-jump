#include <gtest/gtest.h>
#include <hj/types/optional.hpp>
#include <string>
#include <memory>
#include <utility>
#include <type_traits>

// 1. 生命周期跟踪器
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

// 2. 编译期类型特征测试 (Static Assertions)
static_assert(std::is_default_constructible_v<hj::optional<int>>,
              "Should be default constructible");
static_assert(std::is_copy_constructible_v<hj::optional<int>>,
              "Should be copy constructible");
static_assert(std::is_move_constructible_v<hj::optional<int>>,
              "Should be move constructible");

TEST(OptionalMatrix, ConstructionAndAssignment)
{
    // 默认构造
    hj::optional<int> o1;
    ASSERT_FALSE(o1.has_value());

    // nullopt 构造
    hj::optional<int> o2(hj::nullopt);
    ASSERT_FALSE(o2.has_value());

    // 值构造
    hj::optional<int> o3(42);
    ASSERT_TRUE(o3.has_value());
    ASSERT_EQ(*o3, 42);

    // T -> optional 赋值与 nullopt 赋值
    hj::optional<int> o4;
    o4 = 100;
    ASSERT_TRUE(o4.has_value());
    ASSERT_EQ(o4.value(), 100);

    o4 = hj::nullopt;
    ASSERT_FALSE(o4.has_value());
}

TEST(OptionalMatrix, StateTransition)
{
    hj::optional<int> o;
    ASSERT_FALSE(o);

    // empty -> engaged
    o.emplace(10);
    ASSERT_TRUE(o);
    ASSERT_EQ(*o, 10);

    // engaged -> engaged (再次 emplace 覆盖)
    o.emplace(20);
    ASSERT_TRUE(o);
    ASSERT_EQ(*o, 20);

    // engaged -> empty (reset)
    o.reset();
    ASSERT_FALSE(o);
}

TEST(OptionalMatrix, AccessAndOperators)
{
    hj::optional<std::string> o("hello");
    ASSERT_TRUE(o.has_value());

    // operator* 与 operator->
    EXPECT_EQ(*o, "hello");
    EXPECT_EQ(o->size(), 5);

    // Const 重载测试
    const hj::optional<std::string> co = o;
    EXPECT_EQ(*co, "hello");
    EXPECT_EQ(co->size(), 5);
    EXPECT_EQ(co.value(), "hello");
}

TEST(OptionalMatrix, MoveOnlySupport)
{
    // 验证 move-only 类型 (std::unique_ptr) 的所有权流转
    hj::optional<std::unique_ptr<int>> opt_ptr;
    opt_ptr.emplace(std::make_unique<int>(42));

    ASSERT_TRUE(opt_ptr.has_value());
    ASSERT_EQ(**opt_ptr, 42);

    // 移动构造
    hj::optional<std::unique_ptr<int>> moved_opt = std::move(opt_ptr);
    ASSERT_FALSE(opt_ptr.has_value()); // 原对象应为空
    ASSERT_TRUE(moved_opt.has_value());
    ASSERT_EQ(**moved_opt, 42);

    // value_or 与 move-only 组合
    auto def_val = std::make_unique<int>(99);
    auto res     = std::move(moved_opt).value_or(std::move(def_val));
    ASSERT_EQ(*res, 42);
}

TEST(OptionalMatrix, LifecycleTracker)
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

        // 重复 emplace 应先析构旧对象
        opt.emplace(10);
        EXPECT_EQ(Tracker::constructed, 2);
        EXPECT_EQ(Tracker::destroyed, 1);

        // reset 触发析构
        opt.reset();
        EXPECT_FALSE(opt.has_value());
        EXPECT_EQ(Tracker::destroyed, 2);
    }
    EXPECT_EQ(Tracker::constructed, Tracker::destroyed);
}

TEST(OptionalMatrix, SwapTest)
{
    hj::optional<int> a(1);
    hj::optional<int> b(2);

    a.swap(b);
    EXPECT_EQ(a.value(), 2);
    EXPECT_EQ(b.value(), 1);

    // 状态不对称的 swap (engaged <-> empty)
    hj::optional<int> empty_opt;
    a.swap(empty_opt);
    EXPECT_FALSE(a.has_value());
    EXPECT_TRUE(empty_opt.has_value());
    EXPECT_EQ(empty_opt.value(), 2);
}

TEST(OptionalMatrix, ExceptionSafety)
{
    hj::optional<int> o;
#if (__cplusplus >= 201703L) || (defined(_MSC_VER) && _MSC_VER >= 1910)
    EXPECT_THROW(o.value(), std::bad_optional_access);
#else
    EXPECT_THROW(o.value(), boost::bad_optional_access);
#endif
}