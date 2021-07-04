# -*- coding: utf-8 -*-
"""
Created on Thu Jul  1 23:42:42 2021

@author: Administrator
"""

import mpl_finance
import tushare as ts
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from matplotlib import ticker
from matplotlib.pylab import date2num
import numpy as np

ts.set_token('4da980377579b71486cab0d882354b7754b1bd3bb2e2841842c7669d')

sns.set()
pro = ts.pro_api()

df = pro.daily(ts_code='600137.SH',start_date='20200901')
df = df.sort_values(by='trade_date',ascending=True)
df['trade_date2'] = df['trade_date'].copy()
df['trade_date'] = pd.to_datetime(df['trade_date']).map(date2num)
df['dates'] = np.arange(0,len(df))
# print(df.head())

def format_date(x,pos):
    if x<0 or x>len(date_tickers)-1:
        return ''
    return date_tickers[int(x)]

date_tickers = df.trade_date2.values
fig, ax = plt.subplots(figsize=(10,5))
ax.xaxis.set_major_formatter(ticker.FuncFormatter(format_date))
mpl_finance.candlestick_ochl(
    ax=ax,
    quotes=df[['dates', 'open', 'close', 'high', 'low']].values,
    width=0.7,
    colorup='r',
    colordown='g',
    alpha=0.7)
ax.set_title('浪莎股份K线图(2020.9-)', fontsize=20);
