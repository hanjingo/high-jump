#include <gtest/gtest.h>
#include <cstring>
#include <vector>

#ifndef HJ_CRC16_MODBUS_ENABLED
#define HJ_CRC16_XMODEM_ENABLED
#endif

#include <hj/algo/crc16.h>

TEST(hj_crc16, standard_test_vectors)
{
    const char *test1 = "123456789";
#if HJ_CRC16_ALGO == HJ_CRC16_MODBUS
    EXPECT_EQ(hj_crc16(test1, strlen(test1)), 0x4B37);
    EXPECT_EQ(hj_crc16_modbus(test1, strlen(test1)), 0x4B37);
#elif HJ_CRC16_ALGO == HJ_CRC16_XMODEM
    EXPECT_EQ(hj_crc16(test1, strlen(test1)), 0x31C3);
    EXPECT_EQ(hj_crc16_xmodem(test1, strlen(test1)), 0x31C3);
#endif

    const char *test2 = "";
#if HJ_CRC16_ALGO == HJ_CRC16_MODBUS
    EXPECT_EQ(hj_crc16(test2, strlen(test2)), 0xFFFFU);
#elif HJ_CRC16_ALGO == HJ_CRC16_XMODEM
    EXPECT_EQ(hj_crc16(test2, strlen(test2)), 0x0000U);
#endif

    const char *test3 = "a";
    EXPECT_NE(hj_crc16(test3, strlen(test3)), 0U);

    const char *test4 = "abc";
    EXPECT_NE(hj_crc16(test4, strlen(test4)), 0U);
}

TEST(hj_crc16, context_lifecycle_and_reset)
{
    hj_crc16_ctx_t ctx;

    hj_crc16_init(&ctx);
#if HJ_CRC16_ALGO == HJ_CRC16_MODBUS
    EXPECT_EQ(ctx.state, 0xFFFFU);
#elif HJ_CRC16_ALGO == HJ_CRC16_XMODEM
    EXPECT_EQ(ctx.state, 0x0000U);
#endif

    const char *data = "The quick brown fox jumps over the lazy dog";
    hj_crc16_update(&ctx, data, strlen(data));
    uint16_t crc1 = hj_crc16_finalize(&ctx);

    EXPECT_EQ(crc1, hj_crc16(data, strlen(data)));

    hj_crc16_init(&ctx);
    hj_crc16_update(&ctx, data, strlen(data));
    uint16_t crc2 = hj_crc16_finalize(&ctx);
    EXPECT_EQ(crc1, crc2);
}

TEST(hj_crc16, incremental_and_chunked)
{
    const char *full_data = "The quick brown fox jumps over the lazy dog";
    size_t      len       = strlen(full_data);
    uint16_t    expected  = hj_crc16(full_data, len);

    hj_crc16_ctx_t ctx_byte;
    hj_crc16_init(&ctx_byte);
    for(size_t i = 0; i < len; ++i)
    {
        hj_crc16_update(&ctx_byte, &full_data[i], 1);
    }
    EXPECT_EQ(hj_crc16_finalize(&ctx_byte), expected);

    hj_crc16_ctx_t ctx_chunk;
    hj_crc16_init(&ctx_chunk);
    size_t chunks[] = {3, 5, 7, 2, 10};
    size_t offset   = 0;

    for(size_t chunk_len : chunks)
    {
        if(offset + chunk_len <= len)
        {
            hj_crc16_update(&ctx_chunk, full_data + offset, chunk_len);
            offset += chunk_len;
        }
    }
    if(offset < len)
    {
        hj_crc16_update(&ctx_chunk, full_data + offset, len - offset);
    }
    EXPECT_EQ(hj_crc16_finalize(&ctx_chunk), expected);
}

TEST(hj_crc16, null_and_boundary_conditions)
{
    const char *data = "test_boundary_data";

    hj_crc16_init(nullptr);
    hj_crc16_update(nullptr, data, strlen(data));
#if HJ_CRC16_ALGO == HJ_CRC16_MODBUS
    EXPECT_EQ(hj_crc16_finalize(nullptr), 0xFFFFU);
#elif HJ_CRC16_ALGO == HJ_CRC16_XMODEM
    EXPECT_EQ(hj_crc16_finalize(nullptr), 0x0000U);
#endif

    hj_crc16_ctx_t ctx;
    hj_crc16_init(&ctx);
    uint16_t initial_state = ctx.state;

    hj_crc16_update(&ctx, nullptr, 10);
    EXPECT_EQ(ctx.state, initial_state);

    hj_crc16_update(&ctx, data, 0);
    EXPECT_EQ(ctx.state, initial_state);

#if HJ_CRC16_ALGO == HJ_CRC16_MODBUS
    EXPECT_EQ(hj_crc16(nullptr, 0), 0xFFFFU);
    EXPECT_EQ(hj_crc16(nullptr, 10), 0xFFFFU);
    EXPECT_EQ(hj_crc16(data, 0), 0xFFFFU);
#elif HJ_CRC16_ALGO == HJ_CRC16_XMODEM
    EXPECT_EQ(hj_crc16(nullptr, 0), 0x0000U);
    EXPECT_EQ(hj_crc16(nullptr, 10), 0x0000U);
    EXPECT_EQ(hj_crc16(data, 0), 0x0000U);
#endif
}

TEST(hj_crc16, binary_data_coverage)
{
    std::vector<uint8_t> all_bytes(256);
    for(int i = 0; i < 256; ++i)
    {
        all_bytes[i] = static_cast<uint8_t>(i);
    }

    uint16_t result = hj_crc16(all_bytes.data(), all_bytes.size());
    EXPECT_NE(result, 0U);

    uint8_t zero_byte = 0x00;
    uint8_t ff_byte   = 0xFF;
    EXPECT_NE(hj_crc16(&zero_byte, 1), hj_crc16(&ff_byte, 1));
}

TEST(hj_crc16, large_text_crc)
{
    const char *big_data =
        R"(The class everyone had really been looking forward to was Defence Against the Dark Arts, but Quirrell's lessons turned out to be a bit of a joke.His classroom smelled strongly of garlic, which everyone said was to ward off a vampire he'd met in Romania, and was afraid would be coming back to get him one of these days.His turban, he told them, had been given to him by an African prince as a thank-you for getting rid of a troublesome zombie, but they weren't sure they believed this story.For one thing, when Seamus Finnegan asked eagerly to hear how Quirrell had fought off the zombie, Quirrell went pink and sta)";

    uint16_t result = hj_crc16(big_data, strlen(big_data));
    EXPECT_NE(result, 0U);
}