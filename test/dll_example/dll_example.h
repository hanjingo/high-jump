#ifndef DLL_EXAMPLE_H
#define DLL_EXAMPLE_H

#include <libcpp/os/dll.h>

C_STYLE_EXPORT int hello ();

#ifdef _WIN32
extern "C" __declspec (dllexport) int world ();
#else
extern "C" __attribute__ ((visibility ("default"))) int world ();
#endif

#endif