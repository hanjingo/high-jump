#include <hj/log/log.hpp>
#include <hj/os/env.h>
#include <hj/os/options.hpp>
#include <hj/os/signal.hpp>
#include <hj/util/license.hpp>
#include <hj/encoding/i18n.hpp>
#include <hj/testing/crash.hpp>
#include <hj/testing/telemetry.hpp>

// add your code here...
#include <iostream>
#include <hj/testing/error.hpp>
#include <hj/testing/error_handler.hpp>
#include <hj/util/defer.hpp>

#include "err.h"
#include "core.h"
#include "comm.h"

int main(int argc, char *argv[])
{
    // add options parse support
    hj::options opts;

    // add crash handle support
    hj::crash_handler::instance()->prevent_set_unhandled_exception_filter();
    hj::crash_handler::instance()->set_local_path("./");

    // add log support
#ifdef DEBUG
    hj::logger::instance()->set_level(hj::log_lvl::debug);
#else
    hj::logger::instance()->set_level(hj::log_lvl::info);
#endif

    // add i18n support
    hj::i18n::instance().set_locale("en_US");
    hj::i18n::instance().load_translation_auto("./", PACKAGE);

    // add telemetry support
    auto tracer =
        hj::telemetry::make_otlp_file_tracer("otlp_call", "./telemetry.json");

    // add signals handle support
    hj::sigcatch({SIGABRT, SIGTERM}, [](int sig) {});

    // add license check support
    hj::license::verifier vef{LIC_ISSUER, hj::license::sign_algo::none, {}};
    auto                  err = vef.verify_file(LIC_FPATH, PACKAGE, 1);
    if(err)
    {
        LOG_ERROR("license verify failed with err: {}, please check your "
                  "license file: {}",
                  err.message(),
                  LIC_FPATH);
        return -1;
    }

    // add your code here...
    hj::error_handler<err_t> h{[](const char *src, const char *dst) {
        LOG_DEBUG("error handler state transition: {} -> {}", src, dst);
    }};
    if(argc < 2)
    {
        h.match(error(ERR_ARGC_TOO_LESS), [&](const err_t &e) {
            LOG_ERROR(tr("app.err_argc_too_less").c_str(), argc);
        });
        return 1;
    }

    crypto_core crypto_sdk;
    db_core     db_sdk;
    // version check
    int major, minor, patch;
    err = crypto_sdk.version(major, minor, patch);
    if(!err
       && (major < CRYPTO_CORE_VERSION_MAJOR_MIN
           || minor < CRYPTO_CORE_VERSION_MINOR_MIN
           || patch < CRYPTO_CORE_VERSION_PATCH_MIN))
        err = error(CRYPTO_ERR_VERSION_MISMATCH);
    if(err)
    {
        h.match(err, [&](const err_t &e) {
            switch(e.value())
            {
                case CRYPTO_ERR_CORE_NOT_LOAD: {
                    LOG_ERROR(tr("log.crypto_err_core_not_load").c_str());
                    break;
                }
                case CRYPTO_ERR_VERSION_MISMATCH: {
                    LOG_ERROR(tr("log.crypto_err_version_mismatch").c_str(),
                              major,
                              minor,
                              patch,
                              CRYPTO_CORE_VERSION_MAJOR_MIN,
                              CRYPTO_CORE_VERSION_MINOR_MIN,
                              CRYPTO_CORE_VERSION_PATCH_MIN);
                    break;
                }
                default: {
                    LOG_ERROR(tr("app.err_unknow").c_str(), e.value());
                    break;
                }
            }
        });
        return 1;
    }

    major = 0;
    minor = 0;
    patch = 0;
    err   = db_sdk.version(major, minor, patch);
    if(!err
       && (major < DB_CORE_VERSION_MAJOR_MIN
           || minor < DB_CORE_VERSION_MINOR_MIN
           || patch < DB_CORE_VERSION_PATCH_MIN))
        err = error(DB_ERR_VERSION_MISMATCH);
    if(err)
    {
        h.match(err, [&](const err_t &e) {
            switch(e.value())
            {
                case DB_ERR_CORE_NOT_LOAD: {
                    LOG_ERROR(tr("log.db_err_core_not_load").c_str());
                    break;
                }
                case DB_ERR_VERSION_MISMATCH: {
                    LOG_ERROR(tr("log.db_err_version_mismatch").c_str(),
                              major,
                              minor,
                              patch,
                              DB_CORE_VERSION_MAJOR_MIN,
                              DB_CORE_VERSION_MINOR_MIN,
                              DB_CORE_VERSION_PATCH_MIN);
                    break;
                }
                default: {
                    LOG_ERROR(tr("app.err_unknow").c_str(), e.value());
                    break;
                }
            }
        });
        return 1;
    }

    // init
    err = crypto_sdk.init();
    if(!err)
        err = db_sdk.init();

    if(err)
    {
        h.match(err, [&](const err_t &e) {
            switch(e.value())
            {
                case CRYPTO_ERR_INIT_FAIL: {
                    LOG_ERROR(tr("log.crypto_err_init_fail").c_str());
                    break;
                }
                case DB_ERR_INIT_FAIL: {
                    LOG_ERROR(tr("log.db_err_init_fail").c_str());
                    break;
                }
                default: {
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

    std::string subcmd  = opts.parse<std::string>(argc, argv, "subcmd");
    auto        algo    = opts.parse<std::string>(argc, argv, "algo");
    auto        bits    = opts.parse<int>(static_cast<int>(argc), argv, "bits");
    auto        fmt     = opts.parse<std::string>(argc, argv, "fmt");
    auto        input   = opts.parse<std::string>(argc, argv, "input");
    auto        key     = opts.parse<std::string>(argc, argv, "key");
    auto        mode    = opts.parse<std::string>(argc, argv, "mode");
    auto        num     = opts.parse<int>(argc, argv, "num");
    auto        output  = opts.parse<std::string>(argc, argv, "output");
    auto        padding = opts.parse<std::string>(argc, argv, "padding");
    auto        timeout = opts.parse<std::size_t>(argc, argv, "timeout");
    auto        iv      = opts.parse<std::string>(argc, argv, "iv");
    auto        passwd  = opts.parse<std::string>(argc, argv, "passwd");
    auto        content = opts.parse<std::string>(argc, argv, "content");
    if(subcmd == "encrypt")
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
        auto ofmt  = select_encrypt_output_fmt(fmt, output, algo);
        auto err   = crypto_sdk.encrypt(output,
                                      input,
                                      content,
                                      algo,
                                      mode,
                                      key,
                                      padding,
                                      iv,
                                      ofmt);
        if(!err)
        {
            print(output, otype);
        } else
        {
            h.match(err, [&](const err_t &e) {
                switch(e.value())
                {
                    case CRYPTO_ERR_INVALID_FMT: {
                        LOG_ERROR(tr("log.crypto_err_invalid_fmt").c_str(),
                                  ofmt,
                                  fmt_strs(std::vector<std::string>{"hex",
                                                                    "base64",
                                                                    "none"}));
                        break;
                    }
                    case CRYPTO_ERR_INVALID_INPUT: {
                        LOG_ERROR(tr("log.crypto_err_invalid_input").c_str(),
                                  input);
                        break;
                    }
                    case CRYPTO_ERR_INVALID_OUTPUT: {
                        LOG_ERROR(tr("log.crypto_err_invalid_output").c_str(),
                                  output);
                        break;
                    }
                    case CRYPTO_ERR_INVALID_ALGO: {
                        LOG_ERROR(tr("log.crypto_err_invalid_algo").c_str(),
                                  algo,
                                  fmt_strs(all_encrypt_algos));
                        break;
                    }
                    case CRYPTO_ERR_INVALID_KEY: {
                        LOG_ERROR(tr("log.crypto_err_invalid_key").c_str(),
                                  key);
                        break;
                    }
                    case CRYPTO_ERR_INVALID_PADDING: {
                        LOG_ERROR(tr("log.crypto_err_invalid_padding").c_str(),
                                  padding);
                        break;
                    }
                    case CRYPTO_ERR_INVALID_IV: {
                        LOG_ERROR(tr("log.crypto_err_invalid_iv").c_str(), iv);
                        break;
                    }
                    case CRYPTO_ERR_INVALID_MODE: {
                        LOG_ERROR(tr("log.crypto_err_invalid_mode").c_str(),
                                  mode);
                        break;
                    }
                    case CRYPTO_ERR_ENCRYPT_AES_FAILED: {
                        LOG_ERROR(
                            tr("log.crypto_err_encrypt_aes_failed").c_str(),
                            output,
                            input,
                            content,
                            algo,
                            mode,
                            key,
                            padding,
                            iv);
                        break;
                    }
                    case CRYPTO_ERR_ENCRYPT_BASE64_FAILED: {
                        LOG_ERROR(
                            tr("log.crypto_err_encrypt_base64_failed").c_str(),
                            output,
                            input,
                            content);
                        break;
                    }
                    case CRYPTO_ERR_ENCRYPT_DES_FAILED: {
                        LOG_ERROR(
                            tr("log.crypto_err_encrypt_des_failed").c_str(),
                            output,
                            input,
                            content,
                            algo,
                            mode,
                            key,
                            padding,
                            iv);
                        break;
                    }
                    case CRYPTO_ERR_ENCRYPT_MD5_FAILED: {
                        LOG_ERROR(
                            tr("log.crypto_err_encrypt_md5_failed").c_str(),
                            output,
                            input,
                            content);
                        break;
                    }
                    case CRYPTO_ERR_ENCRYPT_SHA256_FAILED: {
                        LOG_ERROR(
                            tr("log.crypto_err_encrypt_sha256_failed").c_str(),
                            output,
                            input,
                            content);
                        break;
                    }
                    case CRYPTO_ERR_ENCRYPT_RSA_FAILED: {
                        LOG_ERROR(
                            tr("log.crypto_err_encrypt_rsa_failed").c_str(),
                            output,
                            input,
                            content,
                            algo,
                            mode,
                            key,
                            padding,
                            iv);
                        break;
                    }
                    default: {
                        LOG_ERROR(tr("app.err_unknow").c_str(), e.value());
                        break;
                    }
                }
            });
        }
    } else if(subcmd == "decrypt")
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
        auto err   = crypto_sdk.decrypt(output,
                                      input,
                                      content,
                                      algo,
                                      mode,
                                      key,
                                      passwd,
                                      padding,
                                      iv,
                                      fmt);
        if(!err)
        {
            print(output, otype);
        } else
        {
            h.match(err, [&](const err_t &e) {
                switch(e.value())
                {
                    case CRYPTO_ERR_INVALID_FMT: {
                        LOG_ERROR(tr("log.crypto_err_invalid_fmt").c_str(),
                                  fmt,
                                  fmt_strs(all_encrypt_fmts));
                        break;
                    }
                    case CRYPTO_ERR_INVALID_INPUT: {
                        LOG_ERROR(tr("log.crypto_err_invalid_input").c_str(),
                                  input);
                        break;
                    }
                    case CRYPTO_ERR_INVALID_OUTPUT: {
                        LOG_ERROR(tr("log.crypto_err_invalid_output").c_str(),
                                  output);
                        break;
                    }
                    case CRYPTO_ERR_INVALID_ALGO: {
                        LOG_ERROR(tr("log.crypto_err_invalid_algo").c_str(),
                                  algo,
                                  fmt_strs(all_decrypt_algos));
                        break;
                    }
                    case CRYPTO_ERR_INVALID_KEY: {
                        LOG_ERROR(tr("log.crypto_err_invalid_key").c_str(),
                                  key);
                        break;
                    }
                    case CRYPTO_ERR_INVALID_PADDING: {
                        LOG_ERROR(tr("log.crypto_err_invalid_padding").c_str(),
                                  padding);
                        break;
                    }
                    case CRYPTO_ERR_INVALID_IV: {
                        LOG_ERROR(tr("log.crypto_err_invalid_iv").c_str(), iv);
                        break;
                    }
                    case CRYPTO_ERR_INVALID_MODE: {
                        LOG_ERROR(tr("log.crypto_err_invalid_mode").c_str(),
                                  mode);
                        break;
                    }
                    case CRYPTO_ERR_DECRYPT_AES_FAILED: {
                        LOG_ERROR(
                            tr("log.crypto_err_decrypt_aes_failed").c_str(),
                            output,
                            input,
                            content,
                            algo,
                            mode,
                            key,
                            padding,
                            iv);
                        break;
                    }
                    case CRYPTO_ERR_DECRYPT_BASE64_FAILED: {
                        LOG_ERROR(
                            tr("log.crypto_err_decrypt_base64_failed").c_str(),
                            output,
                            input,
                            content);
                        break;
                    }
                    case CRYPTO_ERR_DECRYPT_DES_FAILED: {
                        LOG_ERROR(
                            tr("log.crypto_err_decrypt_des_failed").c_str(),
                            output,
                            input,
                            content,
                            algo,
                            mode,
                            key,
                            padding,
                            iv);
                        break;
                    }
                    case CRYPTO_ERR_DECRYPT_MD5_FAILED: {
                        LOG_ERROR(
                            tr("log.crypto_err_decrypt_md5_failed").c_str(),
                            output,
                            input,
                            content);
                        break;
                    }
                    case CRYPTO_ERR_DECRYPT_SHA256_FAILED: {
                        LOG_ERROR(
                            tr("log.crypto_err_decrypt_sha256_failed").c_str(),
                            output,
                            input,
                            content);
                        break;
                    }
                    case CRYPTO_ERR_DECRYPT_RSA_FAILED: {
                        LOG_ERROR(
                            tr("log.crypto_err_decrypt_rsa_failed").c_str(),
                            output,
                            input,
                            content,
                            algo,
                            mode,
                            key,
                            padding,
                            iv);
                        break;
                    }
                    default: {
                        LOG_ERROR(tr("app.err_unknow").c_str(), e.value());
                        break;
                    }
                }
            });
        }
    } else if(subcmd == "keygen")
    {
        // crypto keygen --output <file> --algo <auto/rsa256> --fmt <x509> --mode <none> --bits <512/1024/2048/3072/4096>
        // crypto keygen -o <file> -a <auto/rsa256> -f <x509> -m <none> -b <512/1024/2048/3072/4096>
        LOG_DEBUG("output:{}", output);
        LOG_DEBUG("algo:{}", algo);
        LOG_DEBUG("fmt:{}", fmt);
        LOG_DEBUG("mode:{}", mode);
        LOG_DEBUG("bits:{}", bits);

        std::vector<std::string> keys;
        auto                     otype = select_output_type(output);
        auto err = crypto_sdk.keygen(keys, algo, fmt, mode, bits);
        if(!err)
        {
            print(keys, otype);
        } else
        {
            h.match(err, [&](const err_t &e) {
                switch(e.value())
                {
                    case CRYPTO_ERR_INVALID_ALGO: {
                        LOG_ERROR(tr("log.crypto_err_invalid_algo").c_str(),
                                  algo,
                                  fmt_strs(all_keygen_algos));
                        break;
                    }
                    case CRYPTO_ERR_KEYGEN_FAIL: {
                        LOG_ERROR(tr("log.crypto_err_keygen_fail").c_str(),
                                  algo,
                                  fmt,
                                  mode,
                                  bits);
                        break;
                    }
                    default: {
                        LOG_ERROR(tr("app.err_unknow").c_str(), e.value());
                        break;
                    }
                }
            });
        }
    } else if(subcmd == "guess")
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
    } else if(subcmd == "add")
    {
        // crypto add <xx>
        LOG_DEBUG("passwd:{}", passwd);

        db_sdk.add("dict", passwd);
    } else if(subcmd == "list")
    {
        // crypto add --output <file> --num 100 xxx
        // crypto add --o <file> -n 100 xxx
        LOG_DEBUG("output:{}", output);
        LOG_DEBUG("num:{}", num);
        LOG_DEBUG("content:{}", content);

        std::vector<std::vector<std::string> > outs;
        std::vector<std::string>               contents{content};
        auto otype = select_output_type(output);
        auto err   = db_sdk.query(outs, "dict", "passwords", contents, num);
        if(!err)
        {
            print(outs, otype);
        } else
        {
            h.match(err, [&](const err_t &e) {
                switch(e.value())
                {
                    case DB_ERR_DB_NOT_EXIST: {
                        LOG_ERROR(tr("log.db_err_db_not_exist").c_str(),
                                  "dict");
                        break;
                    }
                    case DB_ERR_SQLITE_GET_CONN_FAIL: {
                        LOG_ERROR(
                            tr("log.db_err_sqlite_get_conn_fail").c_str());
                        break;
                    }
                    default: {
                        LOG_ERROR(tr("app.err_unknow").c_str(), e.value());
                        break;
                    }
                }
            });
        }
    } else if(subcmd == "attach")
    {
        // // crypto attach xxx
        // attach();
    } else if(subcmd == "help")
    {
        // // crypto help
        // help();
    } else
    {
        h.match(error(ERR_INVALID_SUBCMD), [&](const err_t &e) {
            LOG_ERROR(tr("app.err_invalid_subcmd").c_str(),
                      subcmd,
                      fmt_strs(all_subcmds));
        });
        return 1;
    }
}