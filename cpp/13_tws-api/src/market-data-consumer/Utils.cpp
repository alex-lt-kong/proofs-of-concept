/* Copyright (C) 2023 Interactive Brokers LLC. All rights reserved. This code is
 * subject to the terms and conditions of the IB API Non-Commercial License or
 * the IB API Commercial License, as applicable. */

#include "Utils.h"
#include "twsapi/platformspecific.h"

#include <cfloat>
#include <climits>
#include <sstream>
#include <string>

std::string Utils::doubleMaxString(double d, std::string def) {
  if (d == DBL_MAX) {
    return def;
  } else {
    std::ostringstream oss;
    oss.precision(8);
    oss << std::fixed << d;
    std::string str = oss.str();

    std::size_t pos1 = str.find_last_not_of("0");
    if (pos1 != std::string::npos)
      str.erase(pos1 + 1);

    std::size_t pos2 = str.find_last_not_of(".");
    if (pos2 != std::string::npos)
      str.erase(pos2 + 1);

    return str;
  }
}

std::string Utils::intMaxString(int value) {
  return value == INT_MAX ? "" : std::to_string(value);
}

std::string Utils::longMaxString(long value) {
  return value == LONG_MAX ? "" : std::to_string(value);
}

std::string Utils::llongMaxString(long long value) {
  return value == LLONG_MAX ? "" : std::to_string(value);
}

std::string Utils::doubleMaxString(double d) { return doubleMaxString(d, ""); }
