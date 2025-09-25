#include <hj/log/log.hpp>
#include <hj/os/env.h>
#include <hj/os/options.hpp>
#include <hj/os/signal.hpp>
#include <hj/util/license.hpp>
#include <hj/encoding/i18n.hpp>
#include <hj/testing/crash.hpp>
#include <hj/testing/telemetry.hpp>

#ifndef I18N_LOCALE
    #define I18N_LOCALE "en_US"
#endif

// add your code here...
#include <hj/testing/error.hpp>
#include <hj/testing/error_handler.hpp>

#include "err.h"
#include "core.h"
#include "comm.h"

int main(int argc, char* argv[])
{
    // add options parse support
    hj::options opts;

    // add crash handle support
    hj::crash_handler::instance()->prevent_set_unhandled_exception_filter();
    hj::crash_handler::instance()->set_local_path("./");

    // add log support
    hj::logger::instance()->set_level(hj::log_lvl::debug);

    // add i18n support
    hj::i18n::instance().set_locale(I18N_LOCALE);
    hj::i18n::instance().load_translation_auto("./", "framework");

    // add telemetry support
    auto tracer = hj::telemetry::make_otlp_file_tracer("otlp_call", "./telemetry.json");

    // add signals handle support
    hj::sigcatch({SIGABRT, SIGTERM}, [](int sig){});

    // // add license check support
    // hj::license::verifier vef{"tourist", hj::license::sign_algo::none, {}};
    // auto err = vef.verify_file("./license.lic", "tourist", 30, {{"sn", hj::license::get_disk_sn()}});
    // if (err)
    // {
    //     LOG_ERROR("license verify failed with err: {}, please check your license file: {}", err.message(), "./license.lic");
    //     return -1;
    // }

    // add your code here...
    hj::error_handler<err_t> h{[](const char* src, const char* dst){
        LOG_DEBUG("error handler state transition: {} -> {}", src, dst);
    }};
    if (argc < 2) 
    {
        h.match(error(ERR_ARGC_TOO_LESS), [&](const err_t& e){
            LOG_ERROR(tr("app.err_argc_too_less").c_str(), argc);
        });
        return 1;
    }

    lic_core lic_sdk;
    // version check
    int major, minor, patch;
    auto err = lic_sdk.version(major, minor, patch);
    if (!err && (major < LIC_CORE_VERSION_MAJOR_MIN 
            || minor < LIC_CORE_VERSION_MINOR_MIN 
            || patch < LIC_CORE_VERSION_PATCH_MIN))
        err = error(ERR_LIC_CORE_VERSION_MISMATCH);
    if (err) 
    {
        h.match(err, [&](const err_t& e){
            switch (e.value())
            {
                case ERR_LIC_CORE_LOAD_FAIL:
                {
                    LOG_ERROR(tr("app.err_lic_core_load_fail").c_str());
                    break;
                }
                case ERR_LIC_CORE_VERSION_MISMATCH:
                {
                    LOG_ERROR(tr("app.err_lic_core_version_mismatch").c_str(), major, minor, patch, 
                        LIC_CORE_VERSION_MAJOR_MIN, LIC_CORE_VERSION_MINOR_MIN, LIC_CORE_VERSION_PATCH_MIN);
                    break;
                }
                default:
                {
                    LOG_ERROR(tr("app.err_unknow").c_str(), e.value());
                    break;
                }
            }
        });
        return 1;
    }

    // init
    err = lic_sdk.init();    
    if (err) 
    {
        h.match(err, [&](const err_t& e){
            switch (e.value())
            {
                 case ERR_LIC_CORE_LOAD_FAIL:
                {
                    LOG_ERROR(tr("app.err_lic_core_load_fail").c_str());
                    break;
                }
                case LIC_ERR_INIT_FAIL:
                {
                    LOG_ERROR(tr("lic_core.err_init_fail").c_str());
                    break;
                }
                default:
                {
                    LOG_ERROR(tr("app.err_unknow").c_str(), e.value());
                    break;
                }
            }
        });
        return 1;
    }

    // parse options
    opts.add<std::string>("subcmd", std::string(""), "subcommand");
    opts.add_positional("subcmd", 1);
    opts.add<std::string>("input,i", std::string(""), "input lic file path");
    opts.add<int>("time,t", 30, "not less than this days");
    opts.add<std::string>("content", "", "input content");
    opts.add_positional("content", 2);

    std::string subcmd = opts.parse<std::string>(argc, argv, "subcmd");
    auto input = opts.parse<std::string>(argc, argv, "input");
    auto time = opts.parse<int>(static_cast<int>(argc), argv, "time");
    auto content = opts.parse<std::string>(argc, argv, "content");
    if (subcmd == "issue") 
    {
        // lic issue --time <days> xxx
        // lic issue -t <days> xxx
        LOG_DEBUG("time:{}", time);
    }
    else if (subcmd == "verify") 
    {
        // lic verify --input <file> --time <days> xxx
        // lic verify -i <file> -t <days> xxx
        LOG_DEBUG("content:{}", content);
    }
    else if (subcmd == "help") 
    {
        // lic help
    } 
    else
    {
         h.match(error(ERR_INVALID_SUBCMD), [&](const err_t& e){
            LOG_ERROR(tr("app.err_invalid_subcmd").c_str(), subcmd, fmt_strs(all_subcmds));
        });
        return 1;
    }
}