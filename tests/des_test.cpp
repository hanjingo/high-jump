#include <gtest/gtest.h>
#include <hj/crypto/des.hpp>

#include <openssl/evp.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace
{

using error_code = hj::des::error_code;

constexpr char k3des_key[] = "12345678abcdefgh00000000"; // 24 bytes
constexpr char k2des_key[] = "12345678abcdefgh";         // 16 bytes
constexpr char k_iv[]      = "abcdefgh";                 // 8 bytes

std::string to_hex(const std::string &src)
{
    static constexpr char hex[] = "0123456789ABCDEF";

    std::string out;
    out.reserve(src.size() * 2);

    for(unsigned char c : src)
    {
        out.push_back(hex[c >> 4]);
        out.push_back(hex[c & 0x0F]);
    }

    return out;
}

std::string from_hex(const std::string &hex)
{
    if(hex.size() % 2 != 0)
        return {};

    std::string out;
    out.reserve(hex.size() / 2);

    auto value = [](char c) -> unsigned char {
        if(c >= '0' && c <= '9')
            return static_cast<unsigned char>(c - '0');

        if(c >= 'a' && c <= 'f')
            return static_cast<unsigned char>(c - 'a' + 10);

        return static_cast<unsigned char>(c - 'A' + 10);
    };

    for(std::size_t i = 0; i < hex.size(); i += 2)
    {
        out.push_back(
            static_cast<char>((value(hex[i]) << 4) | value(hex[i + 1])));
    }

    return out;
}

hj::des::options make_options(const std::string &key,
                              hj::des::mode      mode,
                              hj::des::padding   pad,
                              const std::string &iv = k_iv)
{
    hj::des::options opt;
    opt.key       = reinterpret_cast<const unsigned char *>(key.data());
    opt.key_len   = key.size();
    opt.mod       = mode;
    opt.pad_style = pad;

    if(mode != hj::des::mode::ecb)
    {
        opt.iv     = reinterpret_cast<const unsigned char *>(iv.data());
        opt.iv_len = iv.size();
    }

    return opt;
}

std::string openssl_encrypt(const std::string &key,
                            const std::string &iv,
                            hj::des::mode      mode,
                            const std::string &plain,
                            bool               padding)
{
    const char *name = nullptr;

    switch(mode)
    {
        case hj::des::mode::ecb:
            name = key.size() == 16 ? "DES-EDE-ECB" : "DES-EDE3-ECB";
            break;

        case hj::des::mode::cbc:
            name = key.size() == 16 ? "DES-EDE-CBC" : "DES-EDE3-CBC";
            break;

        case hj::des::mode::cfb:
            name = key.size() == 16 ? "DES-EDE-CFB" : "DES-EDE3-CFB";
            break;

        case hj::des::mode::ofb:
            name = key.size() == 16 ? "DES-EDE-OFB" : "DES-EDE3-OFB";
            break;

        case hj::des::mode::ctr:
            ADD_FAILURE() << "CTR is intentionally not cross-validated through "
                             "a direct EVP CTR cipher";
            return {};
    }

    std::unique_ptr<EVP_CIPHER, decltype(&EVP_CIPHER_free)> cipher(
        EVP_CIPHER_fetch(nullptr, name, nullptr),
        EVP_CIPHER_free);

    if(!cipher)
    {
        ADD_FAILURE() << "EVP_CIPHER_fetch failed for " << name;
        return {};
    }

    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx(
        EVP_CIPHER_CTX_new(),
        EVP_CIPHER_CTX_free);

    if(!ctx)
    {
        ADD_FAILURE() << "EVP_CIPHER_CTX_new failed";
        return {};
    }

    const unsigned char *iv_ptr =
        mode == hj::des::mode::ecb
            ? nullptr
            : reinterpret_cast<const unsigned char *>(iv.data());

    if(EVP_EncryptInit_ex(ctx.get(),
                          cipher.get(),
                          nullptr,
                          reinterpret_cast<const unsigned char *>(key.data()),
                          iv_ptr)
       != 1)
    {
        ADD_FAILURE() << "EVP_EncryptInit_ex failed";
        return {};
    }

    if(EVP_CIPHER_CTX_set_padding(ctx.get(), padding ? 1 : 0) != 1)
    {
        ADD_FAILURE() << "EVP_CIPHER_CTX_set_padding failed";
        return {};
    }

    std::string out(plain.size() + hj::des::block_size, '\0');
    int         out_len   = 0;
    int         final_len = 0;

    if(EVP_EncryptUpdate(ctx.get(),
                         reinterpret_cast<unsigned char *>(out.data()),
                         &out_len,
                         reinterpret_cast<const unsigned char *>(plain.data()),
                         static_cast<int>(plain.size()))
       != 1)
    {
        ADD_FAILURE() << "EVP_EncryptUpdate failed";
        return {};
    }

    if(EVP_EncryptFinal_ex(ctx.get(),
                           reinterpret_cast<unsigned char *>(out.data())
                               + out_len,
                           &final_len)
       != 1)
    {
        ADD_FAILURE() << "EVP_EncryptFinal_ex failed";
        return {};
    }

    out.resize(static_cast<std::size_t>(out_len + final_len));
    return out;
}

std::string calc_file_md5(const std::filesystem::path &path)
{
    std::ifstream in(path, std::ios::binary);
    if(!in)
        return {};

    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(
        EVP_MD_CTX_new(),
        EVP_MD_CTX_free);

    if(!ctx || EVP_DigestInit_ex(ctx.get(), EVP_md5(), nullptr) != 1)
        return {};

    std::array<unsigned char, 8192> buffer{};

    while(in)
    {
        in.read(reinterpret_cast<char *>(buffer.data()),
                static_cast<std::streamsize>(buffer.size()));

        const auto n = in.gcount();

        if(n > 0
           && EVP_DigestUpdate(ctx.get(),
                               buffer.data(),
                               static_cast<std::size_t>(n))
                  != 1)
        {
            return {};
        }
    }

    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int                               digest_len = 0;

    if(EVP_DigestFinal_ex(ctx.get(), digest.data(), &digest_len) != 1)
    {
        return {};
    }

    std::ostringstream out;

    for(unsigned int i = 0; i < digest_len; ++i)
    {
        out << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<unsigned int>(digest[i]);
    }

    return out.str();
}

} // namespace

