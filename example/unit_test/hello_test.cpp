#include <example/unit_test/hello.hpp>
#include <libcpp/testing/unit_test.hpp>

TEST(test_hello, Name)
{
    libcpp::Hello hello{};
    ASSERT_EQ(hello.Name(), "Hello");
}