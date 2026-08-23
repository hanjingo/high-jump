#include <gtest/gtest.h>
#include <hj/io/chain_buffer.hpp>
#include <cstring>
#include <string>
#include <random>

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

    const uint8_t *first_block_ptr = nullptr;
    buf.for_each_segment([&first_block_ptr](const uint8_t *data, size_t size) {
        if(first_block_ptr == nullptr && size > 0)
        {
            first_block_ptr = data;
        }
    });

    ASSERT_NE(first_block_ptr, nullptr);

    buf.clear();
    ASSERT_TRUE(buf.empty());

    buf.append(d.data(), d.size());

    const uint8_t *second_block_ptr = nullptr;
    buf.for_each_segment([&second_block_ptr](const uint8_t *data, size_t size) {
        if(second_block_ptr == nullptr && size > 0)
        {
            second_block_ptr = data;
        }
    });

    ASSERT_EQ(first_block_ptr, second_block_ptr);

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

TEST(chain_buffer_industrial, invalid_block_size)
{
    EXPECT_THROW(chain_buffer(0), std::invalid_argument);
}

TEST(chain_buffer_industrial, nullptr_append)
{
    chain_buffer buf;
    EXPECT_THROW(buf.append(nullptr, 1), std::invalid_argument);
    EXPECT_NO_THROW(buf.append(nullptr, 0));
}

TEST(chain_buffer_industrial, self_append)
{
    chain_buffer buf;
    std::string  d = "hello";
    buf.append(d.data(), d.size());

    EXPECT_THROW(buf.append(buf), std::invalid_argument);
}

TEST(chain_buffer_industrial, move_append)
{
    chain_buffer a;
    chain_buffer b;
    std::string  data_a = " world";
    std::string  data_b = "hello";
    a.append(data_a.data(), data_a.size()); // a = " world"
    b.append(data_b.data(), data_b.size()); // b = "hello"

    b.append(std::move(a));
    EXPECT_TRUE(a.empty());
    EXPECT_EQ(b.size(), 11u);

    char   out[32] = {0};
    size_t n       = b.read(out, sizeof(out));
    EXPECT_EQ(std::string(out, n), "hello world");
}

TEST(chain_buffer_industrial, move_constructor)
{
    chain_buffer a;
    std::string  data = "test move ctor";
    a.append(data.data(), data.size());

    chain_buffer b(std::move(a));
    EXPECT_TRUE(a.empty());
    EXPECT_EQ(b.size(), data.size());

    char   out[32] = {0};
    size_t n       = b.read(out, sizeof(out));
    EXPECT_EQ(std::string(out, n), data);
}

TEST(chain_buffer_industrial, move_assignment)
{
    chain_buffer a;
    chain_buffer b;
    std::string  data = "test move assign";
    a.append(data.data(), data.size());

    b = std::move(a);
    EXPECT_TRUE(a.empty());
    EXPECT_EQ(b.size(), data.size());

    char   out[32] = {0};
    size_t n       = b.read(out, sizeof(out));
    EXPECT_EQ(std::string(out, n), data);
}

TEST(chain_buffer_industrial, exact_block_boundaries)
{
    size_t cap         = 8;
    size_t test_lens[] = {7, 8, 9, 15, 16, 17};

    for(size_t len : test_lens)
    {
        chain_buffer buf(cap);
        std::string  data(len, 'a');
        buf.append(data.data(), data.size());
        EXPECT_EQ(buf.size(), len);

        std::string out(len, '\0');
        size_t      n = buf.read(&out[0], len);
        EXPECT_EQ(n, len);
        EXPECT_EQ(out, data);
    }
}

TEST(chain_buffer_industrial, consume_exact_boundary)
{
    chain_buffer buf(8);
    std::string  data(16, 'x');
    buf.append(data.data(), data.size());

    size_t c1 = buf.consume(8);
    EXPECT_EQ(c1, 8u);
    EXPECT_EQ(buf.size(), 8u);

    size_t c2 = buf.consume(8);
    EXPECT_EQ(c2, 8u);
    EXPECT_TRUE(buf.empty());
}

