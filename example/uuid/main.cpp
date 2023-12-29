#include <iostream>

#include <libcpp/util/uuid.hpp>

static bool little_endian = false;

int main(int argc, char* argv[])
{
    std::cout << "uuid::gen() = " << libcpp::uuid::gen() << std::endl;

    std::cout << "uuid::gen_u128() = " << libcpp::uuid::gen_u128() << std::endl;

    std::cout << "uuid::gen_u128(little_endian) = " << libcpp::uuid::gen_u128(little_endian) << std::endl;

    std::cin.get();
    return 0;
}