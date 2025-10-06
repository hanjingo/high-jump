/*
 *  This file is part of hj.
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
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <algorithm>

// A simple chain buffer: supports efficient append, read, and consume operations.
// Internally, it manages a chain of memory blocks.
namespace hj
{

class chain_buffer
{
  public:
    static constexpr size_t DEFAULT_BLOCK_SIZE = 4096;

    chain_buffer(size_t block_size = DEFAULT_BLOCK_SIZE)
        : _block_size(block_size)
        , _total_size(0)
        , _read_pos(0)
        , _read_block(0)
    {
        _blocks.emplace_back();
        _blocks.back().reserve(_block_size);
    }

    inline size_t size() const { return _total_size; }
    inline size_t block_size() const { return _block_size; }
    inline bool   empty() const { return _total_size == 0; }

    void append(const void *data, size_t len)
    {
        const uint8_t *p = static_cast<const uint8_t *>(data);
        while(len > 0)
        {
            if(_blocks.empty() || _blocks.back().size() == _block_size)
            {
                _blocks.emplace_back();
                _blocks.back().reserve(_block_size);
            }

            size_t space   = _block_size - _blocks.back().size();
            size_t to_copy = std::min(space, len);
            _blocks.back().insert(_blocks.back().end(), p, p + to_copy);
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
           && _blocks[0].empty())
        {
            _blocks = std::move(other._blocks);
        } else
        {
            for(auto &blk : other._blocks)
                if(!blk.empty())
                    append(blk.data(), blk.size());
        }

        _total_size += other._total_size;
        other.clear();
    }

    size_t read(void *out, size_t len) const
    {
        size_t   copied  = 0;
        size_t   blk_idx = _read_block;
        size_t   pos     = _read_pos;
        uint8_t *p       = static_cast<uint8_t *>(out);

        while(len > 0 && blk_idx < _blocks.size())
        {
            const auto &blk = _blocks[blk_idx];
            if(pos >= blk.size())
            {
                ++blk_idx;
                pos = 0;
                continue;
            }

            size_t avail   = blk.size() - pos;
            size_t to_copy = std::min(avail, len);
            std::memcpy(p + copied, blk.data() + pos, to_copy);
            copied += to_copy;
            len -= to_copy;
            pos += to_copy;
        }
        return copied;
    }

    // Consume up to len bytes from the buffer. If len > size(), only available bytes are consumed.
    //   Returns the actual number of bytes consumed. (Compatible with boost::asio/beast semantics)
    size_t consume(size_t len)
    {
        size_t consumed = 0;
        len             = (len < _total_size) ? len : _total_size;
        while(len > 0 && !_blocks.empty())
        {
            auto  &blk   = _blocks[_read_block];
            size_t avail = blk.size() - _read_pos;
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
        _blocks.emplace_back();
        _blocks.back().reserve(_block_size);
        _total_size = 0;
        _read_pos   = 0;
        _read_block = 0;
    }

  private:
    chain_buffer(const chain_buffer &)            = delete;
    chain_buffer &operator=(const chain_buffer &) = delete;
    chain_buffer(chain_buffer &&)                 = default;
    chain_buffer &operator=(chain_buffer &&)      = default;

  private:
    size_t                            _block_size;
    std::vector<std::vector<uint8_t>> _blocks;
    size_t                            _total_size;

    // Read cursor
    mutable size_t _read_pos;
    mutable size_t _read_block;
};

} // namespace hj

#endif // CHAIN_BUFFER_HPP