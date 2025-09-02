#ifndef BLOOM_FILTER_HPP
#define BLOOM_FILTER_HPP

#include <deque>
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

class bloom_filter 
{
public:
	bloom_filter(size_t expected_insertions, double false_positive_rate)
		: _expected_insertions{expected_insertions}
        , _false_positive_rate{false_positive_rate} 
    {
		if (_false_positive_rate <= 0.0 || _false_positive_rate >= 1.0)
            _false_positive_rate = 0.01; // Default to 1% if invalid

	_num_bits = _optimal_num_of_bits(_expected_insertions, _false_positive_rate);
    _num_bits = (_num_bits < BLOOM_FILTER_MIN_BITS) ? BLOOM_FILTER_MIN_BITS : _num_bits;
    _num_bits = (_num_bits > BLOOM_FILTER_MAX_BITS) ? BLOOM_FILTER_MAX_BITS : _num_bits;

	_num_hash_functions = _optimal_num_of_hash_functions(_expected_insertions, _num_bits);
	_num_hash_functions = (_num_hash_functions < BLOOM_FILTER_MIN_HASHES) ? BLOOM_FILTER_MIN_HASHES : _num_hash_functions;
	_num_hash_functions = (_num_hash_functions > BLOOM_FILTER_MAX_HASHES) ? BLOOM_FILTER_MAX_HASHES : _num_hash_functions;

	_bits.resize(_num_bits, false);
	}

    ~bloom_filter()
    {
    }

    inline void clear() { std::fill(_bits.begin(), _bits.end(), false); }
	inline size_t bit_size() const { return _num_bits; }
	inline size_t hash_count() const { return _num_hash_functions; }

	void add(const std::string& value) 
    {
		for (size_t i = 0; i < _num_hash_functions; ++i) 
        {
			size_t hash = _hash_i(value, i);
			_bits[hash % _num_bits] = true;
		}
	}

	bool contains(const std::string& value) const 
    {
        size_t hash;
		for (size_t i = 0; i < _num_hash_functions; ++i) 
        {
			hash = _hash_i(value, i);
			if (!_bits[hash % _num_bits]) 
                return false;
		}
		return true;
	}

	void serialize(std::string& s) const 
    {
		s.reserve(_bits.size());
		for (bool b : _bits) 
            s += b ? '1' : '0';
	}

	void unserialize(const std::string& s) 
    {
		if (s.size() != _bits.size())
            return;

		for (size_t i = 0; i < s.size(); ++i) 
            _bits[i] = (s[i] == '1');
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
		return h1(value) + i * h2(h1(value));
	}

private:
	size_t _expected_insertions;
	double _false_positive_rate;
	size_t _num_bits;
	size_t _num_hash_functions;
	std::deque<bool> _bits;
};

} // namespace libcpp

#endif // BLOOM_FILTER_HPP