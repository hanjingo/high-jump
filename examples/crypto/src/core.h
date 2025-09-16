#ifndef CORE_H
#define CORE_H

#include <libcpp/os/dll.h>
#include <libcpp/log/log.hpp>

#include "comm.h"
#include "examples/crypto_core/src/api.h"

class crypto_core
{
public:
    crypto_core();
    ~crypto_core();

    err_t version(int& major, int& minor, int& patch);
    err_t init();
    err_t quit();
    err_t encrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& algo,
        const std::string& mode,
        const std::string& key,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt);
    err_t decrypt(
        std::string& out,
        const std::string& in,
        const std::string& content,
        const std::string& algo,
        const std::string& mode,
        const std::string& key,
        const std::string& passwd,
        const std::string& padding,
        const std::string& iv,
        const std::string& fmt);

private:
    void* _dll = nullptr;
    sdk_api _version = nullptr;
    sdk_api _init = nullptr;
    sdk_api _quit = nullptr;
    sdk_api _encrypt = nullptr;
    sdk_api _decrypt = nullptr;
};

#endif