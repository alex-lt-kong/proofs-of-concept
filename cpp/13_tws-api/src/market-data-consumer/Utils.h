/* Copyright (C) 2023 Interactive Brokers LLC. All rights reserved. This code is
 * subject to the terms and conditions of the IB API Non-Commercial License or
 * the IB API Commercial License, as applicable. */
#pragma once
#ifndef TWS_API_SAMPLES_TESTCPPCLIENT_UTILS_H
#define TWS_API_SAMPLES_TESTCPPCLIENT_UTILS_H

#include <cstdint>
#include <string>
#include <vector>

class Utils {

public:
  static std::string doubleMaxString(double d, std::string def);
  static std::string doubleMaxString(double d);
  static std::string intMaxString(int value);
  static std::string longMaxString(long value);
  static std::string llongMaxString(long long value);
};

#endif
