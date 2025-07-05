#include <iostream>

#include <libcpp/types/reflect.hpp>

namespace abc
{
struct hello {
    int age;
};
}

void fhello(int num) {};

int main(int argc, char* argv[])
{
    std::cout << "reflect::type_name(int(1)) = " << libcpp::reflect::type_name(int(1)) << std::endl;
    std::cout << "reflect::type_name(abc::hello()) = " << libcpp::reflect::type_name(abc::hello()) << std::endl;
    std::cout << "reflect::type_name(fhello) = " << libcpp::reflect::type_name(fhello) << std::endl;

    std::cout << "reflect::is_pod(1) = " << libcpp::reflect::is_pod(int(1)) << std::endl;
    std::cout << "reflect::is_pod(abc::hello()) = " << libcpp::reflect::is_pod(abc::hello()) << std::endl;

    std::cout << "reflect::is_same_type(int(1), int(2)) = "
              << libcpp::reflect::is_same_type(int(1), int(2)) << std::endl;
    std::cout << "reflect::is_same_type(int(1), int(2), int(3)) = "
              << libcpp::reflect::is_same_type(int(1), int(2), int(3)) << std::endl;
    std::cout << "reflect::is_same_type(int(1), float(2)) = "
              << libcpp::reflect::is_same_type(int(1), float(2)) << std::endl;
    std::cout << "reflect::is_same_type(int(1), int(2), float(3)) = "
              << libcpp::reflect::is_same_type(int(1), int(2), float(3)) << std::endl;

    std::cin.get();
    return 0;
}