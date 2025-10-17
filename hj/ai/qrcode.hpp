#ifndef QRCODE_HPP
#define QRCODE_HPP

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

#include <qrencode.h>
#include <quirc.h>
#include <cstring>
#include <cctype>
#include <memory>
#include <limits>

namespace hj
{
namespace qrcode
{

enum class error_code
{
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
};

struct bitmap
{
    int                  width{0};
    int                  height{0};
    std::vector<uint8_t> data;
};

namespace detail
{
class error_category : public std::error_category
{
  public:
    const char *name() const noexcept override { return "qrcode"; }

    std::string message(int ev) const override
    {
        switch(static_cast<error_code>(ev))
        {
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

static inline std::error_code make_err(error_code e)
{
    static error_category cat;
    return std::error_code(static_cast<int>(e), cat);
}
} // namespace detail

class builder
{
  public:
    builder()                           = delete;
    ~builder()                          = delete;
    builder(const builder &)            = delete;
    builder &operator=(const builder &) = delete;
    builder(builder &&)                 = delete;
    builder &operator=(builder &&)      = delete;

    static std::error_code encode(bitmap            &out,
                                  const std::string &text,
                                  const int          version  = 0,
                                  const int          ec_level = QR_ECLEVEL_L,
                                  const int          scale    = 4,
                                  const int          margin   = 4)
    {
        if(text.empty())
            return detail::make_err(error_code::invalid_text);
        if(scale <= 0)
            return detail::make_err(error_code::invalid_scale);
        if(margin < 0)
            return detail::make_err(error_code::invalid_margin);

        // Use RAII for QRcode* so it's freed even on early returns
        QRcode *raw_q = QRcode_encodeString(text.c_str(),
                                            version,
                                            static_cast<QRecLevel>(ec_level),
                                            QR_MODE_8,
                                            1);
        if(!raw_q)
            return detail::make_err(error_code::encode_failed);

        std::unique_ptr<QRcode, void (*)(QRcode *)> q(raw_q, [](QRcode *p) {
            if(p)
                QRcode_free(p);
        });

        const int modules = q->width; // number of modules per side
        if(modules <= 0)
            return detail::make_err(error_code::encode_failed);

        // calculate image size and check for overflow
        const uint64_t side = static_cast<uint64_t>(modules)
                              + 2ULL * static_cast<uint64_t>(margin);
        const uint64_t img_size64 = side * static_cast<uint64_t>(scale);
        if(img_size64 == 0)
            return detail::make_err(error_code::encode_failed);

        const uint64_t max_sz =
            static_cast<uint64_t>(std::numeric_limits<size_t>::max());
        // if img_size64 cannot fit into size_t, fail
        if(img_size64 > max_sz)
            return detail::make_err(error_code::encode_failed);

        // check multiplication overflow: img_size64 * img_size64 must not exceed max_sz
        if(img_size64 > 0 && img_size64 > max_sz / img_size64)
            return detail::make_err(error_code::encode_failed);

        const size_t   img_size = static_cast<size_t>(img_size64);
        const uint64_t pixels64 = img_size64 * img_size64; // safe now
        const size_t   pixels   = static_cast<size_t>(pixels64);

        out.width  = static_cast<int>(img_size);
        out.height = static_cast<int>(img_size);
        out.data.assign(pixels, 255); // white background

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
                        out.data[row + x0 + static_cast<size_t>(x)] = 0;
                }
            }
        }

        return std::error_code();
    }

    static std::error_code encode(const std::string &pgm,
                                  const std::string &text,
                                  const int          version  = 0,
                                  const int          ec_level = QR_ECLEVEL_L,
                                  const int          scale    = 4,
                                  const int          margin   = 4)
    {
        bitmap bm;
        auto   ec = encode(bm, text, version, ec_level, scale, margin);
        if(ec)
            return ec;

        // Save as PGM file
        std::ofstream ofs(pgm, std::ios::binary);
        if(!ofs.is_open())
            return detail::make_err(error_code::pgm_write_failed);

        // P5 header
        ofs << "P5\n" << bm.width << " " << bm.height << "\n255\n";
        ofs.write(reinterpret_cast<const char *>(bm.data.data()),
                  bm.data.size());
        if(!ofs)
            return detail::make_err(error_code::pgm_write_failed);
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
        if(bm.data.empty() || bm.width <= 0 || bm.height <= 0)
            return detail::make_err(error_code::invalid_bitmap);
        // RAII for quirc parser
        std::unique_ptr<struct quirc, void (*)(struct quirc *)> q(
            quirc_new(),
            [](struct quirc *p) {
                if(p)
                    quirc_destroy(p);
            });
        if(!q)
            return detail::make_err(error_code::allocation_failed);

        if(quirc_resize(q.get(), bm.width, bm.height) < 0)
            return detail::make_err(error_code::resize_failed);

        int      w = 0, h = 0;
        uint8_t *buf = quirc_begin(q.get(), &w, &h);
        if(!buf)
            return detail::make_err(error_code::begin_parse_failed);

        const size_t expect = static_cast<size_t>(w) * static_cast<size_t>(h);
        if(bm.data.size() < expect)
            return detail::make_err(error_code::bm_data_insufficient);

        std::memcpy(buf, bm.data.data(), expect);
        quirc_end(q.get());

        const int count = quirc_count(q.get());
        for(int i = 0; i < count; ++i)
        {
            struct quirc_code code;
            struct quirc_data data;
            quirc_extract(q.get(), i, &code);
            if(quirc_decode(&code, &data) == 0)
                results.emplace_back(reinterpret_cast<char *>(data.payload),
                                     data.payload_len);
        }

        return std::error_code();
    }

    static std::error_code decode(std::vector<std::string> &results,
                                  const std::string        &pgm)
    {
        std::ifstream ifs(pgm, std::ios::binary);
        if(!ifs.is_open())
            return detail::make_err(error_code::pgm_open_failed);

        std::string magic;
        auto        ec = _read_token(magic, ifs);
        if(ec || magic != "P5")
            return detail::make_err(error_code::magic_parse_failed);

        std::string wtok, htok, maxvtok;
        ec = _read_token(wtok, ifs);
        if(ec)
            return ec;

        ec = _read_token(htok, ifs);
        if(ec)
            return ec;

        ec = _read_token(maxvtok, ifs);
        if(ec)
            return ec;

        int w = 0, h = 0, maxv = 0;
        try
        {
            w    = std::stoi(wtok);
            h    = std::stoi(htok);
            maxv = std::stoi(maxvtok);
        }
        catch(...)
        {
            return detail::make_err(error_code::token_value_convert_failed);
        }

        if(w <= 0 || h <= 0 || maxv <= 0 || maxv > 255)
            return detail::make_err(error_code::token_value_out_of_range);

        bitmap bm;
        bm.width  = w;
        bm.height = h;
        bm.data.resize(static_cast<size_t>(w) * static_cast<size_t>(h));
        ifs.read(reinterpret_cast<char *>(bm.data.data()), bm.data.size());
        if(!ifs)
            return detail::make_err(error_code::pgm_read_failed);

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
            return detail::make_err(error_code::token_empty);

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

} // namespace qrcode
} // namespace hj

#endif // QRCODE_HPP