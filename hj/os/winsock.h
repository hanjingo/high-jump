/*
 *  This file is part of hj.
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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