#include <iostream>

#include <libcpp/crypto/md5.hpp>

int main(int argc, char* argv[])
{
    std::cout << "md5::calc(\"hehehunanchina@live.com\", true):"
              << libcpp::md5::calc("hehehunanchina@live.com") << std::endl;

    std::cout << "md5::calc(\"hehehunanchina@live.com\"):"
              << libcpp::md5::calc("hehehunanchina@live.com", true) << std::endl;

    std::cin.get();
    return 0;
}