/*
 * Security policy:
 *
 * Single DES is intentionally unavailable.
 * OpenSSL 3 places DES in the legacy provider; this test suite
 * verifies that the wrapper does NOT silently load that provider.
 */
TEST(des, single_des_is_unsupported)
{
    const std::string key   = "12345678";
    const std::string plain = "hello world";

    auto opt = make_options(key, hj::des::mode::ecb, hj::des::padding::pkcs7);

    std::string encrypted;

    EXPECT_EQ(hj::des::encrypt(encrypted, plain, opt),
              error_code::unsupported_algorithm);

    const std::string ciphertext(8, '\0');

    EXPECT_EQ(hj::des::decrypt(encrypted, ciphertext, opt),
              error_code::unsupported_algorithm);
}

TEST(des, encrypt)
{
    const std::string key = k3des_key;
    const std::string iv  = k_iv;

    /*
     * Known-answer test from the previous implementation:
     * 3-key TDEA / ECB / PKCS#5.
     */
    {
        auto opt =
            make_options(key, hj::des::mode::ecb, hj::des::padding::pkcs5);

        std::string encrypted;

        ASSERT_EQ(hj::des::encrypt(encrypted, "1", opt), error_code::ok);

        EXPECT_EQ(to_hex(encrypted), "40C3AD88D21D2BCA");
    }

    /*
     * Cross-validation against OpenSSL EVP.
     */
    {
        auto opt =
            make_options(key, hj::des::mode::cbc, hj::des::padding::pkcs7, iv);

        const std::string plain = "hello world";

        std::string actual;

        ASSERT_EQ(hj::des::encrypt(actual, plain, opt), error_code::ok);

        const std::string expected =
            openssl_encrypt(key, iv, hj::des::mode::cbc, plain, true);

        EXPECT_EQ(actual, expected);
    }

    /*
     * All supported padding schemes must round-trip.
     * ISO10126 is intentionally not compared byte-for-byte because
     * its padding bytes are random.
     */
    for(auto pad : {hj::des::padding::pkcs5,
                    hj::des::padding::pkcs7,
                    hj::des::padding::zero,
                    hj::des::padding::iso10126,
                    hj::des::padding::ansix923,
                    hj::des::padding::iso_iec_7816_4})
    {
        auto opt = make_options(key, hj::des::mode::cbc, pad, iv);

        const std::string plain = "hello world 1234";
        std::string       encrypted;

        ASSERT_EQ(hj::des::encrypt(encrypted, plain, opt), error_code::ok);

        ASSERT_FALSE(encrypted.empty());
        ASSERT_EQ(encrypted.size() % hj::des::block_size, 0U);

        std::string decrypted;

        ASSERT_EQ(hj::des::decrypt(decrypted, encrypted, opt), error_code::ok);

        EXPECT_EQ(decrypted, plain);
    }
}

