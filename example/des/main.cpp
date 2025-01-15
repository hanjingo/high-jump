#include <iostream>
#include <string>
#include <libcpp/crypto/des.hpp>

int main(int argc, char* argv[])
{
    std::cout << "des::ecb_decrypt(des::ecb_encrypt(\"1234\", \"1\"), \"1\") = " 
              << libcpp::des::ecb::decrypt(libcpp::des::ecb::encrypt("1234", "1"), "1") << std::endl;

    // libcpp::des::ecb codec;
    // std::cout << codec << "abcd1234";
    return 0;
}