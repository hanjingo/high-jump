#include <gtest/gtest.h>
#include <hj/algo/uuid.hpp>

TEST(uuid, gen)
{
    std::string uuid = hj::uuid::gen();
    ASSERT_EQ(uuid.size() == 36, true);
}

TEST(uuid, gen_u64)
{
    unsigned long long uuid = hj::uuid::gen_u64();
    ASSERT_EQ(uuid != 0, true);
    ASSERT_EQ(uuid != hj::uuid::gen_u64(), true);
    ASSERT_EQ(uuid != hj::uuid::gen_u64(false), true);
    ASSERT_EQ(uuid != hj::uuid::gen_u64(true), true);
}