TEST(chain_buffer_industrial, streaming_consume_partial_and_append)
{
    chain_buffer buf(8);
    std::string  d1 = "ABCDEFGH";
    buf.append(d1.data(), d1.size());

    buf.consume(3);
    EXPECT_EQ(buf.size(), 5u);

    std::string d2 = "IJKLMNOP";
    buf.append(d2.data(), d2.size());
    EXPECT_EQ(buf.size(), 5u + 8u);

    char   out[32] = {0};
    size_t n       = buf.read(out, sizeof(out));
    EXPECT_EQ(n, 13u);
    EXPECT_EQ(std::string(out, n), "DEFGHIJKLMNOP");
}

TEST(chain_buffer_industrial, clear_allocation_and_reuse_semantics)
{
    chain_buffer buf(16);
    buf.append("12345", 5);

    const uint8_t *ptr_before = nullptr;
    buf.for_each_segment(
        [&ptr_before](const uint8_t *p, size_t) { ptr_before = p; });

    buf.clear();
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.block_size(), 16u);

    buf.append("67890", 5);
    const uint8_t *ptr_after = nullptr;
    buf.for_each_segment(
        [&ptr_after](const uint8_t *p, size_t) { ptr_after = p; });

    EXPECT_EQ(ptr_before, ptr_after);
}

std::string
generate_random_string(size_t min_len, size_t max_len, std::mt19937 &rng)
{
    std::uniform_int_distribution<size_t> len_dist(min_len, max_len);
    size_t                                len = len_dist(rng);

    std::string s;
    s.resize(len);
    std::uniform_int_distribution<int> char_dist('a', 'z');
    for(size_t i = 0; i < len; ++i)
    {
        s[i] = static_cast<char>(char_dist(rng));
    }
    return s;
}

TEST(chain_buffer_fuzz, reference_model_randomized)
{
    std::mt19937 rng(42);

    std::vector<uint8_t> reference;

    size_t       test_block_size = 16;
    chain_buffer buf(test_block_size);

    enum Action
    {
        ACTION_APPEND = 0,
        ACTION_CONSUME,
        ACTION_PEEK,
        ACTION_CLEAR,
        ACTION_MAX
    };

    std::uniform_int_distribution<int> action_dist(0, ACTION_MAX - 1);

    int num_iterations = 2000;
    for(int i = 0; i < num_iterations; ++i)
    {
        int action = action_dist(rng);

        switch(action)
        {
            case ACTION_APPEND: {
                std::string data = generate_random_string(1, 30, rng);
                reference.insert(reference.end(), data.begin(), data.end());
                buf.append(data.data(), data.size());
                break;
            }
            case ACTION_CONSUME: {
                if(!reference.empty())
                {
                    std::uniform_int_distribution<size_t> consume_dist(
                        1,
                        reference.size());
                    size_t n = consume_dist(rng);
                    reference.erase(reference.begin(), reference.begin() + n);
                    buf.consume(n);
                }
                break;
            }
            case ACTION_PEEK: {
                std::uniform_int_distribution<size_t> peek_len_dist(
                    1,
                    std::max<size_t>(1, reference.size() + 10));
                size_t peek_len = peek_len_dist(rng);

                std::vector<char> ref_out(peek_len, 0);
                std::vector<char> buf_out(peek_len, 0);

                size_t ref_actual_n =
                    (peek_len < reference.size()) ? peek_len : reference.size();
                if(ref_actual_n > 0)
                {
                    std::memcpy(ref_out.data(), reference.data(), ref_actual_n);
                }

                size_t buf_actual_n = buf.peek(buf_out.data(), peek_len);

                ASSERT_EQ(buf_actual_n, ref_actual_n);
                if(ref_actual_n > 0)
                {
                    ASSERT_EQ(std::memcmp(buf_out.data(),
                                          ref_out.data(),
                                          ref_actual_n),
                              0);
                }
                break;
            }
            case ACTION_CLEAR: {
                reference.clear();
                buf.clear();
                break;
            }
            default:
                break;
        }

        ASSERT_EQ(buf.size(), reference.size());
        ASSERT_EQ(buf.empty(), reference.empty());

        if(!reference.empty())
        {
            std::vector<uint8_t> ref_full(reference.size());
            std::memcpy(ref_full.data(), reference.data(), reference.size());

            std::vector<uint8_t> buf_full(buf.size());
            size_t               read_n = buf.peek(buf_full.data(), buf.size());

            ASSERT_EQ(read_n, reference.size());
            ASSERT_EQ(buf_full, ref_full);
        }
    }
}