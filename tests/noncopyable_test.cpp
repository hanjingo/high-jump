#include <gtest/gtest.h>
#include <hj/types/noncopyable.hpp>

class MyNoncopyable : public hj::noncopyable
{
  public:
    MyNoncopyable() = default;
    int value       = 42;
};

TEST(noncopyable, forbid_copy_assign_runtime)
{
    MyNoncopyable a;
    ASSERT_EQ(a.value, 42);
    SUCCEED();
}

class MyNoncopyableMove
{
  public:
    MyNoncopyableMove() = default;
    DISABLE_COPY_MOVE(MyNoncopyableMove)
    int value = 99;
};

TEST(noncopyable, forbid_move_runtime)
{
    MyNoncopyableMove a;
    ASSERT_EQ(a.value, 99);
    SUCCEED();
}
