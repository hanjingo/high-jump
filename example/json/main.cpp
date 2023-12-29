#include <iostream>

#include <libcpp/encoding/json.hpp>

int main(int argc, char* argv[])
{
    libcpp::json_document doc;
    doc.Parse("{}");

    doc.AddMember("id", 1, doc.GetAllocator());
    std::cout << "doc[\"id\"] = " << doc["id"].GetInt() << std::endl;
    std::cout << "doc = " << libcpp::json::to_string(doc) << std::endl;

    doc.AddMember("name", "hello", doc.GetAllocator());
    std::cout << "doc[\"name\"] = " << doc["name"].GetString() << std::endl;
    std::cout << "doc = " << libcpp::json::to_string(doc) << std::endl;

    doc.RemoveMember("name");
    std::cout << "doc = " << libcpp::json::to_string(doc) << std::endl;

    auto obj = libcpp::json::make_object();
    obj.AddMember("math", 90, doc.GetAllocator());
    doc.AddMember("sub", obj, doc.GetAllocator());
    std::cout << "doc[\"sub\"][\"math\"].GetInt() = " << doc["sub"]["math"].GetInt() << std::endl;
    std::cout << "doc = " << libcpp::json::to_string(doc) << std::endl;

    auto arr1 = libcpp::json::make_array();
    arr1.PushBack(1, doc.GetAllocator());
    arr1.PushBack(2, doc.GetAllocator());
    arr1.PushBack(3, doc.GetAllocator());
    doc.AddMember("arr1", arr1, doc.GetAllocator());
    std::cout << "doc = " << libcpp::json::to_string(doc) << std::endl;

    auto arr2 = libcpp::json::make_array(doc.GetAllocator(), {4, 5, 6});
    doc.AddMember("arr2", arr2, doc.GetAllocator());
    std::cout << "doc = " << libcpp::json::to_string(doc) << std::endl;

    std::cout << "range array>> " << std::endl;
    auto arr = doc["arr1"].GetArray();
    for (auto i = 0; i < arr.Size(); i++) {
        std::cout << " " << arr[i].GetInt() << ", " << std::endl;
    }

    std::cout << "range doc>> " << std::endl;
    for (auto itr = doc.MemberBegin(); itr != doc.MemberEnd(); itr++) {
        std::cout << itr->name.GetString() << ":" << libcpp::json::to_string(itr->value) << std::endl;
    }

    std::cin.get();
    return 0;
}