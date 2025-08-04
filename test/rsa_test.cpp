#include <gtest/gtest.h>
#include <libcpp/crypto/base64.hpp>
#include <libcpp/crypto/rsa.hpp>

TEST(rsa, encode)
{
    // verify it by
    // website:https://www.devglan.com/online-tools/rsa-encryption-decryption

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

    // PKCS1 padding test
    unsigned char encoded1[4096];
    std::size_t encoded1_len = 4096;
    ASSERT_EQ(libcpp::rsa::encode(
                  encoded1,
                  encoded1_len,
                  reinterpret_cast<const unsigned char*>(plain.c_str()),
                  plain.size(),
                  reinterpret_cast<unsigned char*>(pubkey.data()),
                  pubkey.size(),
                  libcpp::rsa::padding::pkcs1),
              true);
    unsigned char decoded1[4096];
    std::size_t decoded1_len = 4096;
    ASSERT_EQ(
        libcpp::rsa::decode(decoded1,
                            decoded1_len,
                            encoded1,
                            encoded1_len,
                            reinterpret_cast<unsigned char*>(prikey.data()),
                            prikey.size(),
                            libcpp::rsa::padding::pkcs1),
        true);
    std::string decoded1_str;
    decoded1_str.assign(reinterpret_cast<char*>(decoded1), decoded1_len);
    ASSERT_STREQ(decoded1_str.c_str(), plain.c_str());

    // NOPADDING padding test
    const int key_size = 64;
    std::vector<unsigned char> plain_data(key_size);
    std::string original_msg = "hello world";
    std::memcpy(plain_data.data(), original_msg.data(), original_msg.size());
    unsigned char encoded[4096];
    std::size_t encoded_len = 4096;
    ASSERT_EQ(
        libcpp::rsa::encode(encoded,
                            encoded_len,
                            plain_data.data(),
                            plain_data.size(),
                            reinterpret_cast<unsigned char*>(pubkey.data()),
                            pubkey.size(),
                            libcpp::rsa::padding::no_padding),
        true);
    unsigned char decoded[4096];
    std::size_t decoded_len = 4096;
    ASSERT_EQ(
        libcpp::rsa::decode(decoded,
                            decoded_len,
                            encoded,
                            encoded_len,
                            reinterpret_cast<unsigned char*>(prikey.data()),
                            prikey.size(),
                            libcpp::rsa::padding::no_padding),
        true);
    ASSERT_EQ(decoded_len, key_size);
    ASSERT_EQ(std::memcmp(decoded, plain_data.data(), key_size), 0);
    ASSERT_EQ(std::memcmp(decoded, original_msg.data(), original_msg.size()),
              0);

    // for stream test
    std::istringstream in(plain);
    std::ostringstream out;
    ASSERT_EQ(libcpp::rsa::encode(
                  out,
                  in,
                  reinterpret_cast<const unsigned char*>(pubkey.data()),
                  pubkey.size(),
                  libcpp::rsa::padding::pkcs1),
              true);
    std::string encoded_stream_str = out.str();
    unsigned char decoded_stream[4096];
    std::size_t decoded_stream_len = 4096;
    ASSERT_EQ(
        libcpp::rsa::decode(
            decoded_stream,
            decoded_stream_len,
            reinterpret_cast<const unsigned char*>(encoded_stream_str.c_str()),
            encoded_stream_str.size(),
            reinterpret_cast<unsigned char*>(prikey.data()),
            prikey.size(),
            libcpp::rsa::padding::pkcs1),
        true);
    std::string decoded_stream_str;
    decoded_stream_str.assign(reinterpret_cast<char*>(decoded_stream),
                              decoded_stream_len);
    ASSERT_STREQ(decoded_stream_str.c_str(), plain.c_str());
}

