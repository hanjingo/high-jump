#include <gtest/gtest.h>
#include <hj/algo/crc32.h>

TEST(crc32, standard_test_vectors)
{
    const char* test1 = "123456789";
    uint32_t result1 = crc32(test1, strlen(test1), 0);
    EXPECT_EQ(result1, 0xCBF43926) << "Standard test vector '123456789' failed";
    
    const char* test2 = "";
    uint32_t result2 = crc32(test2, strlen(test2), 0);
    EXPECT_EQ(result2, 0x00000000) << "Empty string test failed";
    
    const char* test3 = "a";
    uint32_t result3 = crc32(test3, strlen(test3), 0);
    EXPECT_EQ(result3, 0xE8B7BE43) << "Single character 'a' test failed";
     
    const char* test4 = "abc";
    uint32_t result4 = crc32(test4, strlen(test4), 0);
    EXPECT_EQ(result4, 0x352441C2) << "String 'abc' test failed";
}

TEST(crc32, large_text_crc)
{
    const char* big_data = R"(The class everyone had really been looking forward to was Defence Against the Dark Arts, but Quirrell's lessons turned out to be a bit of a joke.His classroom smelled strongly of garlic, which everyone said was to ward off a vampire he'd met in Romania, and was afraid would be coming back to get him one of these days.His turban, he told them, had been given to him by an African prince as a thank-you for getting rid of a troublesome zombie, but they weren't sure they believed this story.For one thing, when Seamus Finnegan asked eagerly to hear how Quirrell had fought off the zombie, Quirrell went pink and sta)";
    EXPECT_EQ(crc32(big_data, strlen(big_data), 0), 0x2E865E47);
}

TEST(crc32, incremental_calculation)
{
    const char* part1 = "Hello, ";
    const char* part2 = "World!";
    
    std::string combined = std::string(part1) + part2;
    uint32_t full_crc = crc32(combined.c_str(), combined.length(), 0);
    
    uint32_t crc1 = crc32(part1, strlen(part1), 0);
    uint32_t crc2 = crc32(part2, strlen(part2), crc1);
    
    EXPECT_EQ(full_crc, crc2) << "Incremental CRC calculation should match full calculation";
}

TEST(crc32, edge_cases)
{
    // EXPECT_EQ(crc32(nullptr, 0, 0), 0);
    // EXPECT_EQ(crc32(nullptr, 10, 123), 123);
    
    const char* data = "test";
    EXPECT_EQ(crc32(data, 0, 0), 0);
}