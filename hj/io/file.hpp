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