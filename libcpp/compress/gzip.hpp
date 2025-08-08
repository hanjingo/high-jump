#ifndef GZIP_HPP
#define GZIP_HPP

#include <zlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <limits>
#include <algorithm>

namespace libcpp
{

static constexpr int GZIP_WINDOW_BITS = 15 + 16; // 15 for window size, 16 for gzip format
static constexpr int GZIP_MAX_SAFE_SZ = std::numeric_limits<size_t>::max() - 1000;

class gzip
{
public:
    enum class err : int
    {
        ok = Z_OK,
        stream_error = Z_STREAM_ERROR,
        data_error = Z_DATA_ERROR,
        mem_error = Z_MEM_ERROR,
        version_error = Z_VERSION_ERROR,
        need_dict = Z_NEED_DICT,
        stream_end = Z_STREAM_END,

        input_invalid,
        buffer_too_small,
        max_output_sz_exceeded,
        read_buffer_error,
        write_buffer_error,
        overflow_error,
        reserve_size_exceeded
    };

    enum class compression_lvl : int
    {
        default_compression   = Z_DEFAULT_COMPRESSION,
        no_compression        = Z_NO_COMPRESSION,
        best_speed            = Z_BEST_SPEED,
        best_compression      = Z_BEST_COMPRESSION
    };

    enum class strategy : int
    {
        default_strategy   = Z_DEFAULT_STRATEGY,
        filtered           = Z_FILTERED,
        huffman_only       = Z_HUFFMAN_ONLY,
        rle                = Z_RLE,
        fixed              = Z_FIXED
    };

    enum class mem_lvl : int
    {
        default_level = 8,
        lvl1          = 1,
        lvl2          = 2,
        lvl3          = 3,
        lvl4          = 4, // Block Size: 1023 Byte
        lvl5          = 5,
        lvl6          = 6,
        lvl7          = 7, 
        lvl8          = 8, // Block Size: 16383 Byte
        lvl9          = 9
    };

    static size_t compress_reserve_sz(const size_t src_sz)
    {
        if (src_sz > GZIP_MAX_SAFE_SZ)
            return GZIP_MAX_SAFE_SZ;
        
        return src_sz + (src_sz / 1000) + 12;
    }

    static size_t decompress_reserve_sz(const size_t src_sz,
                                        const size_t max_output_sz)
    {
        if (max_output_sz > 0)
            return max_output_sz < GZIP_MAX_SAFE_SZ ? max_output_sz : GZIP_MAX_SAFE_SZ;

        if (src_sz == 0)
            return 4096; // 4KB

        size_t estimated;
        if (src_sz <= GZIP_MAX_SAFE_SZ / 4)
            estimated = src_sz * 4;
        else
            estimated = GZIP_MAX_SAFE_SZ;
        
        return estimated < GZIP_MAX_SAFE_SZ ? estimated : GZIP_MAX_SAFE_SZ;
    }

    static err compress(std::vector<unsigned char>& dst,
                        const void* src, 
                        const size_t src_sz, 
                        const compression_lvl compr_lvl = compression_lvl::default_compression,
                        const mem_lvl mem_level = mem_lvl::default_level)
    {
        if (!src || src_sz == 0)
            return err::input_invalid;

        z_stream stream;
        std::memset(&stream, 0, sizeof(stream));
        int ret = deflateInit2(&stream, 
                               static_cast<int>(compr_lvl),
                               Z_DEFLATED,
                               GZIP_WINDOW_BITS,
                               static_cast<int>(mem_level),
                               static_cast<int>(strategy::default_strategy));
        if (ret != Z_OK) 
            return static_cast<err>(ret);

        stream.avail_in = static_cast<uInt>(src_sz);
        stream.next_in = static_cast<Bytef*>(const_cast<void*>(src));
        size_t reserve_size = compress_reserve_sz(src_sz);
        dst.clear();
        dst.reserve(reserve_size);
        if (dst.size() < reserve_size)
            return err::reserve_size_exceeded;

        const size_t chunk_size = 16384; // 16KB chunks
        std::vector<unsigned char> chunk(chunk_size);
        do {
            stream.avail_out = static_cast<uInt>(chunk_size);
            stream.next_out = chunk.data();
            ret = deflate(&stream, Z_FINISH);
            if (ret == Z_STREAM_ERROR) 
            {
                deflateEnd(&stream);
                dst.clear();
                return static_cast<err>(ret);
            }

            size_t have = chunk_size - stream.avail_out;
            dst.insert(dst.end(), chunk.begin(), chunk.begin() + have);
        } while (stream.avail_out == 0);

        deflateEnd(&stream);
        if (ret != Z_STREAM_END) 
        {
            dst.clear();
            return (ret == Z_OK) ? err::buffer_too_small : static_cast<err>(ret);
        }

        return err::ok;
    }

