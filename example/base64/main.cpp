#include <iostream>

#include <libcpp/encoding/base64.hpp>

int main(int argc, char* argv[])
{
    std::cout << "encode(\"https://github.com/hanjingo/libcpp\"): "
              << libcpp::base64::encode("https://github.com/hanjingo/libcpp") << std::endl;

    std::cout << "decode(\"aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==\"): "
              << libcpp::base64::decode("aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==") << std::endl;

    std::cin.get();
    return 0;
}