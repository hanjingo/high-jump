#include <iostream>

#include <libcpp/os/env.hpp>

int main(int argc, char* argv[])
{
    std::cout << "ARCH = " << ARCH << std::endl;

    std::cout << "cpu_cores() = " << libcpp::cpu_cores() << std::endl;

    std::cout << "bind_cpu_core(0) = " << libcpp::bind_cpu_core(0) << std::endl;

    libcpp::setenv("Path", "123");
    std::cout << "setenv(\"Path\", \"123\") " << std::endl;

    std::cout << "getenv(\"Path\") = " << libcpp::getenv("Path") << std::endl;

    std::cin.get();
    return 0;
}