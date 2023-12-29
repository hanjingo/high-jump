#include <iostream>

#include <libcpp/types/any.hpp>

int main(int argc, char* argv[])
{
    libcpp::any n = 1;
    std::cout << "any_cast<int>(1) = " << libcpp::any_cast<int>(n) << std::endl;

    std::cin.get();
    return 0;
}