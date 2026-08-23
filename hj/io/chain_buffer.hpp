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

        explicit buffer_block(size_t cap)
            : data(new uint8_t[cap])
            , capacity(cap)
            , size(0)
        {
        }

        buffer_block(buffer_block &&) noexcept            = default;
        buffer_block &operator=(buffer_block &&) noexcept = default;

        buffer_block(const buffer_block &)            = delete;
        buffer_block &operator=(const buffer_block &) = delete;
    };

  public:
    static constexpr size_t DEFAULT_BLOCK_SIZE = 4096;

    explicit chain_buffer(size_t block_size = DEFAULT_BLOCK_SIZE)
        : _block_size(block_size)
        , _total_size(0)
        , _read_pos(0)
        , _read_block(0)
    {
        _blocks.emplace_back(_block_size);
    }

    chain_buffer(chain_buffer &&) noexcept            = default;
    chain_buffer &operator=(chain_buffer &&) noexcept = default;

    chain_buffer(const chain_buffer &)            = delete;
    chain_buffer &operator=(const chain_buffer &) = delete;

    ~chain_buffer() = default;

    size_t size() const { return _total_size; }
    size_t block_size() const { return _block_size; }
    bool   empty() const { return _total_size == 0; }

    void append(const void *data, size_t len)
    {
        if(len == 0)
            return;

        const uint8_t *p = static_cast<const uint8_t *>(data);
        while(len > 0)
        {
            if(_blocks.empty() || _blocks.back().size == _block_size)
                _blocks.emplace_back(_block_size);

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
        if(other.empty())
            return;

        if(_read_block == 0 && _read_pos == 0 && _blocks.size() == 1
           && _blocks[0].size == 0)
        {
            _blocks     = std::move(other._blocks);
            _total_size = other._total_size;
            other.clear();
        } else
        {
            for(auto &blk : other._blocks)
            {
                if(blk.size > 0)
                {
                    append(blk.data.get(), blk.size);
                }
            }
            other.clear();
        }
    }

    void   append(chain_buffer &&other) { append(other); }
    size_t read(void *out, size_t len)
    {
        size_t   copied  = 0;
        size_t   blk_idx = _read_block;
        size_t   pos     = _read_pos;
        uint8_t *p       = static_cast<uint8_t *>(out);

        while(len > 0 && blk_idx < _blocks.size())
        {
            const auto &blk = _blocks[blk_idx];
            if(pos >= blk.size)
            {
                ++blk_idx;
                pos = 0;
                continue;
            }

            size_t avail   = blk.size - pos;
            size_t to_copy = (std::min) (avail, len);
            std::memcpy(p + copied, blk.data.get() + pos, to_copy);
            copied += to_copy;
            len -= to_copy;
            pos += to_copy;
        }
        return copied;
    }

    // Consume up to len bytes from the buffer. If len > size(), only available bytes are consumed.
    // Returns the actual number of bytes consumed. (Compatible with boost::asio/beast semantics)
    size_t consume(size_t len)
    {
        size_t consumed = 0;
        len             = (len < _total_size) ? len : _total_size;
        while(len > 0 && !_blocks.empty())
        {
            auto  &blk   = _blocks[_read_block];
            size_t avail = blk.size - _read_pos;
            if(len < avail)
            {
                consumed += len;
                _read_pos += len;
                _total_size -= len;
                return consumed;
            } else
            {
                consumed += avail;
                len -= avail;
                _total_size -= avail;
                ++_read_block;
                _read_pos = 0;
            }
        }

        if(_read_block > 0)
        {
            _blocks.erase(_blocks.begin(), _blocks.begin() + _read_block);
            _read_block = 0;
        }

        return consumed;
    }

    // Clear the buffer
    void clear()
    {
        _blocks.clear();
        _blocks.emplace_back(_block_size);
        _total_size = 0;
        _read_pos   = 0;
        _read_block = 0;
    }

  private:
    size_t                    _block_size;
    std::vector<buffer_block> _blocks;
    size_t                    _total_size;

    size_t _read_pos;
    size_t _read_block;
};

} // namespace hj

#endif // CHAIN_BUFFER_HPP