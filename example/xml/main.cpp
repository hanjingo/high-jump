#include <iostream>

#include <libcpp/encoding/xml.hpp>

int main(int argc, char* argv[])
{
    libcpp::xml_document doc;
    doc.root().append_child("man").append_attribute("name").set_value("he");
    doc.child("man").append_attribute("age").set_value(30);
    doc.save_file("./007.xml");

    if (!doc.load_file("./007.xml")) {
        std::cout << "load xml fail" << std::endl;
        return 0;
    }
    std::cout << "load file: ./007.xml success" << std::endl;
    auto childs = doc.children();
    for (libcpp::xml_node_iterator itr = childs.begin(); itr != childs.end(); itr++) {
        std::cout << "find child>> " << itr->name() << std::endl;
        if (itr->name() == std::string("man")) {
            std::cout << "name: " << itr->attribute("name").as_string() << std::endl;
            std::cout << "age: " << itr->attribute("age").as_int() << std::endl;
        }
    }
    auto attr = doc.child("man").find_attribute([](const libcpp::xml_attribute & arg) -> bool{
        return arg.name() == std::string("name");
    });
    std::cout << "find name:" << attr.as_string() << std::endl;

    std::cin.get();
    return 0;
}