TEST(rsa, decode)
{
    // verify it by
    // website:https://www.devglan.com/online-tools/rsa-encryption-decryption

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
    ASSERT_EQ(
        libcpp::rsa::decode(decoded,
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
    ASSERT_EQ(libcpp::rsa::encode(
                  encoded_stream,
                  encoded_stream_len,
                  reinterpret_cast<const unsigned char*>(plain.c_str()),
                  plain.size(),
                  reinterpret_cast<const unsigned char*>(pubkey.data()),
                  pubkey.size()),
              true);

    std::string encode_stream_str;
    encode_stream_str.assign(reinterpret_cast<char*>(encoded_stream),
                             encoded_stream_len);
    std::istringstream in(encode_stream_str);
    std::ostringstream out;
    ASSERT_EQ(
        libcpp::rsa::decode(out,
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
    ASSERT_EQ(
        libcpp::rsa::encode_file(std::string("./rsa_file_test_encode.log"),
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
    ASSERT_EQ(
        libcpp::rsa::decode_file(std::string("./rsa_file_test_decode.log"),
                                 std::string("./rsa_file_test_encode.log"),
                                 prikey),
        true);
}

TEST(rsa, make_key_pair)
{
    std::string prikey;
    std::string pubkey;
    std::string passwd = "test123456";
    std::string plain = "hello world";
    ASSERT_EQ(libcpp::rsa::make_key_pair(pubkey,
                                         prikey,
                                         2048,
                                         libcpp::rsa::key_format::x509,
                                         libcpp::rsa::algo::aes_256_cbc,
                                         passwd),
              true);

    unsigned char encoded[4096];
    std::size_t encoded_len = 4096;
    ASSERT_EQ(libcpp::rsa::encode(
                  encoded,
                  encoded_len,
                  reinterpret_cast<const unsigned char*>(plain.c_str()),
                  plain.size(),
                  reinterpret_cast<unsigned char*>(pubkey.data()),
                  pubkey.size(),
                  libcpp::rsa::padding::pkcs1),
              true);

    unsigned char decoded[4096];
    std::size_t decoded_len = 4096;
    ASSERT_EQ(libcpp::rsa::decode(
                  decoded,
                  decoded_len,
                  encoded,
                  encoded_len,
                  reinterpret_cast<const unsigned char*>(prikey.data()),
                  prikey.size(),
                  libcpp::rsa::padding::pkcs1,
                  reinterpret_cast<const unsigned char*>(passwd.c_str()),
                  passwd.size()),
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
                  libcpp::rsa::padding::pkcs1),
              true);

    std::string pubkey2 =
        R"(MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAJOz3qh46xBSUa21X0g6fBaWmJCcpzmEffwibaovEtOw4LYRr7Pl8R3kwOkfLzyqiMpGYDYKdLCbCVxziijbQ50CAwEAAQ==)";

    ASSERT_EQ(libcpp::rsa::is_pubkey_valid(
                  reinterpret_cast<const unsigned char*>(pubkey2.c_str()),
                  pubkey2.size(),
                  libcpp::rsa::padding::pkcs1),
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
                  libcpp::rsa::padding::pkcs1),
              true);

    std::string prikey2 =
        R"(MIIBUwIBADANBgkqhkiG9w0BAQEFAASCAT0wggE5AgEAAkEAk7PeqHjrEFJRrbVfSDp8FpaYkJynOYR9/CJtqi8S07DgthGvs+XxHeTA6R8vPKqIykZgNgp0sJsJXHOKKNtDnQIDAQABAkAGmXeWJvr3ynnQTWWRvF09hCKSeZFmOkOHz8D/JOXONAxYOPkpNVu3sShS/ccyGMQKjSHEa5Zyo0S9k/vwl+7xAiEAz6bkXHsaut7Sk2Ze4/MZZuRhR6LqE7Q9Y/ecyGyTDFECIQC2F7HqA0RB2PhuD37Gb3JkB1HNUdro9Fj6wVYATXYLjQIgSOP/k0sPRfuDlYRA2OlzyD9wunHAkywYxKednGkocRECIGsm1F31YCQzbjUtzxcsG687E2rz4RK2PuoH/PienHk9AiBIE54w2swcy1YcaL3MnZDN6eWVFYRTZXeue74hbpZ27A==)";

    ASSERT_EQ(libcpp::rsa::is_prikey_valid(
                  reinterpret_cast<const unsigned char*>(prikey2.c_str()),
                  prikey2.size(),
                  libcpp::rsa::padding::pkcs1),
              false);
}