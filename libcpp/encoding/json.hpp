#ifndef JSON_HPP
#define JSON_HPP

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace libcpp
{

using json_document         = rapidjson::Document;
using json_value            = rapidjson::Value;
using json_type             = rapidjson::Type;

using json_string_buffer    = rapidjson::StringBuffer;

template <typename...T>
using json_writer           = rapidjson::Writer<T...>;
using json_string_writer    = json_writer<json_string_buffer>;

template <typename...T>
using json_memory_allocator = rapidjson::MemoryPoolAllocator<T...>;

class json
{
public:
    static std::string to_string(const libcpp::json_document& doc)
    {
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> w{buf};
        doc.Accept(w);
        return buf.GetString();
    }

    static std::string to_string(const libcpp::json_value& value)
    {
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> w{buf};
        value.Accept(w);
        return buf.GetString();
    }

    static json_value make_object()
    {
        return rapidjson::Value(rapidjson::Type::kObjectType);
    }

    static json_value make_array()
    {
        return rapidjson::Value(rapidjson::Type::kArrayType);
    }

    template <typename T, typename ArrT>
    static json_value make_array(T& allocator, const std::initializer_list<ArrT> args)
    {
        rapidjson::Value arr(rapidjson::Type::kArrayType);
        for (auto itr = args.begin(); itr != args.end(); itr++) {
            arr.PushBack(*itr, allocator);
        }

        return arr;
    }

    template <typename ArrT>
    static void make_array(json_document& doc, const std::string& key, const std::initializer_list<ArrT> args)
    {
        rapidjson::Value arr(rapidjson::Type::kArrayType);
        for (auto itr = args.begin(); itr != args.end(); itr++) {
            arr.PushBack(*itr, doc.GetAllocator());
        }

        doc.AddMember(rapidjson::Value(rapidjson::StringRef(key.c_str())), arr, doc.GetAllocator());
    }
};

}

#endif