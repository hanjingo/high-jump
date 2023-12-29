#include <iostream>

#include <libcpp/encoding/endian.hpp>

#include <boost/endian/conversion.hpp>

int main(int argc, char* argv[])
{
    std::cout << "is_big_endian ?= " << libcpp::is_big_endian << std::endl;

    int n = 0x01020304;

    std::cout << "big_endian::covert(int(0x01020304)) = 0x"
              << std::hex << libcpp::big_endian::covert(int(0x01020304)) << std::endl;
    std::cout << "boost::endian::native_to_big(int(0x01020304)) = 0x"
              << std::hex << boost::endian::native_to_big(int(0x01020304)) << std::endl;

    std::cout << "little_endian::covert(int(0x01020304)) = 0x"
              << std::hex << libcpp::little_endian::covert(int(0x01020304)) << std::endl;
    std::cout << "boost::endian::big_tonative(int(0x01020304)) = 0x"
              << std::hex << boost::endian::big_to_native(int(0x01020304)) << std::endl;

    std::cout << "be<<int(0x01020304) = 0x" << std::hex << (libcpp::be << int(0x01020304)) << std::endl;
    std::cout << "le<<int(0x01020304) = 0x" << (libcpp::le << int(0x01020304)) << std::endl;

    std::cout << "le<<int(0x0A01) = 0x" << (libcpp::le << int(0x0A01)) << std::endl;

    std::cin.get();
    return 0;
}