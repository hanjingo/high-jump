#include <iostream>

#include <libcpp/types/optional.hpp>

libcpp::optional<int> hello(int in)
{
    return (in % 2 == 0) ? in : libcpp::optional<int>();
}

int main(int argc, char* argv[])
{
    std::cout << "hello(2).get() = " << hello(2).get() << std::endl;

    std::cout << "hello(3).get_value_or(123) = " << hello(3).get_value_or(123)
              << std::endl;
    std::cout << "hello(2).get_value_or(123) = " << hello(2).get_value_or(123)
              << std::endl;

    std::cin.get();
    return 0;
}