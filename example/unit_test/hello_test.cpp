#include <libcpp/testing/unit_test.hpp>

#include <example/unit_test/hello.hpp>

TEST(test_hello, Name)
{
    libcpp::Hello hello{};
    ASSERT_EQ(hello.Name(), "Hello");
}