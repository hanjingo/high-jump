#include <gtest/gtest.h>
#include <hj/algo/uuid.hpp>

#include <algorithm>
#include <cctype>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

TEST(uuid, gen_u64_basic)
{
    uint64_t id1 = hj::uuid::gen_u64();
    uint64_t id2 = hj::uuid::gen_u64();
    uint64_t id3 = hj::uuid::gen_u64(false);

    EXPECT_NE(id1, 0ULL);
    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);
}

TEST(uuid, gen_u64_endianness)
{
    uint64_t big_endian_id    = hj::uuid::gen_u64(true);
    uint64_t little_endian_id = hj::uuid::gen_u64(false);

    uint64_t swapped = hj::detail::swap64(big_endian_id);
    EXPECT_EQ(hj::detail::swap64(swapped), big_endian_id);

    EXPECT_NE(big_endian_id, little_endian_id);
}

TEST(uuid, gen_rfc_format)
{
    std::string uuid_str = hj::uuid::gen();
    ASSERT_EQ(uuid_str.length(), 36U);

    EXPECT_EQ(uuid_str[8], '-');
    EXPECT_EQ(uuid_str[13], '-');
    EXPECT_EQ(uuid_str[18], '-');
    EXPECT_EQ(uuid_str[23], '-');

    EXPECT_EQ(uuid_str[14], '8');

    char variant_char = uuid_str[19];
    EXPECT_TRUE(variant_char == '8' || variant_char == '9'
                || variant_char == 'a' || variant_char == 'b');

    std::regex uuid_regex("^[0-9a-f]{8}-[0-9a-f]{4}-8[0-9a-f]{3}-[89ab][0-9a-f]"
                          "{3}-[0-9a-f]{12}$");
    EXPECT_TRUE(std::regex_match(uuid_str, uuid_regex));
}

TEST(uuid, gen_string_uniqueness)
{
    constexpr size_t                count = 1000;
    std::unordered_set<std::string> set;
    set.reserve(count);

    for(size_t i = 0; i < count; ++i)
    {
        std::string id     = hj::uuid::gen();
        auto [_, inserted] = set.insert(id);
        EXPECT_TRUE(inserted) << "Duplicate UUID generated: " << id;
    }
}

TEST(uuid, worker_id_initialization)
{
    EXPECT_NO_THROW(hj::detail::snowflake::init(0));
    EXPECT_NE(hj::uuid::gen_u64(), 0ULL);

    EXPECT_NO_THROW(hj::detail::snowflake::init(1023));
    EXPECT_NE(hj::uuid::gen_u64(), 0ULL);

    EXPECT_THROW(hj::detail::snowflake::init(1024), std::invalid_argument);
}

TEST(uuid, multithreaded_uniqueness)
{
    constexpr int thread_count   = 8;
    constexpr int ids_per_thread = 5000;

    std::vector<std::thread>              threads;
    std::vector<std::vector<uint64_t>>    u64_results(thread_count);
    std::vector<std::vector<std::string>> str_results(thread_count);

    for(int t = 0; t < thread_count; ++t)
    {
        threads.emplace_back([t, ids_per_thread, &u64_results, &str_results]() {
            u64_results[t].reserve(ids_per_thread);
            str_results[t].reserve(ids_per_thread);
            for(int i = 0; i < ids_per_thread; ++i)
            {
                u64_results[t].push_back(hj::uuid::gen_u64());
                str_results[t].push_back(hj::uuid::gen());
            }
        });
    }

    for(auto &th : threads)
    {
        th.join();
    }

    std::unordered_set<uint64_t> u64_set;
    u64_set.reserve(thread_count * ids_per_thread);
    for(const auto &vec : u64_results)
    {
        for(uint64_t id : vec)
        {
            auto [_, inserted] = u64_set.insert(id);
            EXPECT_TRUE(inserted)
                << "Concurrency collision detected for u64 ID: " << id;
        }
    }

    std::unordered_set<std::string> str_set;
    str_set.reserve(thread_count * ids_per_thread);
    for(const auto &vec : str_results)
    {
        for(const std::string &id : vec)
        {
            auto [_, inserted] = str_set.insert(id);
            EXPECT_TRUE(inserted)
                << "Concurrency collision detected for string UUID: " << id;
        }
    }
}

struct DummyGenU64
{
    static uint64_t gen_u64() { return 0x123456789ABCDEF0ULL; }
};

struct DummyGenParam
{
    static uint64_t gen_u64(bool big_endian)
    {
        return big_endian ? 0x1122334455667788ULL : 0x8877665544332211ULL;
    }
    static std::string gen() { return "custom-dummy-uuid-string"; }
};

TEST(uuid, custom_generator_extension)
{
    uint64_t custom_id = hj::uuid::gen_u64<DummyGenU64>(false);
    EXPECT_NE(custom_id, 0ULL);

    std::string custom_str = hj::uuid::gen<DummyGenU64>();
    EXPECT_EQ(custom_str.length(), 36U);
    EXPECT_EQ(custom_str[14], '8');

    EXPECT_EQ(hj::uuid::gen_u64<DummyGenParam>(true), 0x1122334455667788ULL);
    EXPECT_EQ(hj::uuid::gen_u64<DummyGenParam>(false), 0x8877665544332211ULL);
    EXPECT_EQ(hj::uuid::gen<DummyGenParam>(), "custom-dummy-uuid-string");
}