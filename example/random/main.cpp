#include <iostream>

#include <libcpp/math/random.hpp>

int main(int argc, char* argv[])
{
    std::cout << "random::range<1, 5>() = " << libcpp::random::range<1, 5>() << std::endl;

    std::cout << "random::range<int>(1, 5) = " << libcpp::random::range<int>(1, 5) << std::endl;

    std::cin.get();
    return 0;
}