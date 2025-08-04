#ifndef SAFE_VECTOR_HPP
#define SAFE_VECTOR_HPP

#include <oneapi/tbb/concurrent_vector.h>

namespace libcpp {
// NOTE: This container was implemented by onetbb
template <typename T>
class safe_vector
{
  public:
    using vector_t = tbb::concurrent_vector<T>;
    using iterator_t = typename vector_t::iterator;
    using const_iterator_t = typename vector_t::const_iterator;
    using range_handler_t = std::function<bool(T&)>;
    using const_range_handler_t = std::function<bool(const T&)>;
    using sort_handler_t = std::function<bool(const T&, const T&)>;

  public:
    safe_vector() : vector_{} {}

    virtual ~safe_vector() {}

    inline T& operator[](std::size_t index) { return vector_[index]; }

    inline void emplace(T&& value) { vector_.emplace_back(std::move(value)); }

    inline void emplace(const T& value) { vector_.emplace_back(value); }

    inline void unsafe_clear() { vector_.clear(); }

    inline std::size_t size() const { return vector_.size(); }

    inline bool empty() const { return vector_.empty(); }

    inline void swap(safe_vector& other) { vector_.swap(other.vector_); }

    inline void range(const range_handler_t& fn)
    {
        for (auto itr = vector_.begin(); itr != vector_.end(); ++itr)
            if (!fn(*itr))
                return;
    }
    inline void range(const const_range_handler_t& fn) const
    {
        for (auto itr = vector_.begin(); itr != vector_.end(); ++itr)
            if (!fn(*itr))
                return;
    }
    inline void sort(const sort_handler_t& fn)
    {
        std::sort(vector_.begin(), vector_.end(), fn);
    }
    inline T& at(std::size_t index) { return vector_.at(index); }
    inline T& front() { return vector_.front(); }
    inline T& back() { return vector_.back(); }

  private:
    vector_t vector_;
};

}  // namespace libcpp

#endif