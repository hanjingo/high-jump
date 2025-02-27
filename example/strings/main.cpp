#include <iostream>

#include <libcpp/util/string_util.hpp>
#include <libcpp/types/string_view.hpp>

int main(int argc, char* argv[])
{
    std::cout << "split(\"abc;123;++\", \";\") >> " << std::endl;
    auto arr1 = libcpp::string::split("abc;123;++", ";");
    for (auto str : arr1) {
        std::cout << str << std::endl;
    }

    std::cout << "equal(\"hello\", \"hello\") = " << libcpp::string::equal("hello", "hello") << std::endl;

    std::cout << "from_wchar(to_wchar(std::string(\"abc\"))) =" <<
        libcpp::string::from_wchar(libcpp::string::to_wchar(std::string("abc"))) << std::endl;

    std::cout << "from_wstring(to_wstring(std::string(\"hello\"))) = "
              << libcpp::string::from_wstring(libcpp::string::to_wstring(std::string("hello"))) << std::endl;

    std::cout << "string_view(\"world\") = " << libcpp::string_view("world") << std::endl;

    std::cin.get();
    return 0;
}