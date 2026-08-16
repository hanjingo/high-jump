#include <gtest/gtest.h>
#include <hj/crypto/sha.hpp>
#include <openssl/evp.h>

#include <array>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <string>
#include <string_view>

namespace
{
std::string to_base64(const std::string &binary)
{
    if(binary.empty())
        return "";

    std::size_t encoded_len = 4 * ((binary.size() + 2) / 3);
    std::string result;
    result.resize(encoded_len);

    int ret =
        EVP_EncodeBlock(reinterpret_cast<unsigned char *>(result.data()),
                        reinterpret_cast<const unsigned char *>(binary.data()),
                        static_cast<int>(binary.size()));

    if(ret < 0)
        return "";

    return result;
}

std::string to_hex(std::string_view binary)
{
    std::ostringstream hex;
    for(unsigned char c : binary)
        hex << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(c);
    return hex.str();
}
} // namespace

TEST(sha256, encode_string)
{
    auto        ok = hj::sha::error_code::ok;
    std::string digest;

    ASSERT_EQ(hj::sha::sha256(digest, "123456"), ok);
    ASSERT_EQ(
        to_hex(digest),
        "8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92");

    const unsigned char src[] = {'a', 'b', 'c'};
    unsigned char       dst[64];
    std::size_t         dst_len = sizeof(dst);

    ASSERT_EQ(hj::sha::encode(dst, dst_len, src, 3, hj::sha::algorithm::sha256),
              ok);
    ASSERT_EQ(dst_len, 32);

    std::string sha256_encoded(reinterpret_cast<char *>(dst), dst_len);
    std::string b64_encoded = to_base64(sha256_encoded);
    ASSERT_STREQ(b64_encoded.c_str(),
                 "ungWv48Bz+pBQUDeXa4iI7ADYaOWF3qctBD/YfIAFa0=");
}

TEST(sha_all_algos, verify_known_vectors)
{
    std::string dst;
    ASSERT_EQ(hj::sha::sha1(dst, "abc"), hj::sha::error_code::ok);
    ASSERT_EQ(to_hex(dst), "a9993e364706816aba3e25717850c26c9cd0d89d");

    ASSERT_EQ(hj::sha::sha512(dst, "abc"), hj::sha::error_code::ok);
    ASSERT_EQ(
        to_hex(dst),
        "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a"
        "2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");
}

TEST(sha, stream_encode)
{
    std::stringstream  in("123456");
    std::ostringstream out;

    ASSERT_EQ(hj::sha::encode(out, in, hj::sha::algorithm::sha256),
              hj::sha::error_code::ok);
    std::string digest = out.str();
    ASSERT_EQ(digest.size(), 32);
    ASSERT_EQ(
        to_hex(digest),
        "8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92");
}

TEST(sha, hash_array_type)
{
    hj::hash_array<32> dst_array;
    ASSERT_EQ(hj::sha::encode(dst_array, "123456", hj::sha::algorithm::sha256),
              hj::sha::error_code::ok);

    std::string_view sv(reinterpret_cast<const char *>(dst_array.data()),
                        dst_array.size());
    ASSERT_EQ(
        to_hex(sv),
        "8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92");
}

TEST(sha256, encode_file)
{
    auto        ok          = hj::sha::error_code::ok;
    const char *input_file  = "sha_test_input.txt";
    const char *output_file = "sha_test_output.bin";

    {
        std::ofstream ofs(input_file, std::ios::binary);
        if(!ofs.is_open())
            GTEST_SKIP() << "Failed to create test input file.";

        ofs << "123456";
    }

    std::remove(output_file);
    ASSERT_EQ(hj::sha::encode_file(output_file,
                                   input_file,
                                   hj::sha::algorithm::sha256),
              ok);

    std::ifstream ifs(output_file, std::ios::binary);
    std::string   sha_bin((std::istreambuf_iterator<char>(ifs)),
                          std::istreambuf_iterator<char>());
    ASSERT_EQ(sha_bin.size(), 32);
    ASSERT_EQ(
        to_hex(sha_bin),
        "8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92");

    std::remove(input_file);
    std::remove(output_file);
}

