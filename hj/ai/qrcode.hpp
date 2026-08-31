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

#ifndef QRCODE_HPP
#define QRCODE_HPP

#include <cctype>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#ifndef QUIRC_MAX_REGIONS
#define QUIRC_MAX_REGIONS 4096
#endif
#include <qrencode.h>
#include <quirc.h>

namespace hj::qrcode
{

constexpr size_t max_bitmap_pixels = 64 * 1024 * 1024;

enum class error_code
{
    success = 0,

    invalid_text = 1,
    invalid_scale,
    invalid_margin,
    invalid_bitmap,
    encode_failed,
    pgm_write_failed,
    allocation_failed,
    resize_failed,
    begin_parse_failed,
    bm_data_insufficient,
    pgm_open_failed,
    pgm_read_failed,
    magic_parse_failed,
    token_value_convert_failed,
    token_value_out_of_range,
    token_empty,
    no_qrcode_found,
    decode_failed,
    bitmap_too_large,
};

class bitmap
{
  public:
    bitmap() = default;

    bitmap(int w, int h, uint8_t fill_val = 255)
    {
        if(!resize(w, h, fill_val))
        {
            width_  = 0;
            height_ = 0;
            data_.clear();
        }
    }

    [[nodiscard]] int width() const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }
    [[nodiscard]] const std::vector<uint8_t> &data() const noexcept
    {
        return data_;
    }
    [[nodiscard]] std::vector<uint8_t> &data() noexcept { return data_; }

    [[nodiscard]] bool empty() const noexcept { return data_.empty(); }

    bool resize(int w, int h, uint8_t fill_val = 255)
    {
        if(w <= 0 || h <= 0)
            return false;

        const auto w64 = static_cast<uint64_t>(w);
        const auto h64 = static_cast<uint64_t>(h);
        const auto max_size =
            static_cast<uint64_t>((std::numeric_limits<size_t>::max)());

        if(w64 > max_size / h64)
            return false;

        const size_t total_pixels = static_cast<size_t>(w64 * h64);

        if(total_pixels > max_bitmap_pixels)
            return false;

        try
        {
            data_.assign(total_pixels, fill_val);
            width_  = w;
            height_ = h;
            return true;
        }
        catch(...)
        {
            return false;
        }
    }

    [[nodiscard]] bool is_valid() const noexcept
    {
        if(width_ <= 0 || height_ <= 0)
            return false;

        const auto w64 = static_cast<uint64_t>(width_);
        const auto h64 = static_cast<uint64_t>(height_);
        const auto max_size =
            static_cast<uint64_t>((std::numeric_limits<size_t>::max)());

        if(w64 > max_size / h64)
            return false;

        const size_t total_pixels = static_cast<size_t>(w64 * h64);
        if(total_pixels > max_bitmap_pixels)
            return false;

        return data_.size() == total_pixels;
    }

  private:
    int                  width_{0};
    int                  height_{0};
    std::vector<uint8_t> data_;
};

namespace detail
{
class error_category final : public std::error_category
{
  public:
    const char *name() const noexcept override { return "hj::qrcode"; }

    std::string message(int ev) const override
    {
        switch(static_cast<error_code>(ev))
        {
            case error_code::success:
                return "Success";
            case error_code::invalid_text:
                return "Invalid text";
            case error_code::invalid_scale:
                return "Invalid scale";
            case error_code::invalid_margin:
                return "Invalid margin";
            case error_code::invalid_bitmap:
                return "Invalid bitmap";
            case error_code::encode_failed:
                return "QR code encoding failed";
            case error_code::pgm_write_failed:
                return "Failed to write PGM file";
            case error_code::allocation_failed:
                return "Memory allocation failed";
            case error_code::resize_failed:
                return "Failed to resize QR code parser";
            case error_code::begin_parse_failed:
                return "Failed to begin QR code parsing";
            case error_code::bm_data_insufficient:
                return "Bitmap data is insufficient";
            case error_code::pgm_open_failed:
                return "Failed to open PGM file";
            case error_code::pgm_read_failed:
                return "Failed to read PGM file";
            case error_code::magic_parse_failed:
                return "Failed to parse PGM magic number";
            case error_code::token_value_convert_failed:
                return "Failed to convert token value";
            case error_code::token_value_out_of_range:
                return "Token value out of range";
            case error_code::token_empty:
                return "Token is empty";
            case error_code::no_qrcode_found:
                return "No QR code found in image";
            case error_code::decode_failed:
                return "Failed to decode detected QR code candidate(s)";
            case error_code::bitmap_too_large:
                return "Bitmap dimensions exceed maximum allowed limits";
            default:
                return "Unknown error";
        }
    }

    bool
    equivalent(int                         code,
               const std::error_condition &condition) const noexcept override
    {
        return std::error_category::equivalent(code, condition);
    }
};

inline const std::error_category &qrcode_err_category_instance()
{
    static error_category instance;
    return instance;
}

static inline bool parse_int(std::string_view sv, int &out_val)
{
    if(sv.empty())
        return false;
    auto res = std::from_chars(sv.data(), sv.data() + sv.size(), out_val);
    return res.ec == std::errc{} && res.ptr == sv.data() + sv.size();
}

} // namespace detail

