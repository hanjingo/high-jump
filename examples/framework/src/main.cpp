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
    libcpp::i18n::instance().load_translation_auto("./", "framework");
    LOG_DEBUG("{}", libcpp::tr("app.title"));

    // add telemetry support
    auto tracer = libcpp::telemetry::make_otlp_file_tracer("otlp_call", telemetry_fpath);

    // add signals handle support
    libcpp::sigcatch({SIGABRT, SIGTERM}, [](int sig){});

    // add license check support
    libcpp::license::token_t token;
    libcpp::license::verifier vef{lic_issuer, libcpp::license::sign_algo::none, {}};
    if (!libcpp::license::is_file_exist(lic_fpath))
    {
        libcpp::license::issuer iss{lic_issuer, libcpp::license::sign_algo::none, {}, 1};
        auto err = iss.issue_file(lic_fpath, user, 30, {{"sn", libcpp::license::get_disk_sn()}});
        if (err)
        {
            LOG_ERROR("generate license file failed: {}, err: {}", lic_fpath, err.message());
            return -1;
        }
    }
    auto err = vef.verify_file(lic_fpath, user, 30, {{"sn", libcpp::license::get_disk_sn()}});
    if (err)
    {
        LOG_ERROR("license verify failed with err: {}, please check your license file: {}", err.message(), lic_fpath);
        return -1;
    }

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