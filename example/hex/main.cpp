#include <iostream>

#include <libcpp/encoding/hex.hpp>

int main(int argc, char* argv[])
{
    const char buf[] = { 'F', 'F' };

    std::cout << "libcpp::hex::from_str<int>(buf) = "
              << libcpp::hex::from_str<int>(buf) << std::endl;

    std::cout << "libcpp::hex::from_str<int>(\"FF\") = "
              << libcpp::hex::from_str<int>("FF") << std::endl;

    std::cout << "libcpp::hex::to_str(255) = " << libcpp::hex::to_str(255)
              << std::endl;

    std::cout << "libcpp::hex::to_str(buf)[0] = " << libcpp::hex::to_str(buf)[0]
              << std::endl;

    std::cin.get();
    return 0;
}