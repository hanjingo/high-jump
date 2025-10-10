#include <gtest/gtest.h>
#include <hj/io/chain_buffer.hpp>
#include <cstring>
#include <string>

using hj::chain_buffer;

TEST(chain_buffer, append_and_read)
{
    chain_buffer buf;
    std::string  data = "hello world";
    buf.append(data.data(), data.size());
    ASSERT_EQ(buf.size(), data.size());

    char   out[32] = {0};
    size_t n       = buf.read(out, sizeof(out));
    ASSERT_EQ(n, data.size());
    ASSERT_EQ(std::string(out, n), data);
}

TEST(chain_buffer, consume)
{
    chain_buffer buf;
    std::string  data = "abcdef";
    buf.append(data.data(), data.size());
    buf.consume(2);
    ASSERT_EQ(buf.size(), 4u);

    char   out[8] = {0};
    size_t n      = buf.read(out, sizeof(out));
    ASSERT_EQ(n, 4u);
    ASSERT_EQ(std::string(out, n), "cdef");
}

TEST(chain_buffer, append_large)
{
    chain_buffer buf(8); // small block size for test
    std::string  data(100, 'x');
    buf.append(data.data(), data.size());
    ASSERT_EQ(buf.size(), 100u);

    char   out[128] = {0};
    size_t n        = buf.read(out, sizeof(out));
    ASSERT_EQ(n, 100u);
    ASSERT_EQ(std::string(out, n), data);
}

TEST(chain_buffer, clear)
{
    chain_buffer buf;
    std::string  data = "test";
    buf.append(data.data(), data.size());
    buf.clear();
    ASSERT_EQ(buf.size(), 0u);
    ASSERT_TRUE(buf.empty());
}

TEST(chain_buffer, append_chain_buffer)
{
    chain_buffer buf1, buf2;
    std::string  d1 = "foo";
    std::string  d2 = "bar";
    buf1.append(d1.data(), d1.size());
    buf2.append(d2.data(), d2.size());
    buf1.append(buf2);

    char   out[16] = {0};
    size_t n       = buf1.read(out, sizeof(out));
    ASSERT_EQ(n, d1.size() + d2.size());
    ASSERT_EQ(std::string(out, n), d1 + d2);
}

TEST(chain_buffer, consume_over)
{
    chain_buffer buf;
    std::string  data = "12345";
    buf.append(data.data(), data.size());

    size_t consumed = buf.consume(100);
    ASSERT_EQ(consumed, 5u);
    ASSERT_EQ(buf.size(), 0u);
    ASSERT_TRUE(buf.empty());
}

TEST(chain_buffer, append_empty_buffer)
{
    chain_buffer buf1, buf2;
    buf1.append(buf2);
    ASSERT_TRUE(buf1.empty());
    std::string d = "x";
    buf2.append(d.data(), d.size());
    buf1.append(buf2);
    ASSERT_EQ(buf1.size(), 1u);
}

TEST(chain_buffer, block_size)
{
    chain_buffer buf(32);
    ASSERT_EQ(buf.block_size(), 32u);
    std::string data(100, 'a');
    buf.append(data.data(), data.size());
    ASSERT_EQ(buf.size(), 100u);
}

TEST(chain_buffer, clear_reuse)
{
    chain_buffer buf;
    std::string  d = "abc";
    buf.append(d.data(), d.size());
    buf.clear();
    ASSERT_TRUE(buf.empty());
    buf.append(d.data(), d.size());
    ASSERT_EQ(buf.size(), 3u);
    char   out[8] = {0};
    size_t n      = buf.read(out, sizeof(out));
    ASSERT_EQ(n, 3u);
    ASSERT_EQ(std::string(out, n), d);
}

TEST(chain_buffer, multi_append_consume)
{
    chain_buffer buf;
    std::string  d1 = "abc", d2 = "defg", d3 = "hij";
    buf.append(d1.data(), d1.size());
    buf.append(d2.data(), d2.size());
    buf.append(d3.data(), d3.size());
    ASSERT_EQ(buf.size(), d1.size() + d2.size() + d3.size());
    char   out[16] = {0};
    size_t n       = buf.read(out, 4);
    ASSERT_EQ(n, 4u);
    ASSERT_EQ(std::string(out, 4), "abcd");
    size_t consumed = buf.consume(4);
    ASSERT_EQ(consumed, 4u);
    n = buf.read(out, 5);
    ASSERT_EQ(n, 5u);
    ASSERT_EQ(std::string(out, 5), "efghi");
}