from ibapi.client import *
from ibapi.wrapper import *
import datetime
import time
import threading

port = 7496


class TestApp(EClient, EWrapper):
    orderId = 0

    def __init__(self):
        EClient.__init__(self, self)

    def nextValidId(self, orderId: OrderId):
        self.orderId = orderId

    def nextId(self):
        self.orderId += 1
        return self.orderId

    def error(self, reqId, errorCode, errorString, advancedOrderReject=""):
        print(f"reqId: {reqId}, errorCode: {errorCode}, errorString: {errorString}, orderReject: {advancedOrderReject}")

    def historicalData(self, reqId, bar: BarData):
        print(reqId, bar)

    def historicalDataEnd(self, reqId, start, end):
        print(f"Historical Data Ended for {reqId}. Started at {start}, ending at {end}")
        self.cancelHistoricalData(reqId)

def main() -> None:
    app = TestApp()
    app.connect("127.0.0.1", port, 7)
    threading.Thread(target=app.run).start()
    time.sleep(1)

    contract = Contract()
    contract.symbol = "2828"
    contract.secType = "STK"
    contract.exchange = "SEHK"
    contract.currency = "HKD"

    app.reqHistoricalData(app.nextId(), contract, "20250624-00:00:00", "10 Y", "1 day", "TRADES", 1, 1, False, [])
    time.sleep(30)
    app.disconnect()

if __name__ == "__main__":
    main()