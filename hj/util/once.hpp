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

#ifndef ONCE_HPP
#define ONCE_HPP

#include <mutex>

#define HJ_ONCE_CAT_IMPL(a, b) a##b
#define HJ_ONCE_CAT(a, b) HJ_ONCE_CAT_IMPL(a, b)

#define HJ_ONCE_IMPL(counter_val, ...)                                         \
    do                                                                         \
    {                                                                          \
        static std::once_flag HJ_ONCE_CAT(do_once_flag_, counter_val);         \
        std::call_once(HJ_ONCE_CAT(do_once_flag_, counter_val),                \
                       [&]() { __VA_ARGS__ });                                 \
    } while(0)

#define HJ_ONCE(...) HJ_ONCE_IMPL(__COUNTER__, __VA_ARGS__)

#endif