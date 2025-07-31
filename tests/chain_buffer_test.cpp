#include <gtest/gtest.h>
#include <libcpp/io/chain_buffer.hpp>
#include <cstring>
#include <string>

using libcpp::chain_buffer;

TEST(ChainBufferTest, AppendAndRead) {
    chain_buffer buf;
    std::string data = "hello world";
    buf.append(data.data(), data.size());
    ASSERT_EQ(buf.size(), data.size());

    char out[32] = {0};
    size_t n = buf.read(out, sizeof(out));
    ASSERT_EQ(n, data.size());
    ASSERT_EQ(std::string(out, n), data);
}

TEST(ChainBufferTest, Consume) {
    chain_buffer buf;
    std::string data = "abcdef";
    buf.append(data.data(), data.size());
    buf.consume(2);
    ASSERT_EQ(buf.size(), 4u);

    char out[8] = {0};
    size_t n = buf.read(out, sizeof(out));
    ASSERT_EQ(n, 4u);
    ASSERT_EQ(std::string(out, n), "cdef");
}

TEST(ChainBufferTest, AppendLarge) {
    chain_buffer buf(8); // small block size for test
    std::string data(100, 'x');
    buf.append(data.data(), data.size());
    ASSERT_EQ(buf.size(), 100u);

    char out[128] = {0};
    size_t n = buf.read(out, sizeof(out));
    ASSERT_EQ(n, 100u);
    ASSERT_EQ(std::string(out, n), data);
}

TEST(ChainBufferTest, Clear) {
    chain_buffer buf;
    std::string data = "test";
    buf.append(data.data(), data.size());
    buf.clear();
    ASSERT_EQ(buf.size(), 0u);
    ASSERT_TRUE(buf.empty());
}

TEST(ChainBufferTest, AppendChainBuffer) {
    chain_buffer buf1, buf2;
    std::string d1 = "foo";
    std::string d2 = "bar";
    buf1.append(d1.data(), d1.size());
    buf2.append(d2.data(), d2.size());
    buf1.append(buf2);

    char out[16] = {0};
    size_t n = buf1.read(out, sizeof(out));
    ASSERT_EQ(n, d1.size() + d2.size());
    ASSERT_EQ(std::string(out, n), d1 + d2);
}