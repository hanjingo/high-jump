#ifndef CHAIN_BUFFER_HPP
#define CHAIN_BUFFER_HPP

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <vector>

// A simple chain buffer: supports efficient append, read, and consume
// operations. Internally, it manages a chain of memory blocks.
namespace libcpp
{

class chain_buffer
{
  public:
    // Default block size for new blocks
    static constexpr size_t DEFAULT_BLOCK_SIZE = 4096;

    chain_buffer (size_t block_size = DEFAULT_BLOCK_SIZE) :
        block_size_ (block_size),
        total_size_ (0),
        read_pos_ (0),
        read_block_ (0)
    {
        blocks_.emplace_back ();
        blocks_.back ().reserve (block_size_);
    }

    // Append data to the buffer
    void append (const void *data, size_t len)
    {
        const uint8_t *p = static_cast<const uint8_t *> (data);
        while (len > 0) {
            if (blocks_.empty () || blocks_.back ().size () == block_size_) {
                blocks_.emplace_back ();
                blocks_.back ().reserve (block_size_);
            }
            size_t space = block_size_ - blocks_.back ().size ();
            size_t to_copy = std::min (space, len);
            blocks_.back ().insert (blocks_.back ().end (), p, p + to_copy);
            p += to_copy;
            len -= to_copy;
            total_size_ += to_copy;
        }
    }

    // Append from another chain_buffer
    void append (const chain_buffer &other)
    {
        for (size_t i = 0; i < other.blocks_.size (); ++i) {
            const auto &blk = other.blocks_[i];
            if (!blk.empty ())
                append (blk.data (), blk.size ());
        }
    }

    // Read data from the buffer without consuming
    // Returns number of bytes actually read
    size_t read (void *out, size_t len) const
    {
        size_t copied = 0;
        size_t blk_idx = read_block_;
        size_t pos = read_pos_;
        uint8_t *p = static_cast<uint8_t *> (out);

        while (len > 0 && blk_idx < blocks_.size ()) {
            const auto &blk = blocks_[blk_idx];
            if (pos >= blk.size ()) {
                ++blk_idx;
                pos = 0;
                continue;
            }
            size_t avail = blk.size () - pos;
            size_t to_copy = std::min (avail, len);
            std::memcpy (p + copied, blk.data () + pos, to_copy);
            copied += to_copy;
            len -= to_copy;
            pos += to_copy;
        }
        return copied;
    }

    // Consume (remove) bytes from the buffer
    void consume (size_t len)
    {
        while (len > 0 && !blocks_.empty ()) {
            auto &blk = blocks_[read_block_];
            size_t avail = blk.size () - read_pos_;
            if (len < avail) {
                read_pos_ += len;
                total_size_ -= len;
                return;
            } else {
                len -= avail;
                total_size_ -= avail;
                ++read_block_;
                read_pos_ = 0;
            }
        }
        // Remove fully consumed blocks
        if (read_block_ > 0) {
            blocks_.erase (blocks_.begin (), blocks_.begin () + read_block_);
            read_block_ = 0;
        }
    }

    // Get total bytes available to read
    size_t size () const { return total_size_; }

    // Check if buffer is empty
    bool empty () const { return total_size_ == 0; }

    // Clear the buffer
    void clear ()
    {
        blocks_.clear ();
        blocks_.emplace_back ();
        blocks_.back ().reserve (block_size_);
        total_size_ = 0;
        read_pos_ = 0;
        read_block_ = 0;
    }

  private:
    size_t block_size_;
    std::vector<std::vector<uint8_t> > blocks_;
    size_t total_size_;
    // Read cursor
    mutable size_t read_pos_;
    mutable size_t read_block_;
};

} // namespace libcpp

#endif // CHAIN_BUFFER_HPP