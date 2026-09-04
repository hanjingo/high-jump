/*
 *  This file is part of high-jump(hj).
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
#ifndef SDK_H
#define SDK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// define sdk common api
typedef struct sdk_context
{
    uint64_t sz;
    void    *user_data;
    void (*cb)(void *);
} sdk_context;

typedef void (*sdk_callback)(void *);
typedef void (*sdk_api)(sdk_context *);

#ifdef __cplusplus
}
#endif

#endif // SDK_H