#include <gtest/gtest.h>
#include <hj/compress/gzip.hpp>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>

using namespace hj;

class GzipTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        test_string = "Hello, World! This is a test string for gzip "
                      "compression and decompression.";
        test_data =
            std::vector<unsigned char>(test_string.begin(), test_string.end());

        large_data.resize(100000);
        large_data.assign(large_data.size(), 0);

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

TEST_F(GzipTest, BasicCompressionDecompression)
{
    std::vector<unsigned char> compressed;
    std::vector<unsigned char> decompressed;

    auto compress_result =
        gzip::compress(compressed, test_data.data(), test_data.size());
    EXPECT_EQ(compress_result, gzip::err::ok);
    EXPECT_GT(compressed.size(), 0);

    auto decompress_result =
        gzip::decompress(decompressed, compressed.data(), compressed.size());
    EXPECT_EQ(decompress_result, gzip::err::ok);
    EXPECT_EQ(decompressed.size(), test_data.size());
    EXPECT_EQ(decompressed, test_data);
}

TEST_F(GzipTest, EmptyDataHandling)
{
    std::vector<unsigned char> compressed;
    std::vector<unsigned char> decompressed;

    auto result = gzip::compress(compressed, nullptr, 0);
    EXPECT_EQ(result, gzip::err::input_invalid);

    unsigned char dummy = 0;
    result              = gzip::compress(compressed, &dummy, 0);
    EXPECT_EQ(result, gzip::err::input_invalid);

    result = gzip::decompress(decompressed, nullptr, 0);
    EXPECT_EQ(result, gzip::err::input_invalid);
}

TEST_F(GzipTest, CompressionLevels)
{
    std::vector<unsigned char> compressed_default;
    std::vector<unsigned char> compressed_fast;
    std::vector<unsigned char> compressed_best;
    std::vector<unsigned char> compressed_none;

    auto result =
        gzip::compress(compressed_default, test_data.data(), test_data.size());
    EXPECT_EQ(result, gzip::err::ok);

    result = gzip::compress(compressed_fast,
                            test_data.data(),
                            test_data.size(),
                            gzip::compression_lvl::best_speed);
    EXPECT_EQ(result, gzip::err::ok);

    result = gzip::compress(compressed_best,
                            test_data.data(),
                            test_data.size(),
                            gzip::compression_lvl::best_compression);
    EXPECT_EQ(result, gzip::err::ok);

    result = gzip::compress(compressed_none,
                            test_data.data(),
                            test_data.size(),
                            gzip::compression_lvl::no_compression);
    EXPECT_EQ(result, gzip::err::ok);

    std::vector<std::vector<unsigned char> *> compressed_vectors = {
        &compressed_default,
        &compressed_fast,
        &compressed_best,
        &compressed_none};

    for(auto *compressed_vec : compressed_vectors)
    {
        std::vector<unsigned char> decompressed;
        result = gzip::decompress(decompressed,
                                  compressed_vec->data(),
                                  compressed_vec->size());
        EXPECT_EQ(result, gzip::err::ok);
        EXPECT_EQ(decompressed, test_data);
    }

    EXPECT_LE(compressed_best.size(), compressed_fast.size());
}

TEST_F(GzipTest, MemoryLevels)
{
    std::vector<unsigned char> compressed_lvl1;
    std::vector<unsigned char> compressed_lvl9;

    auto result = gzip::compress(compressed_lvl1,
                                 large_data.data(),
                                 large_data.size(),
                                 gzip::compression_lvl::default_compression,
                                 gzip::mem_lvl::lvl1);
    EXPECT_EQ(result, gzip::err::ok);

    result = gzip::compress(compressed_lvl9,
                            large_data.data(),
                            large_data.size(),
                            gzip::compression_lvl::default_compression,
                            gzip::mem_lvl::lvl9);
    EXPECT_EQ(result, gzip::err::ok);

    std::vector<unsigned char> decompressed1, decompressed9;

    result = gzip::decompress(decompressed1,
                              compressed_lvl1.data(),
                              compressed_lvl1.size());
    EXPECT_EQ(result, gzip::err::ok);
    EXPECT_EQ(decompressed1, large_data);

    result = gzip::decompress(decompressed9,
                              compressed_lvl9.data(),
                              compressed_lvl9.size());
    EXPECT_EQ(result, gzip::err::ok);
    EXPECT_EQ(decompressed9, large_data);
}

TEST_F(GzipTest, LargeDataCompression)
{
    std::vector<unsigned char> compressed;
    std::vector<unsigned char> decompressed;

    auto start_time = std::chrono::high_resolution_clock::now();

    auto compress_result =
        gzip::compress(compressed, large_data.data(), large_data.size());
    EXPECT_EQ(compress_result, gzip::err::ok);

    auto decompress_result =
        gzip::decompress(decompressed, compressed.data(), compressed.size());
    EXPECT_EQ(decompress_result, gzip::err::ok);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    EXPECT_EQ(decompressed, large_data);
    EXPECT_LT(duration.count(), 5000);

    double ratio =
        gzip::compression_ratio(large_data.size(), compressed.size());
    EXPECT_GT(ratio, 0.8);
}

TEST_F(GzipTest, RandomDataCompression)
{
    std::vector<unsigned char> compressed;
    std::vector<unsigned char> decompressed;

    auto compress_result =
        gzip::compress(compressed, random_data.data(), random_data.size());
    EXPECT_EQ(compress_result, gzip::err::ok);

    auto decompress_result =
        gzip::decompress(decompressed, compressed.data(), compressed.size());
    EXPECT_EQ(decompress_result, gzip::err::ok);

    EXPECT_EQ(decompressed, random_data);

    double ratio =
        gzip::compression_ratio(random_data.size(), compressed.size());
}

TEST_F(GzipTest, MaxOutputSizeLimit)
{
    std::vector<unsigned char> compressed;
    std::vector<unsigned char> decompressed;

    auto compress_result =
        gzip::compress(compressed, test_data.data(), test_data.size());
    EXPECT_EQ(compress_result, gzip::err::ok);

    size_t max_output_size   = test_data.size() * 2;
    auto   decompress_result = gzip::decompress(decompressed,
                                              compressed.data(),
                                              compressed.size(),
                                              max_output_size);
    EXPECT_EQ(decompress_result, gzip::err::ok);
    EXPECT_EQ(decompressed, test_data);
}

TEST_F(GzipTest, StreamCompression)
{
    std::ofstream input_file("test_input.txt", std::ios::binary);
    ASSERT_TRUE(input_file.is_open());
    input_file.write(reinterpret_cast<const char *>(test_data.data()),
                     test_data.size());
    input_file.close();

    std::ifstream in_stream("test_input.txt", std::ios::binary);
    std::ofstream out_stream("test_output.gz", std::ios::binary);
    ASSERT_TRUE(in_stream.is_open());
    ASSERT_TRUE(out_stream.is_open());

    auto compress_result = gzip::compress(out_stream, in_stream);
    EXPECT_EQ(compress_result, gzip::err::ok);

    in_stream.close();
    out_stream.close();

    std::ifstream compressed_file("test_output.gz",
                                  std::ios::binary | std::ios::ate);
    ASSERT_TRUE(compressed_file.is_open());
    size_t compressed_size = compressed_file.tellg();
    EXPECT_GT(compressed_size, 0);
    compressed_file.close();
}

TEST_F(GzipTest, StreamDecompression)
{
    std::vector<unsigned char> compressed;
    auto                       compress_result =
        gzip::compress(compressed, test_data.data(), test_data.size());
    ASSERT_EQ(compress_result, gzip::err::ok);

    std::ofstream compressed_file("test_output.gz", std::ios::binary);
    ASSERT_TRUE(compressed_file.is_open());
    compressed_file.write(reinterpret_cast<const char *>(compressed.data()),
                          compressed.size());
    compressed_file.close();

    std::ifstream in_stream("test_output.gz", std::ios::binary);
    std::ofstream out_stream("test_decompressed.txt", std::ios::binary);
    ASSERT_TRUE(in_stream.is_open());
    ASSERT_TRUE(out_stream.is_open());

    auto decompress_result = gzip::decompress(out_stream, in_stream);
    EXPECT_EQ(decompress_result, gzip::err::ok);

    in_stream.close();
    out_stream.close();

    std::ifstream decompressed_file("test_decompressed.txt", std::ios::binary);
    ASSERT_TRUE(decompressed_file.is_open());

    decompressed_file.seekg(0, std::ios::end);
    size_t file_size = decompressed_file.tellg();
    decompressed_file.seekg(0, std::ios::beg);

    std::vector<char> file_content(file_size);
    decompressed_file.read(file_content.data(), file_size);
    decompressed_file.close();

    EXPECT_EQ(file_size, test_data.size());
    EXPECT_TRUE(std::equal(file_content.begin(),
                           file_content.end(),
                           test_data.begin(),
                           test_data.end()));
}

TEST_F(GzipTest, CRC32Checksum)
{
    auto crc1 = gzip::crc32_checksum(test_data.data(), test_data.size());
    auto crc2 = gzip::crc32_checksum(test_data.data(), test_data.size());

    EXPECT_EQ(crc1, crc2);
    EXPECT_NE(crc1, 0);

    auto empty_crc = gzip::crc32_checksum(nullptr, 0);
    EXPECT_EQ(empty_crc, 0);

    std::vector<unsigned char> modified_data = test_data;
    if(!modified_data.empty())
    {
        modified_data[0] = ~modified_data[0];
        auto modified_crc =
            gzip::crc32_checksum(modified_data.data(), modified_data.size());
        EXPECT_NE(crc1, modified_crc);
    }
}

TEST_F(GzipTest, CompressionRatio)
{
    double ratio1 = gzip::compression_ratio(100, 50);
    EXPECT_DOUBLE_EQ(ratio1, 0.5);

    double ratio2 = gzip::compression_ratio(1000, 100);
    EXPECT_DOUBLE_EQ(ratio2, 0.9);

    double ratio_zero = gzip::compression_ratio(0, 50);
    EXPECT_DOUBLE_EQ(ratio_zero, 0.0);

    double ratio_no_compression = gzip::compression_ratio(100, 100);
    EXPECT_DOUBLE_EQ(ratio_no_compression, 0.0);

    double ratio_negative = gzip::compression_ratio(100, 150);
    EXPECT_DOUBLE_EQ(ratio_negative, -0.5);
}

TEST_F(GzipTest, ErrorHandling)
{
    std::vector<unsigned char> result;

    auto err = gzip::compress(result, nullptr, 100);
    EXPECT_EQ(err, gzip::err::input_invalid);

    err = gzip::decompress(result, nullptr, 100);
    EXPECT_EQ(err, gzip::err::input_invalid);

    std::vector<unsigned char> invalid_data = {0x12, 0x34, 0x56, 0x78};
    err = gzip::decompress(result, invalid_data.data(), invalid_data.size());
    EXPECT_NE(err, gzip::err::ok);
    EXPECT_TRUE(err == gzip::err::data_error || err == gzip::err::stream_error);
}

TEST_F(GzipTest, StreamErrorHandling)
{
    std::vector<unsigned char> dummy_data = {1, 2, 3, 4, 5};

    std::ifstream      invalid_in("non_existent_file.txt");
    std::ostringstream out;
    auto               err = gzip::compress(out, invalid_in);
    EXPECT_EQ(err, gzip::err::input_invalid);

    std::istringstream in(std::string(dummy_data.begin(), dummy_data.end()));
    std::ofstream      invalid_out("/invalid/path/output.gz");
    err = gzip::compress(invalid_out, in);
    EXPECT_EQ(err, gzip::err::input_invalid);
}

TEST_F(GzipTest, BoundaryConditions)
{
    unsigned char              single_byte = 0x42;
    std::vector<unsigned char> compressed, decompressed;

    auto compress_result = gzip::compress(compressed, &single_byte, 1);
    EXPECT_EQ(compress_result, gzip::err::ok);
    EXPECT_GT(compressed.size(), 1);

    auto decompress_result =
        gzip::decompress(decompressed, compressed.data(), compressed.size());
    EXPECT_EQ(decompress_result, gzip::err::ok);
    EXPECT_EQ(decompressed.size(), 1);
}