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

#ifndef FILE_HPP
#define FILE_HPP

#ifndef FSIZE
#define FSIZE unsigned long long
#endif

#ifndef BYTE
#define BYTE(n) ((FSIZE) (n))
#endif

#ifndef KB
#define KB(n) ((FSIZE) (n) * 0x400)
#endif

#ifndef MB
#define MB(n) ((FSIZE) (n) * 0x100000)
#endif

#ifndef GB
#define GB(n) ((FSIZE) (n) * 0x40000000)
#endif

#ifndef TB
#define TB(n) ((FSIZE) (n) * 0x10000000000)
#endif

#endif