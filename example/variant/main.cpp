#include <string>
#include <iostream>

#include <libcpp/types/variant.hpp>

int main(int argc, char* argv[])
{
    libcpp::variant<double, char, std::string> v;
    v = 3.14;
    std::cout << "v = 3.14;" << std::endl;
    std::cout << "libcpp::get<double>(v) = " << libcpp::get<double>(v) << std::endl << std::endl;

    v = 'A';
    std::cout << "v = 'A';" << std::endl;
    std::cout << "libcpp::get<char>(v) = " << libcpp::get<char>(v) << std::endl << std::endl;

    v = "Boost";
    std::cout << "v = \"Boost\";" << std::endl;
    std::cout << "libcpp::get<std::string>(v) = " << libcpp::get<std::string>(v) << std::endl;

    std::cin.get();
    return 0;
}