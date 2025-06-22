#include "/home/mamsds/Downloads/twsapi_macunix.1030.01/IBJts/source/cppclient/client/EWrapper.h"
#include "/home/mamsds/Downloads/twsapi_macunix.1030.01/IBJts/source/cppclient/client/EClientSocket.h"
#include "/home/mamsds/Downloads/twsapi_macunix.1030.01/IBJts/source/cppclient/client/Contract.h"
// #include "/home/mamsds/Downloads/twsapi_macunix.1030.01/IBJts/source/cppclient/client/TwsApiDefs.h"

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

class BTCPriceClient : public EWrapper {
private:
    EClientSocket* const client;
    bool connected;

public:
    BTCPriceClient() : client(new EClientSocket(this)), connected(false) {}

    ~BTCPriceClient() {
        disconnect();
        delete client;
    }

    bool connect(const char* host, int port, int clientId) {
        std::cout << "Connecting to " << host << ":" << port << " with clientId " << clientId << std::endl;
        bool success = client->eConnect(host, port, clientId, false);
        if (success) {
            connected = true;
            client->startApi();
            std::cout << "Connected successfully" << std::endl;
        } else {
            std::cerr << "Connection failed" << std::endl;
        }
        return success;
    }

    void disconnect() {
        if (connected) {
            client->eDisconnect();
            connected = false;
            std::cout << "Disconnected" << std::endl;
        }
    }

    void requestBTCPrice() {
        Contract contract;
        contract.symbol = "BTC";
        contract.secType = "CRYPTO";
        contract.currency = "USD";
        contract.exchange = "GDAX";

        // Request real-time market data
        client->reqMktData(1001, contract, "", false, false, TagValueListSPtr());
        std::cout << "Requested market data for BTC/USD" << std::endl;
    }

    // EWrapper callbacks
    void tickPrice(TickerId tickerId, TickType tickType, double price, const TickAttrib& attribs) override {
        std::string tickName;
        switch (tickType) {
            case BID: tickName = "BID"; break;
            case ASK: tickName = "ASK"; break;
            case LAST: tickName = "LAST"; break;
            default: return;
        }
        std::cout << "Tick. TickerId: " << tickerId << ", Type: " << tickName << ", Price: " << price << std::endl;
    }

    void error(int id, int errorCode, const std::string& errorString, const std::string& advancedOrderRejectJson) override {
        std::cerr << "Error. Id: " << id << ", Code: " << errorCode << ", Msg: " << errorString << std::endl;
    }

    void connectAck() override {
        std::cout << "Connection acknowledged" << std::endl;
    }
};

int main() {
    const char* host = "127.0.0.1";
    int port = 7497; // Use 7496 for paper trading
    int clientId = 0;

    BTCPriceClient btcClient;
    if (!btcClient.connect(host, port, clientId)) {
        return 1;
    }

    btcClient.requestBTCPrice();

    // Run for 30 seconds to collect data
    std::this_thread::sleep_for(std::chrono::seconds(30));

    btcClient.disconnect();
    return 0;
}