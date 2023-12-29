#include <iostream>

#include <libcpp/encoding/bits.hpp>

int main(int argc, char* argv[])
{
    int n = 4;
    std::cout << "bits::get(n, 1) = " << libcpp::bits::get(4, 1) << std::endl;
    std::cout << "bits::get(n, 2) = " << libcpp::bits::get(4, 2) << std::endl;
    std::cout << "bits::get(n, 3) = " << libcpp::bits::get(4, 3) << std::endl;
    std::cout << "bits::get(n, 4) = " << libcpp::bits::get(4, 4) << std::endl;

    std::cout << std::endl;
    std::cout << "bits::put(n, 1) = " << libcpp::bits::put(n, 1) << std::endl;
    std::cout << "bits::put(n, 2) = " << libcpp::bits::put(n, 2) << std::endl;
    std::cout << "bits::put(n, 3, 0) = " << libcpp::bits::put(n, 3, 0) << std::endl;
    std::cout << "bits::get(n, 1) = " << libcpp::bits::get(n, 1) << std::endl;
    std::cout << "bits::get(n, 2) = " << libcpp::bits::get(n, 2) << std::endl;
    std::cout << "bits::get(n, 3) = " << libcpp::bits::get(n, 3) << std::endl;
    std::cout << "bits::get(n, 4) = " << libcpp::bits::get(n, 4) << std::endl;

    std::cout << std::endl;
    std::cout << "bits::reset(n, 1) = " << libcpp::bits::reset(n, 1) << std::endl;
    std::cout << "bits::get(n, 1) = " << libcpp::bits::get(n, 1) << std::endl;
    std::cout << "bits::get(n, 2) = " << libcpp::bits::get(n, 2) << std::endl;
    std::cout << "bits::get(n, 3) = " << libcpp::bits::get(n, 3) << std::endl;
    std::cout << "bits::get(n, 4) = " << libcpp::bits::get(n, 4) << std::endl;

    std::cout << std::endl;
    std::cout << "bits::flip(n) = " << libcpp::bits::flip(n) << std::endl;
    std::cout << "bits::get(n, 1) = " << libcpp::bits::get(n, 1) << std::endl;
    std::cout << "bits::get(n, 2) = " << libcpp::bits::get(n, 2) << std::endl;
    std::cout << "bits::get(n, 3) = " << libcpp::bits::get(n, 3) << std::endl;
    std::cout << "bits::get(n, 4) = " << libcpp::bits::get(n, 4) << std::endl;

    std::cout << std::endl;
    std::cout << "bits::to_string(bool(true)) = " << libcpp::bits::to_string(bool(true)) << std::endl;
    std::cout << "bits::to_string(int(4)) = " << libcpp::bits::to_string(int(4)) << std::endl;
    std::cout << "bits::to_string(uint32_t(0xFF)) = " << libcpp::bits::to_string(uint32_t(0xFF)) << std::endl;
    std::cout << "bits::to_string(uint32_t(0xFFFFFFFF)) = " << libcpp::bits::to_string(uint32_t(0xFFFFFFFF)) << std::endl;

    std::cin.get();
    return 0;
}