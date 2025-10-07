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

namespace hj
{

static constexpr int GZIP_WINDOW_BITS =
    15 + 16; // 15 for window size, 16 for gzip format
static constexpr size_t GZIP_MAX_SAFE_SZ =
    std::numeric_limits<size_t>::max() - 1000;

class gzip
{
  public:
    enum class err : int
    {
        ok            = Z_OK,
        stream_error  = Z_STREAM_ERROR,
        data_error    = Z_DATA_ERROR,
        mem_error     = Z_MEM_ERROR,
        version_error = Z_VERSION_ERROR,
        need_dict     = Z_NEED_DICT,
        stream_end    = Z_STREAM_END,

        input_invalid,
        buffer_too_small,
        max_output_sz_exceeded,
        read_buffer_error,
        write_buffer_error,
        overflow_error,
        zlib_version_mismatch
    };

    enum class compression_lvl : int
    {
        default_compression = Z_DEFAULT_COMPRESSION,
        no_compression      = Z_NO_COMPRESSION,
        best_speed          = Z_BEST_SPEED,
        best_compression    = Z_BEST_COMPRESSION
    };

    enum class strategy : int
    {
        default_strategy = Z_DEFAULT_STRATEGY,
        filtered         = Z_FILTERED,
        huffman_only     = Z_HUFFMAN_ONLY,
        rle              = Z_RLE,
        fixed            = Z_FIXED
    };

    enum class mem_lvl : int
    {
        default_level = 8,
        lvl1          = 1,
        lvl2          = 2,
        lvl3          = 3,
        lvl4          = 4,
        lvl5          = 5,
        lvl6          = 6,
        lvl7          = 7,
        lvl8          = 8,
        lvl9          = 9
    };

    static_assert(ZLIB_VERNUM >= 0x1230,
                  "zlib version too old, require >= 1.2.3");

    static size_t compress_reserve_sz(const size_t src_sz)
    {
        return src_sz + (src_sz / 1000) + 12 + 18;
    }

    static size_t decompress_reserve_sz(const size_t src_sz,
                                        const size_t max_output_sz)
    {
        const size_t MIN_ALLOC      = 4096;              // 4KB
        const size_t MAX_SAFE_ALLOC = 1024 * 1024 * 256; // 256MB
        if(max_output_sz > 0)
            return std::min(max_output_sz, MAX_SAFE_ALLOC);

        if(src_sz == 0)
            return MIN_ALLOC;

        size_t estimated =
            (src_sz <= MAX_SAFE_ALLOC / 4) ? src_sz * 4 : MAX_SAFE_ALLOC;

        return std::max(MIN_ALLOC, std::min(estimated, MAX_SAFE_ALLOC));
    }

    static err compress(
        std::vector<unsigned char> &dst,
        const void                 *src,
        const size_t                src_sz,
        const compression_lvl compr_lvl = compression_lvl::default_compression,
        const mem_lvl         mem_level = mem_lvl::default_level)
    {
        if(!src || src_sz == 0)
            return err::input_invalid;

        z_stream stream;
        std::memset(&stream, 0, sizeof(stream));
        int ret = deflateInit2(
            &stream,
            static_cast<int>(compr_lvl), // level
            Z_DEFLATED,                  // method
            GZIP_WINDOW_BITS,            // windowBits (15+16 for gzip)
            static_cast<int>(mem_level), // memLevel
            static_cast<int>(strategy::default_strategy)); // strategy

        if(ret != Z_OK)
            return static_cast<err>(ret);

        stream.avail_in = static_cast<uInt>(src_sz);
        stream.next_in  = static_cast<Bytef *>(const_cast<void *>(src));
        dst.clear();
        dst.reserve(compress_reserve_sz(src_sz));
        const size_t               chunk_size = 16384; // 16KB chunks
        std::vector<unsigned char> chunk(chunk_size);
        do
        {
            stream.avail_out = static_cast<uInt>(chunk_size);
            stream.next_out  = chunk.data();
            ret              = deflate(&stream, Z_FINISH);
            if(ret == Z_STREAM_ERROR)
            {
                deflateEnd(&stream);
                dst.clear();
                return static_cast<err>(ret);
            }

            size_t have = chunk_size - stream.avail_out;
            dst.insert(dst.end(), chunk.begin(), chunk.begin() + have);

        } while(stream.avail_out == 0);

        deflateEnd(&stream);

        if(ret != Z_STREAM_END)
        {
            dst.clear();
            return (ret == Z_OK) ? err::buffer_too_small
                                 : static_cast<err>(ret);
        }

        return err::ok;
    }

