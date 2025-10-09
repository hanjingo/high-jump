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
#ifndef REGEX_HPP
#define REGEX_HPP

#include <string>
#include <regex>

namespace hj
{

static const std::regex REGEX_IDCARD_CN{R"(([0-9]{17}[0-9xX])$)"};
static const std::regex REGEX_INTEGER{R"(^-?(0|[1-9]\d*)$)"};

static const std::regex REGEX_DECIMALS1{R"((-?[0-9]+(\.\d{0,1})?)$)"};
static const std::regex REGEX_DECIMALS2{R"((-?[0-9]+(\.\d{0,2})?)$)"};
static const std::regex REGEX_DECIMALS3{R"((-?[0-9]+(\.\d{0,3})?)$)"};

static const std::regex REGEX_NUM_LATTER{R"([A-Za-z0-9]+$)"};

static const std::regex REGEX_IP_V4{
    R"(((2[0-4]\d|25[0-5]|[01]?\d\d?)\.){3}(2[0-4]\d|25[0-5]|[01]?\d\d?))"};
static const std::regex REGEX_PORT{
    R"(^(6553[0-5]|655[0-2][0-9]|65[0-4][0-9]{2}|6[0-4][0-9]{3}|[1-5][0-9]{4}|[0-9]{1,4})$)"};
static const std::regex REGEX_MAC{
    R"(([0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2}))"};

}

#endif