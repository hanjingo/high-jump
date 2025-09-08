#include <libcpp/log/log.hpp>
#include <libcpp/os/env.h>
#include <libcpp/os/options.hpp>
#include <libcpp/os/signal.hpp>
#include <libcpp/util/license.hpp>
#include <libcpp/encoding/i18n.hpp>
#include <libcpp/testing/crash.hpp>
#include <libcpp/testing/telemetry.hpp>

int main(int argc, char* argv[])
{
    // parse options
    libcpp::options opts;
    opts.add<std::string>("user", std::string("tourist"));
    opts.add<std::string>("crash_path", std::string("./"));
    opts.add<std::string>("lic_issuer", std::string("defauult"));
    opts.add<std::string>("lic_fpath", std::string("./license.json"));
    auto user = opts.parse<std::string>(argc, argv, "user");
    auto crash_path = opts.parse<std::string>(argc, argv, "crash_path");
    auto lic_issuer = opts.parse<std::string>(argc, argv, "lic_issuer");
    auto lic_fpath = opts.parse<std::string>(argc, argv, "lic_fpath");

    // init log
    libcpp::logger::instance()->set_level(libcpp::log_lvl::log_lvl_debug);

    // add crash handler
    libcpp::crash_handler::instance()->prevent_set_unhandled_exception_filter();
    libcpp::crash_handler::instance()->set_local_path(crash_path);
    LOG_DEBUG("crash path: {}", crash_path);

    // ignore signals
    libcpp::sigcatch({SIGABRT, SIGTERM}, [](int sig){});

    // // add license check
    // libcpp::license::token_t token;
    // libcpp::license::issuer isu{lic_issuer, libcpp::license::sign_algo::none, {"", "", "", ""}, 1};
    // // if ()
    // if (!isu.verify(token, user, 1, {"sn", libcpp::license::get_disk_sn()}))
    // {
    //     LOG_ERROR("license verify failed, please check your license file: {}", lic_fpath);
    //     return -1;
    // }

    // add water mark
    LOG_INFO("{} {}.{}.{}", 
        PACKAGE, 
        MAJOR_VERSION, 
        MINOR_VERSION, 
        PATCH_VERSION);
    LOG_INFO("{} compile time {}", PACKAGE, COMPILE_TIME);
    LOG_INFO("{} email {}", PACKAGE, EMAIL);

    // add your code here...
}