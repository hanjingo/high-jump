#include <gtest/gtest.h>
#include <hj/compress/gzip.hpp>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>

class gzip_test : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        test_string = "Hello, World! This is a test string for hj::gzip "
                      "compression and decompression with options pattern.";
        test_data =
            std::vector<unsigned char>(test_string.begin(), test_string.end());

        large_data.resize(100000, 0xAB);

        std::random_device              rd;
        std::mt19937                    gen(rd());
        std::uniform_int_distribution<> dis(0, 255);

        random_data.resize(50000);
        for(size_t i = 0; i < random_data.size(); ++i)
        {
            random_data[i] = static_cast<unsigned char>(dis(gen));
        }
    }

    void TearDown() override
    {
        std::remove("test_input.txt");
        std::remove("test_output.gz");
        std::remove("test_decompressed.txt");
    }

    std::string                test_string;
    std::vector<unsigned char> test_data;
    std::vector<unsigned char> large_data;
    std::vector<unsigned char> random_data;
};

TEST_F(gzip_test, basic_compression_decompression)
{
    std::vector<unsigned char> compressed;
    std::vector<unsigned char> decompressed;

    auto compress_result =
        hj::gzip::compress(compressed, test_data.data(), test_data.size());
    EXPECT_EQ(compress_result, hj::gzip::err::ok);
    EXPECT_GT(compressed.size(), 0);
    EXPECT_TRUE(hj::gzip::is_gzip_format(compressed.data(), compressed.size()));

    auto decompress_result = hj::gzip::decompress(decompressed,
                                                  compressed.data(),
                                                  compressed.size());
    EXPECT_EQ(decompress_result, hj::gzip::err::ok);
    EXPECT_EQ(decompressed.size(), test_data.size());
    EXPECT_EQ(decompressed, test_data);
}

TEST_F(gzip_test, compression_levels)
{
    const hj::gzip::compression_lvl levels[] = {
        hj::gzip::compression_lvl::default_compression,
        hj::gzip::compression_lvl::no_compression,
        hj::gzip::compression_lvl::best_speed,
        hj::gzip::compression_lvl::best_compression};

    std::vector<unsigned char> compressed_best, compressed_speed;

    for(auto lvl : levels)
    {
        std::vector<unsigned char>    compressed;
        hj::gzip::compression_options opts;
        opts.level = lvl;

        auto res = hj::gzip::compress(compressed,
                                      test_data.data(),
                                      test_data.size(),
                                      opts);
        EXPECT_EQ(res, hj::gzip::err::ok);

        std::vector<unsigned char> decompressed;
        res = hj::gzip::decompress(decompressed,
                                   compressed.data(),
                                   compressed.size());
        EXPECT_EQ(res, hj::gzip::err::ok);
        EXPECT_EQ(decompressed, test_data);

        if(lvl == hj::gzip::compression_lvl::best_compression)
            compressed_best = compressed;
        if(lvl == hj::gzip::compression_lvl::best_speed)
            compressed_speed = compressed;
    }

    EXPECT_LE(compressed_best.size(), compressed_speed.size());
}

TEST_F(gzip_test, memory_levels)
{
    const hj::gzip::mem_lvl mem_levels[] = {hj::gzip::mem_lvl::lvl1,
                                            hj::gzip::mem_lvl::default_level,
                                            hj::gzip::mem_lvl::lvl9};

    for(auto mem : mem_levels)
    {
        std::vector<unsigned char>    compressed;
        hj::gzip::compression_options opts;
        opts.mem = mem;

        auto res = hj::gzip::compress(compressed,
                                      large_data.data(),
                                      large_data.size(),
                                      opts);
        EXPECT_EQ(res, hj::gzip::err::ok);

        std::vector<unsigned char> decompressed;
        res = hj::gzip::decompress(decompressed,
                                   compressed.data(),
                                   compressed.size());
        EXPECT_EQ(res, hj::gzip::err::ok);
        EXPECT_EQ(decompressed, large_data);
    }
}

TEST_F(gzip_test, compression_strategies)
{
    const hj::gzip::strategy strategies[] = {
        hj::gzip::strategy::default_strategy,
        hj::gzip::strategy::filtered,
        hj::gzip::strategy::huffman_only,
        hj::gzip::strategy::rle,
        hj::gzip::strategy::fixed};

    for(auto strat : strategies)
    {
        std::vector<unsigned char>    compressed;
        hj::gzip::compression_options opts;
        opts.strat = strat;

        auto res = hj::gzip::compress(compressed,
                                      test_data.data(),
                                      test_data.size(),
                                      opts);
        EXPECT_EQ(res, hj::gzip::err::ok);

        std::vector<unsigned char> decompressed;
        res = hj::gzip::decompress(decompressed,
                                   compressed.data(),
                                   compressed.size());
        EXPECT_EQ(res, hj::gzip::err::ok);
        EXPECT_EQ(decompressed, test_data);
    }
}

