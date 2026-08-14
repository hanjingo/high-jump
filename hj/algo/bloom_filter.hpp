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

#ifndef HJ_BLOOM_FILTER_HPP
#define HJ_BLOOM_FILTER_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace hj
{
namespace detail
{

inline uint64_t popcount64(uint64_t x) noexcept
{
#if defined(__GNUC__) || defined(__clang__)
    return static_cast<uint64_t>(__builtin_popcountll(x));
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
    return static_cast<uint64_t>(__popcnt64(x));
#else
    x = x - ((x >> 1) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
    return (x * 0x0101010101010101ULL) >> 56;
#endif
}

inline void
mul64x64_128(uint64_t a, uint64_t b, uint64_t &hi, uint64_t &lo) noexcept
{
#if defined(__SIZEOF_INT128__)
    __uint128_t r = static_cast<__uint128_t>(a) * static_cast<__uint128_t>(b);
    hi            = static_cast<uint64_t>(r >> 64);
    lo            = static_cast<uint64_t>(r);
#else
    const uint64_t a_lo = static_cast<uint32_t>(a);
    const uint64_t a_hi = a >> 32;
    const uint64_t b_lo = static_cast<uint32_t>(b);
    const uint64_t b_hi = b >> 32;

    const uint64_t t1 = a_lo * b_lo;
    const uint64_t t2 = a_hi * b_lo + (t1 >> 32);
    const uint64_t t3 = a_lo * b_hi + static_cast<uint32_t>(t2);

    lo = (t3 << 32) | static_cast<uint32_t>(t1);
    hi = a_hi * b_hi + (t2 >> 32) + (t3 >> 32);
#endif
}

inline uint64_t reduce_range(uint64_t hash, uint64_t range) noexcept
{
    // range is guaranteed to be non-zero by bloom_filter.
    uint64_t hi;
    uint64_t lo;
    mul64x64_128(hash, range, hi, lo);
    (void) lo;
    return hi;
}

inline uint64_t rotl64(uint64_t x, unsigned int r) noexcept
{
    return (x << r) | (x >> (64U - r));
}

inline uint64_t fmix64(uint64_t k) noexcept
{
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdULL;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53ULL;
    k ^= k >> 33;
    return k;
}

// MurmurHash3 x64 128.
// The block loads are explicitly little-endian so that the hash result is
// stable across host endianness.
inline uint64_t load_u64_le(const uint8_t *p) noexcept
{
    return (static_cast<uint64_t>(p[0])) | (static_cast<uint64_t>(p[1]) << 8)
           | (static_cast<uint64_t>(p[2]) << 16)
           | (static_cast<uint64_t>(p[3]) << 24)
           | (static_cast<uint64_t>(p[4]) << 32)
           | (static_cast<uint64_t>(p[5]) << 40)
           | (static_cast<uint64_t>(p[6]) << 48)
           | (static_cast<uint64_t>(p[7]) << 56);
}

inline void murmur3_x64_128(const void *key,
                            size_t      len,
                            uint64_t    seed,
                            uint64_t    out[2]) noexcept
{
    const uint8_t *data    = static_cast<const uint8_t *>(key);
    const size_t   nblocks = len / 16;

    uint64_t h1 = seed;
    uint64_t h2 = seed;

    static constexpr uint64_t c1 = 0x87c37b91114253d5ULL;
    static constexpr uint64_t c2 = 0x4cf5ad432745937fULL;

    for(size_t i = 0; i < nblocks; ++i)
    {
        const uint8_t *block = data + i * 16;
        uint64_t       k1    = load_u64_le(block);
        uint64_t       k2    = load_u64_le(block + 8);

        k1 *= c1;
        k1 = rotl64(k1, 31);
        k1 *= c2;
        h1 ^= k1;
        h1 = rotl64(h1, 27);
        h1 += h2;
        h1 = h1 * 5 + 0x52dce729;

        k2 *= c2;
        k2 = rotl64(k2, 33);
        k2 *= c1;
        h2 ^= k2;
        h2 = rotl64(h2, 31);
        h2 += h1;
        h2 = h2 * 5 + 0x38495ab5;
    }

    const uint8_t *tail = data + nblocks * 16;
    uint64_t       k1   = 0;
    uint64_t       k2   = 0;

    switch(len & 15U)
    {
        case 15:
            k2 ^= uint64_t(tail[14]) << 48;
            [[fallthrough]];
        case 14:
            k2 ^= uint64_t(tail[13]) << 40;
            [[fallthrough]];
        case 13:
            k2 ^= uint64_t(tail[12]) << 32;
            [[fallthrough]];
        case 12:
            k2 ^= uint64_t(tail[11]) << 24;
            [[fallthrough]];
        case 11:
            k2 ^= uint64_t(tail[10]) << 16;
            [[fallthrough]];
        case 10:
            k2 ^= uint64_t(tail[9]) << 8;
            [[fallthrough]];
        case 9:
            k2 ^= uint64_t(tail[8]);
            k2 *= c2;
            k2 = rotl64(k2, 33);
            k2 *= c1;
            h2 ^= k2;
            [[fallthrough]];
        case 8:
            k1 ^= uint64_t(tail[7]) << 56;
            [[fallthrough]];
        case 7:
            k1 ^= uint64_t(tail[6]) << 48;
            [[fallthrough]];
        case 6:
            k1 ^= uint64_t(tail[5]) << 40;
            [[fallthrough]];
        case 5:
            k1 ^= uint64_t(tail[4]) << 32;
            [[fallthrough]];
        case 4:
            k1 ^= uint64_t(tail[3]) << 24;
            [[fallthrough]];
        case 3:
            k1 ^= uint64_t(tail[2]) << 16;
            [[fallthrough]];
        case 2:
            k1 ^= uint64_t(tail[1]) << 8;
            [[fallthrough]];
        case 1:
            k1 ^= uint64_t(tail[0]);
            k1 *= c1;
            k1 = rotl64(k1, 31);
            k1 *= c2;
            h1 ^= k1;
            [[fallthrough]];
        case 0:
            break;
    }

    h1 ^= static_cast<uint64_t>(len);
    h2 ^= static_cast<uint64_t>(len);

    h1 += h2;
    h2 += h1;
    h1 = fmix64(h1);
    h2 = fmix64(h2);
    h1 += h2;
    h2 += h1;

    out[0] = h1;
    out[1] = h2;
}

// Customization point for types whose semantic representation is not simply
// a byte range. The default is intentionally restricted to trivially
// copyable types. For structs containing padding bytes or non-canonical
// representations, users should provide a specialization.
template <typename T, typename Enable = void>
struct byte_view
{
    static_assert(std::is_trivially_copyable<T>::value,
                  "hj::bloom_filter: T must be trivially copyable, or provide "
                  "a specialization of hj::detail::byte_view<T>.");

    static const void *data(const T &v) noexcept { return &v; }

    static size_t size(const T &v) noexcept
    {
        (void) v;
        return sizeof(T);
    }
};

template <>
struct byte_view<std::string, void>
{
    static const void *data(const std::string &v) noexcept { return v.data(); }

    static size_t size(const std::string &v) noexcept { return v.size(); }
};

class dynamic_bitset
{
  public:
    explicit dynamic_bitset(size_t num_bits = 0)
        : _num_bits(num_bits)
        , _data((num_bits + 63) / 64, uint64_t(0))
    {
    }

    dynamic_bitset(const dynamic_bitset &)                = default;
    dynamic_bitset(dynamic_bitset &&) noexcept            = default;
    dynamic_bitset &operator=(const dynamic_bitset &)     = default;
    dynamic_bitset &operator=(dynamic_bitset &&) noexcept = default;

    void set(size_t pos) noexcept
    {
        _data[pos >> 6] |= uint64_t(1) << (pos & 63);
    }

    bool test(size_t pos) const noexcept
    {
        return ((_data[pos >> 6] >> (pos & 63)) & 1ULL) != 0;
    }

    void reset() noexcept
    {
        std::fill(_data.begin(), _data.end(), uint64_t(0));
    }

    size_t size() const noexcept { return _num_bits; }

    uint64_t popcount() const noexcept
    {
        uint64_t total = 0;
        for(uint64_t word : _data)
            total += popcount64(word);
        return total;
    }

    dynamic_bitset &operator|=(const dynamic_bitset &other) noexcept
    {
        // The caller is responsible for matching sizes.
        for(size_t i = 0; i < _data.size(); ++i)
            _data[i] |= other._data[i];
        return *this;
    }

    const std::vector<uint64_t> &words() const noexcept { return _data; }

    std::vector<uint64_t> &words() noexcept { return _data; }

  private:
    size_t                _num_bits;
    std::vector<uint64_t> _data;
};

inline void append_u64_be(std::string &out, uint64_t value)
{
    for(int shift = 56; shift >= 0; shift -= 8)
        out.push_back(static_cast<char>((value >> shift) & 0xffU));
}

inline uint64_t read_u64_be(const std::string &data, size_t &offset)
{
    constexpr size_t width = sizeof(uint64_t);

    if(offset > data.size() || data.size() - offset < width)
        throw std::invalid_argument(
            "hj::bloom_filter: truncated serialized uint64");

    const uint8_t *p = reinterpret_cast<const uint8_t *>(data.data() + offset);

    uint64_t value = (static_cast<uint64_t>(p[0]) << 56)
                     | (static_cast<uint64_t>(p[1]) << 48)
                     | (static_cast<uint64_t>(p[2]) << 40)
                     | (static_cast<uint64_t>(p[3]) << 32)
                     | (static_cast<uint64_t>(p[4]) << 24)
                     | (static_cast<uint64_t>(p[5]) << 16)
                     | (static_cast<uint64_t>(p[6]) << 8)
                     | (static_cast<uint64_t>(p[7]));

    offset += width;
    return value;
}

inline bool checked_mul_size(size_t a, size_t b, size_t &result) noexcept
{
    if(b != 0 && a > std::numeric_limits<size_t>::max() / b)
        return false;
    result = a * b;
    return true;
}

} // namespace detail

template <typename T = std::string>
class bloom_filter
{
  public:
    static constexpr uint8_t serialization_version = 2;
    static constexpr size_t  max_hash_count        = 32;
    static constexpr size_t  default_max_bits      = (size_t(1) << 34);

    bloom_filter(size_t   expected_insertions,
                 double   false_positive_rate,
                 uint64_t seed     = 0x9e3779b97f4a7c15ULL,
                 size_t   max_bits = default_max_bits)
        : _seed(seed)
        , _num_bits(0)
        , _num_hashes(0)
        , _num_inserted(0)
        , _bits()
    {
        if(expected_insertions == 0)
            throw std::invalid_argument(
                "bloom_filter: expected_insertions must be > 0");

        if(!std::isfinite(false_positive_rate)
           || !(false_positive_rate > 0.0 && false_positive_rate < 1.0))
            throw std::invalid_argument("bloom_filter: false_positive_rate "
                                        "must be finite and in (0, 1)");

        if(max_bits == 0)
            throw std::invalid_argument("bloom_filter: max_bits must be > 0");

        const size_t m = _optimal_num_bits(expected_insertions,
                                           false_positive_rate,
                                           max_bits);

        _num_bits = std::max<size_t>(m, 64);

        if(_num_bits > max_bits)
            throw std::length_error(
                "bloom_filter: computed bit-array size exceeds max_bits");

        _num_hashes = std::min<size_t>(
            max_hash_count,
            std::max<size_t>(
                1,
                _optimal_num_hashes(expected_insertions, _num_bits)));

        _bits = detail::dynamic_bitset(_num_bits);
    }

    bloom_filter(const bloom_filter &)                = default;
    bloom_filter(bloom_filter &&) noexcept            = default;
    bloom_filter &operator=(const bloom_filter &)     = default;
    bloom_filter &operator=(bloom_filter &&) noexcept = default;
    ~bloom_filter()                                   = default;

    // Not thread-safe. Concurrent mutation/read requires external
    // synchronization.
    void add(const T &value)
    {
        uint64_t h[2];
        _hash_value(value, h);

        for(size_t i = 0; i < _num_hashes; ++i)
            _bits.set(detail::reduce_range(_combine(h, i), _num_bits));

        if(_num_inserted != std::numeric_limits<size_t>::max())
            ++_num_inserted;
    }

    bool contains(const T &value) const
    {
        uint64_t h[2];
        _hash_value(value, h);

        for(size_t i = 0; i < _num_hashes; ++i)
        {
            const size_t pos = static_cast<size_t>(
                detail::reduce_range(_combine(h, i), _num_bits));

            if(!_bits.test(pos))
                return false;
        }

        return true;
    }

    void clear() noexcept
    {
        _bits.reset();
        _num_inserted = 0;
    }

    size_t bit_size() const noexcept { return _num_bits; }

    size_t hash_count() const noexcept { return _num_hashes; }

    // Number of add() calls, not the number of distinct keys.
    size_t insert_count() const noexcept { return _num_inserted; }

    // Kept for source compatibility with the previous implementation.
    size_t approx_insert_count() const noexcept { return _num_inserted; }

    uint64_t seed() const noexcept { return _seed; }

    double fill_ratio() const noexcept
    {
        if(_num_bits == 0)
            return 0.0;

        return static_cast<double>(_bits.popcount())
               / static_cast<double>(_num_bits);
    }

    double estimated_false_positive_rate() const noexcept
    {
        const double ratio = fill_ratio();

        if(ratio <= 0.0)
            return 0.0;
        if(ratio >= 1.0)
            return 1.0;

        return std::pow(ratio, static_cast<double>(_num_hashes));
    }

    bool saturated(double threshold = 0.95) const
    {
        if(!std::isfinite(threshold) || threshold <= 0.0 || threshold > 1.0)
            throw std::invalid_argument(
                "bloom_filter::saturated: threshold must be in (0, 1]");

        return fill_ratio() >= threshold;
    }

    void merge(const bloom_filter &other)
    {
        if(other._num_bits != _num_bits || other._num_hashes != _num_hashes
           || other._seed != _seed)
            throw std::invalid_argument(
                "bloom_filter::merge: filters have incompatible parameters "
                "(bit_size/hash_count/seed must match)");

        _bits |= other._bits;

        if(other._num_inserted
           > std::numeric_limits<size_t>::max() - _num_inserted)
            _num_inserted = std::numeric_limits<size_t>::max();
        else
            _num_inserted += other._num_inserted;
    }

    // Binary format v2:
    //   4 bytes  magic: "BLM2"
    //   1 byte   version: 2
    //   8 bytes  seed, big-endian
    //   8 bytes  num_bits, big-endian
    //   8 bytes  num_hashes, big-endian
    //   8 bytes  insert_count, big-endian
    //   N*8     bitmap words, big-endian
    //
    // The format is fixed-endian and independent of host byte order.
    void serialize(std::string &out) const
    {
        const auto &words = _bits.words();

        size_t word_bytes = 0;
        if(!detail::checked_mul_size(words.size(),
                                     sizeof(uint64_t),
                                     word_bytes))
            throw std::length_error(
                "bloom_filter::serialize: serialized size overflow");

        constexpr size_t header_size = 37;
        if(word_bytes > std::numeric_limits<size_t>::max() - header_size)
            throw std::length_error(
                "bloom_filter::serialize: serialized size overflow");

        const size_t total_size = header_size + word_bytes;

        out.clear();
        out.reserve(total_size);

        out.append("BLM2", 4);
        out.push_back(static_cast<char>(serialization_version));

        detail::append_u64_be(out, _seed);
        detail::append_u64_be(out, static_cast<uint64_t>(_num_bits));
        detail::append_u64_be(out, static_cast<uint64_t>(_num_hashes));
        detail::append_u64_be(out, static_cast<uint64_t>(_num_inserted));

        for(uint64_t word : words)
            detail::append_u64_be(out, word);
    }

    std::string serialize() const
    {
        std::string out;
        serialize(out);
        return out;
    }

    static bloom_filter deserialize(const std::string &blob,
                                    size_t max_bits = default_max_bits)
    {
        constexpr size_t header_size = 37;

        if(max_bits == 0)
            throw std::invalid_argument(
                "bloom_filter::deserialize: max_bits must be > 0");

        if(blob.size() < header_size || blob.compare(0, 4, "BLM2") != 0)
            throw std::invalid_argument(
                "bloom_filter::deserialize: bad magic/header");

        if(static_cast<uint8_t>(blob[4]) != serialization_version)
            throw std::invalid_argument(
                "bloom_filter::deserialize: unsupported version");

        size_t offset = 5;

        const uint64_t seed             = detail::read_u64_be(blob, offset);
        const uint64_t num_bits_u64     = detail::read_u64_be(blob, offset);
        const uint64_t num_hashes_u64   = detail::read_u64_be(blob, offset);
        const uint64_t num_inserted_u64 = detail::read_u64_be(blob, offset);

        if(num_bits_u64 == 0)
            throw std::invalid_argument(
                "bloom_filter::deserialize: num_bits must be > 0");

        if(num_bits_u64 > static_cast<uint64_t>(max_bits))
            throw std::length_error(
                "bloom_filter::deserialize: bit-array exceeds max_bits");

        if(num_bits_u64
           > static_cast<uint64_t>(std::numeric_limits<size_t>::max()))
            throw std::length_error(
                "bloom_filter::deserialize: num_bits does not fit size_t");

        if(num_hashes_u64 == 0 || num_hashes_u64 > max_hash_count)
            throw std::invalid_argument(
                "bloom_filter::deserialize: invalid hash count");

        if(num_inserted_u64
           > static_cast<uint64_t>(std::numeric_limits<size_t>::max()))
            throw std::length_error(
                "bloom_filter::deserialize: insert count does not fit size_t");

        const size_t num_bits  = static_cast<size_t>(num_bits_u64);
        const size_t num_words = num_bits / 64 + ((num_bits % 64) != 0 ? 1 : 0);

        size_t word_bytes = 0;
        if(!detail::checked_mul_size(num_words, sizeof(uint64_t), word_bytes))
            throw std::length_error(
                "bloom_filter::deserialize: bitmap size overflow");

        if(word_bytes > blob.size() - offset
           || blob.size() - offset != word_bytes)
            throw std::invalid_argument(
                "bloom_filter::deserialize: payload size mismatch");

        bloom_filter bf =
            create_empty(num_bits, static_cast<size_t>(num_hashes_u64), seed);

        auto &words = bf._bits.words();

        for(size_t i = 0; i < num_words; ++i)
            words[i] = detail::read_u64_be(blob, offset);

        // Canonical encoding: unused bits in the final word must be zero.
        const size_t unused_bits = num_words * 64 - num_bits;
        if(unused_bits != 0)
        {
            const uint64_t invalid_mask = ~uint64_t(0) << (64 - unused_bits);

            if((words.back() & invalid_mask) != 0)
                throw std::invalid_argument(
                    "bloom_filter::deserialize: non-zero unused bitmap bits");
        }

        if(offset != blob.size())
            throw std::invalid_argument(
                "bloom_filter::deserialize: trailing data");

        bf._num_inserted = static_cast<size_t>(num_inserted_u64);
        return bf;
    }

  private:
    struct raw_tag
    {
    };

    bloom_filter(size_t num_bits, size_t num_hashes, uint64_t seed, raw_tag)
        : _seed(seed)
        , _num_bits(num_bits)
        , _num_hashes(num_hashes)
        , _num_inserted(0)
        , _bits(num_bits)
    {
        if(_num_bits == 0)
            throw std::invalid_argument("bloom_filter: num_bits must be > 0");

        if(_num_hashes == 0 || _num_hashes > max_hash_count)
            throw std::invalid_argument("bloom_filter: invalid hash count");
    }

    static bloom_filter
    create_empty(size_t num_bits, size_t num_hashes, uint64_t seed)
    {
        return bloom_filter(num_bits, num_hashes, seed, raw_tag{});
    }

    static size_t _optimal_num_bits(size_t n, double p, size_t max_bits)
    {
        const long double nd   = static_cast<long double>(n);
        const long double pd   = static_cast<long double>(p);
        const long double log2 = std::log(2.0L);

        const long double m = (-nd * std::log(pd)) / (log2 * log2);

        if(!std::isfinite(m) || m <= 0.0L
           || m > static_cast<long double>(max_bits))
            throw std::length_error("bloom_filter: computed bit-array size is "
                                    "invalid or exceeds max_bits");

        const long double rounded = std::ceil(m);

        if(rounded
           > static_cast<long double>(std::numeric_limits<size_t>::max()))
            throw std::length_error(
                "bloom_filter: computed bit-array size does not fit size_t");

        return static_cast<size_t>(rounded);
    }

    static size_t _optimal_num_hashes(size_t n, size_t m)
    {
        const long double k = static_cast<long double>(m)
                              / static_cast<long double>(n) * std::log(2.0L);

        if(!std::isfinite(k) || k <= 0.0L)
            return 1;

        // Avoid std::llround() here: for extreme user-supplied max_bits, k
        // may exceed the range of long long even though we only need to clamp
        // it to max_hash_count.
        if(k >= static_cast<long double>(max_hash_count))
            return max_hash_count;

        const long double rounded = std::floor(k + 0.5L);
        if(rounded <= 1.0L)
            return 1;

        return static_cast<size_t>(rounded);
    }

    void _hash_value(const T &value, uint64_t out[2]) const
    {
        const void  *data = detail::byte_view<T>::data(value);
        const size_t size = detail::byte_view<T>::size(value);

        if(size != 0 && data == nullptr)
            throw std::invalid_argument("bloom_filter: byte_view returned null "
                                        "data for non-empty value");

        detail::murmur3_x64_128(data, size, _seed, out);
    }

    static uint64_t _combine(const uint64_t h[2], size_t i) noexcept
    {
        // Unsigned overflow is intentional and defined by C++.
        return h[0] + static_cast<uint64_t>(i) * h[1];
    }

  private:
    uint64_t               _seed;
    size_t                 _num_bits;
    size_t                 _num_hashes;
    size_t                 _num_inserted;
    detail::dynamic_bitset _bits;
};

} // namespace hj

#endif