TEST(sha_error_handling, null_pointer_inputs)
{
    unsigned char       dst[32];
    std::size_t         dst_len = sizeof(dst);
    const unsigned char src[]   = "test";

    ASSERT_EQ(hj::sha::encode(nullptr, dst_len, src, 4),
              hj::sha::error_code::invalid_output);
    ASSERT_EQ(hj::sha::encode(dst, dst_len, nullptr, 4),
              hj::sha::error_code::invalid_input);

    hj::sha::hasher h;
    ASSERT_EQ(h.update(nullptr, 10), hj::sha::error_code::invalid_input);
    ASSERT_EQ(h.final(nullptr, dst_len), hj::sha::error_code::invalid_output);

    ASSERT_EQ(hj::sha::encode_file(nullptr, "valid.txt"),
              hj::sha::error_code::invalid_input);
    ASSERT_EQ(hj::sha::encode_file("valid.txt", nullptr),
              hj::sha::error_code::invalid_input);
}

TEST(sha_error_handling, buffer_too_small)
{
    const unsigned char src[] = "123456";
    unsigned char       dst[10];
    std::size_t         dst_len = sizeof(dst);

    ASSERT_EQ(hj::sha::encode(dst, dst_len, src, 6, hj::sha::algorithm::sha256),
              hj::sha::error_code::buffer_too_small);

    hj::sha::hasher h(hj::sha::algorithm::sha256);
    h.update("123456");
    ASSERT_EQ(h.final(dst, dst_len), hj::sha::error_code::buffer_too_small);
}

TEST(sha_error_handling, file_and_stream_errors)
{
    ASSERT_EQ(
        hj::sha::encode_file("out.bin", "non_existent_input_file_xyz.txt"),
        hj::sha::error_code::invalid_input);

    const char *valid_in = "sha_err_test_in.txt";
    {
        std::ofstream ofs(valid_in);
        ofs << "data";
    }

    ASSERT_EQ(
        hj::sha::encode_file("/non_existent_directory_xxx/out.bin", valid_in),
        hj::sha::error_code::invalid_output);

    std::remove(valid_in);

    std::stringstream bad_in;
    bad_in.setstate(std::ios::badbit);
    std::string dst_str;
    ASSERT_EQ(hj::sha::encode(dst_str, bad_in),
              hj::sha::error_code::invalid_input);

    std::stringstream  good_in("123456");
    std::ostringstream bad_out;
    bad_out.setstate(std::ios::badbit);
    ASSERT_EQ(hj::sha::encode(bad_out, good_in),
              hj::sha::error_code::invalid_output);
}

TEST(sha_hasher, lifecycle_and_is_valid)
{
    hj::sha::hasher h1(hj::sha::algorithm::sha256);
    ASSERT_TRUE(h1.is_valid());

    hj::sha::hasher h2(std::move(h1));
    ASSERT_FALSE(h1.is_valid());

    ASSERT_EQ(h1.update("test"), hj::sha::error_code::openssl_internal_error);

    std::size_t out_len = 32;
    uint8_t     buf[32];
    ASSERT_EQ(h1.final(buf, out_len),
              hj::sha::error_code::openssl_internal_error);

    ASSERT_EQ(h1.reset(), hj::sha::error_code::ok);
    ASSERT_TRUE(h1.is_valid());
    ASSERT_EQ(h1.update("test"), hj::sha::error_code::ok);

    std::string digest1, digest2;
    ASSERT_EQ(h2.update("123456"), hj::sha::error_code::ok);
    ASSERT_EQ(h2.final(digest1), hj::sha::error_code::ok);

    ASSERT_FALSE(h2.is_valid());
    ASSERT_TRUE(h2.is_finished());
    ASSERT_EQ(h2.update("more"), hj::sha::error_code::invalid_input);
    ASSERT_EQ(h2.final(digest2), hj::sha::error_code::invalid_input);

    ASSERT_EQ(h2.reset(), hj::sha::error_code::ok);
    ASSERT_TRUE(h2.is_valid());
    ASSERT_FALSE(h2.is_finished());
    ASSERT_EQ(h2.update("123456"), hj::sha::error_code::ok);
    ASSERT_EQ(h2.final(digest2), hj::sha::error_code::ok);
    ASSERT_EQ(digest1, digest2);
}