    static err compress(
        std::ostream         &out,
        std::istream         &in,
        const compression_lvl compr_lvl  = compression_lvl::default_compression,
        const mem_lvl         mem_level  = mem_lvl::default_level,
        const size_t          chunk_size = 16384)
    {
        if(!in || !out)
            return err::input_invalid;

        z_stream stream;
        std::memset(&stream, 0, sizeof(stream));
        int ret = deflateInit2(&stream,
                               static_cast<int>(compr_lvl),
                               Z_DEFLATED,
                               GZIP_WINDOW_BITS,
                               static_cast<int>(mem_level),
                               static_cast<int>(strategy::default_strategy));
        if(ret != Z_OK)
            return static_cast<err>(ret);

        std::vector<unsigned char> inbuf(chunk_size), outbuf(chunk_size);
        err                        result = err::ok;
        while(in)
        {
            in.read(reinterpret_cast<char *>(inbuf.data()), chunk_size);
            std::streamsize read_sz = in.gcount();
            if(read_sz <= 0)
                break;

            stream.avail_in = static_cast<uInt>(read_sz);
            stream.next_in  = inbuf.data();
            do
            {
                stream.avail_out = static_cast<uInt>(chunk_size);
                stream.next_out  = outbuf.data();
                ret = deflate(&stream, in.eof() ? Z_FINISH : Z_NO_FLUSH);
                if(ret == Z_STREAM_ERROR)
                {
                    result = err::stream_error;
                    deflateEnd(&stream);
                    return result;
                }
                size_t have = chunk_size - stream.avail_out;
                out.write(reinterpret_cast<const char *>(outbuf.data()), have);
                if(!out)
                {
                    result = err::write_buffer_error;
                    deflateEnd(&stream);
                    return result;
                }
            } while(stream.avail_out == 0);
        }

        // end up compression
        do
        {
            stream.avail_out = static_cast<uInt>(chunk_size);
            stream.next_out  = outbuf.data();
            ret              = deflate(&stream, Z_FINISH);
            size_t have      = chunk_size - stream.avail_out;
            out.write(reinterpret_cast<const char *>(outbuf.data()), have);
        } while(ret != Z_STREAM_END);

        deflateEnd(&stream);
        return result;
    }

    static err decompress(std::vector<unsigned char> &dst,
                          const void                 *src,
                          const size_t                src_sz,
                          const size_t                max_output_sz = 0)
    {
        if(!src || src_sz == 0)
            return err::input_invalid;

        z_stream stream;
        std::memset(&stream, 0, sizeof(stream));
        int ret = inflateInit2(&stream, GZIP_WINDOW_BITS);
        if(ret != Z_OK)
            return static_cast<err>(ret);

        stream.avail_in = static_cast<uInt>(src_sz);
        stream.next_in  = static_cast<Bytef *>(const_cast<void *>(src));
        dst.clear();
        const size_t               chunk_size = 16384; // 16KB chunks
        std::vector<unsigned char> chunk(chunk_size);
        do
        {
            stream.avail_out = static_cast<uInt>(chunk_size);
            stream.next_out  = chunk.data();
            ret              = inflate(&stream, Z_NO_FLUSH);
            switch(ret)
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
            if(max_output_sz > 0 && dst.size() + have > max_output_sz)
            {
                inflateEnd(&stream);
                dst.clear();
                return err::max_output_sz_exceeded;
            }

            dst.insert(dst.end(), chunk.begin(), chunk.begin() + have);
        } while(stream.avail_out == 0);

        inflateEnd(&stream);
        if(ret != Z_STREAM_END)
        {
            dst.clear();
            return static_cast<err>(ret);
        }

        return err::ok;
    }

    static err decompress(std::ostream &out,
                          std::istream &in,
                          const size_t  max_output_sz = 0,
                          const size_t  chunk_size    = 16384)
    {
        if(!in || !out)
            return err::input_invalid;

        z_stream stream;
        std::memset(&stream, 0, sizeof(stream));
        int ret = inflateInit2(&stream, GZIP_WINDOW_BITS);
        if(ret != Z_OK)
            return static_cast<err>(ret);

        std::vector<unsigned char> inbuf(chunk_size), outbuf(chunk_size);
        err                        result    = err::ok;
        size_t                     total_out = 0;
        while(in)
        {
            in.read(reinterpret_cast<char *>(inbuf.data()), chunk_size);
            std::streamsize read_sz = in.gcount();
            if(read_sz <= 0)
                break;

            stream.avail_in = static_cast<uInt>(read_sz);
            stream.next_in  = inbuf.data();
            do
            {
                stream.avail_out = static_cast<uInt>(chunk_size);
                stream.next_out  = outbuf.data();
                ret              = inflate(&stream, Z_NO_FLUSH);
                switch(ret)
                {
                    case Z_NEED_DICT:
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR: {
                        result = static_cast<err>(ret);
                        inflateEnd(&stream);
                        return result;
                    }
                    default:
                        break;
                }
                size_t have = chunk_size - stream.avail_out;
                total_out += have;
                if(max_output_sz > 0 && total_out > max_output_sz)
                {
                    result = err::max_output_sz_exceeded;
                    inflateEnd(&stream);
                    return result;
                }

                out.write(reinterpret_cast<const char *>(outbuf.data()), have);
                if(!out)
                {
                    result = err::write_buffer_error;
                    inflateEnd(&stream);
                    return result;
                }

            } while(stream.avail_out == 0);
        }
        if(ret != Z_STREAM_END)
            result = static_cast<err>(ret);

        inflateEnd(&stream);
        return result;
    }

    static unsigned long crc32_checksum(const void *data, const size_t size)
    {
        if(!data || size == 0)
            return 0;

        return crc32(0L,
                     static_cast<const Bytef *>(data),
                     static_cast<uInt>(size));
    }

    static double compression_ratio(const size_t original_sz,
                                    const size_t compressed_sz)
    {
        if(original_sz == 0)
            return 0.0;

        return 1.0
               - (static_cast<double>(compressed_sz)
                  / static_cast<double>(original_sz));
    }

    static bool is_gzip_format(const void *data, size_t size)
    {
        if(!data || size < 10)
            return false;

        const unsigned char *bytes = static_cast<const unsigned char *>(data);
        return (bytes[0] == 0x1f && bytes[1] == 0x8b);
    }
};

} // namespace hj

#endif // GZIP_HPP