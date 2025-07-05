#include <iostream>

#include <libcpp/util/uuid.hpp>

static bool little_endian = false;

int main(int argc, char* argv[])
{
    std::cout << "uuid::gen() = " << libcpp::uuid::gen() << std::endl;
    std::cin.get();
    return 0;
}