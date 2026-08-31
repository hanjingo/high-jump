#include <gtest/gtest.h>
#include <hj/ai/qrcode.hpp>
#include <filesystem>
#include <fstream>
#include <cstdio>
#include <set>

namespace fs = std::filesystem;

using namespace hj::qrcode;

TEST(qrcode, encode_decode_memory)
{
    bitmap            bm;
    const std::string payload = "hello world";

    auto ec = builder::encode(bm, payload);
    ASSERT_FALSE(ec) << "encode failed: " << ec.message();
    ASSERT_GT(bm.data().size(), 0u);

    std::vector<std::string> results;
    ec = parser::decode(results, bm);
    ASSERT_FALSE(ec) << "decode failed: " << ec.message();
    ASSERT_FALSE(results.empty());
    EXPECT_EQ(results.front(), payload);
}

TEST(qrcode, encode_decode_pgm_file)
{
    bitmap            bm;
    const std::string payload = "testing-pgm-123";

    auto ec = builder::encode(bm, payload);
    ASSERT_FALSE(ec) << "encode failed: " << ec.message();

    fs::path tmp = fs::temp_directory_path() / "hj_qrcode_test.pgm";

    ec = builder::encode(tmp, payload);
    ASSERT_FALSE(ec) << "pgm write failed: " << ec.message();

    std::vector<std::string> results;
    ec = parser::decode(results, tmp);
    ASSERT_FALSE(ec) << "decode from file failed: " << ec.message();
    ASSERT_FALSE(results.empty());
    EXPECT_EQ(results.front(), payload);

    std::error_code ignore_ec;
    fs::remove(tmp, ignore_ec);
}

TEST(qrcode, encode_empty_text)
{
    bitmap bm;
    auto   ec = builder::encode(bm, std::string());
    EXPECT_EQ(ec, error_code::invalid_text);
}

TEST(qrcode, decode_nonexistent_pgm)
{
    std::vector<std::string> results;

    fs::path nonexistent_path = fs::temp_directory_path()
                                / "hj_qrcode_definitely_nonexistent_12345.pgm";

    std::error_code ignore_ec;
    fs::remove(nonexistent_path, ignore_ec);

    auto ec = parser::decode(results, nonexistent_path);
    EXPECT_EQ(ec, error_code::pgm_open_failed);
}

TEST(qrcode, decode_malformed_magic_pgm)
{
    fs::path tmp = fs::temp_directory_path() / "hj_qrcode_bad_magic.pgm";
    {
        std::ofstream ofs(tmp, std::ios::binary);
        ofs << "P2\n# not a P5 file\n1 1\n255\n";
        char pix = 0;
        ofs.write(&pix, 1);
    }

    std::vector<std::string> results;
    auto                     ec = parser::decode(results, tmp);
    EXPECT_EQ(ec, error_code::magic_parse_failed);

    std::error_code ignore_ec;
    fs::remove(tmp, ignore_ec);
}

TEST(qrcode, decode_truncated_bitmap)
{
    bitmap bm;
    bm.resize(21, 21);
    bm.data().resize(100);

    std::vector<std::string> results;
    auto                     ec = parser::decode(results, bm);
    EXPECT_EQ(ec, error_code::invalid_bitmap);
}

TEST(qrcode, encode_overflow_scale_margin)
{
    bitmap            bm;
    const std::string payload = "x";

    const int big_scale  = 100000;
    const int big_margin = 100000;

    auto ec =
        builder::encode(bm, payload, 0, QR_ECLEVEL_L, big_scale, big_margin);
    EXPECT_EQ(ec, error_code::encode_failed);
}

TEST(qrcode_parameter_test, invalid_scale)
{
    bitmap bm;
    auto   ec = builder::encode(bm, "hello", 0, QR_ECLEVEL_L, 0, 4);
    EXPECT_EQ(ec, error_code::invalid_scale);

    ec = builder::encode(bm, "hello", 0, QR_ECLEVEL_L, -1, 4);
    EXPECT_EQ(ec, error_code::invalid_scale);
}

TEST(qrcode_parameter_test, invalid_margin)
{
    bitmap bm;
    auto   ec = builder::encode(bm, "hello", 0, QR_ECLEVEL_L, 4, -1);
    EXPECT_EQ(ec, error_code::invalid_margin);

    ec = builder::encode(bm, "hello", 0, QR_ECLEVEL_L, 4, 0);
    EXPECT_FALSE(ec) << ec.message();
}

TEST(qrcode_parameter_test, invalid_version)
{
    bitmap bm;
    auto   ec = builder::encode(bm, "hello", 41, QR_ECLEVEL_L);
    EXPECT_EQ(ec, error_code::encode_failed);

    ec = builder::encode(bm, "hello", -1, QR_ECLEVEL_L);
    EXPECT_EQ(ec, error_code::encode_failed);
}

TEST(qrcode_parameter_test, invalid_ec_level)
{
    bitmap bm;
    auto   ec = builder::encode(bm, "hello", 0, 999);
    EXPECT_EQ(ec, error_code::encode_failed);
}

