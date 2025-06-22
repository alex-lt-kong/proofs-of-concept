/* Copyright (C) 2024 Interactive Brokers LLC. All rights reserved. This code is
 * subject to the terms and conditions of the IB API Non-Commercial License or
 * the IB API Commercial License, as applicable. */

#include "Contract.h"
#include "ContractSamples.h"
#include "EClientSocket.h"

#include <chrono>
#include <cstdio>
#include <spdlog/spdlog.h>
#include <thread>

#include "TestCppClient.h"

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
    // Define a contract - Example for Tesla stock
    Contract contract;
    contract.symbol = "TSLA";
    contract.secType = "STK";
    contract.currency = "USD";
    contract.exchange = "SMART";

    // Set market data type to DELAYED (3)
    client.m_pClient->reqMarketDataType(3);
    Contract btc;
    btc.symbol = "BTC";
    btc.secType = "CRYPTO";
    btc.exchange = "OSL";
    btc.currency = "USD";
    // Request market data
    // Parameters: tickerId (unique identifier), contract, genericTickList,
    // snapshot, regulatorySnapshot
    // client.m_pClient->reqMktData(1, contract, "", false, false,
    // TagValueListSPtr());
    Contract eth;
    eth.symbol = "ETH";
    eth.secType = "CRYPTO";
    eth.exchange = "OSL";
    eth.currency = "USD";
    // https://www.interactivebrokers.com/campus/ibkr-api-page/twsapi-doc/#request-tick-data
    client.m_pClient->reqTickByTickData(1, btc, "BidAsk", 10, false);
    client.m_pClient->reqTickByTickData(2, eth, "MidPoint", 10, false);
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
