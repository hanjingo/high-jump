#ifndef REGEX_HPP
#define REGEX_HPP

#include <regex>
#include <string>

namespace libcpp {

static const char* PATTERN_IDCARD_CN = "([0-9]){18}([0-9]|x|X)?$";
static const char* PATTERN_INTEGER = "-?[1-9]\\d+$";

static const char* PATTERN_DECIMALS1 = "(-?[0-9]+(.\\d{0,1})?)$";
static const char* PATTERN_DECIMALS2 = "(-?[0-9]+(.\\d{0,2})?)$";
static const char* PATTERN_DECIMALS3 = "(-?[0-9]+(.\\d{0,3})?)$";

static const char* PATTERN_NUM_LATTER = "[A-Za-z0-9]+$";
static const char* PATTERN_IP_V4 =
    "((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)";
static const char* PATTERN_PORT = "^(6553[0-5]|655[0-2][0-9]|65[0-4][0-9]{2}|6["
                                  "0-4][0-9]{3}|[1-5][0-9]{4}|[0-9]{1,4})$";
static const char* PATTERN_MAC =
    "([0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2})";

}  // namespace libcpp

#endif