TEST(qrcode_payload_test, various_sizes)
{
    const std::vector<size_t> sizes = {1, 10, 100, 500, 800};

    for(size_t sz : sizes)
    {
        std::string payload(sz, 'a');
        bitmap      bm;
        auto        ec = builder::encode(bm, payload);
        ASSERT_FALSE(ec) << "Failed to encode size " << sz << ": "
                         << ec.message();

        std::vector<std::string> results;
        ec = parser::decode(results, bm);
        ASSERT_FALSE(ec) << "Failed to decode size " << sz << ": "
                         << ec.message();
        ASSERT_EQ(results.size(), 1u);
        EXPECT_EQ(results.front(), payload);
    }
}

TEST(qrcode_payload_test, max_qr_payload_boundary)
{
    bitmap bm;

    const size_t max_bytes = 884;
    std::string  max_payload(max_bytes, 'Z');

    auto ec = builder::encode(bm, max_payload, 0, QR_ECLEVEL_L);
    ASSERT_FALSE(ec) << "Failed to encode max payload: " << ec.message();

    std::vector<std::string> results;
    ec = parser::decode(results, bm);
    ASSERT_FALSE(ec) << "Failed to decode max payload: " << ec.message();
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results.front(), max_payload);
}

TEST(qrcode_version_test, explicit_versions)
{
    const std::vector<int> versions = {1, 2, 10, 20};

    for(int ver : versions)
    {
        bitmap      bm;
        std::string payload = "test_version_" + std::to_string(ver);

        int scale  = 4;
        int margin = 4;

        auto ec =
            builder::encode(bm, payload, ver, QR_ECLEVEL_L, scale, margin);
        ASSERT_FALSE(ec) << "Failed at version " << ver << ": " << ec.message();

        std::vector<std::string> results;
        ec = parser::decode(results, bm);
        ASSERT_FALSE(ec) << "Failed to decode version " << ver << ": "
                         << ec.message();
        ASSERT_EQ(results.size(), 1u);
        EXPECT_EQ(results.front(), payload);
    }
}

TEST(qrcode_ec_level_test, all_levels)
{
    const std::vector<int> levels = {QR_ECLEVEL_L,
                                     QR_ECLEVEL_M,
                                     QR_ECLEVEL_Q,
                                     QR_ECLEVEL_H};

    for(int level : levels)
    {
        bitmap      bm;
        std::string payload = "ec_level_test_" + std::to_string(level);

        auto ec = builder::encode(bm, payload, 0, level);
        ASSERT_FALSE(ec) << "Failed at ec_level " << level << ": "
                         << ec.message();

        std::vector<std::string> results;
        ec = parser::decode(results, bm);
        ASSERT_FALSE(ec) << "Failed to decode ec_level " << level << ": "
                         << ec.message();
        ASSERT_EQ(results.size(), 1u);
        EXPECT_EQ(results.front(), payload);
    }
}

TEST(qrcode_utf8_binary_test, complex_utf8_strings)
{
    const std::vector<std::string> test_strings = {
        "你好，世界",
        "hello\nworld\r\nwith\ttabs",
        "中文 🚀 Emoji Test 👾",
        "日本語のテスト - 🈲 - 123",
        std::string("\x00\x01\x02\xFF\xFE\xFD", 6)};

    for(const auto &payload : test_strings)
    {
        bitmap bm;
        auto   ec = builder::encode(bm, payload);
        ASSERT_FALSE(ec) << ec.message();

        std::vector<std::string> results;
        ec = parser::decode(results, bm);
        ASSERT_FALSE(ec) << ec.message();
        ASSERT_EQ(results.size(), 1u);
        EXPECT_EQ(results.front(), payload);
    }
}

TEST(qrcode_multi_decode_test, multiple_qrcodes_in_single_image)
{
    bitmap            bm1, bm2;
    const std::string text1 = "QR_CODE_1_TOP_LEFT";
    const std::string text2 = "QR_CODE_2_BOTTOM_RIGHT";

    ASSERT_FALSE(builder::encode(bm1, text1, 0, QR_ECLEVEL_L, 3, 2));
    ASSERT_FALSE(builder::encode(bm2, text2, 0, QR_ECLEVEL_L, 3, 2));

    const int gap      = 30;
    const int canvas_w = bm1.width() + bm2.width() + gap;
    const int canvas_h = std::max(bm1.height(), bm2.height()) + gap;

    bitmap canvas(canvas_w, canvas_h, 255);
    ASSERT_TRUE(canvas.is_valid());

    for(int r = 0; r < bm1.height(); ++r)
    {
        for(int c = 0; c < bm1.width(); ++c)
        {
            canvas.data()[r * canvas_w + c] = bm1.data()[r * bm1.width() + c];
        }
    }

    const int offset_x = bm1.width() + gap;
    const int offset_y = gap;
    for(int r = 0; r < bm2.height(); ++r)
    {
        for(int c = 0; c < bm2.width(); ++c)
        {
            const int dst_r = offset_y + r;
            const int dst_c = offset_x + c;
            canvas.data()[dst_r * canvas_w + dst_c] =
                bm2.data()[r * bm2.width() + c];
        }
    }

    std::vector<std::string> results;
    auto                     ec = parser::decode(results, canvas);

    ASSERT_FALSE(ec) << "Failed to decode canvas containing multiple QRs: "
                     << ec.message();
    ASSERT_EQ(results.size(), 2u);

    std::set<std::string> detected_set(results.begin(), results.end());
    EXPECT_TRUE(detected_set.count(text1));
    EXPECT_TRUE(detected_set.count(text2));
}