TEST_F(gzip_test, custom_chunk_sizes)
{
    const size_t chunk_sizes[] = {1, 16, 64, 1024, 65536};

    for(size_t chunk_sz : chunk_sizes)
    {
        std::vector<unsigned char>    compressed;
        hj::gzip::compression_options c_opts;
        c_opts.chunk_size = chunk_sz;

        auto res = hj::gzip::compress(compressed,
                                      test_data.data(),
                                      test_data.size(),
                                      c_opts);
        EXPECT_EQ(res, hj::gzip::err::ok);

        std::vector<unsigned char>      decompressed;
        hj::gzip::decompression_options d_opts;
        d_opts.chunk_size = chunk_sz;

        res = hj::gzip::decompress(decompressed,
                                   compressed.data(),
                                   compressed.size(),
                                   d_opts);
        EXPECT_EQ(res, hj::gzip::err::ok);
        EXPECT_EQ(decompressed, test_data);
    }
}

TEST_F(gzip_test, stream_compression_and_decompression)
{
    std::stringstream in_stream(std::ios::binary | std::ios::in
                                | std::ios::out);
    std::stringstream compressed_stream(std::ios::binary | std::ios::in
                                        | std::ios::out);

    in_stream.write(reinterpret_cast<const char *>(test_data.data()),
                    test_data.size());
    in_stream.seekg(0, std::ios::beg);

    hj::gzip::compression_options c_opts;
    c_opts.chunk_size = 32;

    auto compress_res =
        hj::gzip::compress(compressed_stream, in_stream, c_opts);
    EXPECT_EQ(compress_res, hj::gzip::err::ok);

    compressed_stream.seekg(0, std::ios::beg);
    std::stringstream decompressed_stream(std::ios::binary | std::ios::in
                                          | std::ios::out);

    hj::gzip::decompression_options d_opts;
    d_opts.chunk_size = 16;

    auto decompress_res =
        hj::gzip::decompress(decompressed_stream, compressed_stream, d_opts);
    EXPECT_EQ(decompress_res, hj::gzip::err::ok);

    std::string                decompressed_str = decompressed_stream.str();
    std::vector<unsigned char> decompressed_vec(decompressed_str.begin(),
                                                decompressed_str.end());

    EXPECT_EQ(decompressed_vec, test_data);
}

TEST_F(gzip_test, max_output_size_limit)
{
    std::vector<unsigned char> compressed;
    auto                       compress_res =
        hj::gzip::compress(compressed, test_data.data(), test_data.size());
    ASSERT_EQ(compress_res, hj::gzip::err::ok);

    {
        std::vector<unsigned char>      decompressed;
        hj::gzip::decompression_options opts;
        opts.max_output_sz = test_data.size();

        auto res = hj::gzip::decompress(decompressed,
                                        compressed.data(),
                                        compressed.size(),
                                        opts);
        EXPECT_EQ(res, hj::gzip::err::ok);
        EXPECT_EQ(decompressed, test_data);
    }

    {
        std::vector<unsigned char>      decompressed;
        hj::gzip::decompression_options opts;
        opts.max_output_sz = test_data.size() - 1;

        auto res = hj::gzip::decompress(decompressed,
                                        compressed.data(),
                                        compressed.size(),
                                        opts);
        EXPECT_EQ(res, hj::gzip::err::max_output_sz_exceeded);
        EXPECT_TRUE(decompressed.empty());
    }

    {
        std::string payload(reinterpret_cast<const char *>(compressed.data()),
                            compressed.size());
        std::istringstream in(payload, std::ios::binary);
        std::ostringstream out(std::ios::binary);

        hj::gzip::decompression_options opts;
        opts.max_output_sz = test_data.size() - 1;

        auto res = hj::gzip::decompress(out, in, opts);
        EXPECT_EQ(res, hj::gzip::err::max_output_sz_exceeded);
    }
}

TEST_F(gzip_test, invalid_input_handling)
{
    std::vector<unsigned char> dst;
    unsigned char              dummy = 0x01;

    EXPECT_EQ(hj::gzip::compress(dst, nullptr, 100),
              hj::gzip::err::input_invalid);
    EXPECT_EQ(hj::gzip::compress(dst, &dummy, 0), hj::gzip::err::input_invalid);
    EXPECT_EQ(hj::gzip::decompress(dst, nullptr, 100),
              hj::gzip::err::input_invalid);
    EXPECT_EQ(hj::gzip::decompress(dst, &dummy, 0),
              hj::gzip::err::input_invalid);

    {
        hj::gzip::compression_options c_opts;
        c_opts.chunk_size = 0;
        EXPECT_EQ(hj::gzip::compress(dst, &dummy, 1, c_opts),
                  hj::gzip::err::input_invalid);

        hj::gzip::decompression_options d_opts;
        d_opts.chunk_size = 0;
        EXPECT_EQ(hj::gzip::decompress(dst, &dummy, 1, d_opts),
                  hj::gzip::err::input_invalid);
    }

    {
        std::vector<unsigned char> bad_data =
            {0x1f, 0x8b, 0x00, 0x00, 0x99, 0x88, 0x77};
        auto res = hj::gzip::decompress(dst, bad_data.data(), bad_data.size());
        EXPECT_NE(res, hj::gzip::err::ok);
    }
}

