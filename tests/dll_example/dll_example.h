#ifndef DLL_EXAMPLE_H
#define DLL_EXAMPLE_H

#include <libcpp/util/dll.h>

C_STYLE_EXPORT int hello();

extern "C" __attribute__((visibility("default"))) int world();

#endif