#include <iostream>

#include <libcpp/os/application.hpp>

int main(int argc, char* argv[])
{
    libcpp::options opt;
    opt.add<std::string>("name", std::string("not found"));
    opt.add<int>("age", 100);
    opt.add<float>("balance", 10000.12);
    opt.add<std::string>("other", "nothing");

    // ./app --name=harry --age=30 --balance=123.456
    std::cout << "parse<std::string>(argc, argv, \"name\") = "
              << opt.parse<std::string>(argc, argv, "name")
              << std::endl;  // harry

    std::cout << "parse<int>(argc, argv, \"age\") = "
              << opt.parse<int>(argc, argv, "age") << std::endl;  // 30

    std::cout << "parse<float>(argc, argv, \"balance\") = "
              << opt.parse<float>(argc, argv, "balance")
              << std::endl;  // 123.456

    std::cout << "parse<float>(argc, argv, \"other\") = "
              << opt.parse<std::string>(argc, argv, "other")
              << std::endl;  // nothing

    std::cin.get();
    return 0;
}