TEST(des, encrypt_n)
{
    const std::string key = k3des_key;
    const std::string iv  = k_iv;

    /*
     * 2-key TDEA must also work through EVP.
     */
    {
        auto opt = make_options(std::string(k2des_key),
                                hj::des::mode::ecb,
                                hj::des::padding::pkcs5);

        std::string encrypted;

        ASSERT_EQ(hj::des::encrypt(encrypted, "1", opt), error_code::ok);

        // EXPECT_EQ(to_hex(encrypted), "E60BC2FCA8AB2AEC");
    }

    /*
     * no_padding contract:
     * the public API requires complete DES blocks.
     */
    for(auto mode : {hj::des::mode::ecb,
                     hj::des::mode::cbc,
                     hj::des::mode::cfb,
                     hj::des::mode::ofb,
                     hj::des::mode::ctr})
    {
        auto opt = make_options(key, mode, hj::des::padding::no_padding, iv);

        std::string encrypted;

        EXPECT_EQ(hj::des::encrypt(encrypted, "1234567812345678", opt),
                  error_code::ok);

        EXPECT_EQ(encrypted.size(), 16U);

        encrypted.clear();

        EXPECT_EQ(hj::des::encrypt(encrypted, "123456789", opt),
                  error_code::invalid_plain);
    }

    /*
     * Buffer-too-small must be reported before writing past dst.
     */
    {
        auto opt =
            make_options(key, hj::des::mode::ecb, hj::des::padding::pkcs7);

        const std::string            plain = "hello world";
        std::array<unsigned char, 8> out{};
        std::size_t                  out_len = 0;

        EXPECT_EQ(hj::des::encrypt(
                      out.data(),
                      out.size(),
                      out_len,
                      reinterpret_cast<const unsigned char *>(plain.data()),
                      plain.size(),
                      opt),
                  error_code::buffer_too_small);

        EXPECT_EQ(out_len, 0U);
    }
}

TEST(des, encrypt_2key_tdea)
{
    const std::string key = k2des_key;

    auto opt = make_options(key, hj::des::mode::ecb, hj::des::padding::pkcs5);

    std::string actual;

    ASSERT_EQ(hj::des::encrypt(actual, "1", opt), error_code::ok);

    EXPECT_EQ(to_hex(actual), "2AAE73C0284C9B06");

    // Cross-check against OpenSSL EVP.
    const auto expected =
        openssl_encrypt(key, "", hj::des::mode::ecb, "1", true);

    EXPECT_EQ(actual, expected);
}

TEST(des, decrypt)
{
    const std::string key = k3des_key;
    const std::string iv  = k_iv;

    /*
     * Known-answer decrypt.
     */
    {
        auto opt =
            make_options(key, hj::des::mode::ecb, hj::des::padding::pkcs5);

        const std::string encrypted = from_hex("40C3AD88D21D2BCA");

        std::string plain;

        ASSERT_EQ(hj::des::decrypt(plain, encrypted, opt), error_code::ok);

        EXPECT_EQ(plain, "1");
    }

    /*
     * Every mode must round-trip.
     */
    for(auto mode : {hj::des::mode::ecb,
                     hj::des::mode::cbc,
                     hj::des::mode::cfb,
                     hj::des::mode::ofb,
                     hj::des::mode::ctr})
    {
        auto opt = make_options(key, mode, hj::des::padding::pkcs7, iv);

        const std::string plain = "hello world";
        std::string       encrypted;
        std::string       decrypted;

        ASSERT_EQ(hj::des::encrypt(encrypted, plain, opt), error_code::ok);

        ASSERT_EQ(hj::des::decrypt(decrypted, encrypted, opt), error_code::ok);

        EXPECT_EQ(decrypted, plain);
    }
}

TEST(des, decrypt_n)
{
    const std::string key = k3des_key;
    const std::string iv  = k_iv;

    /*
     * Invalid ciphertext length must never reach EVP.
     */
    {
        auto opt =
            make_options(key, hj::des::mode::cbc, hj::des::padding::pkcs7, iv);

        std::string plain;

        EXPECT_EQ(hj::des::decrypt(plain, std::string("1234567"), opt),
                  error_code::invalid_input);
    }

    /*
     * Invalid padding must be rejected.
     */
    {
        auto opt =
            make_options(key, hj::des::mode::cbc, hj::des::padding::pkcs7, iv);

        std::string encrypted;

        ASSERT_EQ(hj::des::encrypt(encrypted, "hello world", opt),
                  error_code::ok);

        ASSERT_FALSE(encrypted.empty());

        encrypted.back() ^= 0x01;

        std::string decrypted;

        EXPECT_EQ(hj::des::decrypt(decrypted, encrypted, opt),
                  error_code::invalid_padding);

        EXPECT_TRUE(decrypted.empty());
    }

    /*
     * Raw buffer decrypt API must honor dst capacity.
     */
    {
        auto opt =
            make_options(key, hj::des::mode::ecb, hj::des::padding::pkcs7);

        const std::string encrypted = from_hex("40C3AD88D21D2BCA");

        std::array<unsigned char, 0> output{};
        std::size_t                  output_len = 0;

        EXPECT_EQ(hj::des::decrypt(
                      output.data(),
                      0,
                      output_len,
                      reinterpret_cast<const unsigned char *>(encrypted.data()),
                      encrypted.size(),
                      opt),
                  error_code::buffer_too_small);

        EXPECT_EQ(output_len, 0U);
    }
}

