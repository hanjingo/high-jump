#ifndef EXPORT_H
#define EXPORT_H

#include <libcpp/os/dll.h>

C_STYLE_EXPORT void id(char* buf, size_t* len);

C_STYLE_EXPORT void version(char* buf, size_t* len);

C_STYLE_EXPORT void description(char* buf, size_t* len);

C_STYLE_EXPORT void category(char* buf, size_t* len);

C_STYLE_EXPORT bool init();

C_STYLE_EXPORT void shutdown();

C_STYLE_EXPORT void call(const char* api, const size_t len, void* ctx);

C_STYLE_EXPORT bool event(const char* event, const size_t len, void* ctx);

#endif