#include <gtest/gtest.h>
#include <libcpp/crypto/rsa.hpp>
#include <libcpp/crypto/base64.hpp>
#include <cstring>

TEST(rsa, encrypt)
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

    // PKCS1 padding test
    unsigned char encrypted1[4096];
    std::size_t encrypted1_len = 4096;
    ASSERT_EQ(libcpp::rsa::encrypt(encrypted1,
                                  encrypted1_len,
                                  reinterpret_cast<const unsigned char*>(plain.c_str()),
                                  plain.size(), 
                                  reinterpret_cast<unsigned char*>(pubkey.data()),
                                  pubkey.size(),
                                  libcpp::rsa::padding::pkcs1), 
              true);
    unsigned char decrypted1[4096];
    std::size_t decrypted1_len = 4096;
    ASSERT_EQ(libcpp::rsa::decrypt(decrypted1, 
                                  decrypted1_len, 
                                  encrypted1, 
                                  encrypted1_len, 
                                  reinterpret_cast<unsigned char*>(prikey.data()), 
                                  prikey.size(),
                                  libcpp::rsa::padding::pkcs1), 
              true);
    std::string decrypted1_str;
    decrypted1_str.assign(reinterpret_cast<char*>(decrypted1), decrypted1_len);
    ASSERT_STREQ(decrypted1_str.c_str(), plain.c_str());

    // NOPADDING padding test
    const int key_size = 64;
    std::vector<unsigned char> plain_data(key_size);
    std::string original_msg = "hello world";
    memcpy(plain_data.data(), original_msg.data(), original_msg.size());
    unsigned char encrypted[4096];
    std::size_t encrypted_len = 4096;
    ASSERT_EQ(libcpp::rsa::encrypt(encrypted,
                                  encrypted_len,
                                  plain_data.data(),
                                  plain_data.size(), 
                                  reinterpret_cast<unsigned char*>(pubkey.data()),
                                  pubkey.size(),
                                  libcpp::rsa::padding::no_padding), 
              true);
    unsigned char decrypted[4096];
    std::size_t decrypted_len = 4096;
    ASSERT_EQ(libcpp::rsa::decrypt(decrypted, 
                                  decrypted_len, 
                                  encrypted, 
                                  encrypted_len, 
                                  reinterpret_cast<unsigned char*>(prikey.data()), 
                                  prikey.size(),
                                  libcpp::rsa::padding::no_padding), 
              true);
    ASSERT_EQ(decrypted_len, key_size);
    ASSERT_EQ(memcmp(decrypted, plain_data.data(), key_size), 0);
    ASSERT_EQ(memcmp(decrypted, original_msg.data(), original_msg.size()), 0);

    // for stream test
    std::istringstream in(plain);
    std::ostringstream out;
    ASSERT_EQ(libcpp::rsa::encrypt(out, 
                                  in, 
                                  reinterpret_cast<const unsigned char*>(pubkey.data()), 
                                  pubkey.size(),
                                  libcpp::rsa::padding::pkcs1), 
        true);
    std::string encrypted_stream_str = out.str();
    unsigned char decrypted_stream[4096];
    std::size_t decrypted_stream_len = 4096;
    ASSERT_EQ(libcpp::rsa::decrypt(decrypted_stream,
                                  decrypted_stream_len,
                                  reinterpret_cast<const unsigned char*>(encrypted_stream_str.c_str()),
                                  encrypted_stream_str.size(), 
                                  reinterpret_cast<unsigned char*>(prikey.data()), 
                                  prikey.size(),
                                  libcpp::rsa::padding::pkcs1), 
              true);
    std::string decrypted_stream_str;
    decrypted_stream_str.assign(reinterpret_cast<char*>(decrypted_stream), decrypted_stream_len);
    ASSERT_STREQ(decrypted_stream_str.c_str(), plain.c_str());
}

TEST(rsa, decrypt)
{
    // verify it by website:https://www.devglan.com/online-tools/rsa-encryption-decryption

    std::string prikey = R"(-----BEGIN RSA PRIVATE KEY-----
MIIBUwIBADANBgkqhkiG9w0BAQEFAASCAT0wggE5AgEAAkEAk7PeqHjrEFJRrbVfSDp8FpaYkJynOYR9/CJtqi8S07DgthGvs+XxHeTA6R8vPKqIykZgNgp0sJsJXHOKKNtDnQIDAQABAkAGmXeWJvr3ynnQTWWRvF09hCKSeZFmOkOHz8D/JOXONAxYOPkpNVu3sShS/ccyGMQKjSHEa5Zyo0S9k/vwl+7xAiEAz6bkXHsaut7Sk2Ze4/MZZuRhR6LqE7Q9Y/ecyGyTDFECIQC2F7HqA0RB2PhuD37Gb3JkB1HNUdro9Fj6wVYATXYLjQIgSOP/k0sPRfuDlYRA2OlzyD9wunHAkywYxKednGkocRECIGsm1F31YCQzbjUtzxcsG687E2rz4RK2PuoH/PienHk9AiBIE54w2swcy1YcaL3MnZDN6eWVFYRTZXeue74hbpZ27A==
-----END RSA PRIVATE KEY-----)";

    std::string pubkey = R"(-----BEGIN PUBLIC KEY-----
MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAJOz3qh46xBSUa21X0g6fBaWmJCcpzmEffwibaovEtOw4LYRr7Pl8R3kwOkfLzyqiMpGYDYKdLCbCVxziijbQ50CAwEAAQ==
-----END PUBLIC KEY-----)";

    std::string plain = "hehehunanchina@live.com";
    unsigned char encrypted[4096];
    std::size_t encrypted_len = 4096;
    ASSERT_EQ(libcpp::rsa::encrypt(
        encrypted,
        encrypted_len,
        reinterpret_cast<const unsigned char*>(plain.c_str()),
        plain.size(),
        reinterpret_cast<unsigned char*>(pubkey.data()),
        pubkey.size()),
        true);

    unsigned char decrypted[4096];
    std::size_t decrypted_len = 4096;
    ASSERT_EQ(libcpp::rsa::decrypt(
        decrypted,
        decrypted_len,
        encrypted,
        encrypted_len,
        reinterpret_cast<unsigned char*>(prikey.data()),
        prikey.size()),
        true);

    std::string decrypted_str;
    decrypted_str.assign(reinterpret_cast<char*>(decrypted), decrypted_len);
    ASSERT_STREQ(decrypted_str.c_str(), plain.c_str());

    // for stream test
    unsigned char encrypted_stream[4096];
    std::size_t encrypted_stream_len = 4096;
    ASSERT_EQ(libcpp::rsa::encrypt(encrypted_stream,
                                  encrypted_stream_len,
                                  reinterpret_cast<const unsigned char*>(plain.c_str()),
                                  plain.size(),
                                  reinterpret_cast<const unsigned char*>(pubkey.data()),
                                  pubkey.size()),
              true);

    std::string encrypt_stream_str;
    encrypt_stream_str.assign(reinterpret_cast<char*>(encrypted_stream), encrypted_stream_len);
    std::istringstream in(encrypt_stream_str);
    std::ostringstream out;
    ASSERT_EQ(libcpp::rsa::decrypt(out,
                                  in,
                                  reinterpret_cast<unsigned char*>(prikey.data()),
                                  prikey.size()),
              true);
    ASSERT_STREQ(out.str().c_str(), plain.c_str());
}

TEST(rsa, encrypt_file)
{
    std::string pubkey = R"(-----BEGIN PUBLIC KEY-----
MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAJOz3qh46xBSUa21X0g6fBaWmJCcpzmEffwibaovEtOw4LYRr7Pl8R3kwOkfLzyqiMpGYDYKdLCbCVxziijbQ50CAwEAAQ==
-----END PUBLIC KEY-----)";

    // file -> rsa file
    ASSERT_EQ(libcpp::rsa::encrypt_file(
        std::string("./rsa_file_test_encrypt.log"),
        std::string("./crypto.log"), 
        pubkey), 
    true);
}

TEST(rsa, decrypt_file)
{
    std::string pubkey = R"(-----BEGIN PUBLIC KEY-----
MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAJOz3qh46xBSUa21X0g6fBaWmJCcpzmEffwibaovEtOw4LYRr7Pl8R3kwOkfLzyqiMpGYDYKdLCbCVxziijbQ50CAwEAAQ==
-----END PUBLIC KEY-----)";

    // file -> rsa file
    ASSERT_EQ(libcpp::rsa::encrypt_file(
        std::string("./rsa_file_test_encrypt1.log"),
        std::string("./crypto.log"), 
        pubkey), 
    true);

    std::string prikey = R"(-----BEGIN RSA PRIVATE KEY-----
MIIBUwIBADANBgkqhkiG9w0BAQEFAASCAT0wggE5AgEAAkEAk7PeqHjrEFJRrbVfSDp8FpaYkJynOYR9/CJtqi8S07DgthGvs+XxHeTA6R8vPKqIykZgNgp0sJsJXHOKKNtDnQIDAQABAkAGmXeWJvr3ynnQTWWRvF09hCKSeZFmOkOHz8D/JOXONAxYOPkpNVu3sShS/ccyGMQKjSHEa5Zyo0S9k/vwl+7xAiEAz6bkXHsaut7Sk2Ze4/MZZuRhR6LqE7Q9Y/ecyGyTDFECIQC2F7HqA0RB2PhuD37Gb3JkB1HNUdro9Fj6wVYATXYLjQIgSOP/k0sPRfuDlYRA2OlzyD9wunHAkywYxKednGkocRECIGsm1F31YCQzbjUtzxcsG687E2rz4RK2PuoH/PienHk9AiBIE54w2swcy1YcaL3MnZDN6eWVFYRTZXeue74hbpZ27A==
-----END RSA PRIVATE KEY-----)";

    // rsa file -> file
    ASSERT_EQ(libcpp::rsa::decrypt_file(
        std::string("./rsa_file_test_decrypt.log"),
        std::string("./rsa_file_test_encrypt1.log"), 
        prikey), 
    true);
}

