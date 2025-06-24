/*Copyright(C) 2024 Interactive Brokers LLC.All rights reserved
        .This code is *subject to the terms and conditions of the IB API Non
    - Commercial License or *the IB API Commercial License,
    as applicable.*/

#include "TestCppClient.h"
#include "ContractSamples.h"
#include "Utils.h"
#include "twsapi/CommonDefs.h"
#include "twsapi/Contract.h"
#include "twsapi/EClientSocket.h"
#include "twsapi/EPosixClientSocketPlatform.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

const int PING_DEADLINE = 2; // seconds
const int SLEEP_BETWEEN_PINGS = 30; // seconds

///////////////////////////////////////////////////////////
// member funcs
//! [socket_init]
TestCppClient::TestCppClient()
    : m_osSignal(2000) // 2-seconds timeout
      ,
      m_pClient(new EClientSocket(this, &m_osSignal)), m_state(ST_CONNECT),
      m_sleepDeadline(0), m_orderId(0), m_extraAuth(false) {
}

//! [socket_init]
TestCppClient::~TestCppClient() {
    // destroy the reader before the client
    if (m_pReader)
        m_pReader.reset();

    delete m_pClient;
}

bool TestCppClient::connect(const char *host, int port, int clientId) {
    // trying to connect
    printf("Connecting to %s:%d clientId:%d\n",
           !(host && *host) ? "127.0.0.1" : host, port, clientId);

    //! [connect]
    bool bRes = m_pClient->eConnect(host, port, clientId, m_extraAuth);
    //! [connect]

    if (bRes) {
        printf("Connected to %s:%d clientId:%d\n", m_pClient->host().c_str(),
               m_pClient->port(), clientId);
        //! [ereader]
        m_pReader = std::unique_ptr<EReader>(new EReader(m_pClient, &m_osSignal));
        m_pReader->start();
        //! [ereader]
    } else
        printf("Cannot connect to %s:%d clientId:%d\n", m_pClient->host().c_str(),
               m_pClient->port(), clientId);

    return bRes;
}

void TestCppClient::disconnect() const {
    m_pClient->eDisconnect();

    printf("Disconnected\n");
}

bool TestCppClient::isConnected() const { return m_pClient->isConnected(); }

void TestCppClient::setConnectOptions(const std::string &connectOptions) {
    m_pClient->setConnectOptions(connectOptions);
}

void TestCppClient::setOptionalCapabilities(
    const std::string &optionalCapabilities) {
    m_pClient->setOptionalCapabilities(optionalCapabilities);
}

void TestCppClient::processMessages() {
    m_osSignal.waitForSignal();
    errno = 0;
    m_pReader->processMsgs();
}

//////////////////////////////////////////////////////////////////
// methods
//! [connectack]
void TestCppClient::connectAck() {
    if (!m_extraAuth && m_pClient->asyncEConnect())
        m_pClient->startApi();
}

//! [connectack]

void TestCppClient::reqCurrentTime() {
    printf("Requesting Current Time\n");

    // set ping deadline to "now + n seconds"
    m_sleepDeadline = time(NULL) + PING_DEADLINE;

    m_state = ST_PING_ACK;

    m_pClient->reqCurrentTime();
}

void TestCppClient::pnlOperation() {
}

void TestCppClient::pnlSingleOperation() {
}

void TestCppClient::tickDataOperation() {
}

void TestCppClient::tickOptionComputationOperation() {
    /*** Requesting real time market data ***/
    std::this_thread::sleep_for(std::chrono::seconds(1));

    m_pClient->reqMarketDataType(4);

    //! [reqmktdata]
    m_pClient->reqMktData(2001, ContractSamples::OptionWithLocalSymbol(), "",
                          false, false, TagValueListSPtr());
    //! [reqmktdata]

    std::this_thread::sleep_for(std::chrono::seconds(10));
    /*** Canceling the market data subscription ***/
    //! [cancelmktdata]
    m_pClient->cancelMktData(2001);
    //! [cancelmktdata]

    m_state = ST_TICKOPTIONCOMPUTATIONOPERATION_ACK;
}

void TestCppClient::delayedTickDataOperation() {
    /*** Requesting delayed market data ***/

    //! [reqmktdata_delayedmd]
    m_pClient->reqMarketDataType(4); // send delayed-frozen (4) market data type
    m_pClient->reqMktData(1013, ContractSamples::HKStk(), "", false, false,
                          TagValueListSPtr());
    m_pClient->reqMktData(1014, ContractSamples::USOptionContract(), "", false,
                          false, TagValueListSPtr());
    //! [reqmktdata_delayedmd]

    std::this_thread::sleep_for(std::chrono::seconds(10));

    /*** Canceling the delayed market data subscription ***/
    //! [cancelmktdata_delayedmd]
    m_pClient->cancelMktData(1013);
    m_pClient->cancelMktData(1014);
    //! [cancelmktdata_delayedmd]

    m_state = ST_DELAYEDTICKDATAOPERATION_ACK;
}

void TestCppClient::marketDepthOperations() {
    /*** Requesting the Deep Book ***/
    //! [reqmarketdepth]
    m_pClient->reqMktDepth(2001, ContractSamples::EurGbpFx(), 5, false,
                           TagValueListSPtr());
    //! [reqmarketdepth]
    std::this_thread::sleep_for(std::chrono::seconds(2));
    /*** Canceling the Deep Book request ***/
    //! [cancelmktdepth]
    m_pClient->cancelMktDepth(2001, false);
    //! [cancelmktdepth]

    /*** Requesting the Deep Book ***/
    //! [reqmarketdepth]
    m_pClient->reqMktDepth(2002, ContractSamples::EuropeanStock(), 5, true,
                           TagValueListSPtr());
    //! [reqmarketdepth]
    std::this_thread::sleep_for(std::chrono::seconds(5));
    /*** Canceling the Deep Book request ***/
    //! [cancelmktdepth]
    m_pClient->cancelMktDepth(2002, true);
    //! [cancelmktdepth]

    m_state = ST_MARKETDEPTHOPERATION_ACK;
}

void TestCppClient::realTimeBars() {
    /*** Requesting real time bars ***/
    //! [reqrealtimebars]
    m_pClient->reqRealTimeBars(3001, ContractSamples::EurGbpFx(), 5, "MIDPOINT",
                               true, TagValueListSPtr());
    //! [reqrealtimebars]
    std::this_thread::sleep_for(std::chrono::seconds(2));
    /*** Canceling real time bars ***/
    //! [cancelrealtimebars]
    m_pClient->cancelRealTimeBars(3001);
    //! [cancelrealtimebars]

    m_state = ST_REALTIMEBARS_ACK;
}

void TestCppClient::marketDataType() {
    //! [reqmarketdatatype]
    /*** By default only real-time (1) market data is enabled
             Sending frozen (2) enables frozen market data
             Sending delayed (3) enables delayed market data and disables
       delayed-frozen market data Sending delayed-frozen (4) enables delayed and
       delayed-frozen market data Sending real-time (1) disables frozen, delayed
       and delayed-frozen market data ***/
    m_pClient->reqMarketDataType(2);
    //! [reqmarketdatatype]

    m_state = ST_MARKETDATATYPE_ACK;
}

void TestCppClient::historicalDataRequests() {
    /*** Requesting historical data ***/
    //! [reqhistoricaldata]
    std::time_t rawtime;
    std::tm timeinfo;
    char queryTime[80];

    std::time(&rawtime);
#if defined(IB_WIN32)
  gmtime_s(&timeinfo, &rawtime);
#else
    gmtime_r(&rawtime, &timeinfo);
#endif
    std::strftime(queryTime, sizeof queryTime, "%Y%m%d-%H:%M:%S", &timeinfo);

    m_pClient->reqHistoricalData(4001, ContractSamples::EurGbpFx(), queryTime,
                                 "1 M", "1 day", "MIDPOINT", 1, 1, false,
                                 TagValueListSPtr());
    m_pClient->reqHistoricalData(4002, ContractSamples::EuropeanStock(),
                                 queryTime, "10 D", "1 min", "TRADES", 1, 1,
                                 false, TagValueListSPtr());
    m_pClient->reqHistoricalData(4003, ContractSamples::USStockAtSmart(),
                                 queryTime, "1 M", "1 day", "SCHEDULE", 1, 1,
                                 false, TagValueListSPtr());
    //! [reqhistoricaldata]
    std::this_thread::sleep_for(std::chrono::seconds(2));
    /*** Canceling historical data requests ***/
    m_pClient->cancelHistoricalData(4001);
    m_pClient->cancelHistoricalData(4002);
    m_pClient->cancelHistoricalData(4003);

    m_state = ST_HISTORICALDATAREQUESTS_ACK;
}

