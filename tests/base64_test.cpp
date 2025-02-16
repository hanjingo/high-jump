#include <gtest/gtest.h>
#include <libcpp/encoding/base64.hpp>

TEST(base64, encode)
{
    ASSERT_STREQ(
        libcpp::base64::encode("https://github.com/hanjingo/libcpp").c_str(), 
        "aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA=="
    );
}

TEST(base64, decode)
{
    ASSERT_STREQ(
        libcpp::base64::decode("aHR0cHM6Ly9naXRodWIuY29tL2hhbmppbmdvL2xpYmNwcA==").c_str(), 
        "https://github.com/hanjingo/libcpp"
    );
}