#include <hj/log/log.hpp>
#include <hj/os/env.h>
#include <hj/os/options.hpp>
#include <hj/os/signal.hpp>
#include <hj/util/license.hpp>
#include <hj/testing/crash.hpp>
#include <hj/testing/telemetry.hpp>

// add your code here...
#include <hj/testing/error.hpp>
#include <hj/testing/error_handler.hpp>

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
    hj::log::logger::instance()->set_level(hj::log::level::debug);
#else
    hj::log::logger::instance()->set_level(hj::log::level::info);
#endif

    // add telemetry support
    auto tracer =
        hj::telemetry::make_otlp_file_tracer("otlp_call", "./telemetry.json");

    // add signals handle support
    hj::sighandler::instance().sigcatch({SIGABRT, SIGTERM}, [](int sig) {});

    // // add license check support
    // hj::license::verifier vef{"tourist", hj::license::sign_algo::none, {}};
    // auto err = vef.verify_file("./license.lic", "tourist", 30, {{"sn", hj::license::get_disk_sn()}});
    // if (err)
    // {
    //     LOG_ERROR("license verify failed with err: {}, please check your license file: {}", err.message(), "./license.lic");
    //     return -1;
    // }

    // add your code here...
    hj::error_handler<err_t> h{[](const char *src, const char *dst) {
        LOG_DEBUG("error handler state transition: {} -> {}", src, dst);
    }};
    if(argc < 2)
    {
        h.match(error(ERR_ARGC_TOO_LESS), [&](const err_t &e) {
            LOG_ERROR("Error: too few arguments: {}", argc);
        });
        return 1;
    }

    lic_core lic_sdk;
    // version check
    int  major, minor, patch;
    auto err = lic_sdk.version(major, minor, patch);
    if(!err
       && (major < LIC_CORE_VERSION_MAJOR_MIN
           || minor < LIC_CORE_VERSION_MINOR_MIN
           || patch < LIC_CORE_VERSION_PATCH_MIN))
        err = error(ERR_LIC_CORE_VERSION_MISMATCH);
    if(err)
    {
        h.match(err, [&](const err_t &e) {
            switch(e.value())
            {
                case ERR_LIC_CORE_LOAD_FAIL: {
                    LOG_ERROR("Error: lic_core load fail");
                    break;
                }
                case ERR_LIC_CORE_VERSION_MISMATCH: {
                    LOG_ERROR("Error: lic_core version {}.{}.{}, the minimum required version is {}.{}.{}",
                              major,
                              minor,
                              patch,
                              LIC_CORE_VERSION_MAJOR_MIN,
                              LIC_CORE_VERSION_MINOR_MIN,
                              LIC_CORE_VERSION_PATCH_MIN);
                    break;
                }
                default: {
                    LOG_ERROR("Error: unknown error code:{}", e.value());
                    break;
                }
            }
        });
        return 1;
    }

    // init
    err = lic_sdk.init();
    if(err)
    {
        h.match(err, [&](const err_t &e) {
            switch(e.value())
            {
                case ERR_LIC_CORE_LOAD_FAIL: {
                    LOG_ERROR("Error: lic_core load fail");
                    break;
                }
                case LIC_ERR_INIT_FAIL: {
                    LOG_ERROR("Error: lic_core init fail");
                    break;
                }
                default: {
                    LOG_ERROR("Error: unknown error code:{}", e.value());
                    break;
                }
            }
        });
        return 1;
    }

    // parse options
    opts.add<std::string>("subcmd", std::string(""), "subcommand");
    opts.add_positional("subcmd", 1);
    opts.add<std::string>("algo,a", std::string("none"), "sign algo");
    opts.add<int>("count,c", 1, "valid count");
    opts.add<std::string>("licensee,l",
                          std::string("tourist"),
                          "licensee name");
    opts.add<std::string>("issuer,i", std::string("default"), "issuer name");
    opts.add<std::string>("output,o", std::string(""), "output lic file path");
    opts.add<int>("time,t", 30, "not less than this days");
    opts.add<std::string>("content", "", "input content");
    opts.add_positional("content", 2);

    std::string subcmd = opts.parse<std::string>(argc, argv, "subcmd");
    auto        algo   = opts.parse<std::string>(argc, argv, "algo");
    auto        count  = opts.parse<int>(static_cast<int>(argc), argv, "count");
    auto        licensee = opts.parse<std::string>(argc, argv, "licensee");
    auto        issuer   = opts.parse<std::string>(argc, argv, "issuer");
    auto        output   = opts.parse<std::string>(argc, argv, "output");
    auto        time    = opts.parse<int>(static_cast<int>(argc), argv, "time");
    auto        content = opts.parse<std::string>(argc, argv, "content");
    if(subcmd == "add")
    {
        // lic add --algo <none/rsa256> --issuer <id> --count <times> key1,key2,...
        // lic add -a <none/rsa256> -i <id> -c <times> key1,key2,...
        LOG_DEBUG("algo:{}", algo);
        LOG_DEBUG("issuer:{}", issuer);
        LOG_DEBUG("count:{}", count);
        LOG_DEBUG("content:{}", content);
        err = lic_sdk.add_issuer(issuer, algo, count, content);
        if(!err)
        {
            print(output, output_type::console);
            return 1;
        }

        h.match(err, [&](const err_t &e) {
            switch(e.value())
            {
                case ERR_LIC_CORE_LOAD_FAIL: {
                    LOG_ERROR("Error: lic_core load fail");
                    break;
                }
                case LIC_ERR_INVALID_PARAM: {
                    LOG_ERROR("Error: lic_core invalid param");
                    break;
                }
                case LIC_ERR_INVALID_TIMES: {
                    LOG_ERROR("Error: lic_core invalid times, count: {}", count);
                    break;
                }
                case LIC_ERR_KEYS_NOT_ENOUGH: {
                    LOG_ERROR("Error: lic_core keys not enough, content: {}",
                              content);
                    break;
                }
                case LIC_ERR_ISSUER_EXISTED: {
                    LOG_ERROR("Error: lic_core issuer exist, issuer_id: {}",
                              issuer);
                    break;
                }
                default: {
                    LOG_ERROR("Error: unknown error code:{}", e.value());
                    break;
                }
            }
        });
    } else if(subcmd == "issue")
    {
        // lic issue --algo <none/rsa256> --licensee <target id> --issuer <id> --output <file> --time <days> xxx:xxx,xxx:xxx
        // lic issue -a <none/rsa256> -l <target id> -i <id> -o <file> -t <days> xxx:xxx,xxx:xxx
        LOG_DEBUG("algo:{}", algo);
        LOG_DEBUG("licensee:{}", licensee);
        LOG_DEBUG("issuer:{}", issuer);
        LOG_DEBUG("output:{}", output);
        LOG_DEBUG("time:{}", time);
        LOG_DEBUG("content:{}", content);

        output_type otype = output_type::console;
        if(!output.empty())
            otype = output_type::file;
        err = lic_sdk.issue(output, algo, licensee, issuer, time, content);
        if(!err)
        {
            print(output, otype);
            return 1;
        }

        h.match(err, [&](const err_t &e) {
            switch(e.value())
            {
                case ERR_LIC_CORE_LOAD_FAIL: {
                    LOG_ERROR("Error: lic_core load fail");
                    break;
                }
                case LIC_ERR_INVALID_PARAM: {
                    LOG_ERROR("Error: lic_core invalid param");
                    break;
                }
                case LIC_ERR_ISSUER_NOT_EXIST: {
                    LOG_ERROR("Error: lic_core issuer exist, issuer_id: {}",
                              issuer);
                    break;
                }
                case LIC_ERR_CLAIM_MISMATCH: {
                    LOG_ERROR("Error: lic_core claim mismatch, content: {}",
                              content);
                    break;
                }
                default: {
                    LOG_ERROR("Error: unknown error code:{}", e.value());
                    break;
                }
            }
        });
    } else if(subcmd == "verify")
    {
        // lic verify --input <file> --time <days> xxx
        // lic verify -i <file> -t <days> xxx
        LOG_DEBUG("content:{}", content);
    } else if(subcmd == "help")
    {
        // lic help
        print("Usage: lic <subcommand> [options] [Content]\n\nSubcommands:\n  add       Add the lic issuer\n  issue     Issue a license file\n  verify    Verify a license file\n  help      Show this help message\n\nOptions:\n  -a, --algo      Signature algorithm, one of [none, rsa256], default: none\n  -c, --count     Valid times, default: 1\n  -l, --licensee  Licensee name, default: tourist\n  -o, --output    Output license file path\n  -t, --time      Not less than this days, default: 30\n\nContent:\n  for subcmd add: key-value pairs for keys, e.g. xxx,xxx\n  for subcmd issue: key-value pairs for claims, e.g. xxx:xxx,xxx:xxx\n  for subcmd verify: key-value pairs for claims, e.g. xxx:xxx,xxx:xxx\n\nExamples:\n  lic add -a none -i hehehunanchina@live.com -c 10 -i\n  lic issue -a none -l tourist -i hehehunanchina@live.com -t 30\n  lic verify -a none -i hehehunanchina@live.com\n", output_type::console);
    } else
    {
        h.match(error(ERR_INVALID_SUBCMD), [&](const err_t &e) {
            LOG_ERROR("Error: unknown subcommand: {}, we expected one of these subcommands: [{}]",
                      subcmd,
                      fmt_strs(all_subcmds));
        });
        return 1;
    }
}