void TestCppClient::optionsOperations() {
    //! [reqsecdefoptparams]
    m_pClient->reqSecDefOptParams(0, "IBM", "", "STK", 8314);
    //! [reqsecdefoptparams]

    //! [calculateimpliedvolatility]
    m_pClient->calculateImpliedVolatility(
        5001, ContractSamples::OptionWithLocalSymbol(), 0.5, 55,
        TagValueListSPtr());
    //! [calculateimpliedvolatility]

    //** Canceling implied volatility ***
    m_pClient->cancelCalculateImpliedVolatility(5001);

    //! [calculateoptionprice]
    m_pClient->calculateOptionPrice(5002,
                                    ContractSamples::OptionWithLocalSymbol(), 0.6,
                                    55, TagValueListSPtr());
    //! [calculateoptionprice]

    //** Canceling option's price calculation ***
    m_pClient->cancelCalculateOptionPrice(5002);

    //! [exercise_options]
    //** Exercising options ***
    m_pClient->exerciseOptions(5003, ContractSamples::OptionWithTradingClass(), 1,
                               1, "", 1, "20231018-12:00:00", "CustAcct", true);
    //! [exercise_options]

    m_state = ST_OPTIONSOPERATIONS_ACK;
}

void TestCppClient::contractOperations() {
    m_pClient->reqContractDetails(209, ContractSamples::EurGbpFx());
    std::this_thread::sleep_for(std::chrono::seconds(2));
    //! [reqcontractdetails]
    m_pClient->reqContractDetails(210, ContractSamples::OptionForQuery());
    m_pClient->reqContractDetails(212, ContractSamples::IBMBond());
    m_pClient->reqContractDetails(213, ContractSamples::IBKRStk());
    m_pClient->reqContractDetails(214, ContractSamples::Bond());
    m_pClient->reqContractDetails(215, ContractSamples::FuturesOnOptions());
    m_pClient->reqContractDetails(216, ContractSamples::SimpleFuture());
    m_pClient->reqContractDetails(219, ContractSamples::Fund());
    m_pClient->reqContractDetails(220, ContractSamples::UsStkNvda());
    m_pClient->reqContractDetails(221, ContractSamples::USStockAtSmart());
    //! [reqcontractdetails]

    //! [reqcontractdetailsnews]
    m_pClient->reqContractDetails(211, ContractSamples::NewsFeedForQuery());
    //! [reqcontractdetailsnews]

    //! [reqcontractdetailscrypto]
    m_pClient->reqContractDetails(217, ContractSamples::CryptoContract());
    //! [reqcontractdetailscrypto]

    //! [reqcontractdetailsbyissuerid]
    m_pClient->reqContractDetails(218, ContractSamples::ByIssuerId());
    //! [reqcontractdetailsbyissuerid]

    m_state = ST_CONTRACTOPERATION_ACK;
}

void TestCppClient::marketScanners() {
}

void TestCppClient::fundamentals() {
}

void TestCppClient::bulletins() {
}

void TestCppClient::accountOperations() {
}

void TestCppClient::orderOperations() {
}

void TestCppClient::ocaSamples() {
}

void TestCppClient::conditionSamples() {
}

void TestCppClient::bracketSample() {
}

void TestCppClient::hedgeSample() {
}

void TestCppClient::testAlgoSamples() {
}

void TestCppClient::financialAdvisorOrderSamples() {
}

void TestCppClient::financialAdvisorOperations() {
}

void TestCppClient::testDisplayGroups() {
    //! [querydisplaygroups]
    m_pClient->queryDisplayGroups(9001);
    //! [querydisplaygroups]

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //! [subscribetogroupevents]
    m_pClient->subscribeToGroupEvents(9002, 1);
    //! [subscribetogroupevents]

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //! [updatedisplaygroup]
    m_pClient->updateDisplayGroup(9002, "8314@SMART");
    //! [updatedisplaygroup]

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //! [subscribefromgroupevents]
    m_pClient->unsubscribeFromGroupEvents(9002);
    //! [subscribefromgroupevents]

    m_state = ST_TICKDATAOPERATION_ACK;
}

void TestCppClient::miscelaneous() {
    /*** Request TWS' current time ***/
    m_pClient->reqCurrentTime();
    /*** Setting TWS logging level  ***/
    m_pClient->setServerLogLevel(5);

    m_state = ST_MISCELANEOUS_ACK;
}

void TestCppClient::reqFamilyCodes() {
}

void TestCppClient::reqMatchingSymbols() {
    /*** Request TWS' mathing symbols ***/
    //! [reqmatchingsymbols]
    m_pClient->reqMatchingSymbols(11001, "IBM");
    //! [reqmatchingsymbols]
    m_state = ST_SYMBOLSAMPLES_ACK;
}

void TestCppClient::reqMktDepthExchanges() {
    /*** Request TWS' market depth exchanges ***/
    //! [reqMktDepthExchanges]
    m_pClient->reqMktDepthExchanges();
    //! [reqMktDepthExchanges]

    m_state = ST_REQMKTDEPTHEXCHANGES_ACK;
}

void TestCppClient::reqNewsTicks() {
    //! [reqmktdata_ticknews]
    m_pClient->reqMktData(12001, ContractSamples::USStockAtSmart(), "mdoff,292",
                          false, false, TagValueListSPtr());
    //! [reqmktdata_ticknews]

    std::this_thread::sleep_for(std::chrono::seconds(5));

    //! [cancelmktdata2]
    m_pClient->cancelMktData(12001);
    //! [cancelmktdata2]

    m_state = ST_REQNEWSTICKS_ACK;
}