inline std::error_code make_error_code(error_code e) noexcept
{
    return std::error_code(static_cast<int>(e),
                           hj::qrcode::detail::qrcode_err_category_instance());
}

} // namespace hj::qrcode

template <>
struct std::is_error_code_enum<hj::qrcode::error_code> : std::true_type
{
};

namespace hj::qrcode
{

class builder
{
  public:
    builder()                           = delete;
    ~builder()                          = delete;
    builder(const builder &)            = delete;
    builder &operator=(const builder &) = delete;
    builder(builder &&)                 = delete;
    builder &operator=(builder &&)      = delete;

    static std::error_code encode(bitmap          &out,
                                  std::string_view text,
                                  int              version  = 0,
                                  int              ec_level = QR_ECLEVEL_L,
                                  int              scale    = 4,
                                  int              margin   = 4)
    {
        if(text.empty())
            return make_error_code(error_code::invalid_text);
        if(scale <= 0)
            return make_error_code(error_code::invalid_scale);
        if(margin < 0)
            return make_error_code(error_code::invalid_margin);

        QRcode *raw_q = QRcode_encodeData(
            static_cast<int>(text.size()),
            reinterpret_cast<const unsigned char *>(text.data()),
            version,
            static_cast<QRecLevel>(ec_level));
        if(!raw_q)
            return make_error_code(error_code::encode_failed);

        std::unique_ptr<QRcode, void (*)(QRcode *)> q(raw_q, [](QRcode *p) {
            if(p)
                QRcode_free(p);
        });

        const int modules = q->width;
        if(modules <= 0)
            return make_error_code(error_code::encode_failed);

        const uint64_t margin64 = static_cast<uint64_t>(margin);
        if(margin64 > (std::numeric_limits<uint64_t>::max)() / 2ULL)
            return make_error_code(error_code::encode_failed);
        const uint64_t double_margin = 2ULL * margin64;

        const uint64_t modules64 = static_cast<uint64_t>(modules);
        if(modules64 > (std::numeric_limits<uint64_t>::max)() - double_margin)
            return make_error_code(error_code::encode_failed);
        const uint64_t side = modules64 + double_margin;

        const uint64_t scale64 = static_cast<uint64_t>(scale);
        if(side > 0 && scale64 > (std::numeric_limits<uint64_t>::max)() / side)
            return make_error_code(error_code::encode_failed);
        const uint64_t img_size64 = side * scale64;

        if(img_size64 == 0)
            return make_error_code(error_code::encode_failed);

        const uint64_t max_size_t =
            static_cast<uint64_t>((std::numeric_limits<size_t>::max)());
        const uint64_t max_int =
            static_cast<uint64_t>((std::numeric_limits<int>::max)());
        const uint64_t safe_max = (max_size_t < max_int) ? max_size_t : max_int;

        if(img_size64 > safe_max)
            return make_error_code(error_code::encode_failed);

        if(img_size64 > safe_max / img_size64)
            return make_error_code(error_code::encode_failed);

        const size_t img_size = static_cast<size_t>(img_size64);

        if(!out.resize(static_cast<int>(img_size),
                       static_cast<int>(img_size),
                       255))
            return make_error_code(error_code::encode_failed);

        for(int r = 0; r < modules; ++r)
        {
            for(int c = 0; c < modules; ++c)
            {
                unsigned char module = q->data[r * modules + c];
                const bool    black  = (module & 0x1);
                if(!black)
                    continue;

                const size_t y0 = static_cast<size_t>(margin + r)
                                  * static_cast<size_t>(scale);
                const size_t x0 = static_cast<size_t>(margin + c)
                                  * static_cast<size_t>(scale);
                for(int y = 0; y < scale; ++y)
                {
                    const size_t row = (y0 + static_cast<size_t>(y)) * img_size;
                    for(int x = 0; x < scale; ++x)
                        out.data()[row + x0 + static_cast<size_t>(x)] = 0;
                }
            }
        }

        return std::error_code();
    }

    static std::error_code encode(const std::filesystem::path &path,
                                  std::string_view             text,
                                  int                          version = 0,
                                  int ec_level = QR_ECLEVEL_L,
                                  int scale    = 4,
                                  int margin   = 4)
    {
        bitmap bm;
        auto   ec = encode(bm, text, version, ec_level, scale, margin);
        if(ec)
            return ec;

        std::ofstream ofs(path, std::ios::binary);
        if(!ofs.is_open())
            return make_error_code(error_code::pgm_write_failed);

        ofs << "P5\n" << bm.width() << " " << bm.height() << "\n255\n";
        ofs.write(reinterpret_cast<const char *>(bm.data().data()),
                  bm.data().size());
        if(!ofs)
            return make_error_code(error_code::pgm_write_failed);
        ofs.close();
        return std::error_code();
    }
};

class parser
{
  public:
    parser()                          = delete;
    ~parser()                         = delete;
    parser(const parser &)            = delete;
    parser &operator=(const parser &) = delete;
    parser(parser &&)                 = delete;
    parser &operator=(parser &&)      = delete;

