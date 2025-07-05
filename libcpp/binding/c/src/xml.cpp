// #include <libcpp/encoding/xml.hpp>
#include <cstring>

extern "C" {
#include "xml.h"
}

// LIBCPP_API bool libcpp_xml_load_file(char* buf, int& len, const char* file_path)
// {
//     libcpp::xml_document doc;
//     return doc.load_file(file_path) && memcpy(buf, doc.value(), len);
// }