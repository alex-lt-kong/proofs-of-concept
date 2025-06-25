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

    def headTimestamp(self, reqId, headTimestamp):
        print(reqId, headTimestamp)


app = TestApp()
app.connect("127.0.0.1", port, 8)
threading.Thread(target=app.run).start()
time.sleep(1)

mycontract = Contract()
mycontract.symbol = "2828"
mycontract.secType = "STK"
mycontract.exchange = "SEHK"
mycontract.currency = "HKD"
app.reqHeadTimeStamp(app.nextId(), mycontract, "TRADES", 1, 1)
time.sleep(5)
app.disconnect()