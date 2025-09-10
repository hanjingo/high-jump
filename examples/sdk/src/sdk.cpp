#include "sdk.h"
#include <libcpp/util/init.hpp>
#include <libcpp/os/env.h>
#include <libcpp/testing/crash.hpp>
#include <libcpp/log/log.hpp>
#include <libcpp/testing/telemetry.hpp>

INIT(
    // add crash handle support
    libcpp::crash_handler::instance()->prevent_set_unhandled_exception_filter();
    libcpp::crash_handler::instance()->set_local_path("./");

    // add log support
    libcpp::logger::instance()->set_level(libcpp::log_lvl::log_lvl_debug);

    // add telemetry support
    auto tracer = libcpp::telemetry::make_otlp_file_tracer("otlp_call", "./telemetry.json");

    // add water mark
    LOG_INFO("{} {}.{}.{}", 
        PACKAGE, 
        MAJOR_VERSION, 
        MINOR_VERSION, 
        PATCH_VERSION);
    LOG_INFO("{} compile time {}", PACKAGE, COMPILE_TIME);
    LOG_INFO("{} email {}", PACKAGE, EMAIL);
)

void init()
{
    // add your code here ...
}
