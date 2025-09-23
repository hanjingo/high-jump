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

    // add license check support
    hj::license::verifier vef{"tourist", hj::license::sign_algo::none, {}};
    auto err = vef.verify_file("./license.lic", "tourist", 30, {{"sn", hj::license::get_disk_sn()}});
    if (err)
    {
        LOG_ERROR("license verify failed with err: {}, please check your license file: {}", err.message(), "./license.lic");
        return -1;
    }

    // add your code here...
}