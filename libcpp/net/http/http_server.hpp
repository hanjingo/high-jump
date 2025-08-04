#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#if defined(OPENSSL_ENABLE)
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include <httplib.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace libcpp {

using http_server = httplib::Server;

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
using http_ssl_server = httplib::SSLServer;
#endif

}  // namespace libcpp

#endif