#include <iostream>

#include <libcpp/crypto/base64.hpp>

int main(int argc, char* argv[])
{
    std::string str_dst;
    std::cout << "libcpp::base64::encode(std::string(\"https://github.com/hanjingo/libcpp\"): "
              << libcpp::base64::encode("https://github.com/hanjingo/libcpp", str_dst) << std::endl;
    std::cout << "str_dst: " << str_dst << std::endl;

    std::cin.get();
    return 0;
}