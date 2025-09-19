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
#include "core.h"
#include "handle.h"
#include "comm.h"

int main(int argc, char* argv[])
{
    // add options parse support
    libcpp::options opts;

    // add crash handle support
    libcpp::crash_handler::instance()->prevent_set_unhandled_exception_filter();
    libcpp::crash_handler::instance()->set_local_path("./");

    // add log support
    libcpp::logger::instance()->set_level(libcpp::log_lvl::debug);

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
        LOG_ERROR("{}", tr("log.err_argc_too_less"));
        return 1;
    }

    crypto_core crypto_sdk;
    db_core db_sdk;
    // version check
    int major, minor, patch;
    auto err = crypto_sdk.version(major, minor, patch);
    if (err) 
    {
        handle_err(err);
        return 1;
    }
    if (major < CRYPTO_CORE_VERSION_MAJOR_MIN 
            || minor < CRYPTO_CORE_VERSION_MINOR_MIN 
            || patch < CRYPTO_CORE_VERSION_PATCH_MIN)
    {
        handle_err(error(CRYPTO_ERR_VERSION_MISMATCH), major, minor, patch);
        return 1;
    }
    major = 0; minor = 0; patch = 0;
    err = db_sdk.version(major, minor, patch);
    if (err) 
    {
        handle_err(err);
        return 1;
    }
    if (major < DB_CORE_VERSION_MAJOR_MIN 
            || minor < DB_CORE_VERSION_MINOR_MIN 
            || patch < DB_CORE_VERSION_PATCH_MIN)
    {
        handle_err(error(DB_ERR_VERSION_MISMATCH), major, minor, patch);
        return 1;
    }

    // init
    err = crypto_sdk.init();
    if (err) 
    {
        handle_err(err);
        return 1;
    }
    err = db_sdk.init();
    if (err) 
    {
        handle_err(err);
        return 1;
    }

    // parse options
    opts.add<std::string>("subcmd", std::string(""), "subcommand");
    opts.add_positional("subcmd", 1);

    opts.add<std::string>("algo,a", std::string(""), "algorithm");
    opts.add<int>("bits,b", 2048, "bits for keygen");
    opts.add<std::string>("fmt,f", std::string("none"), "input output format");
    opts.add<std::string>("input,i", std::string(""), "input file path");
    opts.add<std::string>("key,k", std::string(""), "key");
    opts.add<std::string>("mode,m", std::string(""), "mode");
    opts.add<int>("num,n", -1, "number/count");
    opts.add<std::string>("output,o", std::string(""), "output file path");
    opts.add<std::string>("padding,p", std::string("pkcs7"), "padding mode");
    opts.add<std::size_t>("timeout,t", 30, "timeout in seconds");
    opts.add<std::string>("iv,v", std::string(""), "iv");
    opts.add<std::string>("passwd", std::string(""), "password");
    opts.add<std::string>("content", "", "input content");
    opts.add_positional("content", 2);

    std::string subcmd = opts.parse<std::string>(argc, argv, "subcmd");
    auto algo = opts.parse<std::string>(argc, argv, "algo");
    auto bits = opts.parse<int>(static_cast<int>(argc), argv, "bits");
    auto fmt = opts.parse<std::string>(argc, argv, "fmt");
    auto input = opts.parse<std::string>(argc, argv, "input");
    auto key = opts.parse<std::string>(argc, argv, "key");
    auto mode = opts.parse<std::string>(argc, argv, "mode");
    auto num = opts.parse<int>(argc, argv, "num");
    auto output = opts.parse<std::string>(argc, argv, "output");
    auto padding = opts.parse<std::string>(argc, argv, "padding");
    auto timeout = opts.parse<std::size_t>(argc, argv, "timeout");
    auto iv = opts.parse<std::string>(argc, argv, "iv");
    auto passwd = opts.parse<std::string>(argc, argv, "passwd");
    auto content = opts.parse<std::string>(argc, argv, "content");
    if (subcmd == "encrypt") 
    {
        // crypto encrypt --output <file> --input <file> --algo <auto/aes/base64/des/md5/rsa/sha> --mode <auto/ecb/cbc/cfb/...> --key <key> --padding <auto/pkcs7/zero/...> --iv <iv> --fmt <auto/hex/base64> xxx
        // crypto encrypt -o <file> -i <file> -a <auto/aes/base64/des/md5/rsa/sha> -k <key> -m <auto/ecb/cbc/cfb/...> -p <auto/pkcs7/zero/...> -v <iv> -f <auto/hex/base64> xxx
        LOG_DEBUG("output:{}", output);
        LOG_DEBUG("input:{}", input);
        LOG_DEBUG("algo:{}", algo);
        LOG_DEBUG("mode:{}", mode);
        LOG_DEBUG("key:{}", key);
        LOG_DEBUG("padding:{}", padding);
        LOG_DEBUG("iv:{}", iv);
        LOG_DEBUG("fmt:{}", fmt);
        LOG_DEBUG("content:{}", content);

        auto otype = select_output_type(output);
        auto ofmt = select_encrypt_output_fmt(fmt, output, algo);
        auto err = crypto_sdk.encrypt(output, input, content, algo, mode, key, padding, iv, ofmt);
        if (err)
            handle_err(err, output, input, content, algo, mode, key, padding, iv, ofmt);
        else
            print(output, otype);
    } 
    else if (subcmd == "decrypt")
    {
        LOG_DEBUG("output:{}", output);
        LOG_DEBUG("input:{}", input);
        LOG_DEBUG("algo:{}", algo);
        LOG_DEBUG("mode:{}", mode);
        LOG_DEBUG("key:{}", key);
        LOG_DEBUG("passwd:{}", passwd);
        LOG_DEBUG("padding:{}", padding);
        LOG_DEBUG("iv:{}", iv);
        LOG_DEBUG("fmt:{}", fmt);
        LOG_DEBUG("ctx:{}", content);

        auto otype = select_output_type(output);
        auto err = crypto_sdk.decrypt(output, input, content, algo, mode, key, passwd, padding, iv, fmt);
        if (err)
            handle_err(err, output, input, content, algo, mode, key, padding, iv, fmt);
        else
            print(output, otype);
    }
    else if (subcmd == "keygen")
    {
        // crypto keygen --output <file> --algo <auto/rsa256> --fmt <x509> --mode <none> --bits <512/1024/2048/3072/4096> 
        // crypto keygen -o <file> -a <auto/rsa256> -f <x509> -m <none> -b <512/1024/2048/3072/4096>
        LOG_DEBUG("output:{}", output);
        LOG_DEBUG("algo:{}", algo);
        LOG_DEBUG("fmt:{}", fmt);
        LOG_DEBUG("mode:{}", mode);
        LOG_DEBUG("bits:{}", bits);

        std::vector<std::string> keys;
        auto otype = select_output_type(output);
        auto err = crypto_sdk.keygen(keys, algo, fmt, mode, bits);
        if (err)
            handle_err(err, algo, fmt, mode, bits);
        else
            print(keys, otype);
    }
    else if (subcmd == "guess")
    {
        // // crypto guess --output <file> --input <file> --algo <auto/aes/base64/des/md5/rsa/sha> --mode <auto/ecb/cbc/cfb/...> --key <key> --padding <auto/pkcs7/zero/...> --iv <iv> --fmt <auto/hex/base64> xxx
        // // crypto guess -o <file> -i <file> -a <auto/aes/base64/des/md5/rsa/sha> -k <key> -m <auto/ecb/cbc/cfb/...> -p <auto/pkcs7/zero/...> -v <iv> -f <auto/hex/base64> xxx

        // LOG_DEBUG("output:{}", output);
        // LOG_DEBUG("input:{}", input);
        // LOG_DEBUG("algo:{}", algo);
        // LOG_DEBUG("mode:{}", mode);
        // LOG_DEBUG("key:{}", key);
        // LOG_DEBUG("passwd:{}", passwd);
        // LOG_DEBUG("padding:{}", padding);
        // LOG_DEBUG("iv:{}", iv);
        // LOG_DEBUG("fmt:{}", fmt);
        // LOG_DEBUG("ctx:{}", ctx);

        // guess(output, input, algo, mode, key, passwd, padding, iv, fmt, ctx, timeout);
    }
    else if (subcmd == "add")
    {
        // crypto add <xx>
        LOG_DEBUG("passwd:{}", passwd);

        db_sdk.add("dict", passwd);
    }
    else if (subcmd == "list")
    {
        // crypto add --output <file> --num 100 xxx
        // crypto add --o <file> -n 100 xxx
        LOG_DEBUG("output:{}", output);
        LOG_DEBUG("num:{}", num);
        LOG_DEBUG("content:{}", content);

        std::vector<std::vector<std::string>> outs;
        std::vector<std::string> contents{content};
        auto otype = select_output_type(output);
        auto err = db_sdk.query(outs, "dict", "passwords", contents, num);
        if (err)
            handle_err(err, content, num);
        else
            print(outs, otype);
    }
    else if (subcmd == "attach") 
    {
        // // crypto attach xxx
        // attach();
    }
    else if (subcmd == "help") 
    {
        // // crypto help
        // help();
    } 
    else 
    {
        handle_err(error(ERR_INVALID_SUBCMD), subcmd);
        return 1;
    }
}