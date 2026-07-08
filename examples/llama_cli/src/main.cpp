#include <hj/log/log.hpp>
#include <hj/os/env.h>
#include <hj/os/options.hpp>
#include <hj/os/signal.hpp>

#include <iostream>
#if CRASH_HANDLER_ENABLE == 1
#include <hj/testing/crash.hpp>
#endif

#if TELEMETRY_ENABLE == 1
#include <hj/testing/telemetry.hpp>
#endif

#if LIC_ENABLE == 1
#include <hj/util/license.hpp>
#endif

// add your code here...
// #include <llama.h>

#include <hj/ai/llama.hpp>

void cleanup(llama_model *model, llama_context *ctx, llama_sampler *smpl)
{
    if(smpl)
        llama_sampler_free(smpl); // Correct function name

    if(ctx)
        llama_free(ctx);

    if(model)
        llama_free_model(model);

    llama_backend_free();
}

int main(int argc, char *argv[])
{
#if CRASH_HANDLER_ENABLE == 1
// add crash handle support
#pragma message("crash handler enabled, initializing crash handler...")
    hj::crash_handler::instance()->prevent_set_unhandled_exception_filter();
    hj::crash_handler::instance()->set_local_path("./");
#endif

#if TELEMETRY_ENABLE == 1
// add telemetry support
#pragma message("telemetry enabled, initializing tracer...")
    auto tracer =
        hj::telemetry::make_otlp_file_tracer("otlp_call", "./telemetry.json");
#endif

#if LIC_ENABLE == 1
// add license check support
#pragma message("license check enabled, verifying license...")
    hj::license::verifier vef{LIC_ISSUER, hj::license::sign_algo::none, {}};
    auto                  verify_err = vef.verify_file(LIC_FPATH, PACKAGE, 1);
    if(verify_err)
    {
        std::cerr << "license verify failed with err: " << verify_err.message()
                  << ", please check your license file: " << LIC_FPATH
                  << std::endl;
        return -1;
    }
#endif

    // add your code here...
    //
    // add options parse support
    hj::options opts;

    // add log support
#ifdef DEBUG
    hj::log::logger::instance()->set_level(hj::log::level::debug);
#else
    hj::log::logger::instance()->set_level(hj::log::level::info);
#endif

    // add signals handle support
    hj::sighandler::instance().sigcatch({SIGABRT, SIGTERM}, [](int sig) {});

    // // 1. Initialize llama backend (required once at startup)
    // llama_backend_init();

    // // 2. Set model parameters
    // llama_model_params model_params = llama_model_default_params();
    // model_params.n_gpu_layers       = 99; // Offload all layers to GPU

    // // 3. Load the GGUF model
    // const std::string model_path = "./TinyStories-656K-Q3_K_M.gguf";
    // llama_model      *model =
    //     llama_load_model_from_file(model_path.c_str(), model_params);
    // if(model == nullptr)
    // {
    //     std::cerr << "Error: Failed to load model." << std::endl;
    //     llama_backend_free();
    //     return 1;
    // }

    // // 4. Create context
    // llama_context_params ctx_params = llama_context_default_params();
    // ctx_params.n_ctx                = 2048; // Context window size
    // ctx_params.n_batch              = 512;  // Batch size for processing
    // llama_context *ctx = llama_new_context_with_model(model, ctx_params);
    // if(ctx == nullptr)
    // {
    //     std::cerr << "Error: Failed to create llama context." << std::endl;
    //     llama_free_model(model);
    //     llama_backend_free();
    //     return 1;
    // }

    // // 5. Setup sampler chain CORRECTLY
    // auto           sparams = llama_sampler_chain_default_params();
    // llama_sampler *smpl    = llama_sampler_chain_init(sparams);
    // llama_sampler_chain_add(smpl, llama_sampler_init_top_k(40));
    // llama_sampler_chain_add(smpl, llama_sampler_init_top_p(0.95, 1));
    // llama_sampler_chain_add(smpl, llama_sampler_init_temp(0.7f));
    // llama_sampler_chain_add(smpl, llama_sampler_init_dist(42));

    // // 6. Tokenize the input prompt
    // std::string        prompt   = "User: Tell me a joke.\nAI:";
    // const llama_vocab *vocab    = llama_model_get_vocab(model);
    // auto               n_tokens = llama_tokenize(vocab,
    //                                              prompt.c_str(),
    //                                              prompt.length(),
    //                                              nullptr,
    //                                              0,
    //                                              true,
    //                                              true);
    // n_tokens                    = std::abs(n_tokens);
    // if(n_tokens <= 0)
    // {
    //     std::cerr << "Error: Tokenization failed." << std::endl;
    //     cleanup(model, ctx, smpl);
    //     return 1;
    // }
    // std::vector<llama_token> tokens(n_tokens);
    // n_tokens = llama_tokenize(vocab,
    //                           prompt.c_str(),
    //                           prompt.length(),
    //                           tokens.data(),
    //                           tokens.size(),
    //                           true,
    //                           true);
    // n_tokens = std::abs(n_tokens);
    // tokens.resize(n_tokens);

    // // 7. Clear KV cache and evaluate prompt
    // // llama_kv_cache_clear(ctx);
    // llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    // if(llama_decode(ctx, batch) != 0)
    // {
    //     std::cerr << "Error: Failed to decode prompt." << std::endl;
    //     cleanup(model, ctx, smpl);
    //     return 1;
    // }

    // // 8. Reset sampler state (clears token history)
    // llama_sampler_reset(smpl);

    // // 9. Text Generation Loop
    // int max_tokens         = 100;
    // int n_tokens_generated = 0;
    // for(int i = 0; i < max_tokens; ++i)
    // {
    //     llama_token next_token = llama_sampler_sample(smpl, ctx, -1);
    //     if(llama_token_is_eog(vocab, next_token))
    //         break;

    //     char buf[128];
    //     int  n_chars =
    //         llama_token_to_piece(vocab, next_token, buf, sizeof(buf), 0, false);
    //     if(n_chars > 0)
    //     {
    //         std::cout << std::string(buf, n_chars) << std::flush;
    //     }

    //     llama_sampler_accept(smpl, next_token);
    //     batch = llama_batch_get_one(&next_token, 1);
    //     if(llama_decode(ctx, batch) != 0)
    //     {
    //         std::cerr << "\n❌ Error: Failed to decode token." << std::endl;
    //         break;
    //     }

    //     n_tokens_generated++;
    // }

    // std::cout << "\n\n📊 Generated " << n_tokens_generated << " tokens"
    //           << std::endl;
    // cleanup(model, ctx, smpl);


    // ------------------------- c++ ---------------------------
    // Initialize llama backend (required once at startup)
    hj::llama::backend_init();

    // Set model parameters & Load the GGUF model
    hj::llama::model m{"./TinyStories-656K-Q3_K_M.gguf"};
    if(m.data() == nullptr)
    {
        std::cerr << "Error: Direct path to model failed to load." << std::endl;
        hj::llama::backend_free();
        return 1;
    }

    // Create context for inference
    auto ctx_params    = hj::llama::context::default_params();
    ctx_params.n_ctx   = 2048;
    ctx_params.n_batch = 512;
    hj::llama::context ctx{m, ctx_params};
    if(ctx.data() == nullptr)
    {
        std::cerr << "Error: Failed to create llama context." << std::endl;
        hj::llama::backend_free();
        return 1;
    }

    // Sampler
    hj::llama::sampler sampler{};
    sampler.init_top_k(40);
    sampler.init_top_p(0.95, 1);
    sampler.init_temp(0.7f);
    sampler.init_dist(42);

    // Tokenize the input prompt
    std::string prompt = "User: Tell me a joke.\nAI:";
    auto        tokens = m.tokenize(prompt, true, true);
    if(tokens.empty())
    {
        std::cerr << "Error: Tokenization failed." << std::endl;
        hj::llama::backend_free();
        return 1;
    }

    // Clear KV cache and evaluate prompt
    hj::llama::batch pre_batch{tokens};
    pre_batch.set_logits(static_cast<int32_t>(tokens.size() - 1),
                         true); // Enable logits for the last token
    if(ctx.decode(pre_batch) != 0)
    {
        std::cerr << "Error: Failed to decode prompt." << std::endl;
        hj::llama::backend_free();
        return 1;
    }

    // Reset sampler state
    sampler.reset();

    // Text Generation Loop
    hj::llama::batch loop_batch{1, 0, 1};
    int              max_tokens = 100;
    int              n_past     = static_cast<int>(tokens.size());
    for(int i = 0; i < max_tokens; ++i)
    {
        // Sample the next token
        int32_t sample_idx =
            (n_past == static_cast<int32_t>(tokens.size())) ? (n_past - 1) : 0;
        auto next_token = sampler.sample(ctx, sample_idx);
        if(m.token_is_eog(next_token))
        {
            std::cout << "\n⏹️  EOG token encountered" << std::endl;
            break;
        }

        // Convert token to string and print
        char buf[128];
        int  n_chars = m.token_to_piece(next_token, buf, sizeof(buf), 0, false);
        if(n_chars > 0)
        {
            std::string piece(buf, n_chars);
            std::cout << piece << std::flush;
        }

        // Accept the token (updates sampler state)
        sampler.accept(next_token);

        // Prepare the next token for the next iteration
        loop_batch.set_tokens(&next_token, 1, n_past);
        loop_batch.set_logits(0, true);
        if(ctx.decode(loop_batch) != 0)
        {
            std::cerr << "\n❌ Error: Failed to decode token." << std::endl;
            break;
        }

        n_past++;
    }

    std::cout << std::endl;

    hj::llama::backend_free();
    return 0;
}