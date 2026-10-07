#include <gtest/gtest.h>
#include <hj/ai/ocr.hpp>
#include <fstream>
#include <vector>

namespace
{
std::vector<uint8_t> create_dummy_bmp()
{
    return {0x42, 0x4D, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00,
            0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00};
}
} // namespace

TEST(ocr_test, error_category)
{
    hj::ocr::error_code ec     = hj::ocr::error_code::init_failed;
    std::error_code     std_ec = hj::ocr::make_error_code(ec);

    EXPECT_STREQ(std_ec.category().name(), "hj::ocr");
    EXPECT_EQ(std_ec.value(), static_cast<int>(ec));
    EXPECT_FALSE(std_ec.message().empty());
}

TEST(ocr_test, invalid_language)
{
    std::error_code         ec;
    hj::ocr::parser_options opts;
    opts.language = "invalid_lang_12345";
    auto parser   = hj::ocr::parser::make_unique(opts, ec);

    EXPECT_EQ(parser, nullptr);
    EXPECT_EQ(ec, hj::ocr::error_code::init_failed);
}

TEST(ocr_test, invalid_data_path)
{
    std::error_code         ec;
    hj::ocr::parser_options opts;
    opts.language = "eng";
    opts.datapath = "/non_existent_path_9876";
    auto parser   = hj::ocr::parser::make_unique(opts, ec);

    EXPECT_EQ(parser, nullptr);
    EXPECT_EQ(ec, hj::ocr::error_code::init_failed);
}

TEST(ocr_test, load_image)
{
    std::error_code ec;
    auto img_invalid = hj::ocr::image::load("non_existent_image.png", ec);
    EXPECT_TRUE(img_invalid.empty());
    EXPECT_FALSE(img_invalid);
    EXPECT_EQ(ec, hj::ocr::error_code::image_load_failed);

    const uint8_t bad_data[] = {0x00, 0x01, 0x02, 0x03};
    auto img_bad_mem = hj::ocr::image::load(bad_data, sizeof(bad_data), ec);
    EXPECT_TRUE(img_bad_mem.empty());
    EXPECT_EQ(ec, hj::ocr::error_code::image_load_failed);

    auto img_valid = hj::ocr::image::load("example.png", ec);
    EXPECT_FALSE(img_valid.empty());
    EXPECT_TRUE(img_valid);
    EXPECT_FALSE(ec);

    auto dummy_bmp = create_dummy_bmp();
    auto img_dummy_bmp =
        hj::ocr::image::load(dummy_bmp.data(), dummy_bmp.size(), ec);
    EXPECT_FALSE(img_dummy_bmp.empty());
    EXPECT_TRUE(img_dummy_bmp);
    EXPECT_FALSE(ec);
    EXPECT_NE(img_dummy_bmp.data(), nullptr);
}

TEST(ocr_test, recognize)
{
    std::error_code         ec;
    hj::ocr::parser_options opts;
    opts.language = "eng";
    auto parser   = hj::ocr::parser::make_unique(opts, ec);
    if(ec)
    {
        GTEST_SKIP() << "Tesseract eng language data not found, skipping "
                        "recognition test.";
    }

    hj::ocr::image empty_img;
    EXPECT_EQ(parser->set_image(empty_img), hj::ocr::error_code::invalid_image);

    auto        img_valid = hj::ocr::image::load("example.png", ec);
    std::string result;
    ec = parser->recognize(result, img_valid);
    EXPECT_FALSE(ec);

    auto dummy_bmp = create_dummy_bmp();
    auto valid_dummy_bmp =
        hj::ocr::image::load(dummy_bmp.data(), dummy_bmp.size(), ec);
    ec = parser->recognize(result, valid_dummy_bmp);
    EXPECT_FALSE(ec);
}

TEST(ocr_test, parser_lifecycle)
{
    std::error_code         ec;
    hj::ocr::parser_options opts;
    opts.language = "eng";
    auto parser1  = hj::ocr::parser::make_unique(opts, ec);
    if(ec)
    {
        GTEST_SKIP() << "Tesseract eng language data not found, skipping "
                        "lifecycle test.";
    }

    hj::ocr::parser parser2 = std::move(*parser1);

    auto dummy_bmp = create_dummy_bmp();
    auto valid_img =
        hj::ocr::image::load(dummy_bmp.data(), dummy_bmp.size(), ec);

    EXPECT_FALSE(parser2.set_image(valid_img));
}

TEST(ocr_test, moved_from_parser)
{
    std::error_code ec;

    hj::ocr::parser_options opts;
    opts.language = "eng";
    auto parser1  = hj::ocr::parser::make_unique(opts, ec);

    ASSERT_FALSE(ec);

    hj::ocr::parser parser2 = std::move(*parser1);

    hj::ocr::image image;

    std::string result;

    EXPECT_EQ(parser1->recognize(result, image),
              hj::ocr::error_code::invalid_image);
}