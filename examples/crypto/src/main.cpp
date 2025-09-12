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

#include "err.h"
#include "err_handle.h"
#include "comm.h"
#include "cmd.h"

int main(int argc, char* argv[])
{
    // add options parse support
    libcpp::options opts;

    // add crash handle support
    libcpp::crash_handler::instance()->prevent_set_unhandled_exception_filter();
    libcpp::crash_handler::instance()->set_local_path("./");

    // add log support
    libcpp::logger::instance()->set_level(libcpp::log_lvl::log_lvl_debug);

    // add i18n support
    libcpp::i18n::instance().set_locale("en_US");
    libcpp::i18n::instance().load_translation_auto("./", "crypto");

    // add telemetry support
    auto tracer = libcpp::telemetry::make_otlp_file_tracer("otlp_call", "./telemetry.json");

    // add signals handle support
    libcpp::sigcatch({SIGABRT, SIGTERM}, [](int sig){});

    // add license check support
    // libcpp::license::verifier vef{lic_issuer, libcpp::license::sign_algo::none, {}};
    // auto err = vef.verify_file(lic_fpath, user, 30, {{"sn", libcpp::license::get_disk_sn()}});
    // if (err)
    // {
    //     LOG_ERROR("license verify failed with err: {}, please check your license file: {}", err.message(), lic_fpath);
    //     return -1;
    // }

    // add your code here...
    if (argc < 2) 
    {
        LOG_ERROR("Usage: {} <subcommand> [options]", argv[0]);
        return 1;
    }

    opts.add<std::string>("subcmd", std::string(""), "subcommand");
    opts.add_positional("subcmd", 1);
    opts.add<std::string>("output,o", std::string(""), "output file path");
    opts.add<std::string>("input,i", std::string(""), "input file path");
    opts.add<std::string>("algo,a", std::string("auto"), "algorithm");
    opts.add<std::string>("mode,m", std::string("auto"), "mode");
    opts.add<std::string>("key,k", std::string(""), "key");
    opts.add<std::string>("padding,p", std::string("auto"), "padding mode");
    opts.add<std::string>("iv,v", std::string(""), "iv");
    opts.add<std::string>("fmt,f", std::string("hex"), "input output format");
    opts.add<std::string>("ctx", "", "input content");
    opts.add_positional("ctx", 2);
    std::string subcmd = opts.parse<std::string>(argc, argv, "subcmd");
    if (subcmd == "encrypt") 
    {
        // crypto encrypt --output <file> --input <file> --algo <auto/aes/base64/des/md5/rsa/sha> --mode <auto/ecb/cbc/cfb/...> --key <key> --padding <auto/pkcs7/zero/...> --iv <iv> --fmt <auto/hex/base64> xxx
        // crypto encrypt -o <file> -i <file> -a <auto/aes/base64/des/md5/rsa/sha> -k <key> -m <auto/ecb/cbc/cfb/...> -p <auto/pkcs7/zero/...> -v <iv> -f <auto/hex/base64> xxx
        auto output = opts.parse<std::string>(argc, argv, "output");
        auto input = opts.parse<std::string>(argc, argv, "input");
        auto algo = opts.parse<std::string>(argc, argv, "algo");
        auto mode = opts.parse<std::string>(argc, argv, "mode");
        auto key = opts.parse<std::string>(argc, argv, "key");
        auto padding = opts.parse<std::string>(argc, argv, "padding");
        auto iv = opts.parse<std::string>(argc, argv, "iv");
        auto fmt = opts.parse<std::string>(argc, argv, "fmt");
        auto ctx = opts.parse<std::string>(argc, argv, "ctx");

        LOG_DEBUG("output:{}", output);
        LOG_DEBUG("input:{}", input);
        LOG_DEBUG("algo:{}", algo);
        LOG_DEBUG("mode:{}", mode);
        LOG_DEBUG("key:{}", key);
        LOG_DEBUG("padding:{}", padding);
        LOG_DEBUG("iv:{}", iv);
        LOG_DEBUG("fmt:{}", fmt);
        LOG_DEBUG("ctx:{}", ctx);

        encrypt(output, input, algo, mode, key, padding, iv, fmt, ctx);
    } 
    else if (subcmd == "decrypt")
    {
        auto output = opts.parse<std::string>(argc, argv, "output");
        auto input = opts.parse<std::string>(argc, argv, "input");
        auto algo = opts.parse<std::string>(argc, argv, "algo");
        auto mode = opts.parse<std::string>(argc, argv, "mode");
        auto key = opts.parse<std::string>(argc, argv, "key");
        auto padding = opts.parse<std::string>(argc, argv, "padding");
        auto iv = opts.parse<std::string>(argc, argv, "iv");
        auto fmt = opts.parse<std::string>(argc, argv, "fmt");
        auto ctx = opts.parse<std::string>(argc, argv, "ctx");

        LOG_DEBUG("output:{}", output);
        LOG_DEBUG("input:{}", input);
        LOG_DEBUG("algo:{}", algo);
        LOG_DEBUG("mode:{}", mode);
        LOG_DEBUG("key:{}", key);
        LOG_DEBUG("padding:{}", padding);
        LOG_DEBUG("iv:{}", iv);
        LOG_DEBUG("fmt:{}", fmt);
        LOG_DEBUG("ctx:{}", ctx);

        decrypt(output, input, algo, mode, key, padding, iv, fmt, ctx);
    }
    else if (subcmd == "keygen")
    {
        // crypto keygen --algo <rsa> --bits <2048> --pub <file> --pri <file> --pub-pass <password> --pri-pass <password>
    }
    else if (subcmd == "help") 
    {
        // crypto help --search <content>
    } 
    else 
    {
        handle_err(error(err_invalid_subcmd), subcmd);
        return 1;
    }
}