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

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#if __has_include(<boost/config.hpp>)
#include <boost/serialization/singleton.hpp>
namespace hj
{

template <typename T>
using singleton = boost::serialization::singleton<T>;

}

#else
#error "No suitable singleton implementation found"

#endif

#endif