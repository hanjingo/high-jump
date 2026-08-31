#include <gtest/gtest.h>
#include <hj/ai/asr.hpp>

#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------
// Context & State Initialization Tests
// -----------------------------------------------------------------------------

TEST(asr, default_full_params_matches_strategy)
{
    const auto greedy = hj::asr::context::default_full_params(
        hj::asr::context::sampling_strategy::greedy);
    const auto beam = hj::asr::context::default_full_params(
        hj::asr::context::sampling_strategy::beam_search);

    EXPECT_EQ(greedy.strategy, WHISPER_SAMPLING_GREEDY);
    EXPECT_EQ(beam.strategy, WHISPER_SAMPLING_BEAM_SEARCH);
}

TEST(asr, default_context_params_returns_valid_struct)
{
    const auto ctx_params = hj::asr::context::default_context_params();
    (void) ctx_params;
}

TEST(asr, create_with_empty_path_returns_invalid_argument)
{
    std::error_code ec;
    const auto      ctx = hj::asr::context::create(ec, "");

    EXPECT_FALSE(ctx.valid());
    EXPECT_EQ(ec,
              hj::asr::make_error_code(hj::asr::error_code::invalid_argument));
}

TEST(asr, create_with_non_existent_model_returns_model_load_failed)
{
    std::error_code ec;
    const auto      ctx =
        hj::asr::context::create(ec, "non_existent_model_file.bin");

    EXPECT_FALSE(ctx.valid());
    EXPECT_EQ(ec,
              hj::asr::make_error_code(hj::asr::error_code::model_load_failed));
}

TEST(asr, null_context_reports_invalid_state)
{
    hj::asr::context ctx;
    EXPECT_FALSE(ctx.valid());
    EXPECT_EQ(ctx.n_segments(), 0U);
}

// -----------------------------------------------------------------------------
// Inference Error Handling (Explicit Error Code Assertions)
// -----------------------------------------------------------------------------

TEST(asr, null_context_full_returns_invalid_state)
{
    hj::asr::context         ctx;
    const auto               params = hj::asr::context::default_full_params();
    const std::vector<float> samples{0.0f, 0.25f, -0.5f, 0.75f};

    EXPECT_EQ(ctx.full(params, samples),
              hj::asr::make_error_code(hj::asr::error_code::invalid_state));
    EXPECT_EQ(ctx.full(params, samples.data(), samples.size()),
              hj::asr::make_error_code(hj::asr::error_code::invalid_state));
}

TEST(asr, null_state_full_returns_invalid_state)
{
    hj::asr::state           st;
    const auto               params = hj::asr::context::default_full_params();
    const std::vector<float> samples{0.0f, 0.25f};

    EXPECT_FALSE(st.valid());
    EXPECT_EQ(st.full(nullptr, params, samples),
              hj::asr::make_error_code(hj::asr::error_code::invalid_state));
}

TEST(asr, null_pointer_with_nonzero_count_returns_invalid_argument)
{
    hj::asr::context ctx;
    const auto       params = hj::asr::context::default_full_params();

    EXPECT_EQ(ctx.full(params, static_cast<const float *>(nullptr), 100),
              hj::asr::make_error_code(hj::asr::error_code::invalid_state));
}

TEST(asr, null_context_get_segment_returns_nullopt_or_empty)
{
    hj::asr::context ctx;

    EXPECT_EQ(ctx.get_all_text(), "");
    EXPECT_TRUE(ctx.segments().empty());
    EXPECT_FALSE(ctx.get_segment(0).has_value());
    EXPECT_FALSE(ctx.get_segment(100).has_value());
    EXPECT_FALSE(ctx.get_segment_text(0).has_value());
}

// -----------------------------------------------------------------------------
// Lifecycle & Move Semantics
// -----------------------------------------------------------------------------

