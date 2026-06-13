
#include <gtest/gtest.h>
#include <hj/ai/llama.hpp>

#include <cstdio>

TEST(llama, backend_support_queries_are_callable)
{
    hj::llama::backend_init();

    const bool mmap_support  = hj::llama::supports_mmap();
    const bool mlock_support = hj::llama::supports_mlock();
    const bool gpu_support   = hj::llama::supports_gpu_offload();
    const bool rpc_support   = hj::llama::supports_rpc();

    (void) mmap_support;
    (void) mlock_support;
    (void) gpu_support;
    (void) rpc_support;

    hj::llama::backend_free();
    SUCCEED();
}

TEST(llama, model_default_state)
{
    hj::llama::model m;

    EXPECT_EQ(m.data(), nullptr);
    EXPECT_EQ(m.size(), 0U);
    EXPECT_TRUE(m.desc().empty());

    EXPECT_EQ(m.n_ctx_train(), 0);
    EXPECT_EQ(m.n_embd(), 0);
    EXPECT_EQ(m.n_embd_inp(), 0);
    EXPECT_EQ(m.n_embd_out(), 0);
    EXPECT_EQ(m.n_layer(), 0);
    EXPECT_EQ(m.n_head(), 0);
    EXPECT_EQ(m.n_head_kv(), 0);
    EXPECT_EQ(m.n_swa(), 0);

    EXPECT_EQ(m.get_vocab(), nullptr);
    EXPECT_EQ(m.rope_type(), LLAMA_ROPE_TYPE_NONE);
    EXPECT_FLOAT_EQ(m.rope_freq_scale_train(), 0.0f);

    EXPECT_EQ(m.n_cls_out(), 0U);
    EXPECT_EQ(m.cls_label(0), nullptr);

    char buf[16] = {0};
    EXPECT_EQ(m.meta_val_str("k", buf, sizeof(buf)), -1);
    EXPECT_EQ(m.meta_count(), -1);
    EXPECT_EQ(m.meta_key_by_index(0, buf, sizeof(buf)), -1);

    EXPECT_TRUE(m.chat_template("default").empty());
    EXPECT_EQ(m.n_params(), 0U);
    EXPECT_FALSE(m.has_encoder());
    EXPECT_FALSE(m.has_decoder());
    EXPECT_EQ(m.decoder_start_token(), static_cast<hj::llama::token_t>(-1));
    EXPECT_FALSE(m.is_recurrent());
    EXPECT_FALSE(m.is_hybrid());
    EXPECT_FALSE(m.is_diffusion());

    EXPECT_FALSE(m.save("/tmp/should-not-exist.gguf"));
    EXPECT_FALSE(m.free());
}

TEST(llama, model_load)
{
    hj::llama::model m;
    EXPECT_FALSE(m.load("/tmp/nonexistent-llama-model.gguf"));

    FILE *fp = tmpfile();
    ASSERT_NE(fp, nullptr);
    EXPECT_FALSE(m.load(fp));
    fclose(fp);

    EXPECT_TRUE(m.load("./bge-small-en-v1.5-q8_0.gguf"));
    EXPECT_NE(m.data(), nullptr);
}

TEST(llama, memory_default_state)
{
    hj::llama::memory mem;
    EXPECT_FALSE(mem.can_shift());

    mem.clear(true);
    mem.seq_rm(0, 0, 0);
    mem.seq_cp(0, 1, 0, 0);
    mem.seq_keep(0);
    mem.seq_add(0, 0, 0, 0);
    mem.seq_div(0, 0, 0, 1);
    mem.seq_pos_min(0);
    mem.seq_pos_max(0);
}

TEST(llama, adapter_lora_default_state)
{
    hj::llama::adapter_lora adapter;
    EXPECT_EQ(adapter.data(), nullptr);

    char buf[16] = {0};
    EXPECT_EQ(adapter.meta_val_str("k", buf, sizeof(buf)), -1);
    EXPECT_EQ(adapter.meta_cout(), -1);
    EXPECT_EQ(adapter.meta_key_by_index(0, buf, sizeof(buf)), -1);
    EXPECT_EQ(adapter.meta_val_str_by_index(0, buf, sizeof(buf)), 0U);
    EXPECT_EQ(adapter.get_alora_n_invocation_tokens(), 0U);
    EXPECT_EQ(adapter.get_alora_invocation_tokens(), nullptr);
}

TEST(llama, context_default_state)
{
    hj::llama::context ctx;

    EXPECT_EQ(ctx.get_model(), nullptr);
    EXPECT_EQ(ctx.get_memory(), nullptr);

    EXPECT_FALSE(ctx.attach_threadpool(nullptr, nullptr));
    EXPECT_FALSE(ctx.detach_threadpool());

    EXPECT_EQ(ctx.n_ctx(), 0U);
    EXPECT_EQ(ctx.n_ctx_seq(), 0U);
    EXPECT_EQ(ctx.n_batch(), 0U);
    EXPECT_EQ(ctx.n_ubatch(), 0U);
    EXPECT_EQ(ctx.n_seq_max(), 0U);
    EXPECT_EQ(ctx.n_rs_seq(), 0U);

    std::vector<hj::llama::adapter_lora> loras;
    EXPECT_EQ(ctx.set_adapter_lora(loras, nullptr), -1);

    float dummy_data[1] = {0.0f};
    EXPECT_EQ(ctx.set_adapter_cvec(dummy_data, 1, 1, 0, 1), -1);
}
