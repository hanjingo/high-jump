/*
 * This file is part of high-jump(hj).
 *
 * Copyright 2025 hanjingo <hehehunanchina@live.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
#pragma warning(pop)

#endif // _WIN32

#endif // WINSOCK_H