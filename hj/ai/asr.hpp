#ifndef ASR_HPP
#define ASR_HPP

#include <whisper.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace hj::asr
{

enum class error_code
{
    success = 0,

    invalid_state,
    invalid_argument,
    buffer_overflow,

    model_load_failed,
    inference_failed,
};

class asr_err_category : public std::error_category
{
  public:
    const char *name() const noexcept override { return "hj::asr"; }

    std::string message(int ev) const override
    {
        switch(static_cast<error_code>(ev))
        {
            case error_code::success:
                return "Success";
            case error_code::invalid_state:
                return "Invalid state: whisper_context or state is null";
            case error_code::invalid_argument:
                return "Invalid argument: null pointer or invalid input "
                       "parameters";
            case error_code::buffer_overflow:
                return "Buffer overflow: input audio samples exceed maximum "
                       "size";
            case error_code::model_load_failed:
                return "Model load failed: unable to initialize whisper model "
                       "from file (file missing, format invalid, or OOM)";
            case error_code::inference_failed:
                return "Inference failed: whisper_full() processing error";
            default:
                return "Unknown ASR model error";
        }
    }
};

inline const std::error_category &asr_err_category_instance()
{
    static asr_err_category instance;
    return instance;
}

inline std::error_code make_error_code(error_code e) noexcept
{
    return std::error_code(static_cast<int>(e),
                           hj::asr::asr_err_category_instance());
}

} // namespace hj::asr

template <>
struct std::is_error_code_enum<hj::asr::error_code> : std::true_type
{
};

namespace hj::asr
{

using ctx_t         = whisper_context;
using full_params_t = whisper_full_params;
using ctx_params_t  = whisper_context_params;
using vad_params_t  = whisper_vad_params;
using state_t       = whisper_state;

struct segment
{
    int64_t     start_ms{0};
    int64_t     end_ms{0};
    std::string text;
};

class state
{
  public:
    state() noexcept
        : _state{nullptr}
    {
    }
    explicit state(whisper_state *st) noexcept
        : _state{st}
    {
    }

    ~state()
    {
        if(_state)
            whisper_free_state(_state);
    }

    state(const state &)            = delete;
    state &operator=(const state &) = delete;

    state(state &&other) noexcept
        : _state{std::exchange(other._state, nullptr)}
    {
    }

    state &operator=(state &&other) noexcept
    {
        if(this != &other)
        {
            if(_state)
                whisper_free_state(_state);
            _state = std::exchange(other._state, nullptr);
        }
        return *this;
    }

    [[nodiscard]] bool valid() const noexcept { return _state != nullptr; }

    std::error_code full(whisper_context          *ctx,
                         full_params_t             params,
                         const std::vector<float> &samples)
    {
        return full(ctx, params, samples.data(), samples.size());
    }

    std::error_code full(whisper_context *ctx,
                         full_params_t    params,
                         const float     *samples,
                         size_t           n_samples)
    {
        if(!_state || !ctx)
            return make_error_code(error_code::invalid_state);

        if(n_samples != 0 && samples == nullptr)
            return make_error_code(error_code::invalid_argument);

        if(n_samples > static_cast<size_t>((std::numeric_limits<int>::max)()))
            return make_error_code(error_code::buffer_overflow);

        if(0
           != whisper_full_with_state(ctx,
                                      _state,
                                      params,
                                      samples,
                                      static_cast<int>(n_samples)))
            return make_error_code(error_code::inference_failed);

        return make_error_code(error_code::success);
    }

    [[nodiscard]] size_t n_segments() const noexcept
    {
        if(!_state)
            return 0;

        const int count = whisper_full_n_segments_from_state(_state);
        return count > 0 ? static_cast<size_t>(count) : 0;
    }

    [[nodiscard]] std::optional<segment> get_segment(size_t idx) const
    {
        if(!_state)
            return std::nullopt;

        const size_t total = n_segments();
        if(idx >= total)
            return std::nullopt;

        const int i = static_cast<int>(idx);

        segment seg;
        seg.start_ms = static_cast<int64_t>(
                           whisper_full_get_segment_t0_from_state(_state, i))
                       * 10;
        seg.end_ms   = static_cast<int64_t>(
                           whisper_full_get_segment_t1_from_state(_state, i))
                       * 10;

        if(const char *txt =
               whisper_full_get_segment_text_from_state(_state, i))
        {
            seg.text = txt;
        }

        return seg;
    }

    [[nodiscard]] std::vector<segment> segments() const
    {
        if(!_state)
            return {};

        const size_t         total = n_segments();
        std::vector<segment> result;
        result.reserve(total);

        for(size_t i = 0; i < total; ++i)
        {
            if(auto seg = get_segment(i))
            {
                result.push_back(std::move(*seg));
            }
        }
        return result;
    }

    [[nodiscard]] std::string get_all_text() const
    {
        if(!_state)
            return "";

        std::string  result;
        const size_t total = n_segments();
        for(size_t i = 0; i < total; ++i)
        {
            if(const char *segment_text =
                   whisper_full_get_segment_text_from_state(
                       _state,
                       static_cast<int>(i)))
            {
                result.append(segment_text);
            }
        }
        return result;
    }

  private:
    whisper_state *_state{nullptr};
};

class context
{
  public:
    enum class sampling_strategy
    {
        greedy      = WHISPER_SAMPLING_GREEDY,
        beam_search = WHISPER_SAMPLING_BEAM_SEARCH,
    };

