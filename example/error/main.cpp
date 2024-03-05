#include <iostream>

#include <libcpp/testing/error.hpp>

using err_t = libcpp::error<int>;

static const err_t ok{0};
static const err_t err1{1};

err_t fn(bool bok)
{
    return bok ? ok : err1;
}

int main(int argc, char* argv[])
{
    std::cout << "error<int>(1) = " << libcpp::error<int>(1) << std::endl;
    std::cout << std::endl;

    std::cout << "error_factory::create(1) = " 
        << libcpp::error_factory::create(1) << std::endl;
    std::cout << "error_factory::create(\"hello\") = " 
        << libcpp::error_factory::create("hello") << std::endl;
    std::cout << std::endl;

    std::cout << "ERROR(\"world\") = " << ERROR("world") << std::endl;
    std::cout << "(ERROR(\"world\") == ERROR(\"world\")) = " 
        << (ERROR("world") == ERROR("world")) << std::endl;
    std::cout << std::endl;

    // example: error function return
    auto err = fn(true);
    if (err != ok)
    {
        std::cout << "err1 = " << err << std::endl;
    } 
    else
    {
        std::cout << "ok = " << err << std::endl;
    }

    std::cin.get();
    return 0;
}