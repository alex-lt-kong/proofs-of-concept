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
  printf("Start of C++ Socket Client Test %u\n", attempt);

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
    Contract btc;
    btc.symbol = "BTC";
    btc.secType = "CRYPTO";
    btc.exchange = "OSL";
    btc.currency = "USD";
    Contract eth;
    eth.symbol = "ETH";
    eth.secType = "CRYPTO";
    eth.exchange = "OSL";
    eth.currency = "USD";
    int reqId = 0;
    // https://www.interactivebrokers.com/campus/ibkr-api-page/twsapi-doc/#request-tick-data
    // client.m_pClient->reqTickByTickData(1, btc, "BidAsk", 10, false);
    // client.m_pClient->reqTickByTickData(2, eth, "MidPoint", 10, false);
    client.m_subscribedContracts.push_back(ContractSamples::HKStk0005());
    client.m_subscribedContracts.push_back(ContractSamples::HKStk3011());
    client.m_subscribedContracts.push_back(ContractSamples::TWStkTsmc());
    client.m_subscribedContracts.push_back(ContractSamples::CNStkPingAn());
    client.m_subscribedContracts.push_back(
        ContractSamples::CNStkWesternMining());
    client.m_subscribedContracts.push_back(
        ContractSamples::CNStkDongfangPrecision());
    client.m_subscribedContracts.push_back(ContractSamples::EUStkAsml());
    client.m_subscribedContracts.push_back(ContractSamples::UKStkVod());
    client.m_subscribedContracts.push_back(ContractSamples::UsStkNvda());
    client.m_subscribedContracts.push_back(ContractSamples::UsEtfQqq());
    client.m_subscribedContracts.push_back(ContractSamples::UsEtfBoxx());
    client.m_subscribedContracts.push_back(ContractSamples::UsEtfEzoo());
    for (int i = 0; i < client.m_subscribedContracts.size(); ++i) {
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
