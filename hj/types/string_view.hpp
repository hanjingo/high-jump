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

#ifndef STRING_VIEW_HPP
#define STRING_VIEW_HPP

#if __has_include(<string_view>)
#include <string_view>
namespace hj
{
using string_view = std::string_view;
}

#elif __has_include(<boost/utility/string_view.hpp>)
#include <boost/utility/string_view.hpp>
namespace hj
{
using string_view = boost::string_view;
}

#else
#error                                                                         \
    "No suitable string_view implementation found (need C++17 std::string_view or boost::string_view)"

#endif

#endif // STRING_VIEW_HPP