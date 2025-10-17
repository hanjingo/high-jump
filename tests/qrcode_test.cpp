#include <gtest/gtest.h>
#include <hj/ai/qrcode.hpp>
#include <filesystem>
#include <cstdio>

namespace fs = std::filesystem;

using namespace hj;

TEST(qrcode, encode_decode_memory)
{
    qrcode::bitmap    bm;
    const std::string payload = "hello world";

    auto ec = qrcode::builder::encode(bm, payload);
    ASSERT_EQ(ec.value(), 0) << "encode failed: " << ec.message();
    ASSERT_GT(bm.data.size(), 0u);

    std::vector<std::string> results;
    ec = qrcode::parser::decode(results, bm);
    ASSERT_EQ(ec.value(), 0) << "decode failed: " << ec.message();
    ASSERT_FALSE(results.empty());
    EXPECT_EQ(results.front(), payload);
}

TEST(qrcode, encode_decode_pgm_file)
{
    qrcode::bitmap    bm;
    const std::string payload = "testing-pgm-123";

    auto ec = qrcode::builder::encode(bm, payload);
    ASSERT_EQ(ec.value(), 0) << "encode failed: " << ec.message();

    // write to temp file
    fs::path    tmp  = fs::temp_directory_path() / "hj_qrcode_test.pgm";
    std::string path = tmp.u8string();

    ec = qrcode::builder::encode(path, payload);
    ASSERT_EQ(ec.value(), 0) << "pgm write failed: " << ec.message();

    std::vector<std::string> results;
    ec = qrcode::parser::decode(results, path);
    ASSERT_EQ(ec.value(), 0) << "decode from file failed: " << ec.message();
    ASSERT_FALSE(results.empty());
    EXPECT_EQ(results.front(), payload);

    // cleanup
    std::remove(path.c_str());
}

TEST(qrcode, encode_empty_text)
{
    qrcode::bitmap bm;
    auto           ec = qrcode::builder::encode(bm, std::string());
    // expect error code indicating invalid_text
    EXPECT_EQ(ec.value(), static_cast<int>(qrcode::error_code::invalid_text));
}

TEST(qrcode, decode_nonexistent_pgm)
{
    std::vector<std::string> results;
    auto                     ec = qrcode::parser::decode(
        results,
        std::string("C:/this/path/should/not/exist/hj_qrcode_missing.pgm"));
    EXPECT_EQ(ec.value(),
              static_cast<int>(qrcode::error_code::pgm_open_failed));
}

TEST(qrcode, decode_malformed_magic_pgm)
{
    // create a temporary file with wrong magic
    fs::path    tmp  = fs::temp_directory_path() / "hj_qrcode_bad_magic.pgm";
    std::string path = tmp.u8string();
    {
        std::ofstream ofs(path, std::ios::binary);
        ofs << "P2\n# not a P5 file\n1 1\n255\n";
        char pix = 0;
        ofs.write(&pix, 1);
    }

    std::vector<std::string> results;
    auto                     ec = qrcode::parser::decode(results, path);
    EXPECT_EQ(ec.value(),
              static_cast<int>(qrcode::error_code::magic_parse_failed));

    std::remove(path.c_str());
}

TEST(qrcode, decode_truncated_bitmap)
{
    qrcode::bitmap bm;
    bm.width  = 21;
    bm.height = 21;
    // intentionally smaller than width*height
    bm.data.resize(100);

    std::vector<std::string> results;
    auto                     ec = qrcode::parser::decode(results, bm);
    EXPECT_EQ(ec.value(),
              static_cast<int>(qrcode::error_code::bm_data_insufficient));
}

TEST(qrcode, encode_overflow_scale_margin)
{
    qrcode::bitmap    bm;
    const std::string payload = "x";

    // very large scale and margin to trigger overflow checks in encode
    const int big_scale  = 100000;
    const int big_margin = 100000;

    auto ec = qrcode::builder::encode(bm,
                                      payload,
                                      0,
                                      QR_ECLEVEL_L,
                                      big_scale,
                                      big_margin);
    EXPECT_EQ(ec.value(), static_cast<int>(qrcode::error_code::encode_failed));
}