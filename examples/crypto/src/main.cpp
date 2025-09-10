#include <libcpp/log/log.hpp>
#include <libcpp/os/env.h>
#include <libcpp/os/options.hpp>
#include <libcpp/os/signal.hpp>
#include <libcpp/util/license.hpp>
#include <libcpp/encoding/i18n.hpp>
#include <libcpp/testing/crash.hpp>
#include <libcpp/testing/telemetry.hpp>

// add your code here...
#include <iostream>
#include "encrypt.h"

int main(int argc, char* argv[])
{
    // add options parse support
    libcpp::options opts;
    opts.add<std::string>("user", std::string("tourist"));
    opts.add<std::string>("crash_path", std::string("./"));
    opts.add<std::string>("lic_issuer", std::string("defauult"));
    opts.add<std::string>("lic_fpath", std::string("./license.lic"));
    opts.add<std::string>("telemetry_fpath", std::string("./telemetry.json"));
    auto user = opts.parse<std::string>(argc, argv, "user");
    auto crash_path = opts.parse<std::string>(argc, argv, "crash_path");
    auto lic_issuer = opts.parse<std::string>(argc, argv, "lic_issuer");
    auto lic_fpath = opts.parse<std::string>(argc, argv, "lic_fpath");
    auto telemetry_fpath = opts.parse<std::string>(argc, argv, "telemetry_fpath");

    // add crash handle support
    libcpp::crash_handler::instance()->prevent_set_unhandled_exception_filter();
    libcpp::crash_handler::instance()->set_local_path(crash_path);

    // add log support
    libcpp::logger::instance()->set_level(libcpp::log_lvl::log_lvl_debug);

    // add i18n support
    libcpp::i18n::instance().set_locale(I18N_LOCALE);
    libcpp::i18n::instance().load_translation_auto("./", "crypto");

    // add telemetry support
    auto tracer = libcpp::telemetry::make_otlp_file_tracer("otlp_call", telemetry_fpath);

    // add signals handle support
    libcpp::sigcatch({SIGABRT, SIGTERM}, [](int sig){});

    // add license check support
    libcpp::license::verifier vef{lic_issuer, libcpp::license::sign_algo::none, {}};
    auto err = vef.verify_file(lic_fpath, user, 30, {{"sn", libcpp::license::get_disk_sn()}});
    if (err)
    {
        LOG_ERROR("license verify failed with err: {}, please check your license file: {}", err.message(), lic_fpath);
        return -1;
    }

    // add your code here...

    // options
    // crypto encrypt --input <file> --output <file> --algo <aes/base64/des/md5/rsa/sha> --key <key> --iv <iv> --padding <pkcs7/zero/none>
    // crypto decrypt --input <file> --output <file> --algo <aes/base64/des/md5/rsa/sha> --key <key> --iv <iv> --padding <pkcs7/zero/none>
    // crypto help --search <content>
    if (argc < 2) 
    {
        LOG_ERROR("Usage: {} <subcommand> [options]", argv[0]);
        return 1;
    }

    std::string subcmd = argv[1];
    if (subcmd == "encrypt" || subcmd == "decrypt") 
    {
        opts.add<std::string>("input", "");
        opts.add<std::string>("output", "");
        opts.add<std::string>("algo", "aes");
        opts.add<std::string>("key", "");
        opts.add<std::string>("iv", "");
        opts.add<std::string>("padding", "pkcs7");

        int sub_argc = argc - 2;
        char** sub_argv = argv + 2;
        auto input = opts.parse<std::string>(sub_argc, sub_argv, "input");
        auto output = opts.parse<std::string>(sub_argc, sub_argv, "output");
        auto algo = opts.parse<std::string>(sub_argc, sub_argv, "algo");
        auto key = opts.parse<std::string>(sub_argc, sub_argv, "key");
        auto iv = opts.parse<std::string>(sub_argc, sub_argv, "iv");
        auto padding = opts.parse<std::string>(sub_argc, sub_argv, "padding");

        LOG_DEBUG("subcmd: {}", subcmd);
        LOG_DEBUG("input: {}", input);
        LOG_DEBUG("output: {}", output);
        LOG_DEBUG("algo: {}", algo);
        LOG_DEBUG("key: {}", key);
        LOG_DEBUG("iv: {}", iv);
        LOG_DEBUG("padding: {}", padding);

        if (subcmd == "encrypt") 
        {
            encrypt();
        }
        // else if (subcmd == "decrypt") { ... }
    } 
    else if (subcmd == "help") 
    {
        opts.add<std::string>("search", "");
        int sub_argc = argc - 2;
        char** sub_argv = argv + 2;
        auto search = opts.parse<std::string>(sub_argc, sub_argv, "search");
        LOG_INFO("help search: {}", search);
        // TODO: 打印帮助信息或搜索内容
    } 
    else 
    {
        LOG_ERROR("Unknown subcommand: {}", subcmd);
        return 1;
    }

    
}