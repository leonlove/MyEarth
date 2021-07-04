# -*- coding: utf-8 -*-
"""
Created on Mon May 24 14:18:33 2021

@author: Administrator
"""
# 寻找倍增股
import baostock as bs
import pandas as pd

#### 登陆系统 ####
lg = bs.login()
# 显示登陆返回信息
print('login respond error_code:'+lg.error_code)
print('login respond  error_msg:'+lg.error_msg)

data_time = "2021-06-01"
pre_data_time = "2021-05-31"

#### 获取证券信息 ####
all_stock = bs.query_all_stock(day=data_time)
print('query_all_stock respond error_code:'+all_stock.error_code)
print('query_all_stock respond  error_msg:'+all_stock.error_msg)
data_list = []
while (all_stock.error_code == '0') & all_stock.next():
    # 获取一条记录，将记录合并在一起
    data_list.append(all_stock.get_row_data())
all_stock_ds = pd.DataFrame(data_list, columns=all_stock.fields)
all_stock_ds=all_stock_ds[~all_stock_ds['tradeStatus'].isin(['0'])]

count = 0
code_list = []
for code in all_stock_ds['code'][220:4531]:
    # print(code)
    code_rs = bs.query_history_k_data_plus(code,
    "date,code,open,high,low,close,preclose,volume,amount",
    start_date=pre_data_time, end_date=data_time,
    frequency="d", adjustflag="3")
    data_list.clear();
    while (code_rs.error_code == '0') & code_rs.next():
        # 获取一条记录，将记录合并在一起
        data_list.append(code_rs.get_row_data())
    code_ds = pd.DataFrame(data_list, columns=code_rs.fields)
    if code_ds['volume'][1]=='' or code_ds['volume'][0]=='' or code_ds['code'][0] == 'sz.301001':
        print("111" + code_ds['code'][0])
        continue
        # print(code_ds['volume'][0], code_ds['volume'][1], type(code_ds['volume'][0]))
    print(count, code, code_ds['volume'][1], code_ds['volume'][0], type(code_ds['volume'][1]))
    if int(code_ds['volume'][1]) > int(code_ds['volume'][0]) * 2 :
        code_list.append(code)
        # print(code_ds[['date', 'code', 'volume']], int(code_ds['volume'][1]), int(code_ds['volume'][0]), int(code_ds['volume'][0]) * 2)
    count += 1
    
#### 结果集输出到csv文件 ####   
# all_stock_ds.to_csv("D:\\all_stock1.csv", encoding="gbk", index=False)
file=open('data.txt','w')  
file.write(str(code_list));  
file.close() 

# print(all_stock_ds)

#### 登出系统 ####
bs.logout()