    static std::error_code decode(std::vector<std::string> &results,
                                  const bitmap             &bm)
    {
        results.clear();
        if(!bm.is_valid())
            return make_error_code(error_code::invalid_bitmap);

        const auto   w64          = static_cast<uint64_t>(bm.width());
        const auto   h64          = static_cast<uint64_t>(bm.height());
        const size_t total_pixels = static_cast<size_t>(w64 * h64);

        if(total_pixels > max_bitmap_pixels)
            return make_error_code(error_code::bitmap_too_large);

        std::unique_ptr<struct quirc, void (*)(struct quirc *)> q(
            quirc_new(),
            [](struct quirc *p) {
                if(p)
                    quirc_destroy(p);
            });
        if(!q)
            return make_error_code(error_code::allocation_failed);

        if(quirc_resize(q.get(), bm.width(), bm.height()) < 0)
            return make_error_code(error_code::resize_failed);

        int      w = 0, h = 0;
        uint8_t *buf = quirc_begin(q.get(), &w, &h);
        if(!buf)
            return make_error_code(error_code::begin_parse_failed);

        std::memcpy(buf, bm.data().data(), bm.data().size());
        quirc_end(q.get());

        const int count = quirc_count(q.get());
        if(count == 0)
            return make_error_code(error_code::no_qrcode_found);

        try
        {
            for(int i = 0; i < count; ++i)
            {
                struct quirc_code code;
                struct quirc_data data;
                quirc_extract(q.get(), i, &code);
                if(quirc_decode(&code, &data) == 0)
                    results.emplace_back(reinterpret_cast<char *>(data.payload),
                                         data.payload_len);
            }
        }
        catch(...)
        {
            results.clear();
            return make_error_code(error_code::allocation_failed);
        }

        if(results.empty())
            return make_error_code(error_code::decode_failed);

        return std::error_code();
    }

    static std::error_code decode(std::vector<std::string>    &results,
                                  const std::filesystem::path &path)
    {
        std::ifstream ifs(path, std::ios::binary);
        if(!ifs.is_open())
        {
            results.clear();
            return make_error_code(error_code::pgm_open_failed);
        }

        std::string magic;
        auto        ec = _read_token(magic, ifs);
        if(ec || magic != "P5")
        {
            results.clear();
            return make_error_code(error_code::magic_parse_failed);
        }

        std::string wtok, htok, maxvtok;
        ec = _read_token(wtok, ifs);
        if(ec)
        {
            results.clear();
            return ec;
        }

        ec = _read_token(htok, ifs);
        if(ec)
        {
            results.clear();
            return ec;
        }

        ec = _read_token(maxvtok, ifs);
        if(ec)
        {
            results.clear();
            return ec;
        }

        int w = 0, h = 0, maxv = 0;
        if(!detail::parse_int(wtok, w) || !detail::parse_int(htok, h)
           || !detail::parse_int(maxvtok, maxv))
        {
            results.clear();
            return make_error_code(error_code::token_value_convert_failed);
        }

        if(w <= 0 || h <= 0 || maxv != 255)
        {
            results.clear();
            return make_error_code(error_code::token_value_out_of_range);
        }

        const uint64_t w64 = static_cast<uint64_t>(w);
        const uint64_t h64 = static_cast<uint64_t>(h);
        const uint64_t max_size =
            static_cast<uint64_t>((std::numeric_limits<size_t>::max)());

        if(w64 > max_size / h64
           || static_cast<size_t>(w64 * h64) > max_bitmap_pixels)
        {
            results.clear();
            return make_error_code(error_code::bitmap_too_large);
        }

        bitmap bm(w, h);
        if(!bm.is_valid())
        {
            results.clear();
            return make_error_code(error_code::invalid_bitmap);
        }

        ifs.read(reinterpret_cast<char *>(bm.data().data()), bm.data().size());
        if(!ifs)
        {
            results.clear();
            return make_error_code(error_code::pgm_read_failed);
        }

        char dummy;
        if(ifs.get(dummy))
        {
            results.clear();
            return make_error_code(error_code::pgm_read_failed);
        }

        return decode(results, bm);
    }

  private:
    static std::error_code _read_token(std::string &tk, std::ifstream &ifs)
    {
        tk.clear();
        char c;
        while(ifs.get(c))
        {
            if(std::isspace(static_cast<unsigned char>(c)))
                continue;

            if(c == '#')
            {
                std::string line;
                std::getline(ifs, line);
                continue;
            }

            tk.push_back(c);
            break;
        }

        if(tk.empty())
            return make_error_code(error_code::token_empty);

        while(ifs.get(c))
        {
            if(std::isspace(static_cast<unsigned char>(c)) || c == '#')
            {
                if(c == '#')
                {
                    std::string line;
                    std::getline(ifs, line);
                }
                break;
            }
            tk.push_back(c);
        }

        return std::error_code();
    };
};

} // namespace hj::qrcode

#endif // QRCODE_HPP