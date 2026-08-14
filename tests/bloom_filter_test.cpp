/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <hj/algo/bloom_filter.hpp>

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace hj;

TEST(bloom_filter_test, constructor_valid_parameters)
{
    bloom_filter bf(1000, 0.01);
    EXPECT_GT(bf.bit_size(), 0U);
    EXPECT_GE(bf.bit_size(), 64U);
    EXPECT_GT(bf.hash_count(), 0U);
    EXPECT_LE(bf.hash_count(), bloom_filter<>::max_hash_count);
    EXPECT_EQ(bf.insert_count(), 0U);
    EXPECT_EQ(bf.approx_insert_count(), 0U);
    EXPECT_EQ(bf.seed(), 0x9e3779b97f4a7c15ULL);
    EXPECT_DOUBLE_EQ(bf.fill_ratio(), 0.0);
    EXPECT_DOUBLE_EQ(bf.estimated_false_positive_rate(), 0.0);
}

TEST(bloom_filter_test, constructor_invalid_parameters)
{
    EXPECT_THROW(bloom_filter(0, 0.01), std::invalid_argument);

    EXPECT_THROW(bloom_filter(100, 0.0), std::invalid_argument);
    EXPECT_THROW(bloom_filter(100, -0.05), std::invalid_argument);
    EXPECT_THROW(bloom_filter(100, 1.0), std::invalid_argument);
    EXPECT_THROW(bloom_filter(100, 1.5), std::invalid_argument);
    EXPECT_THROW(bloom_filter(100, std::numeric_limits<double>::quiet_NaN()),
                 std::invalid_argument);
    EXPECT_THROW(bloom_filter(100, std::numeric_limits<double>::infinity()),
                 std::invalid_argument);

    EXPECT_THROW(bloom_filter(100, 0.01, 12345, 0), std::invalid_argument);

    EXPECT_THROW(bloom_filter(1000000, 0.0001, 12345, 100), std::length_error);
}

TEST(bloom_filter_test, basic_add_contains)
{
    bloom_filter bf(100, 0.01);
    bf.add("apple");
    bf.add("banana");
    bf.add("cherry");

    EXPECT_EQ(bf.insert_count(), 3U);
    EXPECT_TRUE(bf.contains("apple"));
    EXPECT_TRUE(bf.contains("banana"));
    EXPECT_TRUE(bf.contains("cherry"));
    EXPECT_FALSE(bf.contains("durian"));
}

TEST(bloom_filter_test, empty_string_support)
{
    bloom_filter bf(50, 0.01);
    EXPECT_FALSE(bf.contains(""));
    bf.add("");
    EXPECT_TRUE(bf.contains(""));
    EXPECT_EQ(bf.insert_count(), 1U);
}

TEST(bloom_filter_test, clear_functionality)
{
    bloom_filter bf(50, 0.01);
    bf.add("foo");
    bf.add("bar");
    EXPECT_TRUE(bf.contains("foo"));
    EXPECT_GT(bf.fill_ratio(), 0.0);

    bf.clear();
    EXPECT_FALSE(bf.contains("foo"));
    EXPECT_FALSE(bf.contains("bar"));
    EXPECT_EQ(bf.insert_count(), 0U);
    EXPECT_DOUBLE_EQ(bf.fill_ratio(), 0.0);
    EXPECT_DOUBLE_EQ(bf.estimated_false_positive_rate(), 0.0);
}

TEST(bloom_filter_test, fill_ratio_and_fpr_estimation)
{
    bloom_filter bf(100, 0.01);
    EXPECT_DOUBLE_EQ(bf.estimated_false_positive_rate(), 0.0);

    for(int i = 0; i < 500; ++i)
    {
        bf.add("item_" + std::to_string(i));
    }

    EXPECT_GT(bf.fill_ratio(), 0.0);
    EXPECT_LE(bf.fill_ratio(), 1.0);

    double estimated_fpr = bf.estimated_false_positive_rate();
    EXPECT_GT(estimated_fpr, 0.0);
    EXPECT_LE(estimated_fpr, 1.0);
}

TEST(bloom_filter_test, saturated_check)
{
    bloom_filter bf(10, 0.1);

    EXPECT_THROW(bf.saturated(0.0), std::invalid_argument);
    EXPECT_THROW(bf.saturated(-0.1), std::invalid_argument);
    EXPECT_THROW(bf.saturated(1.5), std::invalid_argument);

    EXPECT_FALSE(bf.saturated(0.5));

    for(int i = 0; i < 1000; ++i)
    {
        bf.add("saturate_" + std::to_string(i));
    }

    EXPECT_TRUE(bf.saturated(0.5));
}

TEST(bloom_filter_test, merge_success)
{
    bloom_filter bf1(100, 0.01, 12345);
    bloom_filter bf2(100, 0.01, 12345);

    bf1.add("key1");
    bf2.add("key2");

    bf1.merge(bf2);

    EXPECT_TRUE(bf1.contains("key1"));
    EXPECT_TRUE(bf1.contains("key2"));
    EXPECT_EQ(bf1.insert_count(), 2U);
}

TEST(bloom_filter_test, merge_incompatible_filters)
{
    bloom_filter bf1(100, 0.01, 12345);

    bloom_filter bf_diff_seed(100, 0.01, 54321);
    EXPECT_THROW(bf1.merge(bf_diff_seed), std::invalid_argument);

    bloom_filter bf_diff_size(500, 0.01, 12345);
    EXPECT_THROW(bf1.merge(bf_diff_size), std::invalid_argument);
}

