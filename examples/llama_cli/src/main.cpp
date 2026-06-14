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
    // // model_params.n_gpu_layers = 99; // Uncomment to offload all layers to GPU

    // // 3. Load the GGUF model
    // const std::string model_path = "./TinyStories-656K-Q3_K_M.gguf";
    // llama_model      *model =
    //     llama_load_model_from_file(model_path.c_str(), model_params);

    // if(model == nullptr)
    // {
    //     std::cerr << "Error: Direct path to model failed to load." << std::endl;
    //     llama_backend_free();
    //     return 1;
    // }

    // // 4. Create a context for inference
    // llama_context_params ctx_params = llama_context_default_params();
    // ctx_params.n_ctx                = 2048; // Context window size
    // llama_context *ctx = llama_new_context_with_model(model, ctx_params);

    // if(ctx == nullptr)
    // {
    //     std::cerr << "Error: Failed to create llama context." << std::endl;
    //     llama_free_model(model);
    //     llama_backend_free();
    //     return 1;
    // }

    // // 5. Tokenize the input prompt
    // std::string              prompt = "User: Tell me a joke.\nAI:";
    // std::vector<llama_token> tokens(prompt.size() + 4);
    // const llama_vocab       *vocab = llama_model_get_vocab(model);

    // // Convert string to tokens
    // int n_tokens = llama_tokenize(vocab,
    //                               prompt.c_str(),
    //                               prompt.length(),
    //                               tokens.data(),
    //                               tokens.size(),
    //                               true,
    //                               true);
    // tokens.resize(n_tokens);

    // std::cout << "Prompt tokenized successfully. Starting generation...\n\n"
    //           << prompt;

    // // 6. Create a batch for evaluation
    // // llama_batch_get_one is a helper for a simple, single-sequence evaluation
    // llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());

    // // 7. Text Generation Loop
    // int max_tokens = 100;
    // for(int i = 0; i < max_tokens; ++i)
    // {
    //     // Evaluate the batch
    //     if(llama_decode(ctx, batch) != 0)
    //     {
    //         std::cerr << "Error: Failed to decode tokens." << std::endl;
    //         break;
    //     }

    //     // Sample the next token (Greedy sampling used here for simplicity)
    //     auto        logits     = llama_get_logits_ith(ctx, batch.n_tokens - 1);
    //     llama_token next_token = 0;
    //     float       max_logit  = -1e9;

    //     // Simple argmax sampling over vocabulary
    //     int n_vocab = llama_n_vocab(vocab);
    //     for(int v = 0; v < n_vocab; ++v)
    //     {
    //         if(logits[v] > max_logit)
    //         {
    //             max_logit  = logits[v];
    //             next_token = v;
    //         }
    //     }

    //     // Check for End of Generation (EOG/EOS) tokens
    //     if(llama_token_is_eog(vocab, next_token))
    //     {
    //         break;
    //     }

    //     // Convert token to piece/string and print it
    //     char buf[128];
    //     int  n_chars =
    //         llama_token_to_piece(vocab, next_token, buf, sizeof(buf), 0, false);
    //     if(n_chars > 0)
    //     {
    //         std::string piece(buf, n_chars);
    //         std::cout << piece << std::flush;
    //     }

    //     // Prepare the next token for the next iteration
    //     batch = llama_batch_get_one(&next_token, 1);
    // }

    // std::cout << std::endl;

    // // 8. Cleanup
    // llama_free(ctx);         // Clean up the inference context
    // llama_model_free(model); // Clean up the model weights
    // llama_backend_free();    // Free the global backend

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
    auto ctx_params  = hj::llama::context::default_params();
    ctx_params.n_ctx = 2048; // Context window size
    hj::llama::context ctx{m, ctx_params};
    if(ctx.data() == nullptr)
    {
        std::cerr << "Error: Failed to create llama context." << std::endl;
        hj::llama::backend_free();
        return 1;
    }

    // Tokenize the input prompt
    std::string prompt = "User: Tell me a joke.\nAI:";
    auto        tokens = m.tokenize(prompt, true, true);
    std::cout << "Prompt tokenized successfully. Starting generation...\n\n"
              << prompt;

    // Create a batch for evaluation
    auto batch = hj::llama::batch_get_one(tokens);

    // Text Generation Loop
    int max_tokens = 100;
    for(int i = 0; i < max_tokens; ++i)
    {
        // Evaluate the batch
        if(ctx.decode(batch) != 0)
        {
            std::cerr << "Error: Failed to decode tokens." << std::endl;
            break;
        }

        // Sample the next token (Greedy sampling used here for simplicity)
        auto               logits     = ctx.get_logits_ith(batch.n_tokens - 1);
        hj::llama::token_t next_token = 0;
        float              max_logit  = -1e9;

        // Simple argmax sampling over vocabulary
        int n_vocab = m.n_vocab();
        for(int v = 0; v < n_vocab; ++v)
        {
            if(logits[v] > max_logit)
            {
                max_logit  = logits[v];
                next_token = v;
            }
        }

        // Check for End of Generation (EOG/EOS) tokens
        if(m.token_is_eog(next_token))
            break;

        // Convert token to piece/string and print it
        char buf[128];
        int  n_chars = m.token_to_piece(next_token, buf, sizeof(buf), 0, false);

        if(n_chars > 0)
        {
            std::string piece(buf, n_chars);
            std::cout << piece << std::flush;
        }

        // Prepare the next token for the next iteration
        batch = llama_batch_get_one(&next_token, 1);
    }

    std::cout << std::endl;
}