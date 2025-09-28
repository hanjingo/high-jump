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
    // Default block size for new blocks
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

    // Append data to the buffer
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

    // Append from another chain_buffer
    void append(const chain_buffer &other)
    {
        for(size_t i = 0; i < other._blocks.size(); ++i)
        {
            const auto &blk = other._blocks[i];
            if(!blk.empty())
                append(blk.data(), blk.size());
        }
    }

    // Read data from the buffer without consuming
    // Returns number of bytes actually read
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

    // Consume (remove) bytes from the buffer
    void consume(size_t len)
    {
        while(len > 0 && !_blocks.empty())
        {
            auto  &blk   = _blocks[_read_block];
            size_t avail = blk.size() - _read_pos;
            if(len < avail)
            {
                _read_pos += len;
                _total_size -= len;
                return;
            } else
            {
                len -= avail;
                _total_size -= avail;
                ++_read_block;
                _read_pos = 0;
            }
        }
        // Remove fully consumed blocks
        if(_read_block > 0)
        {
            _blocks.erase(_blocks.begin(), _blocks.begin() + _read_block);
            _read_block = 0;
        }
    }

    // Get total bytes available to read
    size_t size() const { return _total_size; }

    // Check if buffer is empty
    bool empty() const { return _total_size == 0; }

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
    size_t                             _block_size;
    std::vector<std::vector<uint8_t> > _blocks;
    size_t                             _total_size;

    // Read cursor
    mutable size_t _read_pos;
    mutable size_t _read_block;
};

} // namespace hj

#endif // CHAIN_BUFFER_HPP