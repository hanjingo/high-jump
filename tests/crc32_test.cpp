#include <gtest/gtest.h>
#include <cstring>
#include <vector>
#include <string>

#include <hj/algo/crc32.h>

TEST(hj_crc32, standard_test_vectors)
{
    const char *test1 = "123456789";
    EXPECT_EQ(hj_crc32(test1, strlen(test1)), 0xCBF43926U);
    EXPECT_EQ(hj_crc32_iso_hdlc(test1, strlen(test1)), 0xCBF43926U);

    const char *test2 = "";
    EXPECT_EQ(hj_crc32(test2, strlen(test2)), 0x00000000U);

    const char *test3 = "a";
    EXPECT_EQ(hj_crc32(test3, strlen(test3)), 0xE8B7BE43U);

    const char *test4 = "abc";
    EXPECT_EQ(hj_crc32(test4, strlen(test4)), 0x352441C2U);

    const char *test5 = "message digest";
    EXPECT_EQ(hj_crc32(test5, strlen(test5)), 0x20159D7FU);
}

TEST(hj_crc32, context_lifecycle_and_reset)
{
    hj_crc32_ctx_t ctx;

    hj_crc32_init(&ctx);
    EXPECT_EQ(ctx.state, 0xFFFFFFFFU);

    const char *data = "The quick brown fox jumps over the lazy dog";
    hj_crc32_update(&ctx, data, strlen(data));
    uint32_t crc1 = hj_crc32_finalize(&ctx);

    EXPECT_EQ(crc1, hj_crc32(data, strlen(data)));

    hj_crc32_init(&ctx);
    hj_crc32_update(&ctx, data, strlen(data));
    uint32_t crc2 = hj_crc32_finalize(&ctx);
    EXPECT_EQ(crc1, crc2);
}

TEST(hj_crc32, incremental_and_chunked)
{
    const char *part1    = "Hello, ";
    const char *part2    = "World!";
    std::string combined = std::string(part1) + part2;

    uint32_t expected = hj_crc32(combined.c_str(), combined.length());

    hj_crc32_ctx_t ctx;
    hj_crc32_init(&ctx);
    hj_crc32_update(&ctx, part1, strlen(part1));
    hj_crc32_update(&ctx, part2, strlen(part2));
    uint32_t stream_crc = hj_crc32_finalize(&ctx);
    EXPECT_EQ(stream_crc, expected);

    hj_crc32_ctx_t ctx_byte;
    hj_crc32_init(&ctx_byte);
    for(size_t i = 0; i < combined.length(); ++i)
    {
        hj_crc32_update(&ctx_byte, &combined[i], 1);
    }
    EXPECT_EQ(hj_crc32_finalize(&ctx_byte), expected);
}

TEST(hj_crc32, null_and_boundary_conditions)
{
    const char *data = "test_data";

    hj_crc32_init(nullptr);
    hj_crc32_update(nullptr, data, strlen(data));
    EXPECT_EQ(hj_crc32_finalize(nullptr), 0U);

    hj_crc32_ctx_t ctx;
    hj_crc32_init(&ctx);
    uint32_t initial_state = ctx.state;

    hj_crc32_update(&ctx, nullptr, 10);
    EXPECT_EQ(ctx.state, initial_state);

    hj_crc32_update(&ctx, data, 0);
    EXPECT_EQ(ctx.state, initial_state);

    EXPECT_EQ(hj_crc32(nullptr, 0), 0U);
    EXPECT_EQ(hj_crc32(nullptr, 10), 0U);
    EXPECT_EQ(hj_crc32(data, 0), 0U);
}

TEST(hj_crc32, binary_data_coverage)
{
    std::vector<uint8_t> all_bytes(256);
    for(int i = 0; i < 256; ++i)
    {
        all_bytes[i] = static_cast<uint8_t>(i);
    }

    uint32_t full_crc = hj_crc32(all_bytes.data(), all_bytes.size());
    EXPECT_EQ(full_crc, 0x29058C73U);

    uint8_t zero_byte = 0x00;
    uint8_t ff_byte   = 0xFF;
    EXPECT_NE(hj_crc32(&zero_byte, 1), hj_crc32(&ff_byte, 1));
}

TEST(hj_crc32, large_text_crc)
{
    const char *big_data =
        R"(The class everyone had really been looking forward to was Defence Against the Dark Arts, but Quirrell's lessons turned out to be a bit of a joke.His classroom smelled strongly of garlic, which everyone said was to ward off a vampire he'd met in Romania, and was afraid would be coming back to get him one of these days.His turban, he told them, had been given to him by an African prince as a thank-you for getting rid of a troublesome zombie, but they weren't sure they believed this story.For one thing, when Seamus Finnegan asked eagerly to hear how Quirrell had fought off the zombie, Quirrell went pink and sta)";

    EXPECT_EQ(hj_crc32(big_data, strlen(big_data)), 0x2E865E47U);
}