TEST_F(gzip_test, stream_error_handling)
{
    std::vector<unsigned char> dummy_data = {1, 2, 3, 4, 5};

    std::ifstream      invalid_in("non_existent_file_hj_gzip.txt");
    std::ostringstream out;
    EXPECT_EQ(hj::gzip::compress(out, invalid_in),
              hj::gzip::err::input_invalid);
    EXPECT_EQ(hj::gzip::decompress(out, invalid_in),
              hj::gzip::err::input_invalid);

    std::istringstream in(std::string(dummy_data.begin(), dummy_data.end()));
    std::ofstream      invalid_out("/non_existent_path_dir/output.gz");
    EXPECT_EQ(hj::gzip::compress(invalid_out, in),
              hj::gzip::err::input_invalid);
}

TEST_F(gzip_test, boundary_conditions)
{
    {
        unsigned char              single_byte = 0x7F;
        std::vector<unsigned char> compressed, decompressed;

        EXPECT_EQ(hj::gzip::compress(compressed, &single_byte, 1),
                  hj::gzip::err::ok);
        EXPECT_GT(compressed.size(), 1);

        EXPECT_EQ(hj::gzip::decompress(decompressed,
                                       compressed.data(),
                                       compressed.size()),
                  hj::gzip::err::ok);
        ASSERT_EQ(decompressed.size(), 1);
        EXPECT_EQ(decompressed[0], single_byte);
    }

    {
        std::vector<unsigned char> compressed, decompressed;
        EXPECT_EQ(hj::gzip::compress(compressed,
                                     large_data.data(),
                                     large_data.size()),
                  hj::gzip::err::ok);
        EXPECT_LT(compressed.size(), large_data.size() / 10);

        EXPECT_EQ(hj::gzip::decompress(decompressed,
                                       compressed.data(),
                                       compressed.size()),
                  hj::gzip::err::ok);
        EXPECT_EQ(decompressed, large_data);
    }
}

TEST_F(gzip_test, is_gzip_format)
{
    std::vector<unsigned char> compressed;
    hj::gzip::compress(compressed, test_data.data(), test_data.size());

    EXPECT_TRUE(hj::gzip::is_gzip_format(compressed.data(), compressed.size()));

    EXPECT_FALSE(hj::gzip::is_gzip_format(nullptr, 100));
    EXPECT_FALSE(hj::gzip::is_gzip_format(compressed.data(), 9));

    EXPECT_FALSE(hj::gzip::is_gzip_format(test_data.data(), test_data.size()));
}

TEST_F(gzip_test, helper_functions)
{
    // 1. CRC32 Checksum
    {
        auto crc1 =
            hj::gzip::crc32_checksum(test_data.data(), test_data.size());
        auto crc2 =
            hj::gzip::crc32_checksum(test_data.data(), test_data.size());

        EXPECT_EQ(crc1, crc2);
        EXPECT_NE(crc1, 0u);

        EXPECT_EQ(hj::gzip::crc32_checksum(nullptr, 0), 0u);
        EXPECT_EQ(hj::gzip::crc32_checksum(test_data.data(), 0), 0u);

        std::vector<unsigned char> modified_data = test_data;
        modified_data[0] ^= 0xFF;
        EXPECT_NE(crc1,
                  hj::gzip::crc32_checksum(modified_data.data(),
                                           modified_data.size()));
    }

    // 2. Compression Ratio
    {
        EXPECT_DOUBLE_EQ(hj::gzip::compression_ratio(100, 50), 0.5);
        EXPECT_DOUBLE_EQ(hj::gzip::compression_ratio(1000, 100), 0.9);
        EXPECT_DOUBLE_EQ(hj::gzip::compression_ratio(0, 50), 0.0);
        EXPECT_DOUBLE_EQ(hj::gzip::compression_ratio(100, 100), 0.0);
        EXPECT_DOUBLE_EQ(hj::gzip::compression_ratio(100, 150), -0.5);
    }

    // 3. Reserve Size Estimates
    {
        EXPECT_GT(hj::gzip::compress_reserve_sz(1000), 1000);
        EXPECT_GT(hj::gzip::decompress_reserve_sz(0, 0), 0);
        EXPECT_EQ(hj::gzip::decompress_reserve_sz(100, 500), 500);
    }
}