TEST(rsa, make_key_pair)
{
    std::string prikey;
    std::string pubkey;
    std::string passwd = "test123456";
    std::string plain = "hello world";
    ASSERT_EQ(libcpp::rsa::make_key_pair(pubkey, prikey, 2048, libcpp::rsa::key_format::x509, 
        libcpp::rsa::mode::aes_256_cbc, passwd), true);

    unsigned char encrypted[4096];
    std::size_t encrypted_len = 4096;
    ASSERT_EQ(libcpp::rsa::encrypt(encrypted,
                                  encrypted_len,
                                  reinterpret_cast<const unsigned char*>(plain.c_str()),
                                  plain.size(), 
                                  reinterpret_cast<unsigned char*>(pubkey.data()),
                                  pubkey.size(),
                                  libcpp::rsa::padding::pkcs1), 
        true);

    unsigned char decrypted[4096];
    std::size_t decrypted_len = 4096;
    ASSERT_EQ(libcpp::rsa::decrypt(decrypted, 
                                  decrypted_len,
                                  encrypted, 
                                  encrypted_len, 
                                  reinterpret_cast<const unsigned char*>(prikey.data()), 
                                  prikey.size(),
                                  libcpp::rsa::padding::pkcs1,
                                  reinterpret_cast<const unsigned char*>(passwd.c_str()),
                                  passwd.size()), 
        true);

    std::string decrypted_str;
    decrypted_str.assign(reinterpret_cast<char*>(decrypted), decrypted_len);
    ASSERT_STREQ(decrypted_str.c_str(), plain.c_str());
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

    std::string pubkey2 = R"(MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAJOz3qh46xBSUa21X0g6fBaWmJCcpzmEffwibaovEtOw4LYRr7Pl8R3kwOkfLzyqiMpGYDYKdLCbCVxziijbQ50CAwEAAQ==)";

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

    std::string prikey2 = R"(MIIBUwIBADANBgkqhkiG9w0BAQEFAASCAT0wggE5AgEAAkEAk7PeqHjrEFJRrbVfSDp8FpaYkJynOYR9/CJtqi8S07DgthGvs+XxHeTA6R8vPKqIykZgNgp0sJsJXHOKKNtDnQIDAQABAkAGmXeWJvr3ynnQTWWRvF09hCKSeZFmOkOHz8D/JOXONAxYOPkpNVu3sShS/ccyGMQKjSHEa5Zyo0S9k/vwl+7xAiEAz6bkXHsaut7Sk2Ze4/MZZuRhR6LqE7Q9Y/ecyGyTDFECIQC2F7HqA0RB2PhuD37Gb3JkB1HNUdro9Fj6wVYATXYLjQIgSOP/k0sPRfuDlYRA2OlzyD9wunHAkywYxKednGkocRECIGsm1F31YCQzbjUtzxcsG687E2rz4RK2PuoH/PienHk9AiBIE54w2swcy1YcaL3MnZDN6eWVFYRTZXeue74hbpZ27A==)";

    ASSERT_EQ(libcpp::rsa::is_prikey_valid(
        reinterpret_cast<const unsigned char*>(prikey2.c_str()),
        prikey2.size(),
        libcpp::rsa::padding::pkcs1), 
    false);
}

TEST(rsa, is_cipher_valid)
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

    // nopadding plain.size() must equal key size - 113
    std::string plain = R"(1234567812345678123456781234567812345678123456781234567812345678)"; 

    std::string encrypted;
    ASSERT_EQ(libcpp::rsa::encrypt(encrypted,
                                   plain,
                                   pubkey,
                                   libcpp::rsa::padding::no_padding), 
              true);

    ASSERT_TRUE(libcpp::rsa::is_cipher_valid(
        encrypted,
        libcpp::rsa::padding::no_padding,
        prikey));

    std::string invalid_cipher = encrypted;
    invalid_cipher.push_back(0xFF);
    ASSERT_FALSE(libcpp::rsa::is_cipher_valid(
        invalid_cipher,
        libcpp::rsa::padding::no_padding,
        prikey));
}