void TestCppClient::reqSmartComponents() {
    static bool bFirstRun = true;

    if (bFirstRun) {
        m_pClient->reqMktData(13001, ContractSamples::USStockAtSmart(), "", false,
                              false, TagValueListSPtr());

        bFirstRun = false;
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    if (m_bboExchange.size() > 0) {
        m_pClient->cancelMktData(13001);

        //! [reqsmartcomponents]
        m_pClient->reqSmartComponents(13002, m_bboExchange);
        //! [reqsmartcomponents]
        m_state = ST_REQSMARTCOMPONENTS_ACK;
    }
}

void TestCppClient::reqNewsProviders() {
    /*** Request TWS' news providers ***/
    //! [reqNewsProviders]
    m_pClient->reqNewsProviders();
    //! [reqNewsProviders]

    m_state = ST_NEWSPROVIDERS_ACK;
}

void TestCppClient::reqNewsArticle() {
    /*** Request TWS' news article ***/
    //! [reqNewsArticle]
    TagValueList *list = new TagValueList();
    // list->push_back((TagValueSPtr)new TagValue("manual", "1"));
    m_pClient->reqNewsArticle(12001, "MST", "MST$06f53098",
                              TagValueListSPtr(list));
    //! [reqNewsArticle]

    m_state = ST_REQNEWSARTICLE_ACK;
}

void TestCppClient::reqHistoricalNews() {
    //! [reqHistoricalNews]
    TagValueList *list = new TagValueList();
    list->push_back((TagValueSPtr) new TagValue("manual", "1"));
    m_pClient->reqHistoricalNews(12001, 8314, "BZ+FLY", "", "", 5,
                                 TagValueListSPtr(list));
    //! [reqHistoricalNews]

    std::this_thread::sleep_for(std::chrono::seconds(1));

    m_state = ST_REQHISTORICALNEWS_ACK;
}

void TestCppClient::reqHeadTimestamp() {
    //! [reqHeadTimeStamp]
    m_pClient->reqHeadTimestamp(14001, ContractSamples::EurGbpFx(), "MIDPOINT", 1,
                                1);
    //! [reqHeadTimeStamp]
    std::this_thread::sleep_for(std::chrono::seconds(1));
    //! [cancelHeadTimestamp]
    m_pClient->cancelHeadTimestamp(14001);
    //! [cancelHeadTimestamp]

    m_state = ST_REQHEADTIMESTAMP_ACK;
}

void TestCppClient::reqHistogramData() {
    //! [reqHistogramData]
    m_pClient->reqHistogramData(15001, ContractSamples::IBMUSStockAtSmart(),
                                false, "1 weeks");
    //! [reqHistogramData]
    std::this_thread::sleep_for(std::chrono::seconds(2));
    //! [cancelHistogramData]
    m_pClient->cancelHistogramData(15001);
    //! [cancelHistogramData]
    m_state = ST_REQHISTOGRAMDATA_ACK;
}

void TestCppClient::rerouteCFDOperations() {
    //! [reqmktdatacfd]
    m_pClient->reqMktData(16001, ContractSamples::USStockCFD(), "", false, false,
                          TagValueListSPtr());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    m_pClient->reqMktData(16002, ContractSamples::EuropeanStockCFD(), "", false,
                          false, TagValueListSPtr());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    m_pClient->reqMktData(16003, ContractSamples::CashCFD(), "", false, false,
                          TagValueListSPtr());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    //! [reqmktdatacfd]

    //! [reqmktdepthcfd]
    m_pClient->reqMktDepth(16004, ContractSamples::USStockCFD(), 10, false,
                           TagValueListSPtr());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    m_pClient->reqMktDepth(16005, ContractSamples::EuropeanStockCFD(), 10, false,
                           TagValueListSPtr());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    m_pClient->reqMktDepth(16006, ContractSamples::CashCFD(), 10, false,
                           TagValueListSPtr());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    //! [reqmktdepthcfd]

    m_state = ST_REROUTECFD_ACK;
}

void TestCppClient::marketRuleOperations() {
    m_pClient->reqContractDetails(17001, ContractSamples::IBMBond());
    m_pClient->reqContractDetails(17002, ContractSamples::IBKRStk());

    std::this_thread::sleep_for(std::chrono::seconds(2));

    //! [reqmarketrule]
    m_pClient->reqMarketRule(26);
    m_pClient->reqMarketRule(635);
    m_pClient->reqMarketRule(1388);
    //! [reqmarketrule]

    m_state = ST_MARKETRULE_ACK;
}

void TestCppClient::continuousFuturesOperations() {
    m_pClient->reqContractDetails(18001, ContractSamples::ContFut());

    //! [reqhistoricaldatacontfut]
    std::time_t rawtime;
    std::tm timeinfo;
    char queryTime[80];

    std::time(&rawtime);
#if defined(IB_WIN32)
  gmtime_s(&timeinfo, &rawtime);
#else
    gmtime_r(&rawtime, &timeinfo);
#endif
    std::strftime(queryTime, sizeof queryTime, "%Y%m%d %H:%M:%S", &timeinfo);

    m_pClient->reqHistoricalData(18002, ContractSamples::ContFut(), queryTime,
                                 "1 Y", "1 month", "TRADES", 0, 1, false,
                                 TagValueListSPtr());

    std::this_thread::sleep_for(std::chrono::seconds(10));

    m_pClient->cancelHistoricalData(18002);
    //! [reqhistoricaldatacontfut]

    m_state = ST_CONTFUT_ACK;
}

void TestCppClient::reqHistoricalTicks() {
    //! [reqhistoricalticks]
    m_pClient->reqHistoricalTicks(19001, ContractSamples::IBMUSStockAtSmart(),
                                  "20170621 09:38:33 US/Eastern", "", 10,
                                  "BID_ASK", 1, true, TagValueListSPtr());
    m_pClient->reqHistoricalTicks(19002, ContractSamples::IBMUSStockAtSmart(),
                                  "20170621 09:38:33 US/Eastern", "", 10,
                                  "MIDPOINT", 1, true, TagValueListSPtr());
    m_pClient->reqHistoricalTicks(19003, ContractSamples::IBMUSStockAtSmart(),
                                  "20170621 09:38:33 US/Eastern", "", 10,
                                  "TRADES", 1, true, TagValueListSPtr());
    //! [reqhistoricalticks]
    m_state = ST_REQHISTORICALTICKS_ACK;
}

void TestCppClient::reqTickByTickData() {
    /*** Requesting tick-by-tick data (only refresh) ***/

    m_pClient->reqTickByTickData(20001, ContractSamples::EuropeanStock(), "Last",
                                 0, false);
    m_pClient->reqTickByTickData(20002, ContractSamples::EuropeanStock(),
                                 "AllLast", 0, false);
    m_pClient->reqTickByTickData(20003, ContractSamples::EuropeanStock(),
                                 "BidAsk", 0, true);
    m_pClient->reqTickByTickData(20004, ContractSamples::EurGbpFx(), "MidPoint",
                                 0, false);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    //! [canceltickbytick]
    m_pClient->cancelTickByTickData(20001);
    m_pClient->cancelTickByTickData(20002);
    m_pClient->cancelTickByTickData(20003);
    m_pClient->cancelTickByTickData(20004);
    //! [canceltickbytick]

    /*** Requesting tick-by-tick data (historical + refresh) ***/
    //! [reqtickbytick]
    m_pClient->reqTickByTickData(20005, ContractSamples::EuropeanStock(), "Last",
                                 10, false);
    m_pClient->reqTickByTickData(20006, ContractSamples::EuropeanStock(),
                                 "AllLast", 10, false);
    m_pClient->reqTickByTickData(20007, ContractSamples::EuropeanStock(),
                                 "BidAsk", 10, false);
    m_pClient->reqTickByTickData(20008, ContractSamples::EurGbpFx(), "MidPoint",
                                 10, true);
    //! [reqtickbytick]

    std::this_thread::sleep_for(std::chrono::seconds(10));

    m_pClient->cancelTickByTickData(20005);
    m_pClient->cancelTickByTickData(20006);
    m_pClient->cancelTickByTickData(20007);
    m_pClient->cancelTickByTickData(20008);

    m_state = ST_REQTICKBYTICKDATA_ACK;
}

void TestCppClient::whatIfSamples() {
}

void TestCppClient::ibkratsSample() {
}

void TestCppClient::wshCalendarOperations() {
    //! [reqmetadata]
    m_pClient->reqWshMetaData(30001);
    //! [reqmetadata]

    std::this_thread::sleep_for(std::chrono::seconds(10));

    m_pClient->cancelWshMetaData(30001);

    //! [reqeventdata]
    m_pClient->reqWshEventData(
        30002, WshEventData(8314, false, false, false, "20220511", "", 5));
    //! [reqeventdata]

    std::this_thread::sleep_for(std::chrono::seconds(3));

    //! [reqeventdata]
    m_pClient->reqWshEventData(30003, WshEventData("{\"watchlist\":[\"8314\"]}",
                                                   false, false, false, "",
                                                   "20220512", INT_MAX));
    //! [reqeventdata]

    std::this_thread::sleep_for(std::chrono::seconds(10));

    m_pClient->cancelWshEventData(30002);
    m_pClient->cancelWshEventData(30003);

    m_state = ST_WSH_ACK;
}

void TestCppClient::rfqOperations() {
}

//! [nextvalidid]
void TestCppClient::nextValidId(OrderId orderId) {
    printf("Next Valid Id: %ld\n", orderId);
    m_orderId = orderId;
    //! [nextvalidid]

    // m_state = ST_TICKOPTIONCOMPUTATIONOPERATION;
    // m_state = ST_TICKDATAOPERATION;
    // m_state = ST_OPTIONSOPERATIONS;
    // m_state = ST_REQTICKBYTICKDATA;
    // m_state = ST_REQHISTORICALTICKS;
    // m_state = ST_CONTFUT;
    // m_state = ST_PNLSINGLE;
    // m_state = ST_PNL;
    // m_state = ST_DELAYEDTICKDATAOPERATION;
    // m_state = ST_MARKETDEPTHOPERATION;
    // m_state = ST_REALTIMEBARS;
    // m_state = ST_MARKETDATATYPE;
    // m_state = ST_HISTORICALDATAREQUESTS;
    // m_state = ST_CONTRACTOPERATION;
    // m_state = ST_MARKETSCANNERS;
    // m_state = ST_FUNDAMENTALS;
    // m_state = ST_BULLETINS;
    // m_state = ST_ACCOUNTOPERATIONS;
    // m_state = ST_ORDEROPERATIONS;
    // m_state = ST_OCASAMPLES;
    // m_state = ST_CONDITIONSAMPLES;
    // m_state = ST_BRACKETSAMPLES;
    // m_state = ST_HEDGESAMPLES;
    // m_state = ST_TESTALGOSAMPLES;
    // m_state = ST_FAORDERSAMPLES;
    // m_state = ST_FAOPERATIONS;
    // m_state = ST_DISPLAYGROUPS;
    // m_state = ST_MISCELANEOUS;
    // m_state = ST_FAMILYCODES;
    // m_state = ST_SYMBOLSAMPLES;
    // m_state = ST_REQMKTDEPTHEXCHANGES;
    // m_state = ST_REQNEWSTICKS;
    // m_state = ST_REQSMARTCOMPONENTS;
    // m_state = ST_NEWSPROVIDERS;
    // m_state = ST_REQNEWSARTICLE;
    // m_state = ST_REQHISTORICALNEWS;
    // m_state = ST_REQHEADTIMESTAMP;
    // m_state = ST_REQHISTOGRAMDATA;
    // m_state = ST_REROUTECFD;
    // m_state = ST_MARKETRULE;
    // m_state = ST_PING;
    // m_state = ST_WHATIFSAMPLES;
    // m_state = ST_WSH;
    m_state = ST_RFQOPERATIONS;
}

void TestCppClient::currentTime(long time) {
    if (m_state == ST_PING_ACK) {
        time_t t = (time_t) time;
        struct tm timeinfo;
        char currentTime[80];

#if defined(IB_WIN32)
    localtime_s(&timeinfo, &t);
    asctime_s(currentTime, sizeof currentTime, &timeinfo);
#else
        localtime_r(&t, &timeinfo);
        asctime_r(&timeinfo, currentTime);
#endif
        printf("The current date/time is: %s", currentTime);

        time_t now = ::time(NULL);
        m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;

        m_state = ST_PING_ACK;
    }
}

//! [error]
void TestCppClient::error(int id, int errorCode, const std::string &errorString,
                          const std::string &advancedOrderRejectJson) {
    if (id == -1) {
        SPDLOG_INFO("Code: {}, Msg: {}", id, errorString);
    } else if (!advancedOrderRejectJson.empty()) {
        printf("Error. Id: %d, Code: %d, Msg: %s, AdvancedOrderRejectJson: "
               "%s\n",
               id, errorCode, errorString.c_str(), advancedOrderRejectJson.c_str());
    } else {
        SPDLOG_ERROR("Id: {}, Code: {}, Msg: {}", id, errorCode, errorString);
    }
}

//! [error]


void TestCppClient::tickPrice(TickerId tickerId, TickType field, double price,
                              const TickAttrib &attribs) {
    std::string tickTypeName = std::to_string(field);
    if (field == TickType::DELAYED_LAST)
        tickTypeName = "DELAYED_LAST";
    else if (field == TickType::DELAYED_ASK)
        tickTypeName = "DELAYED_ASK";
    else if (field == TickType::DELAYED_BID)
        tickTypeName = "DELAYED_BID";
    else if (field == DELAYED_OPEN || DELAYED_HIGH || DELAYED_LOW || DELAYED_CLOSE) return;
    SPDLOG_INFO("Symbol: {:18}, {:15}: {:>8}",
                std::format("{:7} ({})", m_subscribedContracts[tickerId].symbol,
                    m_subscribedContracts[tickerId].exchange),
                tickTypeName, Utils::doubleMaxString(price));
}

//! [ticksize]
void TestCppClient::tickSize(TickerId tickerId, TickType field, Decimal size) {
    return;
    std::string tickTypeName = std::to_string(field);
    if (field == TickType::DELAYED_VOLUME)
        tickTypeName = "DELAYED_VOLUME";
    else if (field == TickType::DELAYED_ASK_SIZE)
        tickTypeName = "DELAYED_ASK_SIZE";
    else if (field == TickType::DELAYED_BID_SIZE)
        tickTypeName = "DELAYED_BID_SIZE";
    else if (field == TickType::DELAYED_LAST_SIZE)
        tickTypeName = "DELAYED_LAST_SIZE";
    SPDLOG_INFO("tickSize() Ticker Id: {}, TickType: {}, Size: {}", tickerId,
                tickTypeName, DecimalFunctions::decimalStringToDisplay(size));
}

//! [ticksize]

void TestCppClient::tickOptionComputation(TickerId tickerId, TickType tickType,
                                          int tickAttrib, double impliedVol,
                                          double delta, double optPrice,
                                          double pvDividend, double gamma,
                                          double vega, double theta,
                                          double undPrice) {
}

//! [tickgeneric]
void TestCppClient::tickGeneric(TickerId tickerId, TickType tickType,
                                double value) {
    if (tickType == TickType::DELAYED_HALTED) {
        SPDLOG_INFO("Symbol: {}, DELAYED_HALTED: {}", m_subscribedContracts[tickerId].symbol,
                    Utils::doubleMaxString(value));
    } else {
        printf("Tick Generic. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId,
               (int) tickType, Utils::doubleMaxString(value).c_str());
    }
}

//! [tickgeneric]

//! [tickstring]
void TestCppClient::tickString(TickerId tickerId, TickType tickType,
                               const std::string &value) {
    return;
    if (tickType == DELAYED_LAST_TIMESTAMP) {
        std::tm tm{};
        const time_t t = stoi(value);
        localtime_r(&t, &tm);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
        SPDLOG_INFO("tickString(DELAYED_LAST_TIMESTAMP) Ticker Id: {}, Value: {}",
                    tickerId, oss.str());
    } else if (tickType == LAST_TIMESTAMP) {
        std::tm tm{};
        const time_t t = stoi(value);
        localtime_r(&t, &tm);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
        SPDLOG_INFO("tickString(LAST_TIMESTAMP) Ticker Id: {}, Value: {}",
                    tickerId, oss.str());
    } else if (tickType == HALTED && value == "0") {
    } else {
        printf("Tick String. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId,
               (int) tickType, value.c_str());
    }
}

//! [tickstring]

void TestCppClient::tickEFP(TickerId tickerId, TickType tickType,
                            double basisPoints,
                            const std::string &formattedBasisPoints,
                            double totalDividends, int holdDays,
                            const std::string &futureLastTradeDate,
                            double dividendImpact,
                            double dividendsToLastTradeDate) {
    printf("TickEFP. %ld, Type: %d, BasisPoints: %s, FormattedBasisPoints: %s, "
           "Total Dividends: %s, HoldDays: %s, Future Last Trade Date: %s, "
           "Dividend Impact: %s, Dividends To Last Trade Date: %s\n",
           tickerId, (int) tickType, Utils::doubleMaxString(basisPoints).c_str(),
           formattedBasisPoints.c_str(),
           Utils::doubleMaxString(totalDividends).c_str(),
           Utils::intMaxString(holdDays).c_str(), futureLastTradeDate.c_str(),
           Utils::doubleMaxString(dividendImpact).c_str(),
           Utils::doubleMaxString(dividendsToLastTradeDate).c_str());
}

//! [orderstatus]
void TestCppClient::orderStatus(OrderId orderId, const std::string &status,
                                Decimal filled, Decimal remaining,
                                double avgFillPrice, int permId, int parentId,
                                double lastFillPrice, int clientId,
                                const std::string &whyHeld,
                                double mktCapPrice) {
    printf("OrderStatus. Id: %ld, Status: %s, Filled: %s, Remaining: %s, "
           "AvgFillPrice: %s, PermId: %s, LastFillPrice: %s, ClientId: %s, "
           "WhyHeld: %s, MktCapPrice: %s\n",
           orderId, status.c_str(),
           DecimalFunctions::decimalStringToDisplay(filled).c_str(),
           DecimalFunctions::decimalStringToDisplay(remaining).c_str(),
           Utils::doubleMaxString(avgFillPrice).c_str(),
           Utils::intMaxString(permId).c_str(),
           Utils::doubleMaxString(lastFillPrice).c_str(),
           Utils::intMaxString(clientId).c_str(), whyHeld.c_str(),
           Utils::doubleMaxString(mktCapPrice).c_str());
}

//! [orderstatus]

//! [openorder]
void TestCppClient::openOrder(OrderId orderId, const Contract &contract,
                              const Order &order,
                              const OrderState &orderState) {
}

//! [openorder]

//! [openorderend]
void TestCppClient::openOrderEnd() { printf("OpenOrderEnd\n"); }
//! [openorderend]

void TestCppClient::winError(const std::string &str, int lastError) {
}

void TestCppClient::connectionClosed() { printf("Connection Closed\n"); }

//! [updateaccountvalue]
void TestCppClient::updateAccountValue(const std::string &key,
                                       const std::string &val,
                                       const std::string &currency,
                                       const std::string &accountName) {
    printf("UpdateAccountValue. Key: %s, Value: %s, Currency: %s, Account "
           "Name: "
           "%s\n",
           key.c_str(), val.c_str(), currency.c_str(), accountName.c_str());
}

//! [updateaccountvalue]

//! [updateportfolio]
void TestCppClient::updatePortfolio(const Contract &contract, Decimal position,
                                    double marketPrice, double marketValue,
                                    double averageCost, double unrealizedPNL,
                                    double realizedPNL,
                                    const std::string &accountName) {
    printf("UpdatePortfolio. %s, %s @ %s: Position: %s, MarketPrice: %s, "
           "MarketValue: %s, AverageCost: %s, UnrealizedPNL: %s, RealizedPNL: "
           "%s, AccountName: %s\n",
           (contract.symbol).c_str(), (contract.secType).c_str(),
           (contract.primaryExchange).c_str(),
           DecimalFunctions::decimalStringToDisplay(position).c_str(),
           Utils::doubleMaxString(marketPrice).c_str(),
           Utils::doubleMaxString(marketValue).c_str(),
           Utils::doubleMaxString(averageCost).c_str(),
           Utils::doubleMaxString(unrealizedPNL).c_str(),
           Utils::doubleMaxString(realizedPNL).c_str(), accountName.c_str());
}

//! [updateportfolio]

//! [updateaccounttime]
void TestCppClient::updateAccountTime(const std::string &timeStamp) {
    printf("UpdateAccountTime. Time: %s\n", timeStamp.c_str());
}

//! [updateaccounttime]

//! [accountdownloadend]
void TestCppClient::accountDownloadEnd(const std::string &accountName) {
    printf("Account download finished: %s\n", accountName.c_str());
}

//! [accountdownloadend]

//! [contractdetails]
void TestCppClient::contractDetails(int reqId,
                                    const ContractDetails &contractDetails) {
    printf("ContractDetails begin. ReqId: %d\n", reqId);
    printContractMsg(contractDetails.contract);
    printContractDetailsMsg(contractDetails);
    printf("ContractDetails end. ReqId: %d\n", reqId);
}

//! [contractdetails]

//! [bondcontractdetails]
void TestCppClient::bondContractDetails(
    int reqId, const ContractDetails &contractDetails) {
    printf("BondContractDetails begin. ReqId: %d\n", reqId);
    printBondContractDetailsMsg(contractDetails);
    printf("BondContractDetails end. ReqId: %d\n", reqId);
}

//! [bondcontractdetails]

void TestCppClient::printContractMsg(const Contract &contract) {
    printf("\tConId: %ld\n", contract.conId);
    printf("\tSymbol: %s\n", contract.symbol.c_str());
    printf("\tSecType: %s\n", contract.secType.c_str());
    printf("\tLastTradeDateOrContractMonth: %s\n",
           contract.lastTradeDateOrContractMonth.c_str());
    printf("\tLastTradeDate: %s\n", contract.lastTradeDate.c_str());
    printf("\tStrike: %s\n", Utils::doubleMaxString(contract.strike).c_str());
    printf("\tRight: %s\n", contract.right.c_str());
    printf("\tMultiplier: %s\n", contract.multiplier.c_str());
    printf("\tExchange: %s\n", contract.exchange.c_str());
    printf("\tPrimaryExchange: %s\n", contract.primaryExchange.c_str());
    printf("\tCurrency: %s\n", contract.currency.c_str());
    printf("\tLocalSymbol: %s\n", contract.localSymbol.c_str());
    printf("\tTradingClass: %s\n", contract.tradingClass.c_str());
}

void TestCppClient::printContractDetailsMsg(
    const ContractDetails &contractDetails) {
}

void TestCppClient::printContractDetailsSecIdList(
    const TagValueListSPtr &secIdList) {
}

void TestCppClient::printContractDetailsIneligibilityReasonList(
    const IneligibilityReasonListSPtr &ineligibilityReasonList) {
    const int ineligibilityReasonListCount =
            ineligibilityReasonList.get() ? ineligibilityReasonList->size() : 0;
    if (ineligibilityReasonListCount > 0) {
        printf("\tIneligibilityReasonList: {");
        for (int i = 0; i < ineligibilityReasonListCount; ++i) {
            const IneligibilityReason *ineligibilityReason =
                    ((*ineligibilityReasonList)[i]).get();
            printf("[id: %s, description: %s];", ineligibilityReason->id.c_str(),
                   ineligibilityReason->description.c_str());
        }
        printf("}\n");
    }
}

void TestCppClient::printBondContractDetailsMsg(
    const ContractDetails &contractDetails) {
    printf("\tSymbol: %s\n", contractDetails.contract.symbol.c_str());
    printf("\tSecType: %s\n", contractDetails.contract.secType.c_str());
    printf("\tCusip: %s\n", contractDetails.cusip.c_str());
    printf("\tCoupon: %s\n",
           Utils::doubleMaxString(contractDetails.coupon).c_str());
    printf("\tMaturity: %s\n", contractDetails.maturity.c_str());
    printf("\tIssueDate: %s\n", contractDetails.issueDate.c_str());
    printf("\tRatings: %s\n", contractDetails.ratings.c_str());
    printf("\tBondType: %s\n", contractDetails.bondType.c_str());
    printf("\tCouponType: %s\n", contractDetails.couponType.c_str());
    printf("\tConvertible: %s\n", contractDetails.convertible ? "yes" : "no");
    printf("\tCallable: %s\n", contractDetails.callable ? "yes" : "no");
    printf("\tPutable: %s\n", contractDetails.putable ? "yes" : "no");
    printf("\tDescAppend: %s\n", contractDetails.descAppend.c_str());
    printf("\tExchange: %s\n", contractDetails.contract.exchange.c_str());
    printf("\tCurrency: %s\n", contractDetails.contract.currency.c_str());
    printf("\tMarketName: %s\n", contractDetails.marketName.c_str());
    printf("\tTradingClass: %s\n", contractDetails.contract.tradingClass.c_str());
    printf("\tConId: %s\n",
           Utils::longMaxString(contractDetails.contract.conId).c_str());
    printf("\tMinTick: %s\n",
           Utils::doubleMaxString(contractDetails.minTick).c_str());
    printf("\tOrderTypes: %s\n", contractDetails.orderTypes.c_str());
    printf("\tValidExchanges: %s\n", contractDetails.validExchanges.c_str());
    printf("\tNextOptionDate: %s\n", contractDetails.nextOptionDate.c_str());
    printf("\tNextOptionType: %s\n", contractDetails.nextOptionType.c_str());
    printf("\tNextOptionPartial: %s\n",
           contractDetails.nextOptionPartial ? "yes" : "no");
    printf("\tNotes: %s\n", contractDetails.notes.c_str());
    printf("\tLong Name: %s\n", contractDetails.longName.c_str());
    printf("\tEvRule: %s\n", contractDetails.evRule.c_str());
    printf("\tEvMultiplier: %s\n",
           Utils::doubleMaxString(contractDetails.evMultiplier).c_str());
    printf("\tAggGroup: %s\n",
           Utils::intMaxString(contractDetails.aggGroup).c_str());
    printf("\tMarketRuleIds: %s\n", contractDetails.marketRuleIds.c_str());
    printf("\tTimeZoneId: %s\n", contractDetails.timeZoneId.c_str());
    printf("\tLastTradeTime: %s\n", contractDetails.lastTradeTime.c_str());
    printf("\tMinSize: %s\n",
           DecimalFunctions::decimalStringToDisplay(contractDetails.minSize)
           .c_str());
    printf("\tSizeIncrement: %s\n",
           DecimalFunctions::decimalStringToDisplay(contractDetails.sizeIncrement)
           .c_str());
    printf("\tSuggestedSizeIncrement: %s\n",
           DecimalFunctions::decimalStringToDisplay(
               contractDetails.suggestedSizeIncrement)
           .c_str());
    printContractDetailsSecIdList(contractDetails.secIdList);
}

//! [contractdetailsend]
void TestCppClient::contractDetailsEnd(int reqId) {
    printf("ContractDetailsEnd. %d\n", reqId);
}

//! [contractdetailsend]

//! [execdetails]
void TestCppClient::execDetails(int reqId, const Contract &contract,
                                const Execution &execution) {
}

//! [execdetails]

//! [execdetailsend]
void TestCppClient::execDetailsEnd(int reqId) {
    printf("ExecDetailsEnd. %d\n", reqId);
}

//! [execdetailsend]

//! [updatemktdepth]
void TestCppClient::updateMktDepth(TickerId id, int position, int operation,
                                   int side, double price, Decimal size) {
    printf("UpdateMarketDepth. %ld - Position: %s, Operation: %d, Side: %d, "
           "Price: %s, Size: %s\n",
           id, Utils::intMaxString(position).c_str(), operation, side,
           Utils::doubleMaxString(price).c_str(),
           DecimalFunctions::decimalStringToDisplay(size).c_str());
}

//! [updatemktdepth]

//! [updatemktdepthl2]
void TestCppClient::updateMktDepthL2(TickerId id, int position,
                                     const std::string &marketMaker,
                                     int operation, int side, double price,
                                     Decimal size, bool isSmartDepth) {
    printf("UpdateMarketDepthL2. %ld - Position: %s, Operation: %d, Side: %d, "
           "Price: %s, Size: %s, isSmartDepth: %d\n",
           id, Utils::intMaxString(position).c_str(), operation, side,
           Utils::doubleMaxString(price).c_str(),
           DecimalFunctions::decimalStringToDisplay(size).c_str(), isSmartDepth);
}

//! [updatemktdepthl2]

//! [updatenewsbulletin]
void TestCppClient::updateNewsBulletin(int msgId, int msgType,
                                       const std::string &newsMessage,
                                       const std::string &originExch) {
    printf("News Bulletins. %d - Type: %d, Message: %s, Exchange of Origin: "
           "%s\n",
           msgId, msgType, newsMessage.c_str(), originExch.c_str());
}

//! [updatenewsbulletin]

//! [managedaccounts]
void TestCppClient::managedAccounts(const std::string &accountsList) {
    printf("Account List: %s\n", accountsList.c_str());
}

//! [managedaccounts]

//! [receivefa]
void TestCppClient::receiveFA(faDataType pFaDataType, const std::string &cxml) {
    std::cout << "Receiving FA: " << (int) pFaDataType << std::endl
            << cxml << std::endl;
}

//! [receivefa]

//! [historicaldata]
void TestCppClient::historicalData(TickerId reqId, const Bar &bar) {
    printf("HistoricalData. ReqId: %ld - Date: %s, Open: %s, High: %s, Low: "
           "%s, "
           "Close: %s, Volume: %s, Count: %s, WAP: %s\n",
           reqId, bar.time.c_str(), Utils::doubleMaxString(bar.open).c_str(),
           Utils::doubleMaxString(bar.high).c_str(),
           Utils::doubleMaxString(bar.low).c_str(),
           Utils::doubleMaxString(bar.close).c_str(),
           DecimalFunctions::decimalStringToDisplay(bar.volume).c_str(),
           Utils::intMaxString(bar.count).c_str(),
           DecimalFunctions::decimalStringToDisplay(bar.wap).c_str());
}

//! [historicaldata]

//! [historicaldataend]
void TestCppClient::historicalDataEnd(int reqId,
                                      const std::string &startDateStr,
                                      const std::string &endDateStr) {
    std::cout << "HistoricalDataEnd. ReqId: " << reqId
            << " - Start Date: " << startDateStr << ", End Date: " << endDateStr
            << std::endl;
}

//! [historicaldataend]

//! [scannerparameters]
void TestCppClient::scannerParameters(const std::string &xml) {
    printf("ScannerParameters. %s\n", xml.c_str());
}

//! [scannerparameters]

//! [scannerdata]
void TestCppClient::scannerData(int reqId, int rank,
                                const ContractDetails &contractDetails,
                                const std::string &distance,
                                const std::string &benchmark,
                                const std::string &projection,
                                const std::string &legsStr) {
    printf("ScannerData. %d - Rank: %d, Symbol: %s, SecType: %s, Currency: %s, "
           "Distance: %s, Benchmark: %s, Projection: %s, Legs String: %s\n",
           reqId, rank, contractDetails.contract.symbol.c_str(),
           contractDetails.contract.secType.c_str(),
           contractDetails.contract.currency.c_str(), distance.c_str(),
           benchmark.c_str(), projection.c_str(), legsStr.c_str());
}

//! [scannerdata]

//! [scannerdataend]
void TestCppClient::scannerDataEnd(int reqId) {
    printf("ScannerDataEnd. %d\n", reqId);
}

//! [scannerdataend]

//! [realtimebar]
void TestCppClient::realtimeBar(TickerId reqId, long time, double open,
                                double high, double low, double close,
                                Decimal volume, Decimal wap, int count) {
    printf("RealTimeBars. %ld - Time: %s, Open: %s, High: %s, Low: %s, Close: "
           "%s, Volume: %s, Count: %s, WAP: %s\n",
           reqId, Utils::longMaxString(time).c_str(),
           Utils::doubleMaxString(open).c_str(),
           Utils::doubleMaxString(high).c_str(),
           Utils::doubleMaxString(low).c_str(),
           Utils::doubleMaxString(close).c_str(),
           DecimalFunctions::decimalStringToDisplay(volume).c_str(),
           Utils::intMaxString(count).c_str(),
           DecimalFunctions::decimalStringToDisplay(wap).c_str());
}

//! [realtimebar]

//! [fundamentaldata]
void TestCppClient::fundamentalData(TickerId reqId, const std::string &data) {
    printf("FundamentalData. ReqId: %ld, %s\n", reqId, data.c_str());
}

//! [fundamentaldata]

void TestCppClient::deltaNeutralValidation(
    int reqId, const DeltaNeutralContract &deltaNeutralContract) {
}

//! [ticksnapshotend]
void TestCppClient::tickSnapshotEnd(int reqId) {
    printf("TickSnapshotEnd: %d\n", reqId);
}

//! [ticksnapshotend]

void TestCppClient::marketDataType(TickerId reqId, int marketDataType) {
    if (marketDataType != 1 && reqId >= 0)
        SPDLOG_WARN("Symbol: {}, MarketDataType: {}", m_subscribedContracts[reqId].symbol, marketDataType);
}

//! [commissionreport]
void TestCppClient::commissionReport(const CommissionReport &commissionReport) {
}

//! [commissionreport]

//! [position]
void TestCppClient::position(const std::string &account,
                             const Contract &contract, Decimal position,
                             double avgCost) {
    printf("Position. %s - Symbol: %s, SecType: %s, Currency: %s, Position: "
           "%s, "
           "Avg Cost: %s\n",
           account.c_str(), contract.symbol.c_str(), contract.secType.c_str(),
           contract.currency.c_str(),
           DecimalFunctions::decimalStringToDisplay(position).c_str(),
           Utils::doubleMaxString(avgCost).c_str());
}

//! [position]

//! [positionend]
void TestCppClient::positionEnd() { printf("PositionEnd\n"); }
//! [positionend]

//! [accountsummary]
void TestCppClient::accountSummary(int reqId, const std::string &account,
                                   const std::string &tag,
                                   const std::string &value,
                                   const std::string &currency) {
    printf("Acct Summary. ReqId: %d, Account: %s, Tag: %s, Value: %s, "
           "Currency: "
           "%s\n",
           reqId, account.c_str(), tag.c_str(), value.c_str(), currency.c_str());
}

//! [accountsummary]

//! [accountsummaryend]
void TestCppClient::accountSummaryEnd(int reqId) {
    printf("AccountSummaryEnd. Req Id: %d\n", reqId);
}

//! [accountsummaryend]

void TestCppClient::verifyMessageAPI(const std::string &apiData) {
    printf("verifyMessageAPI: %s\b", apiData.c_str());
}

void TestCppClient::verifyCompleted(bool isSuccessful,
                                    const std::string &errorText) {
    printf("verifyCompleted. IsSuccessfule: %d - Error: %s\n", isSuccessful,
           errorText.c_str());
}

void TestCppClient::verifyAndAuthMessageAPI(const std::string &apiDatai,
                                            const std::string &xyzChallenge) {
    printf("verifyAndAuthMessageAPI: %s %s\n", apiDatai.c_str(),
           xyzChallenge.c_str());
}

void TestCppClient::verifyAndAuthCompleted(bool isSuccessful,
                                           const std::string &errorText) {
    printf("verifyAndAuthCompleted. IsSuccessful: %d - Error: %s\n", isSuccessful,
           errorText.c_str());
    if (isSuccessful)
        m_pClient->startApi();
}

//! [displaygrouplist]
void TestCppClient::displayGroupList(int reqId, const std::string &groups) {
    printf("Display Group List. ReqId: %d, Groups: %s\n", reqId, groups.c_str());
}

//! [displaygrouplist]

//! [displaygroupupdated]
void TestCppClient::displayGroupUpdated(int reqId,
                                        const std::string &contractInfo) {
    std::cout << "Display Group Updated. ReqId: " << reqId
            << ", Contract Info: " << contractInfo << std::endl;
}

//! [displaygroupupdated]

//! [positionmulti]
void TestCppClient::positionMulti(int reqId, const std::string &account,
                                  const std::string &modelCode,
                                  const Contract &contract, Decimal pos,
                                  double avgCost) {
    printf("Position Multi. Request: %d, Account: %s, ModelCode: %s, Symbol: "
           "%s, "
           "SecType: %s, Currency: %s, Position: %s, Avg Cost: %s\n",
           reqId, account.c_str(), modelCode.c_str(), contract.symbol.c_str(),
           contract.secType.c_str(), contract.currency.c_str(),
           DecimalFunctions::decimalStringToDisplay(pos).c_str(),
           Utils::doubleMaxString(avgCost).c_str());
}

//! [positionmulti]

//! [positionmultiend]
void TestCppClient::positionMultiEnd(int reqId) {
    printf("Position Multi End. Request: %d\n", reqId);
}

//! [positionmultiend]

//! [accountupdatemulti]
void TestCppClient::accountUpdateMulti(int reqId, const std::string &account,
                                       const std::string &modelCode,
                                       const std::string &key,
                                       const std::string &value,
                                       const std::string &currency) {
    printf("AccountUpdate Multi. Request: %d, Account: %s, ModelCode: %s, Key, "
           "%s, Value: %s, Currency: %s\n",
           reqId, account.c_str(), modelCode.c_str(), key.c_str(), value.c_str(),
           currency.c_str());
}

//! [accountupdatemulti]

//! [accountupdatemultiend]
void TestCppClient::accountUpdateMultiEnd(int reqId) {
    printf("Account Update Multi End. Request: %d\n", reqId);
}

//! [accountupdatemultiend]

//! [securityDefinitionOptionParameter]
void TestCppClient::securityDefinitionOptionalParameter(
    int reqId, const std::string &exchange, int underlyingConId,
    const std::string &tradingClass, const std::string &multiplier,
    const std::set<std::string> &expirations, const std::set<double> &strikes) {
    printf("Security Definition Optional Parameter. Request: %d, Trading "
           "Class: "
           "%s, Multiplier: %s\n",
           reqId, tradingClass.c_str(), multiplier.c_str());
}

//! [securityDefinitionOptionParameter]

//! [securityDefinitionOptionParameterEnd]
void TestCppClient::securityDefinitionOptionalParameterEnd(int reqId) {
    printf("Security Definition Optional Parameter End. Request: %d\n", reqId);
}

//! [securityDefinitionOptionParameterEnd]

//! [softDollarTiers]
void TestCppClient::softDollarTiers(int reqId,
                                    const std::vector<SoftDollarTier> &tiers) {
}

//! [softDollarTiers]

//! [familyCodes]
void TestCppClient::familyCodes(const std::vector<FamilyCode> &familyCodes) {
}

//! [familyCodes]

//! [symbolSamples]
void TestCppClient::symbolSamples(
    int reqId, const std::vector<ContractDescription> &contractDescriptions) {
    printf("Symbol Samples (total=%lu) reqId: %d\n", contractDescriptions.size(),
           reqId);

    for (unsigned int i = 0; i < contractDescriptions.size(); i++) {
        Contract contract = contractDescriptions[i].contract;
        std::vector<std::string> derivativeSecTypes =
                contractDescriptions[i].derivativeSecTypes;
        printf("Contract (%u): conId: %ld, symbol: %s, secType: %s, "
               "primaryExchange: %s, currency: %s, ",
               i, contract.conId, contract.symbol.c_str(), contract.secType.c_str(),
               contract.primaryExchange.c_str(), contract.currency.c_str());
        printf("Derivative Sec-types (%lu):", derivativeSecTypes.size());
        for (unsigned int j = 0; j < derivativeSecTypes.size(); j++) {
            printf(" %s", derivativeSecTypes[j].c_str());
        }
        printf(", description: %s, issuerId: %s", contract.description.c_str(),
               contract.issuerId.c_str());
        printf("\n");
    }
}

//! [symbolSamples]

//! [mktDepthExchanges]
void TestCppClient::mktDepthExchanges(
    const std::vector<DepthMktDataDescription> &depthMktDataDescriptions) {
    printf("Mkt Depth Exchanges (%lu):\n", depthMktDataDescriptions.size());

    for (unsigned int i = 0; i < depthMktDataDescriptions.size(); i++) {
        printf("Depth Mkt Data Description [%d] - exchange: %s secType: %s "
               "listingExch: %s serviceDataType: %s aggGroup: %s\n",
               i, depthMktDataDescriptions[i].exchange.c_str(),
               depthMktDataDescriptions[i].secType.c_str(),
               depthMktDataDescriptions[i].listingExch.c_str(),
               depthMktDataDescriptions[i].serviceDataType.c_str(),
               Utils::intMaxString(depthMktDataDescriptions[i].aggGroup).c_str());
    }
}

//! [mktDepthExchanges]

//! [tickNews]
void TestCppClient::tickNews(int tickerId, time_t timeStamp,
                             const std::string &providerCode,
                             const std::string &articleId,
                             const std::string &headline,
                             const std::string &extraData) {
    char timeStampStr[80];
#if defined(IB_WIN32)
  ctime_s(timeStampStr, sizeof(timeStampStr), &(timeStamp /= 1000));
#else
    ctime_r(&(timeStamp /= 1000), timeStampStr);
#endif
    printf("News Tick. TickerId: %d, TimeStamp: %s, ProviderCode: %s, "
           "ArticleId: "
           "%s, Headline: %s, ExtraData: %s\n",
           tickerId, timeStampStr, providerCode.c_str(), articleId.c_str(),
           headline.c_str(), extraData.c_str());
}

//! [tickNews]

//! [smartcomponents]]
void TestCppClient::smartComponents(int reqId,
                                    const SmartComponentsMap &theMap) {
    printf("Smart components: (%lu):\n", theMap.size());

    for (SmartComponentsMap::const_iterator i = theMap.begin(); i != theMap.end();
         i++) {
        printf(" bit number: %d exchange: %s exchange letter: %c\n", i->first,
               std::get<0>(i->second).c_str(), std::get<1>(i->second));
    }
}

//! [smartcomponents]

//! [tickReqParams]
void TestCppClient::tickReqParams(int tickerId, double minTick,
                                  const std::string &bboExchange,
                                  int snapshotPermissions) {
    return;
    printf("tickerId: %d, minTick: %s, bboExchange: %s, snapshotPermissions: "
           "%u\n",
           tickerId, Utils::doubleMaxString(minTick).c_str(), bboExchange.c_str(),
           snapshotPermissions);

    m_bboExchange = bboExchange;
}

//! [tickReqParams]

//! [newsProviders]
void TestCppClient::newsProviders(
    const std::vector<NewsProvider> &newsProviders) {
    printf("News providers (%lu):\n", newsProviders.size());

    for (unsigned int i = 0; i < newsProviders.size(); i++) {
        printf("News provider [%d] - providerCode: %s providerName: %s\n", i,
               newsProviders[i].providerCode.c_str(),
               newsProviders[i].providerName.c_str());
    }
}

//! [newsProviders]

//! [newsArticle]
void TestCppClient::newsArticle(int requestId, int articleType,
                                const std::string &articleText) {
}

//! [newsArticle]

//! [historicalNews]
void TestCppClient::historicalNews(int requestId, const std::string &time,
                                   const std::string &providerCode,
                                   const std::string &articleId,
                                   const std::string &headline) {
    printf("Historical News. RequestId: %d, Time: %s, ProviderCode: %s, "
           "ArticleId: %s, Headline: %s\n",
           requestId, time.c_str(), providerCode.c_str(), articleId.c_str(),
           headline.c_str());
}

//! [historicalNews]

//! [historicalNewsEnd]
void TestCppClient::historicalNewsEnd(int requestId, bool hasMore) {
    printf("Historical News End. RequestId: %d, HasMore: %s\n", requestId,
           (hasMore ? "true" : " false"));
}

//! [historicalNewsEnd]

//! [headTimestamp]
void TestCppClient::headTimestamp(int reqId, const std::string &headTimestamp) {
    printf("Head time stamp. ReqId: %d - Head time stamp: %s,\n", reqId,
           headTimestamp.c_str());
}

//! [headTimestamp]

//! [histogramData]
void TestCppClient::histogramData(int reqId, const HistogramDataVector &data) {
    printf("Histogram. ReqId: %d, data length: %lu\n", reqId, data.size());

    for (const HistogramEntry &entry: data) {
        printf("\t price: %s, size: %s\n",
               Utils::doubleMaxString(entry.price).c_str(),
               DecimalFunctions::decimalStringToDisplay(entry.size).c_str());
    }
}

//! [histogramData]

//! [historicalDataUpdate]
void TestCppClient::historicalDataUpdate(TickerId reqId, const Bar &bar) {
    printf("HistoricalDataUpdate. ReqId: %ld - Date: %s, Open: %s, High: %s, "
           "Low: %s, Close: %s, Volume: %s, Count: %s, WAP: %s\n",
           reqId, bar.time.c_str(), Utils::doubleMaxString(bar.open).c_str(),
           Utils::doubleMaxString(bar.high).c_str(),
           Utils::doubleMaxString(bar.low).c_str(),
           Utils::doubleMaxString(bar.close).c_str(),
           DecimalFunctions::decimalStringToDisplay(bar.volume).c_str(),
           Utils::intMaxString(bar.count).c_str(),
           DecimalFunctions::decimalStringToDisplay(bar.wap).c_str());
}

//! [historicalDataUpdate]

//! [rerouteMktDataReq]
void TestCppClient::rerouteMktDataReq(int reqId, int conid,
                                      const std::string &exchange) {
    printf("Re-route market data request. ReqId: %d, ConId: %d, Exchange: %s\n",
           reqId, conid, exchange.c_str());
}

//! [rerouteMktDataReq]

//! [rerouteMktDepthReq]
void TestCppClient::rerouteMktDepthReq(int reqId, int conid,
                                       const std::string &exchange) {
    printf("Re-route market depth request. ReqId: %d, ConId: %d, Exchange: "
           "%s\n",
           reqId, conid, exchange.c_str());
}

//! [rerouteMktDepthReq]

//! [marketRule]
void TestCppClient::marketRule(
    int marketRuleId, const std::vector<PriceIncrement> &priceIncrements) {
    printf("Market Rule Id: %s\n", Utils::intMaxString(marketRuleId).c_str());
    for (unsigned int i = 0; i < priceIncrements.size(); i++) {
        printf("Low Edge: %s, Increment: %s\n",
               Utils::doubleMaxString(priceIncrements[i].lowEdge).c_str(),
               Utils::doubleMaxString(priceIncrements[i].increment).c_str());
    }
}

//! [marketRule]

//! [pnl]
void TestCppClient::pnl(int reqId, double dailyPnL, double unrealizedPnL,
                        double realizedPnL) {
}

//! [pnl]

//! [pnlsingle]
void TestCppClient::pnlSingle(int reqId, Decimal pos, double dailyPnL,
                              double unrealizedPnL, double realizedPnL,
                              double value) {
    printf("PnL Single. ReqId: %d, pos: %s, daily PnL: %s, unrealized PnL: %s, "
           "realized PnL: %s, value: %s\n",
           reqId, DecimalFunctions::decimalStringToDisplay(pos).c_str(),
           Utils::doubleMaxString(dailyPnL).c_str(),
           Utils::doubleMaxString(unrealizedPnL).c_str(),
           Utils::doubleMaxString(realizedPnL).c_str(),
           Utils::doubleMaxString(value).c_str());
}

//! [pnlsingle]

//! [historicalticks]
void TestCppClient::historicalTicks(int reqId,
                                    const std::vector<HistoricalTick> &ticks,
                                    bool done) {
    for (const HistoricalTick &tick: ticks) {
        std::time_t t = tick.time;
        std::tm tm{};
        localtime_r(&t, &tm);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
        SPDLOG_INFO("historicalTicks(). ReqId: {}, Time: {}, price: {}, "
                    "size: {}",
                    reqId, oss.str(), Utils::doubleMaxString(tick.price),
                    DecimalFunctions::decimalStringToDisplay(tick.size));
    }
}

//! [historicalticks]

//! [historicalticksbidask]
void TestCppClient::historicalTicksBidAsk(
    int reqId, const std::vector<HistoricalTickBidAsk> &ticks, bool done) {
    for (const HistoricalTickBidAsk &tick: ticks) {
        std::time_t t = tick.time;

        std::tm tm{};
        localtime_r(&t, &tm);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
        SPDLOG_INFO(
            "historicalTicksBidAsk(). ReqId: {}, Time: {}, bid: {}, "
            "ask: {}, bidSize: {}, askSize: {}, bidPastLow: {}, "
            "askPastHigh: {}",
            reqId, oss.str(), Utils::doubleMaxString(tick.priceBid).c_str(),
            Utils::doubleMaxString(tick.priceAsk).c_str(),
            DecimalFunctions::decimalStringToDisplay(tick.sizeBid),
            DecimalFunctions::decimalStringToDisplay(tick.sizeAsk),
            tick.tickAttribBidAsk.bidPastLow, tick.tickAttribBidAsk.askPastHigh);
    }
}

//! [historicalticksbidask]

//! [historicaltickslast]
void TestCppClient::historicalTicksLast(
    int reqId, const std::vector<HistoricalTickLast> &ticks, bool done) {
    for (HistoricalTickLast tick: ticks) {
        std::time_t t = tick.time;
        std::tm tm{};
        localtime_r(&t, &tm);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
        SPDLOG_INFO("historicalTicksLast() ReqId: {}, Time: {}, Price: {}, "
                    "Size: {}, Exchange: {}, "
                    "SpecialConditions: {}, unreported: {}, pastLimit: {}",
                    reqId, oss.str(), Utils::doubleMaxString(tick.price).c_str(),
                    DecimalFunctions::decimalStringToDisplay(tick.size).c_str(),
                    tick.exchange, tick.specialConditions,
                    tick.tickAttribLast.unreported, tick.tickAttribLast.pastLimit);
    }
}

//! [historicaltickslast]

//! [tickbytickalllast]
void TestCppClient::tickByTickAllLast(int reqId, int tickType, time_t time,
                                      double price, Decimal size,
                                      const TickAttribLast &tickAttribLast,
                                      const std::string &exchange,
                                      const std::string &specialConditions) {
    std::tm tm{};
    localtime_r(&time, &tm);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
    SPDLOG_INFO(
        "tickByTickAllLast() ReqId: {}, TickType: {}, Time: {}, Price: {}, "
        "Size: {}, PastLimit: {}, Unreported: {}, Exchange: {}, "
        "SpecialConditions:{}",
        reqId, (tickType == 1 ? "Last" : "AllLast"), oss.str(),
        Utils::doubleMaxString(price),
        DecimalFunctions::decimalStringToDisplay(size), tickAttribLast.pastLimit,
        tickAttribLast.unreported, exchange, specialConditions);
}

//! [tickbytickalllast]

//! [tickbytickbidask]
void TestCppClient::tickByTickBidAsk(int reqId, time_t time, double bidPrice,
                                     double askPrice, Decimal bidSize,
                                     Decimal askSize,
                                     const TickAttribBidAsk &tickAttribBidAsk) {
    std::tm tm{};
    localtime_r(&time, &tm);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
    SPDLOG_INFO("tickByTickBidAsk(): ReqId: {}, TickType: BidAsk, Time: {}, "
                "BidPrice: {}, "
                "AskPrice: {}, BidSize: {}, AskSize: {}, BidPastLow: {}, "
                "AskPastHigh: {}",
                reqId, oss.str(), Utils::doubleMaxString(bidPrice).c_str(),
                Utils::doubleMaxString(askPrice).c_str(),
                DecimalFunctions::decimalStringToDisplay(bidSize).c_str(),
                DecimalFunctions::decimalStringToDisplay(askSize).c_str(),
                tickAttribBidAsk.bidPastLow, tickAttribBidAsk.askPastHigh);
}

//! [tickbytickbidask]

void TestCppClient::tickByTickMidPoint(int reqId, const time_t time,
                                       const double midPoint) {
    std::tm tm{};
    localtime_r(&time, &tm);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
    SPDLOG_INFO("Symbol: {:18}, Time: {}, MidPoint: {}",
                std::format("{:7} ({})", !m_subscribedContracts[reqId].description.empty() ? m_subscribedContracts[reqId
                    ].description :
                    m_subscribedContracts[reqId].symbol,
                    m_subscribedContracts[reqId].exchange)
                , oss.str(), Utils::doubleMaxString(midPoint));
}


//! [orderbound]
void TestCppClient::orderBound(long long orderId, int apiClientId,
                               int apiOrderId) {
}

//! [orderbound]

//! [completedorder]
void TestCppClient::completedOrder(const Contract &contract, const Order &order,
                                   const OrderState &orderState) {
}

//! [completedorder]

//! [completedordersend]
void TestCppClient::completedOrdersEnd() {
}

//! [completedordersend]

//! [replacefaend]
void TestCppClient::replaceFAEnd(int reqId, const std::string &text) {
}

//! [replacefaend]

//! [wshMetaData]
void TestCppClient::wshMetaData(int reqId, const std::string &dataJson) {
}

//! [wshMetaData]

//! [wshEventData]
void TestCppClient::wshEventData(int reqId, const std::string &dataJson) {
}

//! [wshEventData]

//! [historicalSchedule]
void TestCppClient::historicalSchedule(
    int reqId, const std::string &startDateTime, const std::string &endDateTime,
    const std::string &timeZone,
    const std::vector<HistoricalSession> &sessions) {
    printf("Historical Schedule. ReqId: %d, Start: %s, End: %s, TimeZone: %s\n",
           reqId, startDateTime.c_str(), endDateTime.c_str(), timeZone.c_str());
    for (unsigned int i = 0; i < sessions.size(); i++) {
        printf("\tSession. Start: %s, End: %s, RefDate: %s\n",
               sessions[i].startDateTime.c_str(), sessions[i].endDateTime.c_str(),
               sessions[i].refDate.c_str());
    }
}

//! [historicalSchedule]

//! [userInfo]
void TestCppClient::userInfo(int reqId, const std::string &whiteBrandingId) {
    printf("User Info. ReqId: %d, WhiteBrandingId: %s\n", reqId,
           whiteBrandingId.c_str());
}

//! [userInfo]
