#include <iostream>
#include <string>

#include <libcpp/crypto/des.hpp>

int main(int argc, char* argv[])
{
    // des::ecb encode
    std::string str_encode_dst;
    auto ret1 = libcpp::des::ecb::encode(std::string("hello world"), std::string("12345678"), str_encode_dst);
    std::cout << "ret1=" << ret1 << std::endl;

    // des::ecb decode
    std::string str_decode_dst;
    std::string str_encoded;
    std::string str_key("12345678");
    auto ret2 = libcpp::des::ecb::encode(std::string("hello world"), str_key, str_encoded);
    auto ret3 = libcpp::des::ecb::decode(str_encoded, str_key, str_decode_dst);
    std::cout << "ret2=" << ret2 << ", ret3=" << ret3 << ", str_decode_dst=" << str_decode_dst << std::endl;

    // des::ecb encode_file
    auto ret4 = libcpp::des::ecb::encode_file(
            std::string("./ecb_file_test.log"), 
            std::string("12345678"), 
            std::string("./ecb_file_test_encode.log"));
    std::cout << "ret4=" << ret4 << std::endl;

    // des::ecb decode_file
    auto ret5 = libcpp::des::ecb::decode_file(
            std::string("./ecb_file_test_encode.log"), 
            std::string("12345678"), 
            std::string("./ecb_file_test_decoded.log"));
    std::cout << "ret5=" << ret5 << std::endl;
    
    return 0;
}