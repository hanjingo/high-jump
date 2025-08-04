#include <fstream>
#include <iostream>
#include <string>

#include <libcpp/encoding/ini.hpp>

int main(int argc, char* argv[])
{
    char text[] = "[section1] \na=123 \nb=abc\n";
    libcpp::ini ini = libcpp::ini::parse(text);
    std::cout << "ini.get_child(\"section1\").get<int>(\"a\") = "
              << ini.get_child("section1").get<int>("a") << std::endl;
    std::cout << "ini.get_child(\"section1\").get<std::string>(\"b\") = "
              << ini.get_child("section1").get<std::string>("b") << std::endl;
    ini.get_child("section1").put<std::string>("c", "def");
    std::cout << "ini.get_child(\"section1\").get<std::string>(\"c\") = "
              << ini.get_child("section1").get<std::string>("c") << std::endl;

    libcpp::ini cfg;
    std::cout << "cfg.read_file(\"cfg.ini\")" << cfg.read_file("cfg.ini");
    std::cout << "cfg.get_child(\"person\").get<int>(\"age\") = "
              << cfg.get_child("person").get<int>("age") << std::endl;

    cfg.get_child("person").put("next_age", 31);
    cfg.write_file("cfg.ini");

    std::cin.get();
    return 0;
}