TEST(asr, move_construct_and_assign_are_safe_for_null_context)
{
    hj::asr::context src;
    hj::asr::context moved(std::move(src));

    EXPECT_EQ(src.n_segments(), 0U);
    EXPECT_EQ(moved.n_segments(), 0U);

    hj::asr::context dst;
    dst = std::move(moved);

    EXPECT_EQ(moved.n_segments(), 0U);
    EXPECT_EQ(dst.n_segments(), 0U);
}

// -----------------------------------------------------------------------------
// Audio PCM Utility
// -----------------------------------------------------------------------------

TEST(asr, convert_pcm16_to_normalized_float)
{
    const int16_t     pcm_samples[] = {0, 32767, -32768, 16384, -16384};
    const std::string raw(reinterpret_cast<const char *>(pcm_samples),
                          sizeof(pcm_samples));

    std::vector<float> out;
    hj::asr::context::convert_pcm16le(out, raw);

    ASSERT_EQ(out.size(), 5U);
    EXPECT_FLOAT_EQ(out[0], 0.0f);
    EXPECT_FLOAT_EQ(out[1], 32767.0f / 32768.0f);
    EXPECT_FLOAT_EQ(out[2], -1.0f);
    EXPECT_FLOAT_EQ(out[3], 0.5f);
    EXPECT_FLOAT_EQ(out[4], -0.5f);
}

TEST(asr, convert_ignores_incomplete_trailing_byte)
{
    const int16_t pcm_samples[] = {1000, -1000};
    std::string   raw(reinterpret_cast<const char *>(pcm_samples),
                      sizeof(pcm_samples));
    raw.push_back('\x7f');

    std::vector<float> out;
    hj::asr::context::convert_pcm16le(out, raw);

    ASSERT_EQ(out.size(), 2U);
    EXPECT_FLOAT_EQ(out[0], 1000.0f / 32768.0f);
    EXPECT_FLOAT_EQ(out[1], -1000.0f / 32768.0f);
}

TEST(asr, convert_appends_to_existing_destination)
{
    const int16_t     pcm_samples[] = {32767};
    const std::string raw(reinterpret_cast<const char *>(pcm_samples),
                          sizeof(pcm_samples));

    std::vector<float> out{-0.25f};
    hj::asr::context::convert_pcm16le(out, raw);

    ASSERT_EQ(out.size(), 2U);
    EXPECT_FLOAT_EQ(out[0], -0.25f);
    EXPECT_FLOAT_EQ(out[1], 32767.0f / 32768.0f);
}

// -----------------------------------------------------------------------------
// Error Category & Code Tests
// -----------------------------------------------------------------------------

TEST(asr, error_category_name_is_correct)
{
    const auto &cat = hj::asr::asr_err_category_instance();
    EXPECT_STREQ(cat.name(), "hj::asr");
}

TEST(asr, error_code_messages_match_definitions)
{
    struct TestCase
    {
        hj::asr::error_code code;
        std::string         expected_message;
    };

    const TestCase test_cases[] = {
        {hj::asr::error_code::success, "Success"},
        {hj::asr::error_code::invalid_state,
         "Invalid state: whisper_context or state is null"},
        {hj::asr::error_code::invalid_argument,
         "Invalid argument: null pointer or invalid input parameters"},
        {hj::asr::error_code::buffer_overflow,
         "Buffer overflow: input audio samples exceed maximum size"},
        {hj::asr::error_code::model_load_failed,
         "Model load failed: unable to initialize whisper model from file "
         "(file "
         "missing, format invalid, or OOM)"},
        {hj::asr::error_code::inference_failed,
         "Inference failed: whisper_full() processing error"},
        {static_cast<hj::asr::error_code>(9999), "Unknown ASR model error"},
    };

    for(const auto &tc : test_cases)
    {
        const std::error_code ec = hj::asr::make_error_code(tc.code);

        // 验证 category 关联正确
        EXPECT_EQ(std::string(ec.category().name()), "hj::asr");
        // 验证 message 输出完全符合预期
        EXPECT_EQ(ec.message(), tc.expected_message);
    }
}