/***************************************************************************
 *   Copyright (C) 2012 by luyi   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Publib License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Publib License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Publib License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

 //////////////////////////////////////////////////////////////////////
// 文件名：CSimpleHttpClient.h
// 功能：定义SimpleHttpClient类，该类用于实现简单的Http客户端
//--------------------------------------------------------------------
// 初始版本：V0.01
// 作者：luyi
// 时间：2008-1-27
//--------------------------------------------------------------------
// 变更记录
//--------------------------------------------------------------------
// 增加struct HttpRequest
// 并在SimpleHttpClient增加ClipHttpRequest方法
// 用于解析URL串 
// 修改OpenConnection实现，增加对代理服务的支持
// luyi 2008-1-28
// 版本号：0.02
//--------------------------------------------------------------------
// 增加SetProxyServer方法，用于设置代理服务器
// 增加SetProxyServer方法，调整OpenConnection方法
// luyi 2008-1-29
// 版本号：V0.03
//--------------------------------------------------------------------
// 增加对服务端以流的方式传输数据的支持，添加RecvStreamData方法
// 增加FreeStreamData方法，用于释放RecvStreamData方法分配的内存
// luyi 2009-2-8
// 版本号：V0.04
//--------------------------------------------------------------------
// 增加对服务端以流的方式传输数据的支持，添加RecvStreamData第二种原型
// 该原型通过客户定义的OnRecvStreamDataFun函数将流出具传递给使用者
// luyi 2009-2-15
// 版本号：V0.05
//--------------------------------------------------------------------
// 修改代码增加对linux的支持
// 修正流方式接受数据的一个BUG
// 增加Request方法和FreeResponse，对请求的过程进行包装
// 用一个方法完成请求和应答操作
// 去掉IE代理的支持（LINUX没有IE）
// luyi 20012-1-30
// 版本号：V0.06
//////////////////////////////////////////////////////////////////////

#ifndef	__SIMPLE_HTTP_CLIENT_H__
#define	__SIMPLE_HTTP_CLIENT_H__

//#define	__LINUX__
#include "stdio.h"

#pragma warning( disable : 4996 )

#ifdef	_WINDOWS
  #if _MSC_VER > 1000
  #pragma once
  #endif // _MSC_VER > 1000

  #ifndef _WINSOCKAPI_
    #ifndef _WINSOCK2API_
      #include "winsock2.h"
    #endif
  #endif
  
  #ifdef	SIMPLEHTTPCLIENT_EXPORTS
	  #undef	SIMPHTTPEXP
	  #define	SIMPHTTPEXP	__declspec(dllexport)
  #else
	  #undef	SIMPHTTPEXP
	  #define	SIMPHTTPEXP	__declspec(dllimport)
  #endif

#endif

#undef	SIMPHTTPEXP
#define	SIMPHTTPEXP



#ifdef	__LINUX__
    #include "sys/types.h"
    #include <netinet/in.h>
    #include <netdb.h>
    #include "sys/socket.h"
  
    #define	SIMPHTTPEXP
  
    #define SOCKET int
    #define INVALID_SOCKET (SOCKET)(~0)
    #define SOCKET_ERROR           (-1)
    
    #define closesocket(s)  close(s)
  
#endif

struct ResponseInfo
{
	char *HttpVersion;		//服务器Http协议版本
	char *ResponseState;	//服务器应答状态（如：返回200，302，404等）
	char *ServerType;		//服务类型（IIS等）
	char *Date;				//标准时间
	char *Location;			//返回的目标（哪个网页，图片等）
	char *ContentLen;		//内容长度
	char *ContentType;		//内容类型
	char *Transfer;			//流编码，如果是流方式，必须为 chunked
};

struct SIMPHTTPEXP HttpRequest
{
public:
	HttpRequest();
	~HttpRequest();
public:
	char *pProtocol;		//协议段（如：Http://）
	char *pHost;			//主机 （如：172.17.1.139；www.sohu.com等）
	unsigned short Port;	//端口
	char *pObject;			//请求目标（如：/a.htm/?t=cfgfile）
};

enum HTTPMethod
{
	GetMethod,
	PostMethod
};

#ifdef  __WINDOWS__
    typedef int (__stdcall *OnRecvStreamDataFun)(char *pStreamData, long nDataLen);
#else
    typedef int (*OnRecvStreamDataFun)(char *pStreamData, long nDataLen);
#endif  //__WINDOWS__

//int OnRecvStreamData(char *pStreamData, long nDataLen);

/////////////////////////////////////////////////////////////////////////////
// 类名: SimpleHttpClient
// 功能：实现简单的Http客户端协议封装，完成发送请求，接受应答的功能
// 相关类型: HttpRequest, ResponseInfo
/////////////////////////////////////////////////////////////////////////////
class SIMPHTTPEXP SimpleHttpClient  
{
public:
	SimpleHttpClient();
	virtual ~SimpleHttpClient();

public:
	//设置代理服务器
	//pProxySvr传入代理服务器主机，传入NULL表示清楚代理服务器设置
	//ProxyPort传入代理端口
	void SetProxyServer(const char *pProxySvr, unsigned short ProxyPort);

	//设置直接使用IE代理
	//	void UseIEProxy();

	//向指定的URL发送请求，并得到返回信息
	//该方法内部自动打开连接，并发送请求，然后得到返回结果，无需手工OpenConnection
	//Method : 传入参数，Get还是Post请求，具体参见SendRequest方法说明
	//pURL : 要请求的URL
	//pData : 如果是Post请求，该参数表示Post的数据块，如果不是Post请求，该参数可以为NULL
	//DataLen : 如果是Post请求，该参数表示Post的数据块的长度，以字节为单位
	//pResponseData : 请求成功，该参数返回服务器应答的数据（可能是一个html也可能使一个数据文件），该参数不能为NULL
	//，返回的数据使用FreeResponse释放
	//pResponseLen : 请求成功，该参数返回服务器应答数据的长度，该参数不能为NULL
	//该方法成功返回0，失败返回负数
	//2表示参数不正确
	//3表示URL格式不正确
	//4表示连接到服务器失败
	//5表示发送请求到服务器失败
	//6表示得到response长度失败
	//7表示得到response内容失败
	int Request(HTTPMethod Method, const char *pURL, char **pResponseData, long *pResponseLen, void *pData=NULL, long DataLen=0);
	
	//释放Request函数返回的Response数据
	void FreeResponse(char *pResponse);

	//连接服务器，成功返回0，失败返回负数，-1表示创建socket失败，-2表示主机无效，-3表示连接失败
	//连接成功后才能发送请求和接受数据
	//pHost ： 传入参数，要连接的主机，可以接受的类型有：IP地址，Url，机器名，LocalHost
	//Port ： 传入参数，表示要打开的端口
	//该函数可以重复调用，每次成功的调用会关闭上一次的连接打开一个新连接
	int	OpenConnection(const char *pHost, unsigned short Port);

	//调用该方法前必须成功调用OpenConnection方法
	//如果是Get方法，pObj必须指定要获取的内容，例如：
	//   http://www.geostar.com.cn/index.htm/?t=a
	//   1      2                 3
	//其中1为协议类型（http://）
	//    2为主机（www.geostar.com.cn）
	//    3以后的内容为要获取的内容（/Index.htm/?t=a）
	//对Get方法pPostData和DataLen无效。
	//如果是Post方法，pObj参数无效，pPostData参数为Post的数据，DataLen参数表示Post数据的字节长度
	//该方法成功返回0，失败返回负数
	//-1表示未调用OpenConnection方法
	//-2表示method参数无效
	//-3表示发送请求失败
	//-4表示接收Response失败
	int SendRequest(HTTPMethod Method, const char *pObj, void *pPostData, long DataLen);

	//SendRequest成功后，调用该方法，可以获得服务器返回的信息，比如：请求成功否等。
	//具体见ResponseInfo结构说明
	void GetResponseInfo(ResponseInfo &Info);

	//在发送请求（SendRequest方法）成功后，调用该方法获得服务器返回的内容（网页，图片，文件等）
	//pBuf，传入参数，用于接收数据的缓冲区地址
	//BufLen，传入参数，用于传入缓冲区大小，字节数
	//返回：成功返回实际接收数据的字节数，该数可能小于Buflen
	//失败返回-1
	//返回0表示服务器终止连接。
	int RecvData(char *pBuf, long BufLen);

	//在发送请求（SendRequest方法）成功后，如果服务端以流的形式返回内容，调用该方法获得服务器返回的内容（网页，图片，文件等）
	//当返回的response的contentlength等于0，表示流返回数据
	//ppBuf，传出参数，用于传出数据的缓冲区地址
	//pBufLen，传出参数，用于传出接收数据的大小，字节数267511

	//返回：成功返回0
	//失败返回-1
	//通过ppBuf返回的数据，需要手工调用release释放
	//注意：当调用RecvStreamData成功后，必须匹配调用FreeStreamData方法释放返回的数据内存
	int RecvStreamData(char **ppBuf, long *pBufLen);

	//在接收流数据成功后（RecvStreamData）成功后，需要调用FreeStreamData释放数据内存
	void FreeStreamData(char *pBuf);

	//在发送请求（SendRequest方法）成功后，如果服务端以流的形式返回内容，调用该方法获得服务器返回的内容（网页，图片，文件等）
	//当返回的response的contentlength等于0，表示流返回数据
	//pFun，客户传入的回调函数，RecvStreamData在接收到数据后回调pFun函数，该函数中无需释放流数据对应的内存。
	//返回：成功返回0
	//失败返回-1
	int RecvStreamData(OnRecvStreamDataFun pFun);

public:
	//将一个URL解析成几个部分，具体解析结果参见struct HttpRequest描述
	//成功返回0, 失败返回负数, -1表示参数无效
	static int ClipHttpRequest(const char *pUrl, HttpRequest *phr);

private:
	//发送数据，成功返回0，失败返回-1
	int Send(void *pSendData, long DataLen);

	//下列函数用于辅助解释Response
	void GetField(const char *Response, const char *pName, char **Val);
	void GetHttpVersion(const char *Response, char **Val);
	void GetResponseState(const char *Response, char **Val);

	//下面函数用于释放ResponseInfo内存
	void SafeReleaseInfo();

private:
	SOCKET				s_;
	ResponseInfo		ResponseInfo_;
#ifdef	__WINDOWS__
	int					SockInit_;
#endif	//__WINDOWS__
	char				*Host_;
	unsigned short		Port_;
	char				*AgentHost_;
	unsigned short		AgentPort_;
};

#ifdef	__WINDOWS__
  #ifndef	SIMPLEHTTPCLIENT_EXPORTS
	  #ifndef	_DEBUG
		  #pragma comment(lib, "ws2_32.lib")
	  #else
		  #pragma comment(lib, "ws2_32.lib")
	  #endif
  #endif
#endif

#endif	//__SIMPLE_HTTP_CLIENT_H__
