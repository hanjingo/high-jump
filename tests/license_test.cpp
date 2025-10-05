#include <gtest/gtest.h>
#include <hj/util/license.hpp>
#include <map>
#include <chrono>

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

    hj::license::issuer  isu{"issuer1",
                            hj::license::sign_algo::rsa256,
                             {"", prikey, "", ""},
                            2};
    hj::license::token_t token;
    auto                 err = isu.issue(token,
                         "harry",
                         30,
                                         {
                             {"disk_sn", hj::license::get_disk_sn()},
                         });
    ASSERT_TRUE(err.value() == 0);

    err = isu.issue_file("./test1.lic",
                         "harry",
                         30,
                         {
                             {"disk_sn", hj::license::get_disk_sn()},
                         });
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

    hj::license::issuer  isu{"issuer1",
                            hj::license::sign_algo::rsa256,
                             {pubkey, prikey, "", ""},
                            2};
    hj::license::token_t token;
    auto                 err = isu.issue(token,
                         "harry",
                         30,
                                         {{"disk_sn", hj::license::get_disk_sn()}});
    ASSERT_TRUE(err.value() == 0);

    hj::license::verifier vefer{"issuer1",
                                hj::license::sign_algo::rsa256,
                                {pubkey, prikey, "", ""}};
    err = vefer.verify(token,
                       "harry",
                       30,
                       {
                           {"disk_sn", hj::license::get_disk_sn()},
                       });
    ASSERT_TRUE(err.value() == 0);

    err = vefer.verify(token,
                       "harry1",
                       30,
                       {
                           {"disk_sn", hj::license::get_disk_sn()},
                       });
    ASSERT_FALSE(err.value() == 0);

    err = isu.issue_file("./test2.lic",
                         "harry",
                         30,
                         {
                             {"disk_sn", hj::license::get_disk_sn()},
                         });
    ASSERT_TRUE(err.value() == 0);

    err = vefer.verify_file("./test2.lic",
                            "harry",
                            30,
                            {
                                {"disk_sn", hj::license::get_disk_sn()},
                            });
    ASSERT_TRUE(err.value() == 0);

    err = vefer.verify_file("./test2.lic",
                            "harry1",
                            30,
                            {
                                {"disk_sn", hj::license::get_disk_sn()},
                            });
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

    hj::license::issuer  isu{"issuer1",
                            hj::license::sign_algo::rsa256,
                             {pubkey, prikey, "", ""},
                            2};
    hj::license::token_t token;
    auto                 err = isu.issue(token,
                         "harry",
                         30,
                                         {{"disk_sn", hj::license::get_disk_sn()}});
    ASSERT_TRUE(err.value() == 0);

    hj::license::verifier vefer{"issuer1",
                                hj::license::sign_algo::rsa256,
                                {pubkey, prikey, "", ""}};
    err = vefer.verify(token,
                       "harry",
                       30,
                       {
                           {"disk_sn", hj::license::get_disk_sn()},
                       });
    ASSERT_TRUE(err.value() == 0);

    isu.release(hj::license::sign_algo::none, {});
    err = isu.issue(token,
                    "harry",
                    30,
                    {{"disk_sn", hj::license::get_disk_sn()}});
    ASSERT_TRUE(err.value() == 0);

    err = vefer.verify(token,
                       "harry",
                       30,
                       {
                           {"disk_sn", hj::license::get_disk_sn()},
                       });
    ASSERT_FALSE(err.value() == 0);
}

TEST(license, parse_valid_token)
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

    hj::license::issuer isu{"test_issuer",
                            hj::license::sign_algo::rsa256,
                            {pubkey, prikey, "", ""},
                            5};

    hj::license::token_t token;
    std::string          disk_sn = hj::license::get_disk_sn();
    auto                 err     = isu.issue(token,
                         "test_licensee",
                         30,
                                             {{"disk_sn", disk_sn},
                                              {"version", "1.0.0"},
                                              {"feature", "premium"},
                                              {"max_users", "100"}});
    ASSERT_TRUE(err.value() == 0);

    hj::license::license_info info;
    err = hj::license::verifier::parse(info, token);

    ASSERT_TRUE(err.value() == 0);

    EXPECT_EQ(info.issuer, "test_issuer");
    EXPECT_EQ(info.audience, "test_licensee");

    auto now = std::chrono::system_clock::now();
    auto time_diff =
        std::chrono::duration_cast<std::chrono::seconds>(now - info.issued_at);
    EXPECT_LT(time_diff.count(), 10);

    auto expiry_diff = std::chrono::duration_cast<std::chrono::hours>(
        info.expires_at - info.issued_at);
    EXPECT_GE(expiry_diff.count(), 30 * 24 - 1);
    EXPECT_LE(expiry_diff.count(), 30 * 24 + 1);

    ASSERT_EQ(info.claims.size(), 4);

    std::map<std::string, std::string> claims_map;
    for(const auto &claim : info.claims)
    {
        claims_map[claim.first] = claim.second;
    }

    EXPECT_EQ(claims_map["disk_sn"], "\"" + disk_sn + "\"");
    EXPECT_EQ(claims_map["version"], "\"1.0.0\"");
    EXPECT_EQ(claims_map["feature"], "\"premium\"");
    EXPECT_EQ(claims_map["max_users"], "\"100\"");
}

