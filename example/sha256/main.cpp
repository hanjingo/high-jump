#include <iostream>

#include <libcpp/crypto/sha256.hpp>

int main(int argc, char* argv[])
{
    std::cout << libcpp::sha256::encode(std::string("12345678")) << std::endl;

    char buf[1024];
    std::size_t sz = 0;
    while (true)
    {
        std::cout << libcpp::sha256::encode(std::cin.getline(buf, sz))
                  << std::endl;
    }

    return 0;
}