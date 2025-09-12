#include <libcpp/log/log.hpp>
#include <libcpp/os/env.h>
#include <libcpp/os/options.hpp>
#include <libcpp/os/signal.hpp>
#include <libcpp/util/license.hpp>
#include <libcpp/encoding/i18n.hpp>
#include <libcpp/testing/crash.hpp>
#include <libcpp/testing/telemetry.hpp>

// add your code here...

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
    libcpp::i18n::instance().set_locale(I18N_LOCALE);
    libcpp::i18n::instance().load_translation_auto("./", "framework");

    // add telemetry support
    auto tracer = libcpp::telemetry::make_otlp_file_tracer("otlp_call", "./telemetry.json");

    // add signals handle support
    libcpp::sigcatch({SIGABRT, SIGTERM}, [](int sig){});

    // add license check support
    libcpp::license::verifier vef{"tourist", libcpp::license::sign_algo::none, {}};
    auto err = vef.verify_file("./license.lic", "tourist", 30, {{"sn", libcpp::license::get_disk_sn()}});
    if (err)
    {
        LOG_ERROR("license verify failed with err: {}, please check your license file: {}", err.message(), "./license.lic");
        return -1;
    }

    // add your code here...
}