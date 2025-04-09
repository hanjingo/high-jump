#include <gtest/gtest.h>
#include <libcpp/util/uuid.hpp>

TEST(uuid, gen)
{
    std::string uuid = libcpp::uuid::gen();
    ASSERT_EQ(uuid.size() == 36, true);
}

TEST(uuid, gen_u64)
{
    unsigned long long uuid = libcpp::uuid::gen_u64();
    ASSERT_EQ(uuid != 0, true);
    ASSERT_EQ(uuid != libcpp::uuid::gen_u64(), true);
    ASSERT_EQ(uuid != libcpp::uuid::gen_u64(false), true);
    ASSERT_EQ(uuid != libcpp::uuid::gen_u64(true), true);
}