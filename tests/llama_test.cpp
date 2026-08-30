#include <gtest/gtest.h>
#include <hj/ai/llama.hpp>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <future>
#include <vector>

namespace
{
std::filesystem::path find_llama_fixture()
{
    const std::filesystem::path source_dir =
        std::filesystem::path(__FILE__).parent_path();

    const std::filesystem::path candidates[] = {
        std::filesystem::current_path() / "stories260K.gguf",
        source_dir / "stories260K.gguf",
        source_dir.parent_path() / "tests" / "stories260K.gguf",
    };

    for(const auto &p : candidates)
    {
        if(std::filesystem::exists(p))
            return p;
    }

    return {};
}

bool is_valid_gguf(const std::filesystem::path &model_path)
{
    std::ifstream in(model_path, std::ios::binary);
    if(!in)
        return false;

    char magic[4] = {0};
    in.read(magic, sizeof(magic));
    return in.gcount() == 4 && magic[0] == 'G' && magic[1] == 'G'
           && magic[2] == 'U' && magic[3] == 'F';
}

void make_read_only(const std::filesystem::path &p)
{
    std::error_code ec;
    std::filesystem::permissions(p,
                                 std::filesystem::perms::owner_read
                                     | std::filesystem::perms::group_read
                                     | std::filesystem::perms::others_read,
                                 std::filesystem::perm_options::replace,
                                 ec);
}

void make_writable(const std::filesystem::path &p)
{
    std::error_code ec;
    std::filesystem::permissions(p,
                                 std::filesystem::perms::owner_write,
                                 std::filesystem::perm_options::add,
                                 ec);
}

#define LOAD_FIXTURE_OR_SKIP(model_var)                                        \
    const std::filesystem::path fixture_path_ = find_llama_fixture();          \
    if(fixture_path_.empty() || !is_valid_gguf(fixture_path_))                 \
    {                                                                          \
        GTEST_SKIP() << "TinyStories fixture not found or invalid: "           \
                     << fixture_path_.string();                                \
    }                                                                          \
    hj::llama::model model_var;                                                \
    auto             err_ = model_var.load(fixture_path_);                     \
    ASSERT_FALSE(err_) << "Failed to load fixture model: " << err_.message();

} // namespace

// ============================================================================
// 1. Model Tests
// ============================================================================

TEST(llama_model, load_success_and_failure_preserves_old)
{
    LOAD_FIXTURE_OR_SKIP(m);
    ASSERT_NE(m.data(), nullptr);

    auto *old_ptr = m.data();

    auto err = m.load("/tmp/nonexistent-model-file-path.gguf");
    EXPECT_TRUE(err);

    EXPECT_EQ(m.data(), old_ptr);
}

TEST(llama_model, move_semantics)
{
    LOAD_FIXTURE_OR_SKIP(m1);
    auto *raw_ptr = m1.data();
    ASSERT_NE(raw_ptr, nullptr);

    // Move constructor
    hj::llama::model m2(std::move(m1));
    EXPECT_EQ(m2.data(), raw_ptr);
    EXPECT_EQ(m1.data(), nullptr);

    // Move assignment
    hj::llama::model m3;
    m3 = std::move(m2);
    EXPECT_EQ(m3.data(), raw_ptr);
    EXPECT_EQ(m2.data(), nullptr);
}

TEST(llama_model, reload)
{
    const auto fixture = find_llama_fixture();
    if(fixture.empty())
        GTEST_SKIP() << "Fixture missing";

    hj::llama::model m;
    ASSERT_FALSE(m.load(fixture));
    auto *first_ptr = m.data();

    ASSERT_FALSE(m.load(fixture));
    auto *second_ptr = m.data();

    EXPECT_NE(second_ptr, nullptr);
}

TEST(llama_model, raw_save)
{
    llama_backend_init();

    LOAD_FIXTURE_OR_SKIP(m);

    const auto tmp =
        std::filesystem::temp_directory_path() / "test_saved_model.gguf";

    std::remove(tmp.string().c_str());

    llama_model_save_to_file(m.data(), tmp.string().c_str());

    EXPECT_TRUE(std::filesystem::exists(tmp));

    llama_backend_free();
}

