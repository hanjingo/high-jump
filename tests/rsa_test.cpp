#include <gtest/gtest.h>
#include <libcpp/crypto/rsa.hpp>
#include <libcpp/crypto/base64.hpp>

TEST(rsa, encode)
{
    // verify it by website:https://www.devglan.com/online-tools/rsa-encryption-decryption

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

    std::string plain = "hello world";
    unsigned char encoded[4096];
    std::size_t encoded_len = 4096;
    ASSERT_EQ(libcpp::rsa::encode(encoded,
                                  encoded_len,
                                  reinterpret_cast<const unsigned char*>(plain.c_str()),
                                  plain.size(), 
                                  reinterpret_cast<unsigned char*>(pubkey.data()),
                                  pubkey.size()), 
              true);

    unsigned char decoded[4096];
    std::size_t decoded_len = 4096;
    ASSERT_EQ(libcpp::rsa::decode(decoded, 
                                  decoded_len, 
                                  encoded, 
                                  encoded_len, 
                                  reinterpret_cast<unsigned char*>(prikey.data()), 
                                  prikey.size()), 
              true);

    std::string decoded_str;
    decoded_str.assign(reinterpret_cast<char*>(decoded), decoded_len);
    ASSERT_STREQ(decoded_str.c_str(), plain.c_str());

    // for stream test
    std::istringstream in(plain);
    std::ostringstream out;
    ASSERT_EQ(libcpp::rsa::encode(out, 
                                  in, 
                                  reinterpret_cast<const unsigned char*>(pubkey.data()), 
                                  pubkey.size()), 
        true);

    std::string encoded_stream_str = out.str();
    unsigned char decoded_stream[4096];
    std::size_t decoded_stream_len = 4096;
    ASSERT_EQ(libcpp::rsa::decode(decoded_stream,
                                  decoded_stream_len,
                                  reinterpret_cast<const unsigned char*>(encoded_stream_str.c_str()),
                                  encoded_stream_str.size(), 
                                  reinterpret_cast<unsigned char*>(prikey.data()), 
                                  prikey.size()), 
              true);

    std::string decoded_stream_str;
    decoded_stream_str.assign(reinterpret_cast<char*>(decoded_stream), decoded_stream_len);
    ASSERT_STREQ(decoded_stream_str.c_str(), plain.c_str());
}

TEST(rsa, decode)
{
    // verify it by website:https://www.devglan.com/online-tools/rsa-encryption-decryption

    std::string prikey = R"(-----BEGIN RSA PRIVATE KEY-----
MIIBUwIBADANBgkqhkiG9w0BAQEFAASCAT0wggE5AgEAAkEAk7PeqHjrEFJRrbVfSDp8FpaYkJynOYR9/CJtqi8S07DgthGvs+XxHeTA6R8vPKqIykZgNgp0sJsJXHOKKNtDnQIDAQABAkAGmXeWJvr3ynnQTWWRvF09hCKSeZFmOkOHz8D/JOXONAxYOPkpNVu3sShS/ccyGMQKjSHEa5Zyo0S9k/vwl+7xAiEAz6bkXHsaut7Sk2Ze4/MZZuRhR6LqE7Q9Y/ecyGyTDFECIQC2F7HqA0RB2PhuD37Gb3JkB1HNUdro9Fj6wVYATXYLjQIgSOP/k0sPRfuDlYRA2OlzyD9wunHAkywYxKednGkocRECIGsm1F31YCQzbjUtzxcsG687E2rz4RK2PuoH/PienHk9AiBIE54w2swcy1YcaL3MnZDN6eWVFYRTZXeue74hbpZ27A==
-----END RSA PRIVATE KEY-----)";

    std::string pubkey = R"(-----BEGIN PUBLIC KEY-----
MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAJOz3qh46xBSUa21X0g6fBaWmJCcpzmEffwibaovEtOw4LYRr7Pl8R3kwOkfLzyqiMpGYDYKdLCbCVxziijbQ50CAwEAAQ==
-----END PUBLIC KEY-----)";

    std::string plain = "hehehunanchina@live.com";
    unsigned char encoded[4096];
    std::size_t encoded_len = 4096;
    ASSERT_EQ(libcpp::rsa::encode(
        encoded,
        encoded_len,
        reinterpret_cast<const unsigned char*>(plain.c_str()),
        plain.size(),
        reinterpret_cast<unsigned char*>(pubkey.data()),
        pubkey.size()),
        true);

    unsigned char decoded[4096];
    std::size_t decoded_len = 4096;
    ASSERT_EQ(libcpp::rsa::decode(
        decoded,
        decoded_len,
        encoded,
        encoded_len,
        reinterpret_cast<unsigned char*>(prikey.data()),
        prikey.size()),
        true);

    std::string decoded_str;
    decoded_str.assign(reinterpret_cast<char*>(decoded), decoded_len);
    ASSERT_STREQ(decoded_str.c_str(), plain.c_str());

    // for stream test
    unsigned char encoded_stream[4096];
    std::size_t encoded_stream_len = 4096;
    ASSERT_EQ(libcpp::rsa::encode(encoded_stream,
                                  encoded_stream_len,
                                  reinterpret_cast<const unsigned char*>(plain.c_str()),
                                  plain.size(),
                                  reinterpret_cast<const unsigned char*>(pubkey.data()),
                                  pubkey.size()),
              true);

    std::string encode_stream_str;
    encode_stream_str.assign(reinterpret_cast<char*>(encoded_stream), encoded_stream_len);
    std::istringstream in(encode_stream_str);
    std::ostringstream out;
    ASSERT_EQ(libcpp::rsa::decode(out,
                                  in,
                                  reinterpret_cast<unsigned char*>(prikey.data()),
                                  prikey.size()),
              true);
    ASSERT_STREQ(out.str().c_str(), plain.c_str());
}

TEST(rsa, encode_file)
{
    std::string pubkey = R"(-----BEGIN PUBLIC KEY-----
MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAJOz3qh46xBSUa21X0g6fBaWmJCcpzmEffwibaovEtOw4LYRr7Pl8R3kwOkfLzyqiMpGYDYKdLCbCVxziijbQ50CAwEAAQ==
-----END PUBLIC KEY-----)";

    // file -> rsa file
    ASSERT_EQ(libcpp::rsa::encode_file(
        std::string("./rsa_file_test_encode.log"),
        std::string("./crypto.log"), 
        pubkey), 
    true);
}

TEST(rsa, decode_file)
{
    std::string prikey = R"(-----BEGIN RSA PRIVATE KEY-----
MIIBUwIBADANBgkqhkiG9w0BAQEFAASCAT0wggE5AgEAAkEAk7PeqHjrEFJRrbVfSDp8FpaYkJynOYR9/CJtqi8S07DgthGvs+XxHeTA6R8vPKqIykZgNgp0sJsJXHOKKNtDnQIDAQABAkAGmXeWJvr3ynnQTWWRvF09hCKSeZFmOkOHz8D/JOXONAxYOPkpNVu3sShS/ccyGMQKjSHEa5Zyo0S9k/vwl+7xAiEAz6bkXHsaut7Sk2Ze4/MZZuRhR6LqE7Q9Y/ecyGyTDFECIQC2F7HqA0RB2PhuD37Gb3JkB1HNUdro9Fj6wVYATXYLjQIgSOP/k0sPRfuDlYRA2OlzyD9wunHAkywYxKednGkocRECIGsm1F31YCQzbjUtzxcsG687E2rz4RK2PuoH/PienHk9AiBIE54w2swcy1YcaL3MnZDN6eWVFYRTZXeue74hbpZ27A==
-----END RSA PRIVATE KEY-----)";

    // rsa file -> file
    ASSERT_EQ(libcpp::rsa::decode_file(
        std::string("./rsa_file_test_decode.log"),
        std::string("./rsa_file_test_encode.log"), 
        prikey), 
    true);
}

TEST(rsa, make_key_pair_x509)
{
    std::string prikey;
    std::string pubkey;
    std::string plain = "hello world";
    ASSERT_EQ(libcpp::rsa::make_key_pair_x509(pubkey, prikey, 2048), true);

    unsigned char encoded[4096];
    std::size_t encoded_len = 4096;
    ASSERT_EQ(libcpp::rsa::encode(encoded,
                                  encoded_len,
                                  reinterpret_cast<const unsigned char*>(plain.c_str()),
                                  plain.size(), 
                                  reinterpret_cast<unsigned char*>(pubkey.data()),
                                  pubkey.size()), 
        true);

    unsigned char decoded[4096];
    std::size_t decoded_len = 4096;
    ASSERT_EQ(libcpp::rsa::decode(decoded, 
                                  decoded_len,
                                  encoded, 
                                  encoded_len, 
                                  reinterpret_cast<unsigned char*>(prikey.data()), 
                                  prikey.size()), 
        true);

    std::string decoded_str;
    decoded_str.assign(reinterpret_cast<char*>(decoded), decoded_len);
    ASSERT_STREQ(decoded_str.c_str(), plain.c_str());
}

