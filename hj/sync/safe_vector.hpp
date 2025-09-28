#ifndef SAFE_VECTOR_HPP
#define SAFE_VECTOR_HPP

#include <oneapi/tbb/concurrent_vector.h>

namespace hj
{
// NOTE: This container was implemented by onetbb
template <typename T>
class safe_vector
{
  public:
    using vector_t              = tbb::concurrent_vector<T>;
    using iterator_t            = typename vector_t::iterator;
    using const_iterator_t      = typename vector_t::const_iterator;
    using range_handler_t       = std::function<bool(T &)>;
    using const_range_handler_t = std::function<bool(const T &)>;
    using sort_handler_t        = std::function<bool(const T &, const T &)>;

  public:
    safe_vector()
        : _vector{}
    {
    }

    virtual ~safe_vector() {}

    inline T &operator[](std::size_t index) { return _vector[index]; }

    inline void emplace(T &&value) { _vector.emplace_back(std::move(value)); }

    inline void emplace(const T &value) { _vector.emplace_back(value); }

    inline void unsafe_clear() { _vector.clear(); }

    inline std::size_t size() const { return _vector.size(); }

    inline bool empty() const { return _vector.empty(); }

    inline void swap(safe_vector &other) { _vector.swap(other._vector); }

    inline void range(const range_handler_t &fn)
    {
        for(auto itr = _vector.begin(); itr != _vector.end(); ++itr)
            if(!fn(*itr))
                return;
    }
    inline void range(const const_range_handler_t &fn) const
    {
        for(auto itr = _vector.begin(); itr != _vector.end(); ++itr)
            if(!fn(*itr))
                return;
    }
    inline void sort(const sort_handler_t &fn)
    {
        std::sort(_vector.begin(), _vector.end(), fn);
    }
    inline T &at(std::size_t index) { return _vector.at(index); }
    inline T &front() { return _vector.front(); }
    inline T &back() { return _vector.back(); }

  private:
    vector_t _vector;
};

}

#endif