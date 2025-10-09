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
#ifndef BLOOM_FILTER_HPP
#define BLOOM_FILTER_HPP

#include <cmath>
#include <cstdint>
#include <string>
#include <functional>
#include <memory>
#include <cstring>
#include <algorithm>

#ifndef BLOOM_FILTER_MIN_BITS
#define BLOOM_FILTER_MIN_BITS 64
#endif

#ifndef BLOOM_FILTER_MAX_BITS
#define BLOOM_FILTER_MAX_BITS (1 << 16) // 8KB bits
#endif

#ifndef BLOOM_FILTER_MIN_HASHES
#define BLOOM_FILTER_MIN_HASHES 1
#endif

#ifndef BLOOM_FILTER_MAX_HASHES
#define BLOOM_FILTER_MAX_HASHES 16
#endif

namespace hj
{
namespace detail
{

class dynamic_bitset
{
  public:
    explicit dynamic_bitset(size_t num_bits)
        : _num_bits(num_bits)
        , _num_words((num_bits + 63) / 64)
        , _data(new uint64_t[_num_words])
    {
        reset();
    }

    dynamic_bitset(const dynamic_bitset &other)
        : _num_bits(other._num_bits)
        , _num_words(other._num_words)
        , _data(new uint64_t[other._num_words])
    {
        std::memcpy(_data.get(),
                    other._data.get(),
                    _num_words * sizeof(uint64_t));
    }

    dynamic_bitset &operator=(const dynamic_bitset &other)
    {
        if(this != &other)
        {
            _num_bits  = other._num_bits;
            _num_words = other._num_words;
            _data.reset(new uint64_t[_num_words]);
            std::memcpy(_data.get(),
                        other._data.get(),
                        _num_words * sizeof(uint64_t));
        }
        return *this;
    }

    void set(size_t pos, bool value = true)
    {
        if(pos >= _num_bits)
            return;

        size_t word = pos / 64;
        size_t bit  = pos % 64;
        if(value)
            _data[word] |= (uint64_t(1) << bit);
        else
            _data[word] &= ~(uint64_t(1) << bit);
    }

    bool test(size_t pos) const
    {
        if(pos >= _num_bits)
            return false;

        size_t word = pos / 64;
        size_t bit  = pos % 64;
        return (_data[word] >> bit) & 1;
    }

    void reset() { std::memset(_data.get(), 0, _num_words * sizeof(uint64_t)); }

    size_t size() const { return _num_bits; }

  private:
    size_t                      _num_bits;
    size_t                      _num_words;
    std::unique_ptr<uint64_t[]> _data;
};

} // namespace detail

template <typename T = std::string>
class bloom_filter
{
  public:
    bloom_filter(size_t expected_insertions, double false_positive_rate)
        : _expected_insertions{expected_insertions}
        , _false_positive_rate{false_positive_rate}
        , _num_bits(0)
        , _num_hash_functions(0)
        , _bits(1)
    {
        if(_expected_insertions == 0)
            _expected_insertions = 100;

        if(_false_positive_rate <= 0.0 || _false_positive_rate >= 1.0)
            _false_positive_rate = 0.01;

        _num_bits =
            _optimal_num_of_bits(_expected_insertions, _false_positive_rate);
        _num_bits = (_num_bits < BLOOM_FILTER_MIN_BITS) ? BLOOM_FILTER_MIN_BITS
                                                        : _num_bits;
        _num_bits = (_num_bits > BLOOM_FILTER_MAX_BITS) ? BLOOM_FILTER_MAX_BITS
                                                        : _num_bits;

        _num_hash_functions =
            _optimal_num_of_hash_functions(_expected_insertions, _num_bits);
        _num_hash_functions = (_num_hash_functions < BLOOM_FILTER_MIN_HASHES)
                                  ? BLOOM_FILTER_MIN_HASHES
                                  : _num_hash_functions;
        _num_hash_functions = (_num_hash_functions > BLOOM_FILTER_MAX_HASHES)
                                  ? BLOOM_FILTER_MAX_HASHES
                                  : _num_hash_functions;

        _bits = detail::dynamic_bitset(_num_bits);
    }

    ~bloom_filter() = default;

    inline void   clear() { _bits.reset(); }
    inline size_t bit_size() const { return _num_bits; }
    inline size_t hash_count() const { return _num_hash_functions; }

    void add(const T &value)
    {
        _assert_not_empty(value);
        for(size_t i = 0; i < _num_hash_functions; ++i)
        {
            size_t hash = _hash_i(value, i);
            _bits.set(hash % _num_bits);
        }
    }

    bool contains(const T &value) const
    {
        _assert_not_empty(value);
        for(size_t i = 0; i < _num_hash_functions; ++i)
        {
            size_t hash = _hash_i(value, i);
            if(!_bits.test(hash % _num_bits))
                return false;
        }
        return true;
    }

    void serialize(std::string &s) const
    {
        s.clear();
        s.reserve(_num_bits);
        for(size_t i = 0; i < _num_bits; ++i)
            s += _bits.test(i) ? '1' : '0';
    }

    bool unserialize(const std::string &s)
    {
        if(s.size() != _num_bits)
            return false;
        for(size_t i = 0; i < s.size(); ++i)
            _bits.set(i, s[i] == '1');
        return true;
    }

  private:
    static size_t _optimal_num_of_bits(size_t n, double p)
    {
        return static_cast<size_t>(-1 * n * std::log(p)
                                   / (std::pow(std::log(2), 2)));
    }

    static size_t _optimal_num_of_hash_functions(size_t n, size_t m)
    {
        if(n == 0)
            return 1;
        return std::max<size_t>(
            1,
            static_cast<size_t>(std::round((double) m / n * std::log(2))));
    }

    static size_t _hash_i(const T &value, size_t i)
    {
        std::hash<T>      h1;
        std::hash<size_t> h2;
        size_t            hval = h1(value);
        size_t            hmix = hval + i * h2(hval);
        return hmix & 0x7FFFFFFFFFFFFFFF;
    }

    template <typename U = T>
    static void _assert_not_empty(const U &value)
    {
        if constexpr(std::is_same<U, std::string>::value)
            assert(!value.empty() && "bloom_filter: value must not be empty");
    }

  private:
    size_t                 _expected_insertions;
    double                 _false_positive_rate;
    size_t                 _num_bits;
    size_t                 _num_hash_functions;
    detail::dynamic_bitset _bits;
};

} // namespace hj

#endif //