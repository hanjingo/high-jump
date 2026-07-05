#ifndef ASR_HPP
#define ASR_HPP

#include <whisper.h>

#include <limits>
#include <vector>
#include <string>

namespace hj
{
namespace asr
{

using ctx_t         = whisper_context;
using full_params_t = whisper_full_params;
using ctx_params_t  = whisper_context_params;
using state_t       = whisper_state;

class context
{
  public:
    enum class sampling_strategy
    {
        greedy      = WHISPER_SAMPLING_GREEDY,
        beam_search = WHISPER_SAMPLING_BEAM_SEARCH,
    };

  public:
    context()
        : _ctx{nullptr}
    {
    }
    explicit context(const std::string &model_path)
        : _ctx{whisper_init_from_file(model_path.c_str())}
    {
    }
    context(const std::string &model_path, ctx_params_t params)
        : _ctx{whisper_init_from_file_with_params(model_path.c_str(), params)}
    {
    }
    context(const context &)            = delete;
    context &operator=(const context &) = delete;

    context(context &&other) noexcept
        : _ctx{other._ctx}
    {
        other._ctx = nullptr;
    }

    context &operator=(context &&other) noexcept
    {
        if(this == &other)
            return *this;

        if(_ctx)
            whisper_free(_ctx);

        _ctx       = other._ctx;
        other._ctx = nullptr;
        return *this;
    }

    ~context()
    {
        if(_ctx)
            whisper_free(_ctx);
    }

    static ctx_params_t default_params()
    {
        return whisper_context_default_params();
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

    static void convert(std::vector<float> &dst, const std::string &src)
    {
        int sample_count = src.size() / sizeof(int16_t);
        dst.reserve(sample_count);
        const int16_t *samples = reinterpret_cast<const int16_t *>(src.data());
        for(int i = 0; i < sample_count; ++i)
        {
            float samplef = static_cast<float>(samples[i]) / 32768.0f;
            if(samplef < -1.0f)
                samplef = -1.0f;

            if(samplef > 1.0f)
                samplef = 1.0f;

            dst.push_back(samplef);
        }
    }

    inline ctx_t *const data() { return _ctx; }

    int full(full_params_t params, const std::vector<float> &samples)
    {
        if(!_ctx)
            return -1;

        if(samples.size()
           > static_cast<size_t>(std::numeric_limits<int>::max()))
            return -1;

        return whisper_full(_ctx,
                            params,
                            samples.data(),
                            static_cast<int>(samples.size()));
    }

    int full(full_params_t params, const float *samples, int n_samples)
    {
        if(!_ctx)
            return -1;

        return whisper_full(_ctx, params, samples, n_samples);
    }

    int n_segments() const
    {
        if(!_ctx)
            return -1;

        return whisper_full_n_segments(_ctx);
    }

    void get_segment_text(std::string &text, const int idx = -1)
    {
        if(!_ctx)
            return;

        const int seg_count = whisper_full_n_segments(_ctx);

        if(idx > -1)
        {
            if(idx >= seg_count)
                return;

            const char *segment_text = whisper_full_get_segment_text(_ctx, idx);
            if(segment_text)
                text = std::string(segment_text);

            return;
        }

        text.clear();
        for(int i = 0; i < seg_count; ++i)
        {
            const char *segment_text = whisper_full_get_segment_text(_ctx, i);
            if(segment_text)
                text.append(std::string(segment_text));
        }
    }

  private:
    whisper_context *_ctx;
};

}
}

#endif // ASR_HPP