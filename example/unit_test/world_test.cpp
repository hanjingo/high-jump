#include <libcpp/testing/unit_test.hpp>

#include <example/unit_test/world.hpp>

TEST(test_world, Name)
{
    libcpp::World world{};
    ASSERT_EQ(world.Name(), "World");
}