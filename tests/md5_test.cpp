
#include <gtest/gtest.h>
#include <hj/crypto/md5.hpp>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

namespace
{

struct rfc_test_vector
{
    const char *input;
    const char *expected_hex;
};

constexpr rfc_test_vector g_rfc_vectors[] = {
    {"", "d41d8cd98f00b204e9800998ecf8427e"},
    {"a", "0cc175b9c0f1b6a831c399e269772661"},
    {"abc", "900150983cd24fb0d6963f7d28e17f72"},
    {"message digest", "f96b697d7cb7938d525a2f31aaf161d0"},
    {"abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b"},
    {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
     "d174ab98d277d9f5a5611c2c9f419d9f"},
    {"123456789012345678901234567890123456789012345678901234567890123456789012"
     "34567890",
     "57edf4a22be3c955ac49da2e2107b67a"}};

bool create_test_file(const std::string &path, const std::string &content)
{
    std::ofstream ofs(path, std::ios::binary);
    if(!ofs.is_open())
        return false;
    ofs << content;
    return true;
}

std::string to_upper(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return str;
}

} // namespace


TEST(md5, rfc1321_vectors)
{
    for(const auto &tv : g_rfc_vectors)
    {
        hj::md5::digest_type digest{};
        ASSERT_EQ(hj::md5::encode(digest, tv.input), hj::md5::error_code::ok);
        EXPECT_EQ(hj::md5::to_hex(digest), tv.expected_hex);
        EXPECT_EQ(hj::md5::to_hex(digest, true), to_upper(tv.expected_hex));

        std::string hex_out;
        ASSERT_EQ(hj::md5::encode_hex(hex_out, tv.input),
                  hj::md5::error_code::ok);
        EXPECT_EQ(hex_out, tv.expected_hex);

        ASSERT_EQ(hj::md5::encode_hex(hex_out, tv.input, true),
                  hj::md5::error_code::ok);
        EXPECT_EQ(hex_out, to_upper(tv.expected_hex));

        std::istringstream in(tv.input);
        ASSERT_EQ(hj::md5::encode(digest, in), hj::md5::error_code::ok);
        EXPECT_EQ(hj::md5::to_hex(digest), tv.expected_hex);

        std::istringstream in_hex(tv.input);
        ASSERT_EQ(hj::md5::encode_hex(hex_out, in_hex),
                  hj::md5::error_code::ok);
        EXPECT_EQ(hex_out, tv.expected_hex);
    }
}

TEST(md5, large_buffer_boundary)
{
    const std::vector<std::size_t> test_sizes = {hj::md5::buf_sz - 1,
                                                 hj::md5::buf_sz,
                                                 hj::md5::buf_sz + 100,
                                                 hj::md5::buf_sz * 2 + 1234};

    for(std::size_t size : test_sizes)
    {
        std::string large_data(size, 'a');

        hj::md5::digest_type expected_digest{};
        ASSERT_EQ(hj::md5::encode(expected_digest, large_data),
                  hj::md5::error_code::ok);

        std::istringstream   iss(large_data);
        hj::md5::digest_type stream_digest{};
        ASSERT_EQ(hj::md5::encode(stream_digest, iss), hj::md5::error_code::ok);

        EXPECT_EQ(expected_digest, stream_digest);
    }
}

TEST(md5, buffer_too_small_error)
{
    uint8_t       dst[10];
    std::size_t   dst_len = sizeof(dst);
    const uint8_t src[]   = "test_data";

    auto ec = hj::md5::encode(dst, dst_len, src, sizeof(src) - 1);
    EXPECT_EQ(ec, hj::md5::error_code::buffer_too_small);
}

TEST(md5, stream_errors)
{
    hj::md5::digest_type digest{};
    std::string          hex_out;

    std::istringstream bad_in("test");
    bad_in.setstate(std::ios::failbit);

    EXPECT_EQ(hj::md5::encode(digest, bad_in),
              hj::md5::error_code::invalid_input);
    EXPECT_EQ(hj::md5::encode_hex(hex_out, bad_in),
              hj::md5::error_code::invalid_input);

    std::istringstream good_in("test");
    std::ostringstream bad_out;
    bad_out.setstate(std::ios::failbit);

    EXPECT_EQ(hj::md5::encode(bad_out, good_in),
              hj::md5::error_code::invalid_output);
}

TEST(md5, file_errors)
{
    const std::string non_existent_src = "definitely_not_exist_file_12345.bin";
    const std::string dst_file         = "md5_file_test_out.bin";
    std::remove(dst_file.c_str());

    EXPECT_EQ(hj::md5::encode_file(dst_file, non_existent_src),
              hj::md5::error_code::invalid_input);

    const std::string valid_src = "md5_file_test_in.txt";
    ASSERT_TRUE(create_test_file(valid_src, "sample content"));

    const std::string invalid_dst = "non_existent_folder_xyz123/output.bin";
    EXPECT_EQ(hj::md5::encode_file(invalid_dst, valid_src),
              hj::md5::error_code::invalid_output);

    std::remove(valid_src.c_str());
}

TEST(md5, encode_file_and_ostream)
{
    const std::string input_file  = "md5_test_file_in.txt";
    const std::string output_file = "md5_test_file_out.bin";
    const std::string content     = "hello world";

    ASSERT_TRUE(create_test_file(input_file, content));

    std::remove(output_file.c_str());
    ASSERT_EQ(hj::md5::encode_file(output_file, input_file),
              hj::md5::error_code::ok);

    std::ifstream        ifs(output_file, std::ios::binary);
    hj::md5::digest_type file_digest{};
    ifs.read(reinterpret_cast<char *>(file_digest.data()),
             hj::md5::digest_length);
    ASSERT_EQ(ifs.gcount(), hj::md5::digest_length);

    EXPECT_EQ(hj::md5::to_hex(file_digest), "5eb63bbbe01eeed093cb22bb8f5acdc3");

    ifs.close();
    std::remove(input_file.c_str());
    std::remove(output_file.c_str());
}

TEST(md5, helper_and_utilities)
{
    EXPECT_EQ(hj::md5::encode_len_reserve(), hj::md5::digest_length);

    auto ctx = hj::md5::make_ctx();
    EXPECT_NE(ctx, nullptr);
}

TEST(md5, system_error_integration)
{
    std::error_code ec_ok          = hj::md5::error_code::ok;
    std::error_code ec_invalid_in  = hj::md5::error_code::invalid_input;
    std::error_code ec_invalid_out = hj::md5::error_code::invalid_output;
    std::error_code ec_buf_small   = hj::md5::error_code::buffer_too_small;
    std::error_code ec_crypto_err  = hj::md5::error_code::crypto_error;
    std::error_code ec_unknown     = hj::md5::error_code::unknown;

    EXPECT_FALSE(ec_ok);
    EXPECT_TRUE(ec_invalid_in);

    EXPECT_STREQ(ec_invalid_in.category().name(), "hj::md5");
    EXPECT_EQ(ec_ok.message(), "Success");
    EXPECT_EQ(ec_invalid_in.message(), "Invalid input stream or source");
    EXPECT_EQ(ec_invalid_out.message(), "Invalid output stream or destination");
    EXPECT_EQ(ec_buf_small.message(), "Destination buffer too small");
    EXPECT_EQ(ec_crypto_err.message(), "OpenSSL EVP crypto operation failed");
    EXPECT_EQ(ec_unknown.message(), "Unknown error");
}