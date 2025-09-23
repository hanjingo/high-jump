#include <gtest/gtest.h>

// #define CRC16_MODBUS_ENABLED
#define CRC16_XMODEM_ENABLED
#include <hj/algo/crc16.h>
#include <cstring>

TEST(crc16, standard_test_vectors)
{
    const char* test1 = "123456789";
    uint16_t result1 = crc16(test1, strlen(test1), 0);
#if CRC16_ALGO == CRC16_MODBUS
    EXPECT_EQ(result1, 0x4B37) << "Standard test vector '123456789' failed, actual: 0x" << std::hex << result1;
#elif CRC16_ALGO == CRC16_XMODEM
    EXPECT_EQ(result1, 0x31C3) << "Standard test vector '123456789' failed, actual: 0x" << std::hex << result1;
#endif
    
    const char* test2 = "";
    uint16_t result2 = crc16(test2, strlen(test2), 0);
#if CRC16_ALGO == CRC16_MODBUS
    EXPECT_EQ(result2, 0xFFFF) << "Empty string test failed, actual: 0x" << std::hex << result2;
#elif CRC16_ALGO == CRC16_XMODEM
    EXPECT_EQ(result2, 0x0000) << "Empty string test failed, actual: 0x" << std::hex << result2;
#endif

    const char* test3 = "a";
    uint16_t result3 = crc16(test3, strlen(test3), 0);
#if CRC16_ALGO == CRC16_MODBUS
    EXPECT_NE(result3, 0) << "Single character 'a' should produce non-zero CRC";
#elif CRC16_ALGO == CRC16_XMODEM
    EXPECT_NE(result3, 0) << "Single character 'a' should produce non-zero CRC";
#endif

    const char* test4 = "abc";
    uint16_t result4 = crc16(test4, strlen(test4), 0);
#if CRC16_ALGO == CRC16_MODBUS
    EXPECT_NE(result4, 0) << "String 'abc' should produce non-zero CRC";
#elif CRC16_ALGO == CRC16_XMODEM
    EXPECT_NE(result4, 0) << "String 'abc' should produce non-zero CRC";
#endif
}

TEST(crc16, large_text_crc)
{
    const char* big_data = R"(The class everyone had really been looking forward to was Defence Against the Dark Arts, but Quirrell's lessons turned out to be a bit of a joke.His classroom smelled strongly of garlic, which everyone said was to ward off a vampire he'd met in Romania, and was afraid would be coming back to get him one of these days.His turban, he told them, had been given to him by an African prince as a thank-you for getting rid of a troublesome zombie, but they weren't sure they believed this story.For one thing, when Seamus Finnegan asked eagerly to hear how Quirrell had fought off the zombie, Quirrell went pink and sta)";
    
    uint16_t result = crc16(big_data, strlen(big_data), 0);
    
    EXPECT_NE(result, 0) << "Large text should produce non-zero CRC";
    EXPECT_NE(result, 0xFFFF) << "Large text should not produce initial value";
}

TEST(crc16, incremental_calculation)
{
    const char* part1 = "Hello, ";
    const char* part2 = "World!";
    
    std::string combined = std::string(part1) + part2;
    uint16_t full_crc = crc16(combined.c_str(), combined.length(), 0);
    
    uint16_t crc1 = crc16(part1, strlen(part1), 0);
    uint16_t crc2 = crc16(part2, strlen(part2), crc1);

#if CRC16_ALGO == CRC16_MODBUS
    EXPECT_EQ(full_crc, crc2) << "Incremental CRC calculation should match full calculation";
#elif CRC16_ALGO == CRC16_XMODEM
    EXPECT_EQ(full_crc, crc2) << "Incremental CRC calculation should match full calculation";
#endif
}

TEST(crc16, edge_cases)
{
const char* data = "test";

#if CRC16_ALGO == CRC16_MODBUS
    EXPECT_EQ(crc16(nullptr, 0, 0), 0xFFFF) << "CRC of null data with zero length should be initial value (0xFFFF)";
    EXPECT_EQ(crc16(nullptr, 10, 123), 123);
    EXPECT_EQ(crc16(data, 0, 0), 0xFFFF) << "CRC of zero length data should be initial value (0xFFFF)";
#elif CRC16_ALGO == CRC16_XMODEM
    EXPECT_EQ(crc16(nullptr, 0, 0), 0x0000) << "CRC of null data with zero length should be initial value (0x0000)";
    EXPECT_EQ(crc16(nullptr, 10, 123), 123);
    EXPECT_EQ(crc16(data, 0, 0), 0x0000) << "CRC of zero length data should be initial value (0x0000)";
#endif
}

TEST(crc16, consistency_check)
{
    const char* test_data = "The quick brown fox jumps over the lazy dog";
    
    uint16_t crc1 = crc16(test_data, strlen(test_data), 0);
    uint16_t crc2 = crc16(test_data, strlen(test_data), 0);
    uint16_t crc3 = crc16(test_data, strlen(test_data), 0);
    
    EXPECT_EQ(crc1, crc2);
    EXPECT_EQ(crc2, crc3);
    
    const char* different_data = "The quick brown fox jumps over the lazy cat";
    uint16_t different_crc = crc16(different_data, strlen(different_data), 0);
    
#if CRC16_ALGO == CRC16_MODBUS
    EXPECT_NE(crc1, different_crc) << "Different data should produce different CRC";
#elif CRC16_ALGO == CRC16_XMODEM
    EXPECT_NE(crc1, different_crc) << "Different data should produce different CRC";
#endif
}

TEST(crc16, incremental_detailed)
{
    const char* parts[] = {"The ", "quick ", "brown ", "fox"};
    size_t num_parts = sizeof(parts) / sizeof(parts[0]);
    
    std::string full_string;
    for (size_t i = 0; i < num_parts; ++i) {
        full_string += parts[i];
    }
    
    uint16_t full_crc = crc16(full_string.c_str(), full_string.length(), 0);
    
    uint16_t incremental_crc = 0;
    for (size_t i = 0; i < num_parts; ++i) {
        incremental_crc = crc16(parts[i], strlen(parts[i]), incremental_crc);
    }

#if CRC16_ALGO == CRC16_MODBUS
    EXPECT_EQ(full_crc, incremental_crc)
        << "Multi-part incremental calculation should match full calculation";
#elif CRC16_ALGO == CRC16_XMODEM
    EXPECT_EQ(full_crc, incremental_crc)
        << "Multi-part incremental calculation should match full calculation";
#endif
}