TEST(qrcode_file_system_test, path_based_file_io)
{
    fs::path tmp_path         = fs::temp_directory_path() / "hj_qr_fs_test.pgm";
    const std::string payload = "FileSystem Path Integration Test 🚀";

    auto ec = builder::encode(tmp_path, payload);
    ASSERT_FALSE(ec) << "Failed to encode to path: " << ec.message();

    std::vector<std::string> results;
    ec = parser::decode(results, tmp_path);
    ASSERT_FALSE(ec) << "Failed to decode from path: " << ec.message();

    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results.front(), payload);

    std::error_code ignore_ec;
    fs::remove(tmp_path, ignore_ec);
}

TEST(qrcode_file_system_test, strict_eof_trailing_garbage)
{
    fs::path tmp_path =
        fs::temp_directory_path() / "hj_qr_trailing_garbage.pgm";
    const std::string payload = "Valid QR Code File";

    ASSERT_FALSE(builder::encode(tmp_path, payload));

    {
        std::ofstream ofs(tmp_path, std::ios::binary | std::ios::app);
        ofs.write("\xDE\xAD\xBE\xEF", 4);
    }

    std::vector<std::string> results;
    auto                     ec = parser::decode(results, tmp_path);
    EXPECT_EQ(ec, error_code::pgm_read_failed);

    std::error_code ignore_ec;
    fs::remove(tmp_path, ignore_ec);
}

// ============================================================================
// PGM Parser Error Path Tests
// ============================================================================

static std::error_code parse_pgm_content(const std::string &content)
{
    fs::path tmp_path =
        fs::temp_directory_path() / "hj_qr_parser_error_test.pgm";
    {
        std::ofstream ofs(tmp_path, std::ios::binary);
        ofs.write(content.data(), content.size());
    }

    std::vector<std::string> results;
    auto                     ec = hj::qrcode::parser::decode(results, tmp_path);

    std::error_code ignore_ec;
    fs::remove(tmp_path, ignore_ec);

    return ec;
}

TEST(qrcode_parser_test, pgm_header_invalid_dimensions)
{
    EXPECT_EQ(parse_pgm_content("P5\n0 10\n255\n"),
              error_code::token_value_out_of_range);
    EXPECT_EQ(parse_pgm_content("P5\n10 0\n255\n"),
              error_code::token_value_out_of_range);
    EXPECT_EQ(parse_pgm_content("P5\n-5 10\n255\n"),
              error_code::token_value_out_of_range);
    EXPECT_EQ(parse_pgm_content("P5\n10 -5\n255\n"),
              error_code::token_value_out_of_range);
}

TEST(qrcode_parser_test, pgm_header_invalid_maxval)
{
    EXPECT_EQ(parse_pgm_content("P5\n10 10\n0\n"),
              error_code::token_value_out_of_range);
    EXPECT_EQ(parse_pgm_content("P5\n10 10\n256\n"),
              error_code::token_value_out_of_range);
}

TEST(qrcode_parser_test, pgm_header_missing_tokens)
{
    EXPECT_EQ(parse_pgm_content("P5\n"), error_code::token_empty);
    EXPECT_EQ(parse_pgm_content("P5\n10\n"), error_code::token_empty);
    EXPECT_EQ(parse_pgm_content("P5\n10 10\n"), error_code::token_empty);
}

TEST(qrcode_parser_test, pgm_header_malformed_integer)
{
    EXPECT_EQ(parse_pgm_content("P5\n10x 10\n255\n"),
              error_code::token_value_convert_failed);
    EXPECT_EQ(parse_pgm_content("P5\n10 10.5\n255\n"),
              error_code::token_value_convert_failed);
}

TEST(qrcode_parser_test, pgm_header_huge_dimensions)
{
    EXPECT_EQ(parse_pgm_content("P5\n9000 8000\n255\n"),
              error_code::bitmap_too_large);
    EXPECT_EQ(parse_pgm_content("P5\n8000 9000\n255\n"),
              error_code::bitmap_too_large);
}

TEST(qrcode_parser_test, pgm_truncated_payload)
{
    EXPECT_EQ(parse_pgm_content("P5\n2 2\n255\n\x00\x00"),
              error_code::pgm_read_failed);
}

TEST(qrcode_parser_test, pgm_comments_and_multiple_whitespace)
{
    std::string content = "P5 \t \n"
                          "# This is the first comment line\n"
                          "  10  \n"
                          "# This is the second comment line \n"
                          " \t 10 \n"
                          "255\n";
    content.append(100, '\0');

    EXPECT_EQ(parse_pgm_content(content), error_code::no_qrcode_found);
}