TEST(bloom_filter_test, serialize_and_deserialize_roundtrip)
{
    bloom_filter bf(200, 0.01, 9999);
    for(int i = 0; i < 50; ++i)
    {
        bf.add("data_" + std::to_string(i));
    }

    std::string blob1;
    bf.serialize(blob1);
    EXPECT_FALSE(blob1.empty());

    std::string blob2 = bf.serialize();
    EXPECT_EQ(blob1, blob2);

    auto bf_deser = bloom_filter<std::string>::deserialize(blob1);

    EXPECT_EQ(bf_deser.bit_size(), bf.bit_size());
    EXPECT_EQ(bf_deser.hash_count(), bf.hash_count());
    EXPECT_EQ(bf_deser.seed(), bf.seed());
    EXPECT_EQ(bf_deser.insert_count(), bf.insert_count());

    for(int i = 0; i < 50; ++i)
    {
        EXPECT_TRUE(bf_deser.contains("data_" + std::to_string(i)));
    }
    EXPECT_FALSE(bf_deser.contains("non_existent_data"));
}

TEST(bloom_filter_test, deserialize_invalid_inputs)
{
    bloom_filter bf(100, 0.01);
    std::string  valid_blob = bf.serialize();

    EXPECT_THROW(bloom_filter<>::deserialize(valid_blob, 0),
                 std::invalid_argument);

    EXPECT_THROW(bloom_filter<>::deserialize("BLM2_short"),
                 std::invalid_argument);

    std::string bad_magic = valid_blob;
    bad_magic[0]          = 'X';
    EXPECT_THROW(bloom_filter<>::deserialize(bad_magic), std::invalid_argument);

    std::string bad_version = valid_blob;
    bad_version[4]          = static_cast<char>(99);
    EXPECT_THROW(bloom_filter<>::deserialize(bad_version),
                 std::invalid_argument);

    std::string truncated_blob = valid_blob.substr(0, valid_blob.size() - 4);
    EXPECT_THROW(bloom_filter<>::deserialize(truncated_blob),
                 std::invalid_argument);

    std::string trailing_blob = valid_blob + "extra_trash";
    EXPECT_THROW(bloom_filter<>::deserialize(trailing_blob),
                 std::invalid_argument);

    EXPECT_THROW(bloom_filter<>::deserialize(valid_blob, 10),
                 std::length_error);
}

TEST(bloom_filter_test, deserialize_canonical_padding_validation)
{
    bloom_filter bf(15, 0.01);
    ASSERT_NE(bf.bit_size() % 64, 0U);

    std::string blob = bf.serialize();

    blob[blob.size() - 8] |= static_cast<char>(0x80);

    EXPECT_THROW(bloom_filter<>::deserialize(blob), std::invalid_argument);
}

TEST(bloom_filter_test, copy_constructor_and_assignment)
{
    bloom_filter bf1(50, 0.01);
    bf1.add("x");
    bf1.add("y");

    bloom_filter bf2 = bf1;
    EXPECT_TRUE(bf2.contains("x"));
    EXPECT_TRUE(bf2.contains("y"));
    EXPECT_EQ(bf2.insert_count(), 2U);

    bloom_filter bf3(100, 0.05);
    bf3 = bf1;
    EXPECT_TRUE(bf3.contains("x"));
    EXPECT_TRUE(bf3.contains("y"));
    EXPECT_EQ(bf3.bit_size(), bf1.bit_size());
}

TEST(bloom_filter_test, move_constructor_and_assignment)
{
    bloom_filter bf1(50, 0.01);
    bf1.add("move_me");

    bloom_filter bf2 = std::move(bf1);
    EXPECT_TRUE(bf2.contains("move_me"));
    EXPECT_EQ(bf2.insert_count(), 1U);

    bloom_filter bf3(100, 0.05);
    bf3 = std::move(bf2);
    EXPECT_TRUE(bf3.contains("move_me"));
    EXPECT_EQ(bf3.insert_count(), 1U);
}

TEST(bloom_filter_test, type_support_pod_types)
{
    bloom_filter<int> bf_int(30, 0.01);
    bf_int.add(42);
    bf_int.add(7);
    EXPECT_TRUE(bf_int.contains(42));
    EXPECT_TRUE(bf_int.contains(7));
    EXPECT_FALSE(bf_int.contains(99));

    bloom_filter<double> bf_double(30, 0.01);
    bf_double.add(3.14159);
    EXPECT_TRUE(bf_double.contains(3.14159));
    EXPECT_FALSE(bf_double.contains(2.71828));

    struct Point
    {
        int x;
        int y;
    };
    bloom_filter<Point> bf_point(30, 0.01);
    Point               p1{1, 2};
    Point               p2{3, 4};
    bf_point.add(p1);

    EXPECT_TRUE(bf_point.contains(p1));
    EXPECT_FALSE(bf_point.contains(p2));
}

struct CustomKey
{
    uint32_t    id;
    std::string tag;
};

namespace hj
{
namespace detail
{
template <>
struct byte_view<CustomKey, void>
{
    static const void *data(const CustomKey &v) noexcept
    {
        return v.tag.data();
    }
    static size_t size(const CustomKey &v) noexcept { return v.tag.size(); }
};
} // namespace detail
} // namespace hj

TEST(bloom_filter_test, type_support_custom_byte_view_specialization)
{
    bloom_filter<CustomKey> bf(30, 0.01);
    CustomKey               k1{1, "key_alpha"};
    CustomKey               k2{2, "key_beta"};

    bf.add(k1);

    EXPECT_TRUE(bf.contains(k1));
    EXPECT_FALSE(bf.contains(k2));
}