TEST(rsa, is_pubkey_valid)
{
    std::string pubkey1 = R"(-----BEGIN PUBLIC KEY-----
MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAJOz3qh46xBSUa21X0g6fBaWmJCcpzmEffwibaovEtOw4LYRr7Pl8R3kwOkfLzyqiMpGYDYKdLCbCVxziijbQ50CAwEAAQ==
-----END PUBLIC KEY-----)";

    ASSERT_EQ(libcpp::rsa::is_pubkey_valid(
        reinterpret_cast<const unsigned char*>(pubkey1.c_str()),
        pubkey1.size(),
        RSA_PKCS1_PADDING), 
    true);

    std::string pubkey2 = R"(MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAJOz3qh46xBSUa21X0g6fBaWmJCcpzmEffwibaovEtOw4LYRr7Pl8R3kwOkfLzyqiMpGYDYKdLCbCVxziijbQ50CAwEAAQ==)";

    ASSERT_EQ(libcpp::rsa::is_pubkey_valid(
        reinterpret_cast<const unsigned char*>(pubkey2.c_str()),
        pubkey2.size(),
        RSA_PKCS1_PADDING), 
    false);
}

TEST(rsa, is_prikey_valid)
{
    std::string prikey1 = R"(-----BEGIN RSA PRIVATE KEY-----
MIIBUwIBADANBgkqhkiG9w0BAQEFAASCAT0wggE5AgEAAkEAk7PeqHjrEFJRrbVfSDp8FpaYkJynOYR9/CJtqi8S07DgthGvs+XxHeTA6R8vPKqIykZgNgp0sJsJXHOKKNtDnQIDAQABAkAGmXeWJvr3ynnQTWWRvF09hCKSeZFmOkOHz8D/JOXONAxYOPkpNVu3sShS/ccyGMQKjSHEa5Zyo0S9k/vwl+7xAiEAz6bkXHsaut7Sk2Ze4/MZZuRhR6LqE7Q9Y/ecyGyTDFECIQC2F7HqA0RB2PhuD37Gb3JkB1HNUdro9Fj6wVYATXYLjQIgSOP/k0sPRfuDlYRA2OlzyD9wunHAkywYxKednGkocRECIGsm1F31YCQzbjUtzxcsG687E2rz4RK2PuoH/PienHk9AiBIE54w2swcy1YcaL3MnZDN6eWVFYRTZXeue74hbpZ27A==
-----END RSA PRIVATE KEY-----)";

    ASSERT_EQ(libcpp::rsa::is_prikey_valid(
        reinterpret_cast<const unsigned char*>(prikey1.c_str()),
        prikey1.size(),
        RSA_PKCS1_PADDING), 
    true);

    std::string prikey2 = R"(MIIBUwIBADANBgkqhkiG9w0BAQEFAASCAT0wggE5AgEAAkEAk7PeqHjrEFJRrbVfSDp8FpaYkJynOYR9/CJtqi8S07DgthGvs+XxHeTA6R8vPKqIykZgNgp0sJsJXHOKKNtDnQIDAQABAkAGmXeWJvr3ynnQTWWRvF09hCKSeZFmOkOHz8D/JOXONAxYOPkpNVu3sShS/ccyGMQKjSHEa5Zyo0S9k/vwl+7xAiEAz6bkXHsaut7Sk2Ze4/MZZuRhR6LqE7Q9Y/ecyGyTDFECIQC2F7HqA0RB2PhuD37Gb3JkB1HNUdro9Fj6wVYATXYLjQIgSOP/k0sPRfuDlYRA2OlzyD9wunHAkywYxKednGkocRECIGsm1F31YCQzbjUtzxcsG687E2rz4RK2PuoH/PienHk9AiBIE54w2swcy1YcaL3MnZDN6eWVFYRTZXeue74hbpZ27A==)";

    ASSERT_EQ(libcpp::rsa::is_prikey_valid(
        reinterpret_cast<const unsigned char*>(prikey2.c_str()),
        prikey2.size(),
        RSA_PKCS1_PADDING), 
    false);
}