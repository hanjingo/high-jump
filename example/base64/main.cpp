#include <iostream>

#include <libcpp/crypto/base64.hpp>

int main(int argc, char* argv[])
{
    // base64 encode
    std::string str_encode_dst;
    std::cout << "libcpp::base64::encode(std::string(\"https://github.com/hanjingo/libcpp\")="
              << libcpp::base64::encode("https://github.com/hanjingo/libcpp", str_encode_dst) << std::endl;
    std::cout << "str_encode_dst=" << str_encode_dst << std::endl;

    // base64 decode
    std::string str_decode_dst;
    std::cout << "libcpp::base64::decode(std::string(\"aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==\"), str_decode_dst)="
              << libcpp::base64::decode(std::string("aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA=="), str_decode_dst)
              << std::endl;
    std::cout << "str_decode_dst=" << str_decode_dst << std::endl;

    // base64 encode file
    std::cout << "libcpp::base64::encode_file(std::string(\"./base64_file_test.log\"), std::string(\"./base64_file_test_encode.log\"))="
              << libcpp::base64::encode_file(std::string("./base64_file_test.log"), std::string("./base64_file_test_encode.log"))
              << std::endl;

    // base64 decode file
    std::cout << "libcpp::base64::decode_file(std::string(\"./base64_file_test_encode.log\"), std::string(\"./base64_file_test_decode.log\")="
              << libcpp::base64::decode_file(std::string("./base64_file_test_encode.log"), std::string("./base64_file_test_decode.log"))
              << std::endl;

    std::cin.get();
    return 0;
}