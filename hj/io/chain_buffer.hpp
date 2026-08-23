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
#ifndef CHAIN_BUFFER_HPP
#define CHAIN_BUFFER_HPP

#include <vector>
#include <memory>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <utility>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#ifndef _IOVEC_DEFINED
#define _IOVEC_DEFINED
struct iovec
{
    void  *iov_base;
    size_t iov_len;
};
#endif
#else
#include <sys/uio.h>
#endif

namespace hj
{

class chain_buffer
{
  private:
    struct buffer_block
    {
        std::unique_ptr<uint8_t[]> data;
        size_t                     capacity;
        size_t                     size;
        size_t                     read_offset;

        explicit buffer_block(size_t cap)
            : data(new uint8_t[cap])
            , capacity(cap)
            , size(0)
            , read_offset(0)
        {
        }

        buffer_block(buffer_block &&) noexcept            = default;
        buffer_block &operator=(buffer_block &&) noexcept = default;

        buffer_block(const buffer_block &)            = delete;
        buffer_block &operator=(const buffer_block &) = delete;

        const uint8_t *readable_data() const
        {
            return data.get() + read_offset;
        }
        uint8_t *readable_data() { return data.get() + read_offset; }
        size_t   readable_size() const { return size - read_offset; }
    };

  public:
    static constexpr size_t DEFAULT_BLOCK_SIZE = 4096;

    explicit chain_buffer(size_t block_size = DEFAULT_BLOCK_SIZE)
        : _block_size(block_size)
        , _head_index(0)
        , _total_size(0)
    {
        if(_block_size == 0)
        {
            throw std::invalid_argument(
                "chain_buffer: block_size must be greater than zero");
        }
        _blocks.emplace_back(_block_size);
    }

    chain_buffer(chain_buffer &&other) noexcept
        : _block_size(other._block_size)
        , _head_index(other._head_index)
        , _blocks(std::move(other._blocks))
        , _total_size(other._total_size)
    {
        other.release();
    }

    chain_buffer &operator=(chain_buffer &&other) noexcept
    {
        if(this != &other)
        {
            _block_size = other._block_size;
            _head_index = other._head_index;
            _blocks     = std::move(other._blocks);
            _total_size = other._total_size;
            other.release();
        }
        return *this;
    }

    chain_buffer(const chain_buffer &)            = delete;
    chain_buffer &operator=(const chain_buffer &) = delete;

    ~chain_buffer() = default;

    size_t size() const { return _total_size; }
    size_t block_size() const { return _block_size; }
    bool   empty() const { return _total_size == 0; }
    size_t block_count() const noexcept { return _blocks.size(); }
    size_t readable_size() const noexcept { return _total_size; }

    std::vector<struct iovec> iovecs() const
    {
        std::vector<struct iovec> vec;
        if(_total_size == 0)
            return vec;

        for(size_t i = _head_index; i < _blocks.size(); ++i)
        {
            const auto &blk = _blocks[i];
            if(blk.readable_size() > 0)
            {
                struct iovec iov;
#if defined(_WIN32)
                iov.iov_len  = static_cast<ULONG>(blk.readable_size());
                iov.iov_base = reinterpret_cast<char *>(
                    const_cast<uint8_t *>(blk.readable_data()));
#else
                iov.iov_base = const_cast<void *>(
                    static_cast<const void *>(blk.readable_data()));
                iov.iov_len = blk.readable_size();
#endif
                vec.push_back(iov);
            }
        }
        return vec;
    }

    void append(const void *data, size_t len)
    {
        if(len == 0)
            return;

        if(data == nullptr)
        {
            throw std::invalid_argument("chain_buffer::append: data pointer "
                                        "cannot be nullptr when len > 0");
        }

        if(len > SIZE_MAX - _total_size)
        {
            throw std::overflow_error(
                "chain_buffer::append: total size overflow");
        }

        const uint8_t *p = static_cast<const uint8_t *>(data);
        while(len > 0)
        {
            if(_blocks.empty() || _blocks.back().size == _block_size)
            {
                if(_head_index > 0 && _head_index >= _blocks.size())
                {
                    _blocks.clear();
                    _head_index = 0;
                }

                _blocks.emplace_back(_block_size);
            }

            auto  &back_blk = _blocks.back();
            size_t space    = back_blk.capacity - back_blk.size;
            size_t to_copy  = (std::min) (space, len);

            std::memcpy(back_blk.data.get() + back_blk.size, p, to_copy);
            back_blk.size += to_copy;

            p += to_copy;
            len -= to_copy;
            _total_size += to_copy;
        }
    }

    void append(chain_buffer &other)
    {
        if(this == &other)
        {
            throw std::invalid_argument(
                "chain_buffer::append: cannot append buffer to itself");
        }

        if(other.empty())
            return;

        other.for_each_segment([this](const uint8_t *data, size_t size) {
            this->append(data, size);
        });

        other.clear();
    }

    void append(chain_buffer &&other)
    {
        if(this == &other)
        {
            throw std::invalid_argument("chain_buffer::append: cannot append "
                                        "buffer to itself (rvalue)");
        }

        if(other.empty())
            return;

        if(empty())
        {
            _blocks     = std::move(other._blocks);
            _head_index = other._head_index;
            _total_size = other._total_size;
            other.release();
            return;
        }

        other.for_each_segment([this](const uint8_t *data, size_t size) {
            this->append(data, size);
        });

        other.clear();
    }

    template <typename Callback>
    void for_each_segment(Callback &&cb) const
    {
        if(_total_size == 0)
            return;

        for(size_t i = _head_index; i < _blocks.size(); ++i)
        {
            const auto &blk = _blocks[i];
            if(blk.readable_size() > 0)
            {
                std::forward<Callback>(cb)(blk.readable_data(),
                                           blk.readable_size());
            }
        }
    }

    size_t peek(void *out, size_t len) const
    {
        size_t   copied = 0;
        uint8_t *p      = static_cast<uint8_t *>(out);

        for_each_segment([&](const uint8_t *data, size_t size) {
            if(len == 0)
                return;
            size_t to_copy = (std::min) (size, len);
            std::memcpy(p + copied, data, to_copy);
            copied += to_copy;
            len -= to_copy;
        });

        return copied;
    }

    size_t read(void *out, size_t len) const { return peek(out, len); }

    size_t consume(size_t len)
    {
        size_t consumed     = 0;
        len                 = (len < _total_size) ? len : _total_size;
        size_t original_len = len;

        while(len > 0 && _head_index < _blocks.size())
        {
            auto  &blk   = _blocks[_head_index];
            size_t avail = blk.readable_size();
            if(len < avail)
            {
                blk.read_offset += len;
                consumed += len;
                len = 0;
            } else
            {
                consumed += avail;
                len -= avail;
                _head_index++;
            }
        }

        _total_size -= consumed;
        if(_total_size == 0)
            clear();

        return original_len;
    }

    void clear()
    {
        for(auto &blk : _blocks)
        {
            blk.size        = 0;
            blk.read_offset = 0;
        }
        _head_index = 0;
        if(_blocks.empty())
            _blocks.emplace_back(_block_size);

        _total_size = 0;
    }

    void release()
    {
        _blocks.clear();
        _head_index = 0;
        _blocks.emplace_back(_block_size);
        _total_size = 0;
    }

  private:
    size_t                    _block_size;
    size_t                    _head_index;
    std::vector<buffer_block> _blocks;
    size_t                    _total_size;
};

} // namespace hj

#endif // CHAIN_BUFFER_HPP