    static err compress(std::ostream& out,
                        std::istream& in,
                        const compression_lvl compr_lvl = compression_lvl::default_compression,
                        const mem_lvl mem_level = mem_lvl::default_level,
                        const size_t max_input_sz = 0)
    {
        if (!in || !out)
            return err::input_invalid;

        in.seekg(0, std::ios::end);
        std::streampos in_size = in.tellg();
        in.seekg(0, std::ios::beg);
        if (in_size < 0 || (max_input_sz > 0 && static_cast<size_t>(in_size) > max_input_sz))
            return err::input_invalid;

        size_t size = static_cast<size_t>(in_size);
        std::vector<char> buffer(size);
        in.read(buffer.data(), size);
        if (in.gcount() != static_cast<std::streamsize>(size))
            return err::read_buffer_error;

        std::vector<unsigned char> compressed;
        auto ret = compress(compressed, buffer.data(), size, compr_lvl, mem_level);
        if (ret != err::ok)
            return ret;

        out.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());
        if (!out)
            return err::write_buffer_error;
        
        return err::ok;
    }

    static err decompress(std::vector<unsigned char>& dst,
                          const void* src, 
                          const size_t src_sz, 
                          const size_t max_output_sz = 0)
    {
        if (!src || src_sz == 0)
            return err::input_invalid;

        z_stream stream;
        std::memset(&stream, 0, sizeof(stream));

        int ret = inflateInit2(&stream, GZIP_WINDOW_BITS);
        if (ret != Z_OK)
            return static_cast<err>(ret);

        stream.avail_in = static_cast<uInt>(src_sz);
        stream.next_in = static_cast<Bytef*>(const_cast<void*>(src));
        size_t reserve_size = decompress_reserve_sz(src_sz, max_output_sz);
        dst.clear();
        dst.reserve(reserve_size);
        const size_t chunk_size = 16384; // 16KB chunks
        std::vector<unsigned char> chunk(chunk_size);
        do {
            stream.avail_out = static_cast<uInt>(chunk_size);
            stream.next_out = chunk.data();
            ret = inflate(&stream, Z_NO_FLUSH);
            
            switch (ret) 
            {
                case Z_NEED_DICT:
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    inflateEnd(&stream);
                    dst.clear();
                    return static_cast<err>(ret);
                default: 
                    break;
            }

            size_t have = chunk_size - stream.avail_out;
            dst.insert(dst.end(), chunk.begin(), chunk.begin() + have);
            if (max_output_sz > 0 && dst.size() > max_output_sz) 
            {
                inflateEnd(&stream);
                dst.clear();
                return err::buffer_too_small;
            }
        } while (stream.avail_out == 0);

        inflateEnd(&stream);
        if (ret != Z_STREAM_END)
            return static_cast<err>(ret);

        return err::ok;
    }

    static err decompress(std::ostream& out,
                          std::istream& in,
                          const size_t max_output_sz = 0)
    {
        if (!in || !out)
            return err::input_invalid;

        in.seekg(0, std::ios::end);
        size_t size = in.tellg();
        in.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);
        in.read(buffer.data(), size);
        if (in.gcount() != static_cast<std::streamsize>(size))
            return err::read_buffer_error;

        std::vector<unsigned char> decompressed(decompress_reserve_sz(size, max_output_sz));
        auto ret = decompress(decompressed, buffer.data(), size, max_output_sz);
        if (ret != err::ok)
            return ret;

        out.write(reinterpret_cast<const char*>(decompressed.data()), decompressed.size());
        return err::ok;
    }

    static unsigned long crc32_checksum(const void* data, const size_t size)
    {
        if (!data || size == 0)
            return 0;

        return crc32(0L, static_cast<const Bytef*>(data), static_cast<uInt>(size));
    }

    static double compression_ratio(const size_t original_sz, const size_t compressed_sz)
    {
        if (original_sz == 0)
            return 0.0;

        return 1.0 - (static_cast<double>(compressed_sz) / static_cast<double>(original_sz));
    }
};

} // namespace libcpp

#endif // GZIP_HPP