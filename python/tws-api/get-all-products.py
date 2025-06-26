import requests
import pandas as pd
import time

pd.set_option('display.max_columns', None)  # None means no limit
pd.set_option('display.width', 1000)

url = 'https://www.interactivebrokers.com/webrest/search/products-by-exchangeId'

headers = {
    'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64; rv:128.0) Gecko/20100101 Firefox/128.0',
    'Accept': 'application/json, text/plain, */*',
    'Accept-Language': 'en-US,en;q=0.5',
    'Accept-Encoding': 'gzip, deflate, br, zstd',
    'Referer': 'https://www.interactivebrokers.com/en/trading/products-exchanges.php',
    'Content-Type': 'application/json;charset=utf-8',
    'Origin': 'https://www.interactivebrokers.com',
    'Connection': 'keep-alive',
    'Cookie': 'x-sess-uuid=0.6142017.1750902936.2e3be520; IBKRcampus_gdpr_optIn=true; IBKRcampus_mktng_optIn=true; IBKRcampus_functional_optIn=true; NONAUTH_AB_UUID=d8b045a5-33f7-49c2-acce-47f2ff3d5c8b; IB_PRIV_PREFS=0%7C0%7C0; device.info=eyJpZCI6Ijc1NmYyZDk0IiwibWFjIjoiMTY6MjQ6QUQ6NDU6OTg6REIifQ==; SBID=gem5tl162smc8ije4o; F2A_UUID=797e9a7e-cf1f-47c0-f300-55147f53ca4e; F2A_NONAUTH_AB=797e9a7e-cf1f-47c0-f300-55147f53ca4e; IB_LANG=en; ttcsid_CAH4O7JC77U32TPDO3GG=1750902651505::lJ_IJK--ds0fSXl10tWb.3.1750902989362; Campus_tag_ga_3DZW8R5ZLR=GS2.1.s1750900756$o5$g1$t1750902531$j60$l0$h0; Campus_tag_ga=GA1.1.1039173649.1750737366; _gcl_au=1.1.1851753132.1750753765; _ga_7FYT07EYXG=GS2.1.s1750902650$o2$g1$t1750902974$j12$l0$h0; _ga=GA1.2.1719732512.1750753765; sm_uuid=1750754430922; _tt_enable_cookie=1; _ttp=01JYGGWV2KFN68TK6C2C55NB9A_.tt.1; ttcsid=1750902651505::4rmgsYTu9t7OSP9zygG3.2.1750902975760; RT="z=1^&dm=interactivebrokers.com^&si=41bc2ae3-031b-4018-962f-5d99037a4bbf^&ss=mccq5ol9^&sl=2^&tt=33z^&obo=1^&rl=1^&nu=3dp3ydk7^&cl=aijx"; web=-1; PHPSESSID=19bddde9cf3242b31071d2c27613917b; AKA_A2=A; ppc_last_visited_page=https://www.interactivebrokers.com/campus/ibkr-quant-news/backtesting-py-an-introductory-guide-to-backtesting-with-python/; iab=1; _gid=GA1.2.1435934023.1750902651; _rdt_uuid=1750383535781.85cd76be-5233-4a55-bcb0-3750cd40f9d8; _uetsid=fd3645a0522f11f0bc3663e2f42bf26b; _uetvid=542cef804d7711f08431e7d27b7d9e08',
    'Sec-Fetch-Dest': 'empty',
    'Sec-Fetch-Mode': 'cors',
    'Sec-Fetch-Site': 'same-origin',
    'Priority': 'u=0'
}

data = {
    "domain": "com",
    "exchangeId": "SEHK",
    "pageNumber": 0,
    "pageSize": 100,
    "sortDirection": "asc",
    "sortField": "symbol"
}

try:
    df = pd.DataFrame()
    while True:
        data['pageNumber'] += 1
        resp = requests.post(url, headers=headers, json=data)
        resp.raise_for_status()  # Raises an HTTPError if the HTTP request returned an unsuccessful status code
        json_data = resp.json()

        # Convert to DataFrame
        df_page = pd.DataFrame(json_data['products'])
        if df_page.empty:
            break
        df = pd.concat([df, df_page], axis=0)
        # Display the DataFrame
        print('df_page.head():')
        print(df_page.head())
        print(f'df.shape: {df.shape}\ndf.tail():')
        print(df.tail())
        time.sleep(5)

    breakpoint()

except requests.exceptions.RequestException as e:
    print(f"Error making the request: {e}")
except ValueError as e:
    print(f"Error parsing JSON: {e}")