#include <iostream>

#include <libcpp/os/env.hpp>

int main(int argc, char* argv[])
{
    libcpp::setenv("Path", "123");
    std::cout << "setenv(\"Path\", \"123\") " << std::endl;

    std::cout << "getenv(\"Path\") = " << libcpp::getenv("Path") << std::endl;

    std::cin.get();
    return 0;
}