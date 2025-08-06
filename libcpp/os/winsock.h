#ifndef WINSOCK_H
#define WINSOCK_H

#ifdef _WIN32

// Ensure clean Windows headers
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

// Prevent winsock.h from being included (force winsock2.h usage)
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

// Include modern WinSock2 API only
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>


// Automatically link required libraries
#pragma comment(lib, "ws2_32.lib")

// Suppress deprecation warnings for legacy winsock functions
#pragma warning(push)
#pragma warning(disable : 4996)

#endif  // _WIN32

#endif  // WINSOCK_H