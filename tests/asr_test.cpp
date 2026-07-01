#include <gtest/gtest.h>
#include <hj/ai/asr.hpp>

#include <utility>
#include <vector>

TEST(asr_test, default_full_params_matches_strategy)
{
    const auto greedy = hj::asr::context::default_full_params(
        hj::asr::context::sampling_strategy::greedy);
    const auto beam = hj::asr::context::default_full_params(
        hj::asr::context::sampling_strategy::beam_search);

    EXPECT_EQ(greedy.strategy, WHISPER_SAMPLING_GREEDY);
    EXPECT_EQ(beam.strategy, WHISPER_SAMPLING_BEAM_SEARCH);
}

TEST(asr_test, null_context_reports_invalid_state)
{
    hj::asr::context ctx;
    EXPECT_EQ(ctx.n_segments(), -1);
}

TEST(asr_test, null_context_full_with_vector_returns_error)
{
    hj::asr::context         ctx;
    const auto               params = hj::asr::context::default_full_params();
    const std::vector<float> samples{0.0f, 0.25f, -0.5f, 0.75f};

    EXPECT_EQ(ctx.full(params, samples), -1);
}

TEST(asr_test, null_context_full_with_pointer_returns_error)
{
    hj::asr::context ctx;
    const auto       params    = hj::asr::context::default_full_params();
    const float      samples[] = {0.0f, 0.25f, -0.5f, 0.75f};

    EXPECT_EQ(ctx.full(params, samples, 4), -1);
}

TEST(asr_test, null_context_get_segment_text_keeps_existing_text)
{
    hj::asr::context ctx;
    std::string      text = "keep";

    ctx.get_segment_text(text);
    EXPECT_EQ(text, "keep");

    ctx.get_segment_text(text, 0);
    EXPECT_EQ(text, "keep");
}

TEST(asr_test, move_construct_and_assign_are_safe_for_null_context)
{
    hj::asr::context src;
    hj::asr::context moved(std::move(src));

    EXPECT_EQ(src.n_segments(), -1);
    EXPECT_EQ(moved.n_segments(), -1);

    hj::asr::context dst;
    dst = std::move(moved);

    EXPECT_EQ(moved.n_segments(), -1);
    EXPECT_EQ(dst.n_segments(), -1);
}
