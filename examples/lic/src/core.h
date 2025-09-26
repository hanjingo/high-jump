#ifndef CORE_H
#define CORE_H

#include <hj/os/dll.h>

#include "err.h"
#include "comm.h"

#include "examples/lic_core/src/lic_core.h"

// ------------------------------ lic core ------------------------------
class lic_core
{
public:
    lic_core();
    ~lic_core();

    err_t load();
    err_t unload();
    err_t reload();

    // API 
    err_t version(int& major, int& minor, int& patch);
    err_t init();
    err_t quit();
    err_t add_issuer(
        const std::string& issuer,
        const std::string& algo,
        const int times,
        const std::string& key_str = "");
    err_t issue(
        std::string& out,
        const std::string& algo,
        const std::string& licensee,
        const std::string& issuer,
        const int time, // days
        const std::string& claim_str);

private:
    void* _dll = nullptr;
    sdk_api _version = nullptr;
    sdk_api _init = nullptr;
    sdk_api _quit = nullptr;
    sdk_api _add_issuer = nullptr;
    sdk_api _issue = nullptr;
    sdk_api _keygen = nullptr;
};

#endif // CORE_H