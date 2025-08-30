#include <gtest/gtest.h>
#include <libcpp/util/license.hpp>

TEST(license, issue)
{
    std::string prikey = R"(-----BEGIN RSA PRIVATE KEY-----
MIIBOgIBAAJAevxSYQggOUn0bfka93jW0E2wkakW9gxE8h/RQTcsk06WYYQJB6CZ
cjLeXwVsoIC5T8Ph3lvRIGmVCEQQ0peBowIDAQABAkBSGbtMt0X7uJkKCS+tYOfW
auaidoSzgIFOOVtR8+k39GEB1yywvTrAspRQ0UB3tV2B9DeKUVlKm51l3UWCXJyx
AiEAzcAxj1Eg/F0EJAqgYj6IRUgJSZgwrKozohqH8e9LV38CIQCZBYtcI+iyT+lP
8UklJzkguLZ6drrtD4lI8pxr2ReH3QIhALUKQhF7L20fY39bIliP8VQU2Kc7FMk5
Ugl3Etuc1Ux9AiA+OYz0CP4lFG3RvlJ6Mzr93V8G6aUVpU20RkPpbzwsWQIhAKXc
afQ0WRgZier6MrdwlXd70JZIpgc6kLeOz2GuV4lX
-----END RSA PRIVATE KEY-----)";

    libcpp::license::issuer isu{"issuer1", libcpp::license::sign_algo::rsa256, {"", prikey, "", ""}, 1};
    libcpp::license::token_t token;
    auto err = isu.issue(
        token,
        "harry", 
        30, 
        {
            {"disk_sn", libcpp::license::get_disk_sn()}, 
        }
    );
    ASSERT_TRUE(err.value() == 0);
}

TEST(license, verify)
{
        std::string prikey = R"(-----BEGIN RSA PRIVATE KEY-----
MIIBOgIBAAJAevxSYQggOUn0bfka93jW0E2wkakW9gxE8h/RQTcsk06WYYQJB6CZ
cjLeXwVsoIC5T8Ph3lvRIGmVCEQQ0peBowIDAQABAkBSGbtMt0X7uJkKCS+tYOfW
auaidoSzgIFOOVtR8+k39GEB1yywvTrAspRQ0UB3tV2B9DeKUVlKm51l3UWCXJyx
AiEAzcAxj1Eg/F0EJAqgYj6IRUgJSZgwrKozohqH8e9LV38CIQCZBYtcI+iyT+lP
8UklJzkguLZ6drrtD4lI8pxr2ReH3QIhALUKQhF7L20fY39bIliP8VQU2Kc7FMk5
Ugl3Etuc1Ux9AiA+OYz0CP4lFG3RvlJ6Mzr93V8G6aUVpU20RkPpbzwsWQIhAKXc
afQ0WRgZier6MrdwlXd70JZIpgc6kLeOz2GuV4lX
-----END RSA PRIVATE KEY-----)";

    std::string pubkey = R"(-----BEGIN PUBLIC KEY-----
MFswDQYJKoZIhvcNAQEBBQADSgAwRwJAevxSYQggOUn0bfka93jW0E2wkakW9gxE
8h/RQTcsk06WYYQJB6CZcjLeXwVsoIC5T8Ph3lvRIGmVCEQQ0peBowIDAQAB
-----END PUBLIC KEY-----)";

    libcpp::license::issuer isu{"issuer1", libcpp::license::sign_algo::rsa256, {pubkey, prikey, "", ""}, 1};
    libcpp::license::token_t token;
    auto err = isu.issue(
        token,
        "harry", 
        30,
        {
            {"disk_sn", libcpp::license::get_disk_sn()}
        }
    );
    ASSERT_TRUE(err.value() == 0);

    err = isu.verify(
        token,
        "harry",
        30,
        {
            {"disk_sn", libcpp::license::get_disk_sn()}, 
        }
    );
    ASSERT_TRUE(err.value() == 0);

    err = isu.verify(
        token,
        "harry1",
        30,
        {
            {"disk_sn", libcpp::license::get_disk_sn()}, 
        }
    );
    ASSERT_FALSE(err.value() == 0);
}

TEST(license, release)
{
        std::string prikey = R"(-----BEGIN RSA PRIVATE KEY-----
MIIBOgIBAAJAevxSYQggOUn0bfka93jW0E2wkakW9gxE8h/RQTcsk06WYYQJB6CZ
cjLeXwVsoIC5T8Ph3lvRIGmVCEQQ0peBowIDAQABAkBSGbtMt0X7uJkKCS+tYOfW
auaidoSzgIFOOVtR8+k39GEB1yywvTrAspRQ0UB3tV2B9DeKUVlKm51l3UWCXJyx
AiEAzcAxj1Eg/F0EJAqgYj6IRUgJSZgwrKozohqH8e9LV38CIQCZBYtcI+iyT+lP
8UklJzkguLZ6drrtD4lI8pxr2ReH3QIhALUKQhF7L20fY39bIliP8VQU2Kc7FMk5
Ugl3Etuc1Ux9AiA+OYz0CP4lFG3RvlJ6Mzr93V8G6aUVpU20RkPpbzwsWQIhAKXc
afQ0WRgZier6MrdwlXd70JZIpgc6kLeOz2GuV4lX
-----END RSA PRIVATE KEY-----)";

    std::string pubkey = R"(-----BEGIN PUBLIC KEY-----
MFswDQYJKoZIhvcNAQEBBQADSgAwRwJAevxSYQggOUn0bfka93jW0E2wkakW9gxE
8h/RQTcsk06WYYQJB6CZcjLeXwVsoIC5T8Ph3lvRIGmVCEQQ0peBowIDAQAB
-----END PUBLIC KEY-----)";

    libcpp::license::issuer isu{"issuer1", libcpp::license::sign_algo::rsa256, {pubkey, prikey, "", ""}, 1};
    libcpp::license::token_t token;
    auto err = isu.issue(
        token,
        "harry", 
        30,
        {
            {"disk_sn", libcpp::license::get_disk_sn()}
        }
    );
    ASSERT_TRUE(err.value() == 0);

    err = isu.verify(
        token,
        "harry",
        30,
        {
            {"disk_sn", libcpp::license::get_disk_sn()}, 
        }
    );
    ASSERT_TRUE(err.value() == 0);

    isu.release(libcpp::license::sign_algo::none, {});
    err = isu.verify(
        token,
        "harry",
        30,
        {
            {"disk_sn", libcpp::license::get_disk_sn()}, 
        }
    );
    ASSERT_FALSE(err.value() == 0);
}