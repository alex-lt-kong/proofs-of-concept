/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is
 * subject to the terms and conditions of the IB API Non-Commercial License or
 * the IB API Commercial License, as applicable. */
#include "ContractSamples.h"
#include "TestCppClient.h"
#include "twsapi/Contract.h"
#include "twsapi/EClientSocket.h"

#include <chrono>
#include <cstdio>
#include <spdlog/spdlog.h>
#include <thread>

const unsigned MAX_ATTEMPTS = 50;
const unsigned SLEEP_TIME = 10;

/* IMPORTANT: always use your paper trading account. The code below will submit
 * orders as part of the demonstration. */
/* IB will not be responsible for accidental executions on your live account. */
/* Any stock or option symbols displayed are for illustrative purposes only and
 * are not intended to portray a recommendation. */
/* Before contacting our API support team please refer to the available
 * documentation. */

int main(int argc, char **argv) {
  const char *host = argc > 1 ? argv[1] : "";
  int port = argc > 2 ? atoi(argv[2]) : 0;
  if (port <= 0)
    port = 7496;
  const char *connectOptions = argc > 3 ? argv[3] : "+PACEAPI";
  int clientId = 0;

  unsigned attempt = 0;
  spdlog::set_pattern("%Y-%m-%dT%T.%f%z | %^%8l%$ | %20! | %v");
  //spdlog::set_pattern("[source %s] [function %!] [line %#] %v");
  SPDLOG_INFO("Start of C++ Socket Client Test %u\n", attempt);

  for (;;) {
    ++attempt;
    printf("Attempt %u of %u\n", attempt, MAX_ATTEMPTS);

    TestCppClient client;

    // Run time error will occur (here) if TestCppClient.exe is compiled in
    // debug mode but TwsSocketClient.dll is compiled in Release mode
    // TwsSocketClient.dll (in Release Mode) is copied by API installer into
    // SysWOW64 folder within Windows directory

    if (connectOptions) {
      client.setConnectOptions(connectOptions);
    }

    client.connect(host, port, clientId);

    // Set market data type to DELAYED (3)
    client.m_pClient->reqMarketDataType(3);
    client.m_subscribedContracts.push_back(ContractSamples::CryptoContract());
    client.m_subscribedContracts.push_back(ContractSamples::FxUsdHkd());
    client.m_subscribedContracts.push_back(ContractSamples::FxUsdCnh());
    for (int i = 0; i < client.m_subscribedContracts.size(); ++i) {
      spdlog::info("reqTickByTickData(): {} (id: {})",
                   client.m_subscribedContracts[i].symbol, i);
      client.m_pClient->reqTickByTickData(i, client.m_subscribedContracts[i], "MidPoint", 10, false);
    }
    const auto offset = client.m_subscribedContracts.size();
    client.m_subscribedContracts.push_back(ContractSamples::HKStk0005());
    client.m_subscribedContracts.push_back(ContractSamples::HKStk3011());
    client.m_subscribedContracts.push_back(ContractSamples::IndexHsi());
    client.m_subscribedContracts.push_back(ContractSamples::IndexN225());
    client.m_subscribedContracts.push_back(ContractSamples::TWStkTsmc());
    client.m_subscribedContracts.push_back(ContractSamples::CNStkPingAn());
    client.m_subscribedContracts.push_back(
      ContractSamples::CNStkWesternMining());
    client.m_subscribedContracts.push_back(
      ContractSamples::CNStkDongfangPrecision());
    client.m_subscribedContracts.push_back(
      ContractSamples::JPStkToyota());
    client.m_subscribedContracts.push_back(ContractSamples::EUStkAsml());
    client.m_subscribedContracts.push_back(ContractSamples::UKStkVod());
    client.m_subscribedContracts.push_back(ContractSamples::UsStkNvda());
    client.m_subscribedContracts.push_back(ContractSamples::UsEtfQqq());
    client.m_subscribedContracts.push_back(ContractSamples::UsEtfBoxx());
    client.m_subscribedContracts.push_back(ContractSamples::UsEtfEzoo());
    for (auto i = offset; i < client.m_subscribedContracts.size(); ++i) {
      spdlog::info("Requesting market data for contract: {} (id: {})",
                   client.m_subscribedContracts[i].symbol, i);
      client.m_pClient->reqMktData(i, client.m_subscribedContracts[i], "",
                                   false, false, TagValueListSPtr());
    }
    while (client.isConnected()) {
      client.processMessages();
    }
    if (attempt >= MAX_ATTEMPTS) {
      break;
    }

    printf("Sleeping %u seconds before next attempt\n", SLEEP_TIME);
    std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
  }

  printf("End of C++ Socket Client Test\n");
}
