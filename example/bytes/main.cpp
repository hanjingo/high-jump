#include <iostream>

#include <libcpp/encoding/bytes.hpp>

using namespace libcpp;

bool little_endian = false;

int main(int argc, char* argv[])
{
    std::cout << std::endl;
    unsigned char arr1[1];
    std::cout << "bool_to_bytes(true, arr1)[0] = " << int(bool_to_bytes(true, arr1)[0]) << std::endl;
    std::cout << "bytes_to_bool(arr1) = " << bytes_to_bool(arr1) << std::endl;

    std::cout << std::endl;
    unsigned char arr2[4] = {};
    std::cout << "int_to_bytes(123456789, arr2)[0] = " << int(int_to_bytes(123456789, arr2)[0]) << std::endl;
    std::cout << "bytes_to_int(arr2) = " << bytes_to_int(arr2) << std::endl;

    memset(arr2, 0, 4);
    std::cout << "int_to_bytes(123456789, arr2, little_endian)[0] = " << int(int_to_bytes(123456789, arr2, little_endian)[0]) << std::endl;
    std::cout << "bytes_to_int(arr2, false) = " << bytes_to_int(arr2, little_endian) << std::endl;

    std::cout << std::endl;
    unsigned char arr3[4] = {};
    std::cout << "long_to_bytes(123, arr3)[0] = " << int(long_to_bytes(123, arr3)[0]) << std::endl;
    std::cout << "bytes_to_long(arr3) = " << bytes_to_long(arr3) << std::endl;

    memset(arr3, 0, 4);
    std::cout << "long_to_bytes(123, arr3, little_endian)[0] = " << int(long_to_bytes(123, arr3, little_endian)[0]) << std::endl;
    std::cout << "bytes_to_long(arr3, little_endian) = " << bytes_to_long(arr3, little_endian) << std::endl;

    std::cout << std::endl;
    unsigned char arr4[8] = {};
    std::cout << "long_long_to_bytes(123, arr4)[0] = " << int(long_long_to_bytes(123, arr4)[0]) << std::endl;
    std::cout << "bytes_to_long_long(arr4) = " << bytes_to_long_long(arr4) << std::endl;

    memset(arr4, 0, 8);
    std::cout << "long_long_to_bytes(123, arr4, little_endian)[0] = " << int(long_long_to_bytes(123, arr4, little_endian)[0]) << std::endl;
    std::cout << "bytes_to_long_long(arr4, little_endian) = " << bytes_to_long_long(arr4, little_endian) << std::endl;

    std::cout << std::endl;
    unsigned char arr5[2] = {};
    std::cout << "short_to_bytes(123, arr5)[0] = " << int(short_to_bytes(123, arr5)[0]) << std::endl;
    std::cout << "bytes_to_short(arr5) = " << bytes_to_short(arr5) << std::endl;

    memset(arr5, 0, 2);
    std::cout << "short_to_bytes(123, arr5, little_endian) = " << int(short_to_bytes(123, arr5, little_endian)[0]) << std::endl;
    std::cout << "bytes_to_short(arr5, little_endian) = " << bytes_to_short(arr5, little_endian) << std::endl;

    std::cout << std::endl;
    unsigned char arr6[8] = {};
    std::cout << "double_to_bytes(123.456, arr6)[0] = " << int(double_to_bytes(123.456, arr6)[0]) << std::endl;
    std::cout << "bytes_to_double(arr6) = " << bytes_to_double(arr6) << std::endl;

    std::cout << std::endl;
    unsigned char arr7[4] = {};
    std::cout << "float_to_bytes(123.456, arr7)[0] = " << int(float_to_bytes(123.456, arr7)[0]) << std::endl;
    std::cout << "bytes_to_float(arr7) = " << bytes_to_float(arr7) << std::endl;

    std::cout << std::endl;
    std::string str = "hello";
    unsigned char arr8[6] = {};
    std::cout << "string_to_bytes(std::string(\"hello\"), arr8)[0] = " << int(string_to_bytes(std::string("hello"), arr8)[0]) << std::endl;
    std::cout << "bytes_to_string(arr8) = " << bytes_to_string(arr8, 6) << std::endl;
    std::string str1;
    std::cout << "bytes_to_string(arr8, 6, str1) = " << bytes_to_string(arr8, 6, str1) << std::endl;

    std::cin.get();
    return 0;
}