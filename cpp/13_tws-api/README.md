# TWS API from Interactive Brokers

## Environment

### Install either IB Gateway or Trader Workstation (TWS)

- Download IB Gateway from [here](https://ibkrcampus.com/campus/ibkr-api-page/twsapi-doc/#tws-download). IB Gateway is a close-sourced gateway running locally, all our communication with IB backend will be through this gateway component.

  - You will likely need to tweak the IB Gateway's configurations according to [this page](https://ibkrcampus.com/campus/ibkr-api-page/twsapi-doc/#tws-settings)

### Build TWS API

- Download from IBKR's [API software page](https://interactivebrokers.github.io/#)

- Build

```bash
cd ./twsapi_macunix.1030.01/IBJts/source/cppclient/client/
mkdir build && cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j 4
sudo cmake --install ./
```

- Sample output:

```bash
-- Install configuration: "Release"
-- Installing: /usr/local/lib/libtwsapi.so
-- Up-to-date: /usr/local/include/twsapi/CommissionReport.h
-- Up-to-date: /usr/local/include/twsapi/CommonDefs.h
-- Up-to-date: /usr/local/include/twsapi/Contract.h
-- Up-to-date: /usr/local/include/twsapi/ContractCondition.h
-- Up-to-date: /usr/local/include/twsapi/Decimal.h
-- Up-to-date: /usr/local/include/twsapi/DefaultEWrapper.h
-- Up-to-date: /usr/local/include/twsapi/DepthMktDataDescription.h
-- Up-to-date: /usr/local/include/twsapi/EClient.h
-- Up-to-date: /usr/local/include/twsapi/EClientException.h
...
```

### Build `Intel Decimal Floating Point Library`

- Refer to `./twsapi_macunix.1030.01/IBJts/source/cppclient/Intel_lib_build.txt`
- Copy the artifact `libbid.a` to a directory that CMake can link.

### TestCppClient

- Initial version of `./src/TestCppClient/` is downloaded from IB's [API software page](https://interactivebrokers.github.io/#). Usually one can jump to this page from IKBRCampus's [TWS API Documentation](https://ibkrcampus.com/campus/ibkr-api-page/twsapi-doc/#find-the-api)
