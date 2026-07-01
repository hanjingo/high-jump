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
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <hj/ai/asr.hpp>

#include "err.h"

static const std::vector<std::string> all_subcmds{"translate"};

std::string fmt_strs(const std::vector<std::string> &vec)
{
    std::string s;
    for(const auto &e : vec)
        s += e + ", ";

    s = s.substr(0, s.size() - 2); // remove trailing comma and space
    return s;
}

bool read_wav_pcm(const std::string &filename, std::vector<float> &pcmf32)
{
    std::ifstream file(filename, std::ios::binary);
    if(!file.is_open())
        return false;

    file.seekg(44, std::ios::beg);

    int16_t sample;
    while(file.read(reinterpret_cast<char *>(&sample), sizeof(sample)))
    {
        pcmf32.push_back(((float) sample) / 32768.0f);
    }
    return true;
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

    // add your code here...
    hj::error_handler<err_t> h{[](const char *src, const char *dst) {
        LOG_DEBUG("error handler state transition: {} -> {}", src, dst);
    }};
    if(argc < 2)
    {
        h.match(error(ERR_ARGC_TOO_LESS), [&](const err_t &e) {
            LOG_ERROR("Error: too few arguments", argc);
        });
        return 1;
    }

    opts.add<std::string>("subcmd", std::string(""), "subcommand");
    opts.add_positional("subcmd", 1);

    opts.add<std::string>("model,m", std::string(""), "model file path");
    opts.add<std::string>("file,f", std::string(""), "audio file path");

    std::string subcmd     = opts.parse<std::string>(argc, argv, "subcmd");
    auto        model_path = opts.parse<std::string>(argc, argv, "model");
    auto        file_path  = opts.parse<std::string>(argc, argv, "file");
    if(subcmd == "translate")
    {
        // // init ctx
        // auto ctx = whisper_init_from_file(model_path.c_str());
        // if(ctx == nullptr)
        // {
        //     LOG_ERROR("Failed to initialize Whisper context.");
        //     return -1;
        // }

        // // load pcm
        // std::vector<float> pcmf32;
        // if(!read_wav_pcm(file_path, pcmf32))
        // {
        //     LOG_ERROR("Failed to read WAV file: {}", file_path);
        //     whisper_free(ctx);
        //     return 1;
        // }

        // // set params
        // whisper_full_params wparams =
        //     whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
        // wparams.print_progress   = false;
        // wparams.print_special    = false;
        // wparams.print_realtime   = false;
        // wparams.print_timestamps = true;
        // wparams.language         = "auto";

        // // parse
        // if(whisper_full(ctx, wparams, pcmf32.data(), pcmf32.size()) != 0)
        // {
        //     LOG_ERROR("Failed to process audio file.");
        //     whisper_free(ctx);
        //     return -1;
        // }

        // // print segments
        // auto n_segments = whisper_full_n_segments(ctx);
        // for(int i = 0; i < n_segments; ++i)
        // {
        //     const char *text = whisper_full_get_segment_text(ctx, i);
        //     LOG_INFO("Segment {}: {}", i, text);
        // }

        // whisper_free(ctx);


        // load pcm
        std::vector<float> pcmf32;
        if(!read_wav_pcm(file_path, pcmf32))
        {
            LOG_ERROR("Failed to read WAV file: {}", file_path);
            return 1;
        }

        auto             params = hj::asr::context::default_params();
        hj::asr::context ctx(model_path, params);

        auto wparams = hj::asr::context::default_full_params(
            hj::asr::context::sampling_strategy::greedy);
        wparams.print_progress   = false;
        wparams.print_special    = false;
        wparams.print_realtime   = false;
        wparams.print_timestamps = true;
        wparams.language         = "auto";
        ctx.full(wparams, pcmf32.data(), pcmf32.size());

        auto n_segments = ctx.n_segments();
        for(int i = 0; i < n_segments; ++i)
        {
            std::string text;
            ctx.get_segment_text(text, i);
            LOG_INFO("Segment {}: {}", i, text);
        }
    } else
    {
        h.match(error(ERR_INVALID_SUBCMD), [&](const err_t &e) {
            LOG_ERROR("Error: unknown subcommand: {}, we expected one of these "
                      "subcommands: [{}]",
                      subcmd,
                      fmt_strs(all_subcmds));
        });
        return 1;
    }
}