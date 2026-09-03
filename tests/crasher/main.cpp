#include <iostream>
#include <string>
#include <filesystem>
#include <hj/testing/crash.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace fs = std::filesystem;

static std::string g_dump_dir = "./dumps";

#if defined(_WIN32)
static bool crasher_dump_callback(const wchar_t      *dump_dir,
                                  const wchar_t      *minidump_id,
                                  void               *context,
                                  EXCEPTION_POINTERS *exinfo,
                                  MDRawAssertionInfo *assertion,
                                  bool                succeeded)
{
    (void) dump_dir;
    (void) minidump_id;
    (void) context;
    (void) exinfo;
    (void) assertion;

    std::string log_path = (fs::path(g_dump_dir) / "callback.log").string();
    if(succeeded)
    {
        hj::crash_print("[crasher] Breakpad callback triggered successfully",
                        log_path.c_str());
    } else
    {
        hj::crash_print("[crasher] Breakpad callback triggered with failure",
                        log_path.c_str());
    }
    return succeeded;
}
#elif defined(__APPLE__)
static bool crasher_dump_callback(const char *dump_dir,
                                  const char *minidump_id,
                                  void       *context,
                                  bool        succeeded)
{
    (void) dump_dir;
    (void) minidump_id;
    (void) context;

    std::string log_path = (fs::path(g_dump_dir) / "callback.log").string();
    if(succeeded)
    {
        hj::crash_print("[crasher] Breakpad callback triggered successfully",
                        log_path.c_str());
    } else
    {
        hj::crash_print("[crasher] Breakpad callback triggered with failure",
                        log_path.c_str());
    }
    return succeeded;
}
#else
static bool
crasher_dump_callback(const google_breakpad::MinidumpDescriptor &descriptor,
                      void                                      *context,
                      bool                                       succeeded)
{
    (void) descriptor;
    (void) context;

    std::string log_path = (fs::path(g_dump_dir) / "callback.log").string();
    if(succeeded)
    {
        hj::crash_print("[crasher] Breakpad callback triggered successfully",
                        log_path.c_str());
    } else
    {
        hj::crash_print("[crasher] Breakpad callback triggered with failure",
                        log_path.c_str());
    }
    return succeeded;
}
#endif

void trigger_crash(const std::string &type)
{
    std::cout << "[crasher] Triggering crash type: " << type << std::endl;

    if(type == "segfault")
    {
        volatile int *ptr = nullptr;
        int           val = *ptr;
        (void) val;
    } else if(type == "divbyzero")
    {
        volatile int a = 1;
        volatile int b = 0;
        volatile int c = a / b;
        (void) c;
    } else if(type == "abort")
    {
        std::abort();
    } else
    {
        std::cerr << "[crasher] Unknown crash type: " << type << std::endl;
        std::exit(1);
    }
}

int main(int argc, char *argv[])
{
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")(
        "type,t",
        po::value<std::string>()->default_value("segfault"),
        "crash type: segfault, divbyzero, abort")(
        "dir,d",
        po::value<std::string>()->default_value("./dumps"),
        "dump directory");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help"))
    {
        std::cout << desc << "\n";
        return 0;
    }

    std::string crash_type = vm["type"].as<std::string>();
    g_dump_dir             = vm["dir"].as<std::string>();

    auto &handler = hj::crash_handler::instance();
    handler.init(g_dump_dir, crasher_dump_callback);

    std::cout << "[crasher] Crash handler initialized at: " << g_dump_dir
              << std::endl;

    trigger_crash(crash_type);

    return 0;
}