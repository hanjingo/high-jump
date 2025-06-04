#include <iostream>
#include <string>

#include <libcpp/testing/crash.hpp>

#ifdef _WIN32
bool dump_callback(const wchar_t* dump_dir,
                   const wchar_t* minidump_id,
                   void* context,
                   EXCEPTION_POINTERS* exinfo,
                   MDRawAssertionInfo* assertion,
                   bool succeeded)
{
    std::wstring w_dump_dir(dump_dir);
    std::string str_dump_dir(w_dump_dir.length(), ' ');
    std::copy(w_dump_dir.begin(), w_dump_dir.end(), str_dump_dir.begin());

    std::wstring w_minidump_id(minidump_id);
    std::string str_minidump_id(w_minidump_id.length(), ' ');
    std::copy(w_minidump_id.begin(), w_minidump_id.end(), str_minidump_id.begin());
    
    std::cout << (succeeded ? "Succeed" : "Fail")  << " To Create Dump File With Path =" 
              << str_dump_dir << "/" << str_minidump_id << std::endl;
    return true;
}
#elif __APPLE__
bool dump_callback(const char *dump_dir, const char *minidump_id, void *context, bool succeeded)
{
    std::cout << (succeeded ? "Succeed" : "Fail")  << " To Create Dump File With Path =" 
              << std::string(dump_dir) << "/" << std::string(minidump_id) << std::endl;
    return true;
}
#else
bool dump_callback(const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded)
{
    std::cout << (succeeded ? "Succeed" : "Fail")  << " To Create Dump File"<< std::endl;
    return true;
}
#endif

int main(int argc, char* argv[])
{
    
#ifdef _WIN32
    libcpp::prevent_set_unhandled_exception_filter();
#endif

    libcpp::crash_handler::instance()->set_dump_callback(dump_callback);
    libcpp::crash_handler::instance()->set_local_path("./");

    // crash it 
    int *p = nullptr;
    *p = 1;

    return 0;
}