  public:
    context() noexcept
        : _ctx{nullptr}
    {
    }

    ~context()
    {
        if(_ctx)
            whisper_free(_ctx);
    }

    context(const context &)      = delete;
    context &operator=(context &) = delete;

    context(context &&other) noexcept
        : _ctx{std::exchange(other._ctx, nullptr)}
    {
    }

    context &operator=(context &&other) noexcept
    {
        if(this != &other)
        {
            if(_ctx)
                whisper_free(_ctx);
            _ctx = std::exchange(other._ctx, nullptr);
        }
        return *this;
    }

    static context
    create(std::error_code   &err,
           const std::string &model_path,
           ctx_params_t       params = whisper_context_default_params())
    {
        if(model_path.empty())
        {
            err = make_error_code(error_code::invalid_argument);
            return {};
        }

        context ctx;
        ctx._ctx =
            whisper_init_from_file_with_params(model_path.c_str(), params);
        if(!ctx._ctx)
        {
            err = make_error_code(error_code::model_load_failed);
            return {};
        }

        err = make_error_code(error_code::success);
        return std::move(ctx);
    }

    [[nodiscard]] bool valid() const noexcept { return _ctx != nullptr; }

    [[nodiscard]] state create_state() const
    {
        if(!_ctx)
            return state{};
        return state{whisper_init_state(_ctx)};
    }

    static ctx_params_t default_context_params()
    {
        return whisper_context_default_params();
    }

    [[deprecated("Use default_context_params() instead.")]]
    static ctx_params_t default_params()
    {
        return default_context_params();
    }

    static full_params_t default_full_params(
        const sampling_strategy strategy = sampling_strategy::greedy)
    {
        switch(strategy)
        {
            case sampling_strategy::greedy:
                return whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
            case sampling_strategy::beam_search:
                return whisper_full_default_params(
                    WHISPER_SAMPLING_BEAM_SEARCH);
            default:
                return whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
        }
    }

    static void convert_pcm16le(std::vector<float> &dst, std::string_view src)
    {
        const size_t sample_count = src.size() / sizeof(int16_t);
        dst.reserve(dst.size() + sample_count);

        for(size_t i = 0; i < sample_count; ++i)
        {
            int16_t raw_sample = 0;
            std::memcpy(&raw_sample,
                        src.data() + i * sizeof(int16_t),
                        sizeof(int16_t));

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            raw_sample = static_cast<int16_t>(
                (static_cast<uint16_t>(raw_sample) >> 8)
                | (static_cast<uint16_t>(raw_sample) << 8));
#endif

            const float samplef = static_cast<float>(raw_sample) / 32768.0f;
            dst.push_back(std::clamp(samplef, -1.0f, 1.0f));
        }
    }

    std::error_code full(full_params_t             params,
                         const std::vector<float> &samples)
    {
        return full(params, samples.data(), samples.size());
    }

    std::error_code
    full(full_params_t params, const float *samples, size_t n_samples)
    {
        if(!_ctx)
            return make_error_code(error_code::invalid_state);

        if(n_samples != 0 && samples == nullptr)
            return make_error_code(error_code::invalid_argument);

        if(n_samples > static_cast<size_t>((std::numeric_limits<int>::max)()))
            return make_error_code(error_code::buffer_overflow);

        if(0
           != whisper_full(_ctx, params, samples, static_cast<int>(n_samples)))
            return make_error_code(error_code::inference_failed);

        return make_error_code(error_code::success);
    }

    [[nodiscard]] size_t n_segments() const noexcept
    {
        if(!_ctx)
            return 0;

        const int count = whisper_full_n_segments(_ctx);
        return count > 0 ? static_cast<size_t>(count) : 0;
    }

    [[nodiscard]] std::optional<segment> get_segment(size_t idx) const
    {
        if(!_ctx)
            return std::nullopt;

        const size_t total = n_segments();
        if(idx >= total)
            return std::nullopt;

        const int i = static_cast<int>(idx);

        segment seg;
        seg.start_ms =
            static_cast<int64_t>(whisper_full_get_segment_t0(_ctx, i)) * 10;
        seg.end_ms =
            static_cast<int64_t>(whisper_full_get_segment_t1(_ctx, i)) * 10;

        if(const char *txt = whisper_full_get_segment_text(_ctx, i))
        {
            seg.text = txt;
        }

        return seg;
    }

    [[nodiscard]] std::vector<segment> segments() const
    {
        if(!_ctx)
            return {};

        const size_t         total = n_segments();
        std::vector<segment> result;
        result.reserve(total);

        for(size_t i = 0; i < total; ++i)
        {
            if(auto seg = get_segment(i))
            {
                result.push_back(std::move(*seg));
            }
        }
        return result;
    }

    [[nodiscard]] std::optional<std::string> get_segment_text(size_t idx) const
    {
        if(auto seg = get_segment(idx))
        {
            return std::move(seg->text);
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string get_all_text() const
    {
        if(!_ctx)
            return "";

        std::string  result;
        const size_t total = n_segments();
        for(size_t i = 0; i < total; ++i)
        {
            if(const char *segment_text =
                   whisper_full_get_segment_text(_ctx, static_cast<int>(i)))
            {
                result.append(segment_text);
            }
        }
        return result;
    }

    [[nodiscard]] whisper_context *raw_context() noexcept { return _ctx; }

  private:
    whisper_context *_ctx{nullptr};
};

} // namespace hj::asr

#endif // ASR_HPP