TEST(license, parse_none_algorithm_token)
{
    hj::license::issuer isu{"none_issuer", hj::license::sign_algo::none, {}, 3};

    hj::license::token_t token;
    auto                 err = isu.issue(token,
                         "none_licensee",
                         7,
                                         {{"license_type", "trial"}, {"company", "TestCorp"}});
    ASSERT_TRUE(err.value() == 0);

    hj::license::license_info info;
    err = hj::license::verifier::parse(info, token);

    ASSERT_TRUE(err.value() == 0);
    EXPECT_EQ(info.issuer, "none_issuer");
    EXPECT_EQ(info.audience, "none_licensee");

    ASSERT_EQ(info.claims.size(), 2);
    std::map<std::string, std::string> claims_map;
    for(const auto &claim : info.claims)
    {
        claims_map[claim.first] = claim.second;
    }

    EXPECT_EQ(claims_map["license_type"], "\"trial\"");
    EXPECT_EQ(claims_map["company"], "\"TestCorp\"");
}

TEST(license, parse_empty_claims_token)
{
    hj::license::issuer isu{"empty_claims_issuer",
                            hj::license::sign_algo::none,
                            {},
                            2};

    hj::license::token_t token;
    auto err = isu.issue(token, "empty_claims_licensee", 15, {});
    ASSERT_TRUE(err.value() == 0);

    hj::license::license_info info;
    err = hj::license::verifier::parse(info, token);

    ASSERT_TRUE(err.value() == 0);
    EXPECT_EQ(info.issuer, "empty_claims_issuer");
    EXPECT_EQ(info.audience, "empty_claims_licensee");

    EXPECT_EQ(info.claims.size(), 0);
}

TEST(license, parse_invalid_token)
{
    hj::license::license_info info;

    auto err = hj::license::verifier::parse(info, "");
    EXPECT_NE(err.value(), 0);
    EXPECT_EQ(
        err.value(),
        static_cast<int>(hj::license::error_code::parse_license_info_failed));

    err = hj::license::verifier::parse(info, "invalid.token.format");
    EXPECT_NE(err.value(), 0);
    EXPECT_EQ(
        err.value(),
        static_cast<int>(hj::license::error_code::parse_license_info_failed));

    err = hj::license::verifier::parse(info, "this_is_not_a_jwt_token");
    EXPECT_NE(err.value(), 0);
    EXPECT_EQ(
        err.value(),
        static_cast<int>(hj::license::error_code::parse_license_info_failed));
}

TEST(license, parse_malformed_jwt_token)
{
    hj::license::license_info info;

    auto err = hj::license::verifier::parse(
        info,
        "eyJ0eXAiOiJKV1QiLCJhbGciOiJub25lIn0.eyJpc3MiOiJ0ZXN0In0");
    EXPECT_NE(err.value(), 0);

    err = hj::license::verifier::parse(
        info,
        "invalid_base64.invalid_base64.invalid_base64");
    EXPECT_NE(err.value(), 0);
}

TEST(license, parse_complex_claims_token)
{
    hj::license::issuer isu{"complex_issuer",
                            hj::license::sign_algo::none,
                            {},
                            3};

    hj::license::token_t token;
    auto                 err = isu.issue(token,
                         "complex_licensee",
                         90,
                                         {{"user_id", "12345"},
                                          {"permissions", "read,write,admin"},
                                          {"metadata", "{\"key\":\"value\"}"},
                                          {"expiry_warning", "30_days_before"},
                                          {"renewal_url", "https://example.com/renew"}});
    ASSERT_TRUE(err.value() == 0);

    hj::license::license_info info;
    err = hj::license::verifier::parse(info, token);

    ASSERT_TRUE(err.value() == 0);
    EXPECT_EQ(info.issuer, "complex_issuer");
    EXPECT_EQ(info.audience, "complex_licensee");

    ASSERT_EQ(info.claims.size(), 5);
    std::map<std::string, std::string> claims_map;
    for(const auto &claim : info.claims)
    {
        claims_map[claim.first] = claim.second;
    }

    EXPECT_EQ(claims_map["user_id"], "\"12345\"");
    EXPECT_EQ(claims_map["permissions"], "\"read,write,admin\"");
    EXPECT_EQ(claims_map["metadata"], "\"{\\\"key\\\":\\\"value\\\"}\"");
    EXPECT_EQ(claims_map["expiry_warning"], "\"30_days_before\"");
    EXPECT_EQ(claims_map["renewal_url"], "\"https://example.com/renew\"");
}