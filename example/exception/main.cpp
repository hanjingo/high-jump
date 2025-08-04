#include <iostream>
#include <list>
#include <vector>

#include <libcpp/testing/exception.hpp>

int main(int argc, char* argv[])
{
    try
    {
        libcpp::throw_if_null(nullptr);
    }
    catch (const char* e)
    {
        std::cout << e << std::endl;
    }

    try
    {
        int a = 0;
        libcpp::throw_if_not_null(&a);
    }
    catch (const char* e)
    {
        std::cout << e << std::endl;
    }

    try
    {
        std::list<int> li{ 1, 2, 3 };
        libcpp::throw_if_exists(li, 2);
    }
    catch (const char* e)
    {
        std::cout << e << std::endl;
    }

    try
    {
        std::vector<int> arr{ 1, 2, 3 };
        libcpp::throw_if_not_exists(arr, 0);
    }
    catch (const char* e)
    {
        std::cout << e << std::endl;
    }

    std::cin.get();
    return 0;
}