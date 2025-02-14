#ifndef DLL_H
#define DLL_H

// export/import dll
#if defined(_MSC_VER) //  Microsoft
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) //  GCC
#define DLL_EXPORT __attribute__((visibility("default")))
#define DLL_IMPORT
#else // Warnning
#define DLL_EXPORT
#define DLL_IMPORT
#pragma warning Unknown dynamic link import/export semantics.
#endif

// export/import c Style dll
#ifdef _cplusplus
#define C_STYLE_EXPORT extern "C" DLL_EXPORT
#define C_STYLE_IMPORT DLL_IMPORT
#else
#define C_STYLE_EXPORT DLL_EXPORT
#define C_STYLE_IMPORT DLL_IMPORT
#endif

#endif