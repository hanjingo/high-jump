#include <iostream>
#include <fstream>
#include <string>

#include <libcpp/encoding/ini.hpp>

int main(int argc, char* argv[])
{
    std::string text = "[section1] \na=123 \nb=abc\n";
    libcpp::ini ini = libcpp::ini::parse(text);

    // std::cout << "ini.get_child(\"section1\").get<int>(\"a\") = " 
    //           << ini.get_child("section1").get<int>("a") << std::endl;

    // std::cout << "ini.get_child(\"section1\").[\"a\"] = " 
    //           << ini.get_child("section1")["a"].to_string() << std::endl;

    // std::cout << "js[\"id\"] = " << js["id"] << std::endl;
    // std::cout << "js = " << js << std::endl;

    // ini.get_child("section1").put<std::string>("c", "def");
    // std::cout << ini.get_child("section1").get<std::string>("c") << std::endl;

    // js["name"] = "hello";
    // std::cout << "js[\"name\"] = " << js["name"]<< std::endl;
    // std::cout << "js = " << js << std::endl;

    // js.erase("name");
    // std::cout << "js = " << js << std::endl;

    // auto arr1 = libcpp::json::array();
    // arr1.push_back(1);
    // arr1.push_back(2);
    // arr1.push_back(3);
    // std::cout << "arr1 = " << arr1 << std::endl;

    // auto arr2 = libcpp::json::array({4, 5, 6});
    // std::cout << "arr2 = " << arr2 << std::endl;

    // std::cout << "range array>> " << std::endl;
    // for (auto& element : arr1) {
    //     std::cout << element << '\n';
    // }

    // std::cout << "range js>> " << std::endl;
    // for (auto& element : js.items()) {
    //     std::cout << element.key() << " : " << element.value() << "\n";
    // }

    // std::cout << "range js by iterator>>" << std::endl;
    // for (auto itr = js.begin(); itr != js.end(); ++itr) {
    //     std::cout << itr.key() << " : " << itr.value() << "\n";
    // }

    // std::cout << "write json file>>" << std::endl;
    // std::ofstream out("./007.json");
    // out << std::setw(4) << js << std::endl;

    // std::cout << "write json file>>" << std::endl;
    // std::ifstream in("./007.json");
    // libcpp::json js1;
    // in >> js1;
    // std::cout << "read json file js1=" << js1 << std::endl;

    std::cin.get();
    return 0;
}