#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <hj/testing/crash.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace fs = std::filesystem;

static char g_log_path[1024] = {0};

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

    if(succeeded)
    {
        hj::crash_print("[crasher] Breakpad callback triggered successfully",
                        g_log_path);
    } else
    {
        hj::crash_print("[crasher] Breakpad callback triggered with failure",
                        g_log_path);
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

    if(succeeded)
    {
        hj::crash_print("[crasher] Breakpad callback triggered successfully",
                        g_log_path);
    } else
    {
        hj::crash_print("[crasher] Breakpad callback triggered with failure",
                        g_log_path);
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

    if(succeeded)
    {
        hj::crash_print("[crasher] Breakpad callback triggered successfully",
                        g_log_path);
    } else
    {
        hj::crash_print("[crasher] Breakpad callback triggered with failure",
                        g_log_path);
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
        std::exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    try
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
            return EXIT_SUCCESS;
        }

        std::string crash_type = vm["type"].as<std::string>();
        std::string dump_dir   = vm["dir"].as<std::string>();

        std::string log_file_path =
            (fs::path(dump_dir) / "callback.log").string();
        std::strncpy(g_log_path, log_file_path.c_str(), sizeof(g_log_path) - 1);

        auto &handler = hj::crash_handler::instance();
        if(!handler.init(dump_dir, crasher_dump_callback))
        {
            std::cerr
                << "[crasher] Error: Failed to initialize crash handler at: "
                << dump_dir << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << "[crasher] Crash handler initialized successfully at: "
                  << dump_dir << std::endl;

        trigger_crash(crash_type);
    }
    catch(const std::exception &e)
    {
        std::cerr << "[crasher] Exception in main: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}