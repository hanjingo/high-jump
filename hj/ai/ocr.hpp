#ifndef OCR_HPP
#define OCR_HPP

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <utility>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

namespace hj::ocr
{

enum class error_code
{
    init_failed,

    image_load_failed,
    invalid_image,

    text_extraction_failed
};

class ocr_err_category final : public std::error_category
{
  public:
    const char *name() const noexcept override { return "hj::ocr"; }

    std::string message(int ev) const override
    {
        switch(static_cast<error_code>(ev))
        {
            case error_code::init_failed:
                return "Initialization failed";
            case error_code::image_load_failed:
                return "Image load failed";
            case error_code::invalid_image:
                return "Invalid image";
            case error_code::text_extraction_failed:
                return "Text extraction failed";
            default:
                return "Unknown ocr error";
        }
    }
};

inline const std::error_category &ocr_err_category_instance()
{
    static ocr_err_category instance;
    return instance;
}

inline std::error_code make_error_code(error_code e) noexcept
{
    return std::error_code(static_cast<int>(e),
                           hj::ocr::ocr_err_category_instance());
}

} // namespace hj::ocr

template <>
struct std::is_error_code_enum<hj::ocr::error_code> : std::true_type
{
};

namespace hj::ocr
{

using pix_t = Pix;

enum class engine_mode
{
    oem_tesseract_only          = tesseract::OEM_TESSERACT_ONLY,
    oem_lstm_only               = tesseract::OEM_LSTM_ONLY,
    oem_tesseract_lstm_combined = tesseract::OEM_TESSERACT_LSTM_COMBINED,
    oem_default                 = tesseract::OEM_DEFAULT
};

namespace detail
{
struct text_deleter
{
    void operator()(char *ptr) const noexcept
    {
        if(ptr)
            delete[] ptr;
    }
};
} // namespace detail

class image
{
  public:
    image() noexcept = default;
    explicit image(Pix *pix) noexcept
        : _pix(pix)
    {
    }

    ~image()
    {
        if(_pix)
            pixDestroy(&_pix);
    }

    image(const image &)            = delete;
    image &operator=(const image &) = delete;

    image(image &&other) noexcept
        : _pix(std::exchange(other._pix, nullptr))
    {
    }

    image &operator=(image &&other) noexcept
    {
        if(this != &other)
        {
            reset();
            _pix = std::exchange(other._pix, nullptr);
        }

        return *this;
    }
    explicit operator bool() const noexcept { return _pix != nullptr; }

    static image load(const std::filesystem::path &filename,
                      std::error_code             &ec)
    {
        Pix *pix = pixRead(filename.string().c_str());
        if(pix == nullptr)
        {
            ec = make_error_code(error_code::image_load_failed);
            return image(nullptr);
        }

        ec.clear();
        return image(pix);
    }
    static image
    load(const uint8_t *data, std::size_t size, std::error_code &ec)
    {
        Pix *pix = pixReadMem(data, size);
        if(pix == nullptr)
        {
            ec = make_error_code(error_code::image_load_failed);
            return image(nullptr);
        }

        ec.clear();
        return image(pix);
    }

    [[nodiscard]]
    bool empty() const noexcept
    {
        return _pix == nullptr;
    }

    [[nodiscard]]
    pix_t *data() noexcept
    {
        return _pix;
    }

    void reset() noexcept
    {
        if(_pix)
        {
            pixDestroy(&_pix);
            _pix = nullptr;
        }
    }

  private:
    pix_t *_pix = nullptr;
};

struct parser_options
{
    std::string           language = "eng";
    std::filesystem::path datapath;
    engine_mode           engine = engine_mode::oem_lstm_only;
};

class parser
{
  private:
    struct parser_key
    {
        explicit parser_key() = default;
    };

  public:
    explicit parser(parser_key) {}
    ~parser()
    {
        if(_api)
            _api->End();
    }
    parser(const parser &)            = delete;
    parser &operator=(const parser &) = delete;

    parser(parser &&other) noexcept
    {
        if(this != &other)
        {
            if(_api)
                _api->End();

            _api = std::move(other._api);
        }
    }
    parser &operator=(parser &&other) noexcept
    {
        if(this != &other)
        {
            if(_api)
                _api->End();

            _api = std::move(other._api);
        }
        return *this;
    }

    static std::unique_ptr<parser> make_unique(const parser_options &options,
                                               std::error_code      &ec)
    {
        auto instance  = std::make_unique<parser>(parser::parser_key{});
        instance->_api = std::make_unique<tesseract::TessBaseAPI>();

        const std::string datapath = options.datapath.empty()
                                         ? std::string{}
                                         : options.datapath.string();
        const char *path_ptr = datapath.empty() ? nullptr : datapath.c_str();
        if(instance->_api->Init(path_ptr, options.language.data()) != 0)
        {
            ec = make_error_code(error_code::init_failed);
            return nullptr;
        }

        ec.clear();
        return instance;
    }

    static std::unique_ptr<parser> make_unique(std::error_code &ec)
    {
        return make_unique(parser_options{}, ec);
    }

    [[nodiscard]] std::error_code set_image(image &img) noexcept
    {
        if(img.empty())
            return make_error_code(error_code::invalid_image);

        _api->SetImage(img.data());
        return {};
    }

    [[nodiscard]]
    std::error_code get_text(std::string &result)
    {
        std::unique_ptr<char[], detail::text_deleter> text(_api->GetUTF8Text());
        if(!text)
            return make_error_code(error_code::text_extraction_failed);

        result.assign(text.get());
        return {};
    }

    [[nodiscard]]
    std::error_code recognize(std::string &result, image &img)
    {
        auto ec = set_image(img);
        if(ec)
            return ec;

        return get_text(result);
    }

  private:
    std::unique_ptr<tesseract::TessBaseAPI> _api;
};

} // namespace hj::ocr

#endif // OCR_HPP