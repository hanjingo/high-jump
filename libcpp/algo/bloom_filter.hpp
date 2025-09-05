#ifndef BLOOM_FILTER_HPP
#define BLOOM_FILTER_HPP

#include <bitset>
#include <cmath>
#include <cstdint>
#include <string>
#include <functional>

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

namespace libcpp
{


class bloom_filter {
public:
	bloom_filter(size_t expected_insertions, double false_positive_rate)
		: _expected_insertions{expected_insertions}
		, _false_positive_rate{false_positive_rate}
	{
		// Parameter validation
		if (_expected_insertions == 0)
			_expected_insertions = 100; // Avoid division by zero

		if (_false_positive_rate <= 0.0 || _false_positive_rate >= 1.0)
			_false_positive_rate = 0.01; // Default to 1% if invalid

		_num_bits = _optimal_num_of_bits(_expected_insertions, _false_positive_rate);
		_num_bits = (_num_bits < BLOOM_FILTER_MIN_BITS) ? BLOOM_FILTER_MIN_BITS : _num_bits;
		_num_bits = (_num_bits > BLOOM_FILTER_MAX_BITS) ? BLOOM_FILTER_MAX_BITS : _num_bits;

		_num_hash_functions = _optimal_num_of_hash_functions(_expected_insertions, _num_bits);
		_num_hash_functions = (_num_hash_functions < BLOOM_FILTER_MIN_HASHES) ? BLOOM_FILTER_MIN_HASHES : _num_hash_functions;
		_num_hash_functions = (_num_hash_functions > BLOOM_FILTER_MAX_HASHES) ? BLOOM_FILTER_MAX_HASHES : _num_hash_functions;
	}

    ~bloom_filter()
    {
    }

	inline void clear() { _bits.reset(); }
	inline size_t bit_size() const { return _num_bits; }
	inline size_t hash_count() const { return _num_hash_functions; }

	void add(const std::string& value) 
	{
		for (size_t i = 0; i < _num_hash_functions; ++i) 
		{
			size_t hash = _hash_i(value, i);
			_bits.set(hash % _num_bits);
		}
	}

	bool contains(const std::string& value) const 
	{
		for (size_t i = 0; i < _num_hash_functions; ++i) 
		{
			size_t hash = _hash_i(value, i);
			if (!_bits.test(hash % _num_bits))
				return false;
		}

		return true;
	}

	void serialize(std::string& s) const 
	{
		s.clear();
		s.reserve(_num_bits);
		for (size_t i = 0; i < _num_bits; ++i)
			s += _bits.test(i) ? '1' : '0';
	}

	bool unserialize(const std::string& s) 
	{
		if (s.size() != _num_bits)
			return false;

		for (size_t i = 0; i < s.size(); ++i)
			_bits.set(i, s[i] == '1');

		return true;
	}

private:
	static size_t _optimal_num_of_bits(size_t n, double p) 
	{
		return static_cast<size_t>(-1 * n * std::log(p) / (std::pow(std::log(2), 2)));
	}
    
	static size_t _optimal_num_of_hash_functions(size_t n, size_t m) 
	{
        if (n == 0) 
            return 1;
            
		return std::max<size_t>(1, static_cast<size_t>(std::round((double)m / n * std::log(2))));
	}


	static size_t _hash_i(const std::string& value, size_t i) 
	{
		std::hash<std::string> h1;
		std::hash<size_t> h2;
		size_t hval = h1(value);
		size_t hmix = hval + i * h2(hval);
		return hmix & 0x7FFFFFFFFFFFFFFF; // ensure non-negative, mask to 63 bits
	}

private:
	size_t _expected_insertions;
	double _false_positive_rate;
	size_t _num_bits;
	size_t _num_hash_functions;
	std::bitset<BLOOM_FILTER_MAX_BITS> _bits;
};

} // namespace libcpp

#endif // BLOOM_FILTER_HPP