/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 */

#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

#include <stdexcept>
#include <boost/circular_buffer.hpp>

namespace hj
{

enum class ring_buffer_policy
{
    overwrite,
    reject
};

template <typename T, ring_buffer_policy Policy = ring_buffer_policy::overwrite>
class ring_buffer
{
  private:
    boost::circular_buffer<T> buffer_;

  public:
    explicit ring_buffer(size_t capacity)
        : buffer_(capacity)
    {
    }

    void push_back(const T &item)
    {
        _check_full();
        buffer_.push_back(item);
    }
    void push_back(T &&item)
    {
        _check_full();
        buffer_.push_back(std::move(item));
    }

    bool try_push_back(const T &item)
    {
        if constexpr(Policy == ring_buffer_policy::reject)
        {
            if(buffer_.full())
                return false;
        }
        buffer_.push_back(item);
        return true;
    }

    bool try_push_back(T &&item)
    {
        if constexpr(Policy == ring_buffer_policy::reject)
        {
            if(buffer_.full())
                return false;
        }
        buffer_.push_back(std::move(item));
        return true;
    }

    template <typename... Args>
    void emplace_back(Args &&...args)
    {
        _check_full();
        buffer_.push_back(T(std::forward<Args>(args)...));
    }

    template <typename... Args>
    bool try_emplace_back(Args &&...args)
    {
        if constexpr(Policy == ring_buffer_policy::reject)
        {
            if(buffer_.full())
                return false;
        }
        buffer_.push_back(T(std::forward<Args>(args)...));
        return true;
    }

    void pop_front() { buffer_.pop_front(); }
    void pop_back() { buffer_.pop_back(); }
    void clear() noexcept { buffer_.clear(); }

    T       &front() { return buffer_.front(); }
    const T &front() const { return buffer_.front(); }

    T       &back() { return buffer_.back(); }
    const T &back() const { return buffer_.back(); }

    T       &operator[](size_t n) { return buffer_[n]; }
    const T &operator[](size_t n) const { return buffer_[n]; }

    T       &at(size_t n) { return buffer_.at(n); }
    const T &at(size_t n) const { return buffer_.at(n); }

    [[nodiscard]] std::size_t size() const noexcept { return buffer_.size(); }
    [[nodiscard]] std::size_t capacity() const noexcept
    {
        return buffer_.capacity();
    }
    [[nodiscard]] bool empty() const noexcept { return buffer_.empty(); }
    [[nodiscard]] bool full() const noexcept { return buffer_.full(); }

    auto begin() { return buffer_.begin(); }
    auto begin() const { return buffer_.begin(); }
    auto end() { return buffer_.end(); }
    auto end() const { return buffer_.end(); }

  private:
    inline void _check_full() const
    {
        if constexpr(Policy == ring_buffer_policy::reject)
        {
            if(buffer_.full())
                throw std::overflow_error(
                    "ring_buffer is full: cannot push_back");
        }
    }
};

} // namespace hj

#endif