TEST(des, encrypt_file)
{
    const auto base = std::filesystem::temp_directory_path() / "hj_des_test";

    const auto plain_path  = base.string() + "_plain.bin";
    const auto cipher_path = base.string() + "_cipher.bin";

    const std::string plain = "high-jump DES EVP test\n"
                              "binary:\x00\x01\x02\x03\xFE\xFF\n";

    {
        std::ofstream out(plain_path, std::ios::binary);
        ASSERT_TRUE(out);
        out.write(plain.data(), static_cast<std::streamsize>(plain.size()));
        ASSERT_TRUE(out);
    }

    const std::string key = k3des_key;
    auto              opt =
        make_options(key, hj::des::mode::cbc, hj::des::padding::pkcs7, k_iv);

    ASSERT_EQ(hj::des::encrypt_file(cipher_path, plain_path, opt),
              error_code::ok);

    ASSERT_TRUE(std::filesystem::exists(cipher_path));
    ASSERT_GT(std::filesystem::file_size(cipher_path), 0U);

    std::filesystem::remove(plain_path);
    std::filesystem::remove(cipher_path);
}

TEST(des, decrypt_file)
{
    const auto base =
        std::filesystem::temp_directory_path() / "hj_des_test_decrypt";

    const auto plain_path  = base.string() + "_plain.bin";
    const auto cipher_path = base.string() + "_cipher.bin";
    const auto output_path = base.string() + "_output.bin";

    const std::string plain = "high-jump DES EVP file round-trip\n"
                              "0123456789ABCDEF\n";

    {
        std::ofstream out(plain_path, std::ios::binary);
        ASSERT_TRUE(out);
        out.write(plain.data(), static_cast<std::streamsize>(plain.size()));
        ASSERT_TRUE(out);
    }

    const std::string key = k3des_key;
    auto              opt =
        make_options(key, hj::des::mode::ctr, hj::des::padding::pkcs7, k_iv);

    ASSERT_EQ(hj::des::encrypt_file(cipher_path, plain_path, opt),
              error_code::ok);

    ASSERT_EQ(hj::des::decrypt_file(output_path, cipher_path, opt),
              error_code::ok);

    EXPECT_EQ(calc_file_md5(plain_path), calc_file_md5(output_path));

    std::filesystem::remove(plain_path);
    std::filesystem::remove(cipher_path);
    std::filesystem::remove(output_path);
}

TEST(des, invalid_options)
{
    const std::string key = k3des_key;

    {
        auto opt = make_options(std::string("short"),
                                hj::des::mode::ecb,
                                hj::des::padding::pkcs7);

        std::string encrypted;

        EXPECT_EQ(hj::des::encrypt(encrypted, "hello", opt),
                  error_code::invalid_key);
    }

    {
        auto opt = make_options(key,
                                hj::des::mode::cbc,
                                hj::des::padding::pkcs7,
                                "short");

        std::string encrypted;

        EXPECT_EQ(hj::des::encrypt(encrypted, "hello", opt),
                  error_code::invalid_iv);
    }

    {
        hj::des::options opt;
        opt.key       = reinterpret_cast<const unsigned char *>(key.data());
        opt.key_len   = key.size();
        opt.mod       = hj::des::mode::ecb;
        opt.pad_style = hj::des::padding::pkcs7;
        opt.iv        = reinterpret_cast<const unsigned char *>(k_iv);
        opt.iv_len    = 8;

        std::string encrypted;

        EXPECT_EQ(hj::des::encrypt(encrypted, "hello", opt),
                  error_code::invalid_iv);
    }
}

TEST(des, provider_policy)
{
    /*
     * The wrapper itself must never turn single DES into a supported
     * algorithm. This remains true even if an application separately
     * configures OpenSSL providers.
     */
    const std::string key = "12345678";
    auto opt = make_options(key, hj::des::mode::ecb, hj::des::padding::pkcs7);

    std::string encrypted;

    EXPECT_EQ(hj::des::encrypt(encrypted, "hello", opt),
              error_code::unsupported_algorithm);
}
