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

#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

#include <cstddef>
#include <utility>
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
    using container_type   = boost::circular_buffer<T>;
    using value_type       = typename container_type::value_type;
    using size_type        = std::size_t;
    using difference_type  = typename container_type::difference_type;
    using reference        = typename container_type::reference;
    using const_reference  = typename container_type::const_reference;
    using pointer          = typename container_type::pointer;
    using const_pointer    = typename container_type::const_pointer;
    using iterator         = typename container_type::iterator;
    using const_iterator   = typename container_type::const_iterator;
    using reverse_iterator = typename container_type::reverse_iterator;
    using const_reverse_iterator =
        typename container_type::const_reverse_iterator;

    explicit ring_buffer(std::size_t capacity)
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

    reference       front() { return buffer_.front(); }
    const_reference front() const { return buffer_.front(); }

    reference       back() { return buffer_.back(); }
    const_reference back() const { return buffer_.back(); }

    reference       operator[](std::size_t n) { return buffer_[n]; }
    const_reference operator[](std::size_t n) const { return buffer_[n]; }

    reference       at(std::size_t n) { return buffer_.at(n); }
    const_reference at(std::size_t n) const { return buffer_.at(n); }

    [[nodiscard]] std::size_t size() const noexcept { return buffer_.size(); }
    [[nodiscard]] std::size_t capacity() const noexcept
    {
        return buffer_.capacity();
    }
    [[nodiscard]] bool empty() const noexcept { return buffer_.empty(); }
    [[nodiscard]] bool full() const noexcept { return buffer_.full(); }

    iterator       begin() noexcept { return buffer_.begin(); }
    const_iterator begin() const noexcept { return buffer_.begin(); }
    const_iterator cbegin() const noexcept { return buffer_.cbegin(); }

    iterator       end() noexcept { return buffer_.end(); }
    const_iterator end() const noexcept { return buffer_.end(); }
    const_iterator cend() const noexcept { return buffer_.cend(); }

    reverse_iterator       rbegin() noexcept { return buffer_.rbegin(); }
    const_reverse_iterator rbegin() const noexcept { return buffer_.rbegin(); }
    const_reverse_iterator crbegin() const noexcept
    {
        return buffer_.crbegin();
    }

    reverse_iterator       rend() noexcept { return buffer_.rend(); }
    const_reverse_iterator rend() const noexcept { return buffer_.rend(); }
    const_reverse_iterator crend() const noexcept { return buffer_.crend(); }

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