TEST(llama_model, save)
{
    LOAD_FIXTURE_OR_SKIP(m);

    std::filesystem::path tmp_save =
        std::filesystem::temp_directory_path() / "test_saved_model.gguf";
    if(std::filesystem::exists(tmp_save))
    {
        std::filesystem::remove(tmp_save);
    }

    auto err = m.save(tmp_save);
    EXPECT_FALSE(err) << "Save failed: " << err.message();
    EXPECT_TRUE(std::filesystem::exists(tmp_save));

    std::filesystem::remove(tmp_save);
}

TEST(llama_model, tokenize_variants)
{
    LOAD_FIXTURE_OR_SKIP(m);
    std::error_code err;

    // Standard tokenize
    auto tokens = m.tokenize("Hello world", true, false, err);
    EXPECT_FALSE(err);
    EXPECT_FALSE(tokens.empty());

    // Tokenize empty
    auto empty_tokens = m.tokenize("", true, false, err);
    EXPECT_FALSE(err);
    EXPECT_TRUE(empty_tokens.empty());

    // Tokenize unicode
    auto unicode_tokens = m.tokenize("你好，世界！🌍", true, false, err);
    EXPECT_FALSE(err);
    EXPECT_FALSE(unicode_tokens.empty());

    // Tokenize special tokens
    auto special_tokens = m.tokenize("<|endoftext|>", true, true, err);
    EXPECT_FALSE(err);
    EXPECT_FALSE(special_tokens.empty());
}

TEST(llama_model, metadata_and_vocab)
{
    LOAD_FIXTURE_OR_SKIP(m);

    EXPECT_GT(m.n_vocab(), 0);
    EXPECT_NE(m.get_vocab(), nullptr);

    // Metadata
    int32_t count = m.meta_count();
    EXPECT_GE(count, 0);

    if(count > 0)
    {
        char key_buf[128] = {0};
        char val_buf[128] = {0};
        EXPECT_GT(m.meta_key_by_index(0, key_buf, sizeof(key_buf)), 0);
        EXPECT_GE(m.meta_val_str(key_buf, val_buf, sizeof(val_buf)), 0);
    }

    // Chat Template
    std::string tmpl = m.chat_template(nullptr);
    (void) tmpl;
}

TEST(llama_batch, capacity_and_size)
{
    hj::llama::batch b(512, 0, 1);
    EXPECT_EQ(b.capacity(), 512);
    EXPECT_EQ(b.size(), 0);
    EXPECT_TRUE(b.empty());
}

TEST(llama_batch, set_tokens_and_overflow)
{
    hj::llama::batch                b(4, 0, 1);
    std::vector<hj::llama::token_t> tokens = {1, 2, 3};

    auto err = b.set_tokens(tokens);
    EXPECT_FALSE(err);
    EXPECT_EQ(b.size(), 3);
    EXPECT_FALSE(b.empty());

    // Overflow check
    std::vector<hj::llama::token_t> overflow_tokens = {1, 2, 3, 4, 5};
    err = b.set_tokens(overflow_tokens);
    EXPECT_EQ(err, hj::llama::error_code::buffer_overflow);
}

TEST(llama_batch, edge_cases_nullptr_zero_negative)
{
    hj::llama::batch b(16, 0, 1);

    // nullptr
    auto err = b.set_tokens(nullptr, 5, 0);
    EXPECT_EQ(err, hj::llama::error_code::invalid_argument);

    // zero tokens
    std::vector<hj::llama::token_t> empty_tokens;
    err = b.set_tokens(empty_tokens);
    EXPECT_FALSE(err);
    EXPECT_EQ(b.size(), 0);

    // negative tokens count
    err = b.set_tokens(nullptr, -5, 0);
    EXPECT_EQ(err, hj::llama::error_code::invalid_argument);

    // Exception handling for invalid initialization
    EXPECT_THROW(hj::llama::batch(-10, 0, 1), std::invalid_argument);
    EXPECT_THROW(hj::llama::batch(0, 0, 1), std::invalid_argument);
}

TEST(llama_batch, move_semantics)
{
    std::vector<hj::llama::token_t> tokens = {10, 20, 30};
    hj::llama::batch                b1(tokens);

    EXPECT_EQ(b1.size(), 3);
    EXPECT_EQ(b1.capacity(), 3);

    // Move Constructor
    hj::llama::batch b2(std::move(b1));
    EXPECT_EQ(b2.size(), 3);
    EXPECT_EQ(b2.capacity(), 3);
    EXPECT_EQ(b1.capacity(), 0);
    EXPECT_EQ(b1.data()->token, nullptr); // Safe state

    // Move Assignment
    hj::llama::batch b3(10, 0, 1);
    b3 = std::move(b2);
    EXPECT_EQ(b3.size(), 3);
    EXPECT_EQ(b2.capacity(), 0);
}

TEST(llama_batch, set_logits)
{
    std::vector<hj::llama::token_t> tokens = {1, 2, 3};
    hj::llama::batch                b(tokens);

    b.set_logits(0, true);
    b.set_logits(1, false);

    EXPECT_TRUE(b.get().logits[0]);
    EXPECT_FALSE(b.get().logits[1]);

    // Out of range set_logits should be safe (no-op)
    b.set_logits(100, true);
    b.set_logits(-1, true);
}

TEST(llama_context, model_and_context_lifecycle)
{
    LOAD_FIXTURE_OR_SKIP(m);

    auto               params = hj::llama::context::default_params();
    hj::llama::context ctx(m, params);

    EXPECT_TRUE(ctx);
    EXPECT_NE(ctx.data(), nullptr);
    EXPECT_GT(ctx.n_ctx(), 0U);
}

TEST(llama_context, invalid_model_and_failure)
{
    hj::llama::model   empty_m;
    hj::llama::context ctx;

    auto err = ctx.init(empty_m, hj::llama::context::default_params());
    EXPECT_EQ(err, hj::llama::error_code::invalid_argument);
    EXPECT_FALSE(ctx);
}

TEST(llama_context, move_semantics)
{
    LOAD_FIXTURE_OR_SKIP(m);
    hj::llama::context ctx1(m, hj::llama::context::default_params());

    auto *raw_ctx = ctx1.data();
    ASSERT_NE(raw_ctx, nullptr);

    // Move constructor
    hj::llama::context ctx2(std::move(ctx1));
    EXPECT_EQ(ctx2.data(), raw_ctx);
    EXPECT_EQ(ctx1.data(), nullptr);

    // Move assignment
    hj::llama::context ctx3;
    ctx3 = std::move(ctx2);
    EXPECT_EQ(ctx3.data(), raw_ctx);
    EXPECT_EQ(ctx2.data(), nullptr);
}

TEST(llama_context, memory_view_and_threadpool)
{
    LOAD_FIXTURE_OR_SKIP(m);
    hj::llama::context ctx(m, hj::llama::context::default_params());

    auto mem = ctx.get_memory();
    EXPECT_TRUE(mem);

    // Threadpool attach/detach (Passing nullptr is supported as default clear)
    auto err = ctx.attach_threadpool(nullptr, nullptr);
    EXPECT_FALSE(err);

    err = ctx.detach_threadpool();
    EXPECT_FALSE(err);
}

TEST(llama_context, decode_logits_and_embedding)
{
    LOAD_FIXTURE_OR_SKIP(m);
    hj::llama::context ctx(m, hj::llama::context::default_params());

    std::error_code err;
    auto            tokens = m.tokenize("Test prompt", true, false, err);
    ASSERT_FALSE(err);
    ASSERT_FALSE(tokens.empty());

    hj::llama::batch b(tokens);
    b.set_logits(b.size() - 1, true);

    err = ctx.decode(b);
    EXPECT_FALSE(err) << "Decode failed: " << err.message();

    // Check Logits
    const float *logits = ctx.get_logits_ith(b.size() - 1);
    EXPECT_NE(logits, nullptr);

    float *embd = ctx.get_embeddings();
    (void) embd;
}

TEST(llama_sampler, greedy_sampler)
{
    hj::llama::sampler smpl = hj::llama::sampler::greedy();
    EXPECT_NE(smpl.data(), nullptr);
}

TEST(llama_sampler, custom_options_chaining)
{
    LOAD_FIXTURE_OR_SKIP(m);

    hj::llama::sampler_options opts;
    opts.top_k             = 20;
    opts.top_p             = 0.8f;
    opts.min_p             = 0.05f;
    opts.temperature       = 0.7f;
    opts.penalty_last_n    = 32;
    opts.penalty_repeat    = 1.1f;
    opts.penalty_frequency = 0.1f;
    opts.penalty_present   = 0.1f;
    opts.vocab             = m.get_vocab();

    hj::llama::sampler smpl(opts);
    EXPECT_NE(smpl.data(), nullptr);
}

TEST(llama_sampler, grammar_sampler)
{
    LOAD_FIXTURE_OR_SKIP(m);

    hj::llama::sampler_options opts;
    opts.vocab        = m.get_vocab();
    opts.grammar_str  = "root ::= [a-z]+";
    opts.grammar_root = "root";

    hj::llama::sampler smpl(opts);
    EXPECT_NE(smpl.data(), nullptr);
}

TEST(llama_sampler, sample_accept_reset_workflow)
{
    LOAD_FIXTURE_OR_SKIP(m);
    hj::llama::context ctx(m, hj::llama::context::default_params());

    std::error_code err;
    auto            tokens = m.tokenize("Once upon a time", true, false, err);
    ASSERT_FALSE(err);

    hj::llama::batch b(tokens);
    b.set_logits(b.size() - 1, true);
    ASSERT_FALSE(ctx.decode(b));

    hj::llama::sampler smpl = hj::llama::sampler::greedy();

    // Sample token
    hj::llama::token_t sampled = smpl.sample(ctx, b.size() - 1, err);
    EXPECT_FALSE(err);
    EXPECT_GE(sampled, 0);

    // Accept token
    err = smpl.accept(sampled);
    EXPECT_FALSE(err);

    // Reset sampler
    smpl.reset();
}

TEST(llama_sampler, move_semantics)
{
    hj::llama::sampler smpl1    = hj::llama::sampler::greedy();
    auto              *raw_smpl = smpl1.data();
    ASSERT_NE(raw_smpl, nullptr);

    // Move constructor
    hj::llama::sampler smpl2(std::move(smpl1));
    EXPECT_EQ(smpl2.data(), raw_smpl);
    EXPECT_EQ(smpl1.data(), nullptr);

    // Move assignment
    hj::llama::sampler smpl3;
    smpl3 = std::move(smpl2);
    EXPECT_EQ(smpl3.data(), raw_smpl);
    EXPECT_EQ(smpl2.data(), nullptr);
}

namespace
{
std::filesystem::path find_lora_fixture()
{
    const std::filesystem::path source_dir =
        std::filesystem::path(__FILE__).parent_path();

    const std::filesystem::path candidates[] = {
        std::filesystem::current_path() / "test-lora.gguf",
        source_dir / "test-lora.gguf",
        source_dir.parent_path() / "tests" / "test-lora.gguf",
    };

    for(const auto &p : candidates)
    {
        if(std::filesystem::exists(p) && is_valid_gguf(p))
            return p;
    }

    return {};
}
} // namespace

TEST(llama_adapter_lora, invalid_and_null_inputs)
{
    LOAD_FIXTURE_OR_SKIP(m);

    hj::llama::model        empty_model;
    hj::llama::adapter_lora adapter1(empty_model, "/path/to/lora.gguf");
    EXPECT_EQ(adapter1.data(), nullptr);

    hj::llama::adapter_lora adapter2(m, nullptr);
    EXPECT_EQ(adapter2.data(), nullptr);

    hj::llama::adapter_lora adapter3(m, "/nonexistent/lora_adapter.gguf");
    EXPECT_EQ(adapter3.data(), nullptr);

    hj::llama::adapter_lora null_adapter;
    char                    buffer[128] = {0};
    EXPECT_EQ(null_adapter.meta_count(), -1);
    EXPECT_EQ(null_adapter.meta_val_str("general.name", buffer, sizeof(buffer)),
              -1);
    EXPECT_EQ(null_adapter.meta_key_by_index(0, buffer, sizeof(buffer)), -1);
    EXPECT_EQ(null_adapter.meta_val_str_by_index(0, buffer, sizeof(buffer)),
              0U);
    EXPECT_EQ(null_adapter.get_alora_n_invocation_tokens(), 0U);
    EXPECT_EQ(null_adapter.get_alora_invocation_tokens(), nullptr);
}

TEST(llama_adapter_lora, move_semantics)
{
    const auto lora_path = find_lora_fixture();
    if(lora_path.empty())
    {
        GTEST_SKIP() << "LoRA fixture test-lora.gguf not found, skipping valid "
                        "load test.";
    }

    LOAD_FIXTURE_OR_SKIP(m);
    hj::llama::adapter_lora lora1(m, lora_path.string().c_str());
    ASSERT_NE(lora1.data(), nullptr);

    auto *raw_ptr = lora1.data();

    // Move constructor
    hj::llama::adapter_lora lora2(std::move(lora1));
    EXPECT_EQ(lora2.data(), raw_ptr);
    EXPECT_EQ(lora1.data(), nullptr);

    // Move assignment
    hj::llama::adapter_lora lora3;
    lora3 = std::move(lora2);
    EXPECT_EQ(lora3.data(), raw_ptr);
    EXPECT_EQ(lora2.data(), nullptr);
}

TEST(llama_adapter_lora, metadata_and_alora)
{
    const auto lora_path = find_lora_fixture();
    if(lora_path.empty())
    {
        GTEST_SKIP()
            << "LoRA fixture test-lora.gguf not found, skipping metadata test.";
    }

    LOAD_FIXTURE_OR_SKIP(m);
    hj::llama::adapter_lora lora(m, lora_path.string().c_str());
    ASSERT_NE(lora.data(), nullptr);

    // Metadata Key/Val Count
    int32_t count = lora.meta_count();
    EXPECT_GE(count, 0);

    if(count > 0)
    {
        char key_buf[128] = {0};
        char val_buf[128] = {0};
        EXPECT_GT(lora.meta_key_by_index(0, key_buf, sizeof(key_buf)), 0);
        EXPECT_GE(lora.meta_val_str(key_buf, val_buf, sizeof(val_buf)), 0);
        EXPECT_GT(lora.meta_val_str_by_index(0, val_buf, sizeof(val_buf)), 0U);
    }

    // aLoRA specific checks
    uint64_t n_inv = lora.get_alora_n_invocation_tokens();
    if(n_inv > 0)
    {
        EXPECT_NE(lora.get_alora_invocation_tokens(), nullptr);
    }
}

TEST(llama_adapter_lora, bind_to_context_single_and_multiple_scales)
{
    LOAD_FIXTURE_OR_SKIP(m);
    hj::llama::context ctx(m, hj::llama::context::default_params());
    ASSERT_TRUE(ctx);

    const auto lora_path = find_lora_fixture();

    std::vector<hj::llama::adapter_lora> invalid_loras;
    invalid_loras.emplace_back();
    float invalid_scales[] = {1.0f};
    EXPECT_EQ(ctx.set_adapter_lora(invalid_loras, invalid_scales), -1);

    if(lora_path.empty())
    {
        GTEST_SKIP() << "LoRA fixture test-lora.gguf not found, skipping valid "
                        "context bind test.";
    }

    hj::llama::adapter_lora lora1(m, lora_path.string().c_str());
    ASSERT_NE(lora1.data(), nullptr);

    std::vector<hj::llama::adapter_lora> single_lora;
    single_lora.push_back(std::move(lora1));

    float scale_full = 1.0f;
    EXPECT_EQ(ctx.set_adapter_lora(single_lora, &scale_full), 0);

    float scale_half = 0.5f;
    EXPECT_EQ(ctx.set_adapter_lora(single_lora, &scale_half), 0);

    float scale_zero = 0.0f;
    EXPECT_EQ(ctx.set_adapter_lora(single_lora, &scale_zero), 0);

    hj::llama::adapter_lora lora_a(m, lora_path.string().c_str());
    hj::llama::adapter_lora lora_b(m, lora_path.string().c_str());

    std::vector<hj::llama::adapter_lora> multi_loras;
    multi_loras.push_back(std::move(lora_a));
    multi_loras.push_back(std::move(lora_b));

    float multi_scales[] = {0.7f, 0.3f};
    EXPECT_EQ(ctx.set_adapter_lora(multi_loras, multi_scales), 0);

    std::vector<hj::llama::adapter_lora> empty_loras;
    EXPECT_EQ(ctx.set_adapter_lora(empty_loras, nullptr), 0);
}

TEST(llama_backend, isolated_model_lifecycle)
{
    const auto fixture = find_llama_fixture();
    if(fixture.empty())
        GTEST_SKIP() << "Fixture missing";

    {
        hj::llama::model m;
        auto             err = m.load(fixture);
        EXPECT_FALSE(err);
        EXPECT_NE(m.data(), nullptr);
    }

    {
        hj::llama::model m2(fixture);
        EXPECT_NE(m2.data(), nullptr);
    }
}

TEST(llama_backend, model_load_without_manual_backend)
{
    hj::llama::model m;

    auto err = m.load("/tmp/nonexistent_model_file_for_backend_test.gguf");

    EXPECT_TRUE(err);
    EXPECT_EQ(err, hj::llama::error_code::file_not_found);

    EXPECT_EQ(m.data(), nullptr);
}

TEST(llama_backend, multiple_models_refcount_and_destruction)
{
    const auto fixture = find_llama_fixture();
    if(fixture.empty())
        GTEST_SKIP() << "Fixture missing";

    hj::llama::model m1;
    hj::llama::model m2;

    ASSERT_FALSE(m1.load(fixture));
    ASSERT_FALSE(m2.load(fixture));

    ASSERT_NE(m1.data(), nullptr);
    ASSERT_NE(m2.data(), nullptr);

    m1 = hj::llama::model{};
    EXPECT_EQ(m1.data(), nullptr);

    EXPECT_NE(m2.data(), nullptr);
    EXPECT_GT(m2.n_vocab(), 0);
    EXPECT_GT(m2.n_params(), 0U);

    std::error_code err;
    auto            tokens = m2.tokenize("Hello test", true, false, err);
    EXPECT_FALSE(err);
    EXPECT_FALSE(tokens.empty());
}

TEST(llama_lifecycle, context_outlives_or_invalidates_model)
{
    const auto fixture = find_llama_fixture();
    if(fixture.empty())
        GTEST_SKIP() << "Fixture missing";

    hj::llama::model m;
    ASSERT_FALSE(m.load(fixture));

    auto               params = hj::llama::context::default_params();
    hj::llama::context ctx(m, params);
    ASSERT_TRUE(ctx);

    m = hj::llama::model{};
    EXPECT_EQ(m.data(), nullptr);

    EXPECT_NO_THROW({ ctx.reset(); });
    EXPECT_FALSE(ctx);
}

TEST(llama_backend, thread_safety_and_free_race)
{
    const auto fixture = find_llama_fixture();
    if(fixture.empty())
        GTEST_SKIP() << "Fixture missing";

    constexpr int num_iterations = 2;

    for(int i = 0; i < num_iterations; ++i)
    {
        std::promise<void>       go;
        std::shared_future<void> ready(go.get_future());

        auto thread_a = std::async(std::launch::async, [ready, &fixture]() {
            ready.wait();
            hj::llama::model m;
            (void) m.load(fixture);
        });

        auto thread_b = std::async(std::launch::async, [ready, &fixture]() {
            ready.wait();
            hj::llama::model m(fixture);
            m = hj::llama::model{};
        });

        auto thread_c = std::async(std::launch::async, [ready, &fixture]() {
            ready.wait();
            hj::llama::backend_guard b;
            hj::llama::model         m(fixture);
            if(m.data())
            {
                hj::llama::context ctx(m);
                (void) ctx.n_ctx();
            }
        });

        go.set_value();

        thread_a.get();
        thread_b.get();
        thread_c.get();
    }
}

TEST(llama_model, save_failure_cases)
{
    LOAD_FIXTURE_OR_SKIP(m);

    {
        std::filesystem::path bad_path = std::filesystem::temp_directory_path()
                                         / "non_existent_dir_12345"
                                         / "model.gguf";

        auto err = m.save(bad_path);
        EXPECT_TRUE(err)
            << "Saving to non-existent directory should return error";
        EXPECT_EQ(err, hj::llama::error_code::file_not_found);
    }

    {
        std::filesystem::path dir_path =
            std::filesystem::temp_directory_path() / "test_dir_as_file";
        std::filesystem::create_directories(dir_path);

        auto err = m.save(dir_path);
        EXPECT_TRUE(err) << "Saving to directory path should fail";

        std::filesystem::remove(dir_path);
    }

    {
        std::filesystem::path unreadable_dir =
            std::filesystem::temp_directory_path() / "no_perm_dir";
        std::filesystem::create_directories(unreadable_dir);

        make_read_only(unreadable_dir);

        std::filesystem::path target_file = unreadable_dir / "target.gguf";
        auto                  err         = m.save(target_file);

        EXPECT_TRUE(err);

        make_writable(unreadable_dir);
        std::filesystem::remove_all(unreadable_dir);
    }

    {
        std::filesystem::path overwrite_file =
            std::filesystem::temp_directory_path() / "overwrite_target.gguf";

        {
            std::ofstream ofs(overwrite_file);
            ofs << "dummy data";
        }
        ASSERT_TRUE(std::filesystem::exists(overwrite_file));

        auto err = m.save(overwrite_file);
        EXPECT_FALSE(err) << "Overwriting existing file should succeed: "
                          << err.message();
        EXPECT_TRUE(std::filesystem::exists(overwrite_file));

        std::filesystem::remove(overwrite_file);
    }
}