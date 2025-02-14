#ifndef API_H
#define API_H

#include <libcpp/util/dll.hpp>

// combine export and import macro
#if defined(LIBCPP_EXPORT)
#define LIBCPP_API C_STYLE_EXPORT
#else
#define LIBCPP_API C_STYLE_IMPORT
#endif

#endif