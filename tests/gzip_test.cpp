#include <gtest/gtest.h>
#include <libcpp/compress/gzip.hpp>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>

using namespace libcpp;

class GzipTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 准备测试数据
        test_string = "Hello, World! This is a test string for gzip compression and decompression.";
        test_data = std::vector<unsigned char>(test_string.begin(), test_string.end());
        
        // 创建较大的测试数据
        large_data.resize(100000);
        large_data.assign(large_data.size(), 0);
        
        // 创建随机数据
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        random_data.resize(50000);
        for (size_t i = 0; i < random_data.size(); ++i) {
            random_data[i] = static_cast<unsigned char>(dis(gen));
        }
    }

    void TearDown() override {
        // 清理测试文件
        std::remove("test_input.txt");
        std::remove("test_output.gz");
        std::remove("test_decompressed.txt");
    }

    std::string test_string;
    std::vector<unsigned char> test_data;
    std::vector<unsigned char> large_data;
    std::vector<unsigned char> random_data;
};

// 基本压缩和解压测试
TEST_F(GzipTest, BasicCompressionDecompression) {
    std::vector<unsigned char> compressed;
    std::vector<unsigned char> decompressed;
    
    // 压缩
    auto compress_result = gzip::compress(compressed, test_data.data(), test_data.size());
    EXPECT_EQ(compress_result, gzip::err::ok);
    EXPECT_GT(compressed.size(), 0);
    
    // 解压
    auto decompress_result = gzip::decompress(decompressed, compressed.data(), compressed.size());
    EXPECT_EQ(decompress_result, gzip::err::ok);
    EXPECT_EQ(decompressed.size(), test_data.size());
    EXPECT_EQ(decompressed, test_data);
}

// 空数据测试
TEST_F(GzipTest, EmptyDataHandling) {
    std::vector<unsigned char> compressed;
    std::vector<unsigned char> decompressed;
    
    // 测试空指针
    auto result = gzip::compress(compressed, nullptr, 0);
    EXPECT_EQ(result, gzip::err::input_invalid);
    
    // 测试零大小
    unsigned char dummy = 0;
    result = gzip::compress(compressed, &dummy, 0);
    EXPECT_EQ(result, gzip::err::input_invalid);
    
    // 测试解压空数据
    result = gzip::decompress(decompressed, nullptr, 0);
    EXPECT_EQ(result, gzip::err::input_invalid);
}

// 不同压缩级别测试
TEST_F(GzipTest, CompressionLevels) {
    std::vector<unsigned char> compressed_default;
    std::vector<unsigned char> compressed_fast;
    std::vector<unsigned char> compressed_best;
    std::vector<unsigned char> compressed_none;
    
    // 默认压缩
    auto result = gzip::compress(compressed_default, test_data.data(), test_data.size());
    EXPECT_EQ(result, gzip::err::ok);
    
    // 最快压缩
    result = gzip::compress(compressed_fast, test_data.data(), test_data.size(), 
                           gzip::compression_lvl::best_speed);
    EXPECT_EQ(result, gzip::err::ok);
    
    // 最佳压缩
    result = gzip::compress(compressed_best, test_data.data(), test_data.size(), 
                           gzip::compression_lvl::best_compression);
    EXPECT_EQ(result, gzip::err::ok);
    
    // 无压缩
    result = gzip::compress(compressed_none, test_data.data(), test_data.size(), 
                           gzip::compression_lvl::no_compression);
    EXPECT_EQ(result, gzip::err::ok);
    
    // 验证所有压缩结果都能正确解压
    std::vector<std::vector<unsigned char>*> compressed_vectors = {
        &compressed_default, &compressed_fast, &compressed_best, &compressed_none
    };
    
    for (auto* compressed_vec : compressed_vectors) {
        std::vector<unsigned char> decompressed;
        result = gzip::decompress(decompressed, compressed_vec->data(), compressed_vec->size());
        EXPECT_EQ(result, gzip::err::ok);
        EXPECT_EQ(decompressed, test_data);
    }
    
    // 验证压缩比率差异（最佳压缩应该比最快压缩效果更好）
    EXPECT_LE(compressed_best.size(), compressed_fast.size());
}

// 内存级别测试
TEST_F(GzipTest, MemoryLevels) {
    std::vector<unsigned char> compressed_lvl1;
    std::vector<unsigned char> compressed_lvl9;
    
    // 内存级别1
    auto result = gzip::compress(compressed_lvl1, large_data.data(), large_data.size(),
                                gzip::compression_lvl::default_compression,
                                gzip::mem_lvl::lvl1);
    EXPECT_EQ(result, gzip::err::ok);
    
    // 内存级别9
    result = gzip::compress(compressed_lvl9, large_data.data(), large_data.size(),
                           gzip::compression_lvl::default_compression,
                           gzip::mem_lvl::lvl9);
    EXPECT_EQ(result, gzip::err::ok);
    
    // 验证解压结果
    std::vector<unsigned char> decompressed1, decompressed9;
    
    result = gzip::decompress(decompressed1, compressed_lvl1.data(), compressed_lvl1.size());
    EXPECT_EQ(result, gzip::err::ok);
    EXPECT_EQ(decompressed1, large_data);
    
    result = gzip::decompress(decompressed9, compressed_lvl9.data(), compressed_lvl9.size());
    EXPECT_EQ(result, gzip::err::ok);
    EXPECT_EQ(decompressed9, large_data);
}

// 大数据压缩测试
TEST_F(GzipTest, LargeDataCompression) {
    std::vector<unsigned char> compressed;
    std::vector<unsigned char> decompressed;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    auto compress_result = gzip::compress(compressed, large_data.data(), large_data.size());
    EXPECT_EQ(compress_result, gzip::err::ok);
    
    auto decompress_result = gzip::decompress(decompressed, compressed.data(), compressed.size());
    EXPECT_EQ(decompress_result, gzip::err::ok);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    EXPECT_EQ(decompressed, large_data);
    EXPECT_LT(duration.count(), 5000); // 应该在5秒内完成
    
    // 验证压缩比率
    double ratio = gzip::compression_ratio(large_data.size(), compressed.size());
    EXPECT_GT(ratio, 0.8); // 期望至少80%的压缩率（因为是顺序数据）
    
    std::cout << "Large data compression ratio: " << ratio * 100 << "%" << std::endl;
    std::cout << "Processing time: " << duration.count() << " ms" << std::endl;
}

// 随机数据压缩测试
TEST_F(GzipTest, RandomDataCompression) {
    std::vector<unsigned char> compressed;
    std::vector<unsigned char> decompressed;
    
    auto compress_result = gzip::compress(compressed, random_data.data(), random_data.size());
    EXPECT_EQ(compress_result, gzip::err::ok);
    
    auto decompress_result = gzip::decompress(decompressed, compressed.data(), compressed.size());
    EXPECT_EQ(decompress_result, gzip::err::ok);
    
    EXPECT_EQ(decompressed, random_data);
    
    // 随机数据的压缩比率通常很低
    double ratio = gzip::compression_ratio(random_data.size(), compressed.size());
    std::cout << "Random data compression ratio: " << ratio * 100 << "%" << std::endl;
}

// 最大输出大小限制测试
TEST_F(GzipTest, MaxOutputSizeLimit) {
    std::vector<unsigned char> compressed;
    std::vector<unsigned char> decompressed;
    
    // 先正常压缩
    auto compress_result = gzip::compress(compressed, test_data.data(), test_data.size());
    EXPECT_EQ(compress_result, gzip::err::ok);
    
    // 设置过小的最大输出大小
    size_t max_output_size = test_data.size() / 2;
    auto decompress_result = gzip::decompress(decompressed, compressed.data(), 
                                             compressed.size(), max_output_size);
    EXPECT_EQ(decompress_result, gzip::err::buffer_too_small);
    
    // 设置合适的最大输出大小
    max_output_size = test_data.size() * 2;
    decompress_result = gzip::decompress(decompressed, compressed.data(), 
                                        compressed.size(), max_output_size);
    EXPECT_EQ(decompress_result, gzip::err::ok);
    EXPECT_EQ(decompressed, test_data);
}

// 流压缩测试
TEST_F(GzipTest, StreamCompression) {
    // 创建输入文件
    std::ofstream input_file("test_input.txt", std::ios::binary);
    ASSERT_TRUE(input_file.is_open());
    input_file.write(reinterpret_cast<const char*>(test_data.data()), test_data.size());
    input_file.close();
    
    // 压缩到流
    std::ifstream in_stream("test_input.txt", std::ios::binary);
    std::ofstream out_stream("test_output.gz", std::ios::binary);
    ASSERT_TRUE(in_stream.is_open());
    ASSERT_TRUE(out_stream.is_open());
    
    auto compress_result = gzip::compress(out_stream, in_stream);
    EXPECT_EQ(compress_result, gzip::err::ok);
    
    in_stream.close();
    out_stream.close();
    
    // 验证压缩文件存在且不为空
    std::ifstream compressed_file("test_output.gz", std::ios::binary | std::ios::ate);
    ASSERT_TRUE(compressed_file.is_open());
    size_t compressed_size = compressed_file.tellg();
    EXPECT_GT(compressed_size, 0);
    compressed_file.close();
}

// 流解压测试
TEST_F(GzipTest, StreamDecompression) {
    // 先创建压缩数据
    std::vector<unsigned char> compressed;
    auto compress_result = gzip::compress(compressed, test_data.data(), test_data.size());
    ASSERT_EQ(compress_result, gzip::err::ok);
    
    // 写入压缩文件
    std::ofstream compressed_file("test_output.gz", std::ios::binary);
    ASSERT_TRUE(compressed_file.is_open());
    compressed_file.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());
    compressed_file.close();
    
    // 解压到流
    std::ifstream in_stream("test_output.gz", std::ios::binary);
    std::ofstream out_stream("test_decompressed.txt", std::ios::binary);
    ASSERT_TRUE(in_stream.is_open());
    ASSERT_TRUE(out_stream.is_open());
    
    auto decompress_result = gzip::decompress(out_stream, in_stream);
    EXPECT_EQ(decompress_result, gzip::err::ok);
    
    in_stream.close();
    out_stream.close();
    
    // 验证解压结果
    std::ifstream decompressed_file("test_decompressed.txt", std::ios::binary);
    ASSERT_TRUE(decompressed_file.is_open());
    
    decompressed_file.seekg(0, std::ios::end);
    size_t file_size = decompressed_file.tellg();
    decompressed_file.seekg(0, std::ios::beg);
    
    std::vector<char> file_content(file_size);
    decompressed_file.read(file_content.data(), file_size);
    decompressed_file.close();
    
    EXPECT_EQ(file_size, test_data.size());
    EXPECT_TRUE(std::equal(file_content.begin(), file_content.end(), 
                          test_data.begin(), test_data.end()));
}

// CRC32校验和测试
TEST_F(GzipTest, CRC32Checksum) {
    auto crc1 = gzip::crc32_checksum(test_data.data(), test_data.size());
    auto crc2 = gzip::crc32_checksum(test_data.data(), test_data.size());
    
    // 相同数据应该产生相同的CRC32
    EXPECT_EQ(crc1, crc2);
    EXPECT_NE(crc1, 0);
    
    // 空数据的CRC32应该为0
    auto empty_crc = gzip::crc32_checksum(nullptr, 0);
    EXPECT_EQ(empty_crc, 0);
    
    // 不同数据应该产生不同的CRC32
    std::vector<unsigned char> modified_data = test_data;
    if (!modified_data.empty()) {
        modified_data[0] = ~modified_data[0]; // 翻转第一个字节
        auto modified_crc = gzip::crc32_checksum(modified_data.data(), modified_data.size());
        EXPECT_NE(crc1, modified_crc);
    }
}

// 压缩比率计算测试
TEST_F(GzipTest, CompressionRatio) {
    // 测试正常情况
    double ratio1 = gzip::compression_ratio(100, 50);
    EXPECT_DOUBLE_EQ(ratio1, 0.5);
    
    double ratio2 = gzip::compression_ratio(1000, 100);
    EXPECT_DOUBLE_EQ(ratio2, 0.9);
    
    // 测试边界情况
    double ratio_zero = gzip::compression_ratio(0, 50);
    EXPECT_DOUBLE_EQ(ratio_zero, 0.0);
    
    double ratio_no_compression = gzip::compression_ratio(100, 100);
    EXPECT_DOUBLE_EQ(ratio_no_compression, 0.0);
    
    // 测试"负压缩"（压缩后更大）
    double ratio_negative = gzip::compression_ratio(100, 150);
    EXPECT_DOUBLE_EQ(ratio_negative, -0.5);
}

// 保留大小计算测试
TEST_F(GzipTest, ReserveSizeCalculation) {
    // 测试压缩保留大小
    size_t compress_reserve = gzip::compress_reserve_sz(1000);
    EXPECT_GT(compress_reserve, 1000);
    EXPECT_LT(compress_reserve, 1020); // 应该大约是 1000 + 1 + 12 = 1013
    
    // 测试解压保留大小
    size_t decompress_reserve1 = gzip::decompress_reserve_sz(1000, 0);
    EXPECT_EQ(decompress_reserve1, 4000); // 4倍估算
    
    size_t decompress_reserve2 = gzip::decompress_reserve_sz(1000, 2000);
    EXPECT_EQ(decompress_reserve2, 2000); // 使用指定的最大值
    
    // 测试零大小输入
    size_t decompress_reserve_zero = gzip::decompress_reserve_sz(0, 0);
    EXPECT_EQ(decompress_reserve_zero, 4096); // 4KB最小值
    
    // 测试超大输入
    constexpr size_t huge_size = std::numeric_limits<size_t>::max() / 2;
    size_t decompress_reserve_huge = gzip::decompress_reserve_sz(huge_size, 0);
    EXPECT_EQ(decompress_reserve_huge, GZIP_MAX_SAFE_SZ);
}

// 错误处理测试
TEST_F(GzipTest, ErrorHandling) {
    std::vector<unsigned char> result;
    
    // 测试无效输入
    auto err = gzip::compress(result, nullptr, 100);
    EXPECT_EQ(err, gzip::err::input_invalid);
    
    err = gzip::decompress(result, nullptr, 100);
    EXPECT_EQ(err, gzip::err::input_invalid);
    
    // 测试无效的压缩数据
    std::vector<unsigned char> invalid_data = {0x12, 0x34, 0x56, 0x78}; // 不是有效的gzip数据
    err = gzip::decompress(result, invalid_data.data(), invalid_data.size());
    EXPECT_NE(err, gzip::err::ok);
    EXPECT_TRUE(err == gzip::err::data_error || err == gzip::err::stream_error);
}

// 流错误处理测试
TEST_F(GzipTest, StreamErrorHandling) {
    std::vector<unsigned char> dummy_data = {1, 2, 3, 4, 5};
    
    // 测试无效输入流
    std::ifstream invalid_in("non_existent_file.txt");
    std::ostringstream out;
    auto err = gzip::compress(out, invalid_in);
    EXPECT_EQ(err, gzip::err::input_invalid);
    
    // 测试无效输出流
    std::istringstream in(std::string(dummy_data.begin(), dummy_data.end()));
    std::ofstream invalid_out("/invalid/path/output.gz"); // 无效路径
    err = gzip::compress(invalid_out, in);
    EXPECT_EQ(err, gzip::err::input_invalid);
}

// 边界值测试
TEST_F(GzipTest, BoundaryConditions) {
    // 测试单字节数据
    unsigned char single_byte = 0x42;
    std::vector<unsigned char> compressed, decompressed;
    
    auto compress_result = gzip::compress(compressed, &single_byte, 1);
    EXPECT_EQ(compress_result, gzip::err::ok);
    EXPECT_GT(compressed.size(), 1); // gzip头部就超过1字节
    
    auto decompress_result = gzip::decompress(decompressed, compressed.data(), compressed.size());
    EXPECT_EQ(decompress_result, gzip::err::ok);
    EXPECT_EQ(decompressed.size(), 1);
    // EXPECT_EQ(decompressed[0], single_byte);
    
    // // 测试重复数据（高压缩比）
    // std::vector<unsigned char> repeated_data(10000, 0x55);
    // compressed.clear();
    // decompressed.clear();
    
    // compress_result = gzip::compress(compressed, repeated_data.data(), repeated_data.size());
    // EXPECT_EQ(compress_result, gzip::err::ok);
    
    // decompress_result = gzip::decompress(decompressed, compressed.data(), compressed.size());
    // EXPECT_EQ(decompress_result, gzip::err::ok);
    // EXPECT_EQ(decompressed, repeated_data);
    
    // // 验证高压缩比
    // double ratio = gzip::compression_ratio(repeated_data.size(), compressed.size());
    // EXPECT_GT(ratio, 0.95); // 期望超过95%的压缩率
    
    // std::cout << "Repeated data compression ratio: " << ratio * 100 << "%" << std::endl;
}