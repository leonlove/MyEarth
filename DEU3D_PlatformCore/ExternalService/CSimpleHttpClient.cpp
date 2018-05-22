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
//
// CSimpleHttpClient.cpp: implementation of the SimpleHttpClient class.
//
//////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////// 
#include "memory.h"
#include "stdlib.h"
#include "CSimpleHttpClient.h"
#include "string.h"

#define BUFLEN (1024*1024)
#define SOCKINITFAIL -1

int RecvData(SOCKET s, char *pBuf, long BufLen);

HttpRequest::HttpRequest()
	: pProtocol(NULL) 
	, pHost(NULL)
	, Port(0)
	, pObject(NULL)
{
}

HttpRequest::~HttpRequest()
{
	if (NULL != pProtocol)
	{
		delete []pProtocol;
		pProtocol = NULL;
	}
	if (NULL != pHost)
	{
		delete []pHost;
		pHost = NULL;
	}
	Port = 0;
	if (NULL != pObject)
	{
		delete []pObject;
		pObject = NULL;
	}
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#include <string>
using namespace std;

SimpleHttpClient::SimpleHttpClient()
	: s_(INVALID_SOCKET)
#ifdef  __WINDOWS__
	, SockInit_(SOCKINITFAIL)
#endif  //__WINDOWS__
	, Host_(NULL)
	, Port_(0)
	, AgentHost_(NULL)
	, AgentPort_(0)
{
#ifdef	__WINDOWS__
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);

	SockInit_ = ::WSAStartup(wVersionRequested, &wsaData);
	if (SockInit_ == 0)
	{
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		{
			::WSACleanup();
			SockInit_ = SOCKINITFAIL;
		}
	}
#endif	//__WINDOWS__
	memset(&ResponseInfo_, '\0', sizeof(ResponseInfo_));
}

SimpleHttpClient::~SimpleHttpClient()
{
	if (INVALID_SOCKET != s_)
	{
		::closesocket(s_);
		s_ = INVALID_SOCKET;
	}
#ifdef	__WINDOWS__
	if (SockInit_ == 0)
	{
		::WSACleanup();
	}
#endif
	if (NULL != Host_)
	{
		delete Host_;
		Host_ = NULL;
	}
	if (NULL != AgentHost_)
	{
		delete AgentHost_;
		AgentHost_ = NULL;
	}

	SafeReleaseInfo();
}

void SimpleHttpClient::SafeReleaseInfo()
{
	if (NULL != ResponseInfo_.HttpVersion)		delete []ResponseInfo_.HttpVersion;
	if (NULL != ResponseInfo_.ResponseState)	delete []ResponseInfo_.ResponseState;	//服务器应答状态(如:返回200,302,404等)
	if (NULL != ResponseInfo_.ServerType)		delete []ResponseInfo_.ServerType;		//服务类型(IIS等)
	if (NULL != ResponseInfo_.Date)				delete []ResponseInfo_.Date;				//标准时间
	if (NULL != ResponseInfo_.Location)			delete []ResponseInfo_.Location;			//返回的目标(哪个网页,图片等)
	if (NULL != ResponseInfo_.ContentLen)		delete []ResponseInfo_.ContentLen;		//内容长度
	if (NULL != ResponseInfo_.ContentType)		delete []ResponseInfo_.ContentType;		//内容类型
	if (NULL != ResponseInfo_.Transfer)		delete []ResponseInfo_.Transfer;		//流编码，如果是流方式，必须为chunked

	memset(&ResponseInfo_, '\0', sizeof(ResponseInfo_));
}


//连接服务器,成功返回0,失败返回-1
//连接成功后才能发送请求和接受数据
//pHost:传入参数,要连接的主机,可以接受的类型有:IP地址,URL(www.sina.com等),机器名(如果可识别),LocalHost
//Port:传入参数,表示要打开的端口
int	SimpleHttpClient::OpenConnection(const char *pHost, unsigned short Port)
{
#ifdef  __WINDOWS__
	if (SockInit_ != 0)
	{
		return -1;
	}
#endif  //__WINDOWS__
	if (s_ != -1)
	{
		::closesocket(s_);
		s_ = -1;
	}
	if (NULL != Host_)
	{
		delete Host_;
		Host_ = NULL;
	}
	Port_ = 0;

	s_ = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
	if (s_ == INVALID_SOCKET)
	{
		s_ = -1;
		return -1;
	}

	int			err = 0;
	bool		connSuccess = false;
	sockaddr_in sd;

	memset(&sd, '\0', sizeof(sd));
	sd.sin_family = AF_INET;
//	sd.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
//	sd.sin_addr.S_un.S_un_b.s_b1 = 127;
//	sd.sin_addr.S_un.S_un_b.s_b2 = 0;
//	sd.sin_addr.S_un.S_un_b.s_b3 = 0;
//	sd.sin_addr.S_un.S_un_b.s_b4 = 1;

	const char *pTmpHost = NULL;
	unsigned TmpPort = 0;
	if (NULL != AgentHost_)
	{	//如果设置了代理服务器就连接代理服务
		pTmpHost = AgentHost_;
		TmpPort = AgentPort_;
	}
	else
	{	//否则就直接连接主机
		pTmpHost = pHost;
		TmpPort = Port;
	}
	hostent *p = ::gethostbyname(pTmpHost);
	if (p != NULL)
	{
#ifdef  __WINDOWS__
		memcpy(&sd.sin_addr.S_un.S_un_b, p->h_addr_list[0], sizeof(sd.sin_addr.S_un.S_un_b));
#endif
#ifdef  __LINUX__
        sd.sin_addr = *((in_addr *)p->h_addr);
#endif
		sd.sin_port = htons(TmpPort);
		int err = connect(s_, (const sockaddr *)&sd, sizeof(sd));
		if (err != SOCKET_ERROR)
		{	//连接成功
			Host_ = new char[strlen(pHost) + 1];
			strcpy(Host_, pHost);
			Port_ = Port;

			connSuccess = true;

           /* sockaddr_in peername,sname;
            int namelen = 1000,snamelen = 1000;
            err = getsockname(s_,(struct sockaddr*)&sname,&snamelen);
            err = WSAGetLastError();
            err = getpeername(s_,(struct sockaddr*)&peername,&namelen);
             err = WSAGetLastError();
            std::string strAddr = inet_ntoa(peername.sin_addr);
            int nPort = ntohs(peername.sin_port);
            std::string str = "";*/
		}
	}
	else
	{
		::closesocket(s_);
		s_ = -1;
		return -2;
	}
	if (!connSuccess)
	{	//连接主机失败
		::closesocket(s_);
		s_ = -1;
		return -3;
	}

	return 0;
}

//调用该方法前必须成功调用OpenConnection方法
//如果是Get方法,pObj必须指定要获取的内容,例如:
//   http://www.geostar.com.cn/index.htm/?t=a
//   1      2                 3
//其中1为协议类型(http://)
//    2为主机(www.geostar.com.cn)
//    3以后的内容为要获取的内容(/Index.htm/?t=a)
//对Get方法pPostData和DataLen无效。
//如果是Post方法,pObj参数无效,pPostData参数为Post的数据,DataLen参数表示Post数据的字节长度
//该方法成功返回0,失败返回负数
//-1表示未调用OpenConnection方法
//-2表示method参数无效
//-3表示发送请求失败
//-4表示接收Response失败
int SimpleHttpClient::SendRequest(HTTPMethod Method, const char *pObj, void *pPostData, long DataLen)
{
	if (s_ == -1 || Host_ == NULL)
	{
		return -1;
	}

	//创建Http请求协议包:Get方法
	string httprequest;
	char tmpstr[256] = "";

	if (AgentHost_ != NULL)
	{
		sprintf(tmpstr, "HTTP://%s:%d", Host_, Port_);
	}

	switch (Method)
	{
	case GetMethod:
		//第一行,请求路径和协议版本
		httprequest += "GET ";
		httprequest += tmpstr;
		if (NULL != pObj)
			httprequest += pObj;
		httprequest += " HTTP/1.1\r\n";
		break;
	case PostMethod:
		//第一行,请求路径和协议版本
//		httprequest += "POST * HTTP/1.1\r\n";
		httprequest += "POST ";
		httprequest += tmpstr;
		if (NULL != pObj)
			httprequest += pObj;
		httprequest += " HTTP/1.1\r\n";
		memset(tmpstr, '\0', sizeof(tmpstr));

		//Content-Length是Post必须具备的参数
		httprequest += "Content-Length: ";
		sprintf(tmpstr, "%d", DataLen);
		httprequest += tmpstr;
		httprequest += "\r\n";
		memset(tmpstr, '\0', sizeof(tmpstr));
		break;
	default:
		return -2;
	}
	//第二行,主机
	httprequest += "Host: ";
	httprequest += Host_;
	if (Port_ != 0)
	{
		sprintf(tmpstr, ":%d", Port_);
		httprequest += tmpstr;
		memset(tmpstr, '\0', sizeof(tmpstr));
	}
	httprequest += "\r\n";
	//第三行,接受数据类型
	httprequest += "Accept: *.*\r\n";
	//第四行,浏览器类型
	httprequest += "User-Agent: SimpleHttpClient\r\n";//MSIE6.00; Windows 2000\r\n";
	//Cache设置
	httprequest += "Cache-Control:no-cache\r\n";
	//第五行,连接设置,保持
	httprequest += "Connection: Keep-Alive\r\n";
	//第六行
	httprequest += "Accept-Language: zh-cn\r\n";
	//第七行,Content Type
//	httprequest += "Content-Type: multipart/form-data; boundary=---------------------------7d33a816d302b6\r\n";
	//第八行,Content Type
//	httprequest += "Accept-Encoding: gzip, deflate\r\n";
//	if (Method == PostMethod)
//	{
//		//Content-Length是Post必须具备的参数
//		httprequest += "Content-Length: ";
//		sprintf(tmpstr, "%d", DataLen);
//		httprequest += tmpstr;
//		httprequest += "\r\n";
//		memset(tmpstr, '\0', sizeof(tmpstr));
//	}
	//第九行,Cookie,可选
	//第十行,请求数据的起始字节位置(用于断点续传),可选
	//最后一行,空行
	httprequest += "\r\n";

	char *pBuf = NULL;
	long SendLen;
//	if (Method == PostMethod)
//	{	//如果是Post方法,发送请求后接着发送Post数据
//		pBuf = new char[httprequest.size() + DataLen];
//		SendLen = httprequest.size() + DataLen;
//		memcpy(pBuf, httprequest.c_str(), httprequest.size());
//		memcpy(pBuf + httprequest.size(), pPostData, DataLen);
//	}
//	else
//	{
//		pBuf = new char[httprequest.size()];
//		SendLen = httprequest.size();
//		memcpy(pBuf, httprequest.c_str(), httprequest.size());
//	}
	
	pBuf = new char[httprequest.size()];
	SendLen = httprequest.size();
	memcpy(pBuf, httprequest.c_str(), httprequest.size());

	int err = 0;
	err = Send(pBuf, SendLen);

	delete []pBuf;
	if (err == -1)
	{
		return -3;
	}
	if (Method == PostMethod)
	{
		err = Send(pPostData, DataLen);
		if (err == -1)
		{
			return -3;
		}
	}

	//接受Response头
	char c[2] = "";
	char end[] = "\r\n\r\n";
	bool getresponse = false;
	string Response;
	while (true)
	{
		err = recv(s_, c, 1, 0);
		if (err == SOCKET_ERROR)
		{
			return -4;
		}
		if (err == 0)
		{	//连接中断,传输结束
			return -4;
		}
		Response += c;
		if (Response.size() >= 4)
		{
			if (memcmp(Response.c_str() + Response.size() - 4, end, 4) == 0)
			{
				getresponse = true;
				break;
			}
		}
	}
	SafeReleaseInfo();
	GetField(Response.c_str(), "Server", &ResponseInfo_.ServerType);
	GetField(Response.c_str(), "Date", &ResponseInfo_.Date);
	GetField(Response.c_str(), "Location", &ResponseInfo_.Location);
	GetField(Response.c_str(), "Content-Length", &ResponseInfo_.ContentLen);
	GetField(Response.c_str(), "Content-Type", &ResponseInfo_.ContentType);
	GetField(Response.c_str(), "Transfer-Encoding", &ResponseInfo_.Transfer);

	GetHttpVersion(Response.c_str(), &ResponseInfo_.HttpVersion);
	GetResponseState(Response.c_str(), &ResponseInfo_.ResponseState);

	return 0;
}

int SimpleHttpClient::Send(void *pSendData, long DataLen)
{
	if (s_ == -1)
	{
		return -1;
	}
	char *ptmp = (char *)pSendData;
	int num = 0;
	int sum = 0;
	while (DataLen > sum)
	{
		num = send(s_, ptmp + sum, DataLen - sum, 0);
		if (num == SOCKET_ERROR)
		{
			return -1;
		}
		sum += num;
	}
	return 0;
}

void SimpleHttpClient::GetField(const char *Response, const char *pName, char **Val)
{
	if (Response == NULL || pName == NULL || Val == NULL)
	{
		return;
	}
	*Val = NULL;

	const char *pfind = strstr(Response, pName);
	if (pfind == NULL)
	{
		return;
	}
	const char *pfindrt = strstr(pfind, "\r\n");
	if (pfindrt == NULL)
	{
		return;
	}
	pfind = strstr(pfind, ":");
	if (pfind == NULL || pfind > pfindrt)
	{
		return;
	}
	++pfind;
	while ((*pfind == ' ' || *pfind == '\t') && pfind < pfindrt) 
		++pfind;
	if (pfind >= pfindrt)
	{
		return;
	}

	*Val = new char[pfindrt - pfind + 1];
	memcpy(*Val, pfind, pfindrt - pfind);
	(*Val)[pfindrt - pfind] = '\0';
}

void SimpleHttpClient::GetHttpVersion(const char *Response, char **Val)
{
	if (Response == NULL || Val == NULL)
	{
		return;
	}
	*Val = NULL;

	const char *pfindrt = strstr(Response, "\r\n");
	if (pfindrt == NULL)
		return;

	const char *pfind = Response;
	while ((*pfind != ' ' && *pfind != '\t')  && pfind < pfindrt)
		++pfind;
	if (*pfind == '\0' || pfind >= pfindrt)
		return;

	*Val = new char[pfind - Response + 1];
	memcpy(*Val, Response, pfind - Response);
	(*Val)[pfind - Response] = '\0';
}

void SimpleHttpClient::GetResponseState(const char *Response, char **Val)
{
	if (Response == NULL || Val == NULL)
	{
		return;
	}
	*Val = new char[4];
	strcpy(*Val, "404");

	const char *pfindrt = strstr(Response, "\r\n");
	if (pfindrt == NULL)
		return;

	//找到第一个空格
	const char *pfind = Response;
	while ((*pfind != ' ' && *pfind != '\t') && pfind < pfindrt)
		++pfind;
	if (*pfind == '\0' || pfind >= pfindrt)
		return;

	//从第一个空格起找到第一个不是空格的位置
	while ((*pfind == ' ' || *pfind == '\t') && pfind < pfindrt) 
		++pfind;
	if (pfind >= pfindrt)
		return;

	//找到下一个空格
	const char *pfind1 = pfind;
	while ((*pfind1 != ' ' && *pfind1 != '\t') && pfind1 < pfindrt)
		++pfind1;
	if (*pfind == '\0' || pfind >= pfindrt)
		return;

	delete *Val;
	*Val = new char[pfind1-pfind+1];
	memcpy(*Val, pfind, pfind1-pfind);
	(*Val)[pfind1-pfind] = '\0';
}

//SendRequest成功后,调用该方法,可以获得服务器返回的信息,比如:请求成功否等。
//具体见ResponseInfo结构说明
void SimpleHttpClient::GetResponseInfo(ResponseInfo &Info)
{
	Info.ContentLen		= ResponseInfo_.ContentLen;
	Info.ContentType	= ResponseInfo_.ContentType;
	Info.Date			= ResponseInfo_.Date;
	Info.HttpVersion	= ResponseInfo_.HttpVersion;
	Info.Location		= ResponseInfo_.Location;
	Info.ResponseState	= ResponseInfo_.ResponseState;
	Info.ServerType		= ResponseInfo_.ServerType;
	Info.Transfer		= ResponseInfo_.Transfer;
}

//在发送请求(SendRequest方法)成功后,调用该方法获得服务器返回的内容(网页,图片,文件等)
//pBuf,传入参数,用于接收数据的缓冲区地址
//BufLen,传入参数,用于传入缓冲区大小,字节数
//返回:成功返回实际接收数据的字节数,该数可能小于Buflen
//失败返回-1
//返回0表示服务器终止连接。
int SimpleHttpClient::RecvData(char *pBuf, long BufLen)
{
	if (s_ == -1)
	{
		return -1;
	}
	return recv(s_, pBuf, BufLen, 0);
}

//将接收到的流数据内容拼接起来,成功返回0,失败返回-1
int GetStreamContent(char *pBuf, long *pContentLen)
{
	if (NULL == pBuf || NULL == pContentLen)
	{
		return -1;
	}
	long blocklen = 0;
	char *pDealPos = pBuf;
	char *pNextPos = NULL;
	long DataLen = 0;

	while (true)
	{
		blocklen = 0;
		sscanf(pDealPos, "%X", &blocklen);
		if (blocklen <= 0)
		{
			*pContentLen = DataLen;
			return 0;
		}
		pNextPos = strstr(pDealPos, "\r\n");
		if (pNextPos == NULL)
		{
			return -1;
		}
		pDealPos = pNextPos + 2;
		memcpy(pBuf + DataLen, pDealPos, blocklen);
		DataLen += blocklen;
		pDealPos += blocklen + 2;
	}
	return 0;
}

//在发送请求(SendRequest方法)成功后,如果服务端以流的形式返回内容,调用该方法获得服务器返回的内容(网页,图片,文件等)
//当返回的response的contentlength等于0,表示流返回数据
//ppBuf,传出参数,用于传出数据的缓冲区地址
//pBufLen,传出参数,用于传出接收数据的大小,字节数
//返回:成功返回0
//失败返回-1
//通过ppBuf返回的数据,需要手工调用release释放
//注意:当调用RecvStreamData成功后,必须匹配调用FreeStreamData方法释放返回的数据内存

/*
int SimpleHttpClient::RecvStreamData(char **ppBuf, long *pBufLen)
{
	if (s_ == -1)
	{
		return -1;
	}

	long buflen = BUFLEN;
	char *pbuf = new char[buflen];
	long recvlen = 0;
	long recvret = 0;
	long contentlen = 0;

	while (true)
	{
		recvret = recv(s_, pbuf + recvlen, buflen - recvlen, 0);
		if (recvret == 0)
		{	//连接终止,表示接收结束
			//接收完毕,去除流的头和尾
			if (GetStreamContent(pbuf, &contentlen) != 0)
			{
				::closesocket(s_);
				s_ = -1;
				delete []pbuf;
				return -1;
			}
			*ppBuf = pbuf;
			*pBufLen = contentlen;
			break;
		}
		else if (recvret == SOCKET_ERROR)
		{
			::closesocket(s_);
			s_ = -1;
			delete []pbuf;
			return -1;
		}
		recvlen += recvret;
		if (buflen - recvlen < 128)
		{
			char *ptmp = new char[buflen + BUFLEN];
			if (NULL == ptmp)
			{
				::closesocket(s_);
				s_ = -1;
				delete []pbuf;
				return -1;
			}
			memcpy(ptmp, pbuf, recvlen);
			delete []pbuf;
			pbuf = ptmp;
			buflen += BUFLEN;
		}
	}
	return 0;
}
*/

//如果是用流的方式传送数据 ，通过该方法得到每个块的大小。
//返回块的大小，失败返回－1
long GetBlockLen(SOCKET s)
{
	char c[2] = "";
	char end[] = "\r\n";
	int err = 0;
	string head;
	long len = 0;
	while (true)
	{
		err = recv(s, c, 1, 0);
		if (err == SOCKET_ERROR)
		{
			return -1;
		}
		if (err == 0)
		{	//连接中断,传输结束
			return -1;
		}
		head += c;
		if (head.size() >= 2)
		{
			if (memcmp(head.c_str() + head.size() - 2, end, 2) == 0)
			{
				break;
			}
		}
	}
	sscanf(head.c_str(), "%X", &len);
	return len;
}

//在发送请求(SendRequest方法)成功后,如果服务端以流的形式返回内容,调用该方法获得服务器返回的内容(网页,图片,文件等)
//当返回的response的contentlength等于0,表示流返回数据
//ppBuf,传出参数,用于传出数据的缓冲区地址
//pBufLen,传出参数,用于传出接收数据的大小,字节数
//返回:成功返回0
//失败返回-1
//通过ppBuf返回的数据,需要手工调用release释放
//注意:当调用RecvStreamData成功后,必须匹配调用FreeStreamData方法释放返回的数据内存
int SimpleHttpClient::RecvStreamData(char **ppBuf, long *pBufLen)
{
	if (s_ == -1)
	{
		return -1;
	}

	long buflen = BUFLEN;
	char blockhead[8] = "";
	char blockend[2] = "";
	char *pbuf = new char[buflen];
	unsigned long recvret = 0;
	long blocklen = 0;
	long datalen = 0;

	while (true)
	{
		//接收块头
		blocklen = GetBlockLen(s_);
		if (blocklen == 0)
		{
		    //结束块
			::closesocket(s_);
			s_ = -1;
			pbuf[datalen] = 0;
			*ppBuf = pbuf;
			*pBufLen = datalen;
			return 0;
		}
		if (blocklen < 0)
		{  //错误
			::closesocket(s_);
			s_ = -1;
			*ppBuf = pbuf;
			*pBufLen = datalen;
			return -1;
		}
		//如果内存不足重新分配内存
		if (datalen + blocklen >= buflen)
		{
			char *ptmp = new char[(((datalen + blocklen) / BUFLEN) + 1) * BUFLEN];
			if (NULL == ptmp)
			{
				::closesocket(s_);
				s_ = -1;
				*ppBuf = pbuf;
				*pBufLen = datalen;
				return -1;
			}
			memcpy(ptmp, pbuf, datalen);
			delete []pbuf;
			pbuf = ptmp;
			buflen = (((datalen + blocklen) / BUFLEN) + 1) * BUFLEN;
		}
		//接收块内容
		recvret = ::RecvData(s_, pbuf + datalen, blocklen);
		if (recvret <= 0)
		{	//连接终止,表示接收结束,或者链接错误。
			::closesocket(s_);
			s_ = -1;
			*ppBuf = pbuf;
			*pBufLen = datalen;
			return -1;
		}
		datalen += blocklen;

		//接收块结束标识
		recvret = ::RecvData(s_, blockend, sizeof(blockend));
		if (recvret < sizeof(blockend))
		{
			::closesocket(s_);
			s_ = -1;
			*ppBuf = pbuf;
			*pBufLen = datalen;
			return -1;
		}
	}
	*ppBuf = pbuf;
	*pBufLen = datalen;
	return 0;
}

//在接收流数据成功后(RecvStreamData)成功后,需要调用FreeStreamData释放数据内存
void SimpleHttpClient::FreeStreamData(char *pBuf)
{
	if (NULL != pBuf)
	{
		delete []pBuf;
	}
}

//接收流的每个块的头标识,通过blocklen参数带回block长度
//成功返回1,连接终止返回0,失败返回负数(无有效头返回-99)
int RecvStreamBlockHeader(SOCKET s, long &blocklen)
{
	char blockhead[32] = "";		//块的头不可能超过32字节
	char c;
	long recvret = 0;
	long i = 0;

	blocklen = 0;

	for (i = 0; i < 32; i++)
	{
		recvret = recv(s, &c, 1, 0);
		if (recvret <= 0)	return recvret;

		blockhead[i] = c;
		if (i > 0 && blockhead[i - 1] == '\r' && blockhead[i] == '\n')
			break;
	}
	if (i == 32)	return -99;
	sscanf(blockhead, "%X", &blocklen);
	return 1;
}

//接收数据,必须填满pbuf才返回
//成功返回接收字节数,连接终止返回0,失败返回-1
int RecvData(SOCKET s, char *pBuf, long BufLen)
{
	int recvret = 0;
	long recvlen = 0;
	while (recvlen < BufLen)
	{
		recvret = recv(s, pBuf + recvlen, BufLen - recvlen, 0);
		if (recvret <= 0)
		{
			return recvret;
		}
		recvlen += recvret;
	}
	return recvlen;
}

//职责:从服务端收取流的块数据,填充到缓冲区,如果缓冲区长度不够,自动扩展缓冲区空间,传出缓冲区中数据的长度
//返回:成功返回接收数据字节数,服务端终止传输返回0,失败返回-1
//blocklen 参数不能为<=0
int RecvStreamBlock(SOCKET s, char *&pbuf, long &buflen, long blocklen)
{
	if (buflen - blocklen < 0)
	{
		char *ptmp = new char[blocklen];
		if (ptmp == NULL)
			return -1;
		if (NULL != pbuf)
			delete []pbuf;
		pbuf = ptmp;
		buflen = blocklen;
	}
	int recvret = 0;
	recvret = RecvData(s, pbuf, blocklen);
	return recvret;
}

//接收流的每个块的尾标识
//成功返回1,连接终止返回0,失败返回负数(无有效尾返回-99)
int RecvStreamBlockEnd(SOCKET s)
{
	int recvret = 0;
	char end[2] = "";
	
	recvret = RecvData(s, end, 2);
	if (recvret <= 0)
		return recvret;
	if (end[0] == '\r' && end[1] == '\n')
		return 1;
	else
		return -99;
}

//在发送请求(SendRequest方法)成功后,如果服务端以流的形式返回内容,调用该方法获得服务器返回的内容(网页,图片,文件等)
//当返回的response的contentlength等于0,表示流返回数据
//pFun,客户传入的回调函数,RecvStreamData在接收到数据后回调pFun函数,该函数中无需释放流数据对应的内存。
//返回:成功返回0
//失败返回-1
int SimpleHttpClient::RecvStreamData(OnRecvStreamDataFun pFun)
{
	class ReleaseAll
	{
	public:
		static void releasedata(SOCKET &s, char *pbuf)
		{
			::closesocket(s);
			s = -1;
			delete []pbuf;
		}
	};

	if (pFun == NULL || s_ == -1)
	{
		return -1;
	}

	long buflen = 0;
	char *pbuf = NULL;
	long contentlen = 0;
	int recvret = 0;

	while (true)
	{
		//接收流块的头
		recvret = RecvStreamBlockHeader(s_, contentlen);
		if (recvret == 0)
		{	//连接终止
			ReleaseAll::releasedata(s_, pbuf);
			return 0;
		}
		else if (recvret < 0)
		{	//接收失败
			ReleaseAll::releasedata(s_, pbuf);
			return -1;
		}

		if (contentlen > 0)
		{
			//接收块头成功,并且块中有数据,开始接收数据
			recvret = RecvStreamBlock(s_, pbuf, buflen, contentlen);
			if (recvret == 0)
			{	//连接终止
				ReleaseAll::releasedata(s_, pbuf);
				return 0;
			}
			else if (recvret < 0 || recvret != contentlen)
			{	//接收失败
				ReleaseAll::releasedata(s_, pbuf);
				return -1;
			}
			else
			{	//接收数据成功
				//回调
				pFun(pbuf, contentlen);
			}
		}

		//接收流块的尾
		recvret = RecvStreamBlockEnd(s_);
		if (recvret == 0)
		{
			ReleaseAll::releasedata(s_, pbuf);
			return 0;
		}
		else if (recvret < 0)
		{
			ReleaseAll::releasedata(s_, pbuf);
			return -1;
		}
		else
		{	//接收尾部成功,继续下一个块
			continue;
		}
	}
	return 0;
}

//将一个URL解析成几个部分,具体解析结果参见struct HttpRequest描述
//成功返回0, 失败返回负数, -1表示参数无效
int SimpleHttpClient::ClipHttpRequest(const char *pUrl, HttpRequest *phr)
{
	if (NULL == pUrl || NULL == phr)
	{
		return -1;
	}
	const char *pfind = NULL, *pfind1 = NULL;
	pfind = pUrl;

	memset(phr, '\0', sizeof(HttpRequest));

	pfind1 = strstr(pfind, "//");
	if (pfind1 != NULL)
	{
		phr->pProtocol = new char[pfind1 - pfind + 3];
		strncpy(phr->pProtocol, pfind, pfind1 - pfind + 2);
		phr->pProtocol[pfind1 - pfind + 2] = 0;
		pfind = pfind1 + 2;
	}
	else
	{
		phr->pProtocol = new char[1];
		phr->pProtocol[0] = 0;
	}

	pfind1 = strstr(pfind, "/");
	if (pfind1 != NULL)
	{
		const char *pfind2 = strstr(pfind, ":");
		if (pfind2 == NULL)
		{	//没有指定端口号,端口号为80
			phr->Port = 80;
			phr->pHost = new char[pfind1 - pfind + 1];
			strncpy(phr->pHost, pfind, pfind1 - pfind);
			phr->pHost[pfind1 - pfind] = 0;
			pfind = pfind1;
		}
		else
		{	//有就取出端口号
			if (pfind2 > pfind1)
			{	//冒号在斜线后面表示没有指定端口号,端口号为80
				phr->Port = 80;
				phr->pHost = new char[pfind1 - pfind + 1];
				strncpy(phr->pHost, pfind, pfind1 - pfind);
				phr->pHost[pfind1 - pfind] = 0;
				pfind = pfind1;
			}
			else
			{	//指定了端口号
				phr->pHost = new char[pfind2 - pfind + 1];
				strncpy(phr->pHost, pfind, pfind2 - pfind);
				phr->pHost[pfind2 - pfind] = 0;
				phr->Port = (unsigned short)atol(pfind2 + 1);
				pfind = pfind1;
			}
		}
		//有请求目标
		phr->pObject = new char[strlen(pfind) + 1];
		strcpy(phr->pObject, pfind);
	}
	else
	{	//全部内容可能就是host和端口号
		phr->pObject = new char[1];
		phr->pObject[0] = 0;

		//找到端口号和host
		const char *pfind2 = strstr(pfind, ":");
		if (pfind2 == NULL)
		{	//没有指定端口号,端口号为80
			phr->Port = 80;
			phr->pHost = new char[strlen(pfind) + 1];
			strcpy(phr->pHost, pfind);
//			phr->pObject[strlen(pfind)] = 0;
		}
		else
		{
			phr->Port = (unsigned short)atol(pfind2 + 1);
			phr->pHost = new char[pfind2 - pfind + 1];
			strncpy(phr->pHost, pfind, pfind2 - pfind);
			phr->pHost[pfind2 - pfind] = 0;
		}
	}
	return 0;
}

//设置代理服务器
//pProxySvr传入代理服务器主机,传入NULL表示清除代理服务器设置
//ProxyPort传入代理端口
void SimpleHttpClient::SetProxyServer(const char *pProxySvr, unsigned short ProxyPort)
{
	if (AgentHost_ != NULL)
	{
		delete []AgentHost_;
		AgentHost_ = NULL;
	}
	AgentPort_ = 0;

	if (pProxySvr != NULL)
	{
		AgentHost_ = new char[strlen(pProxySvr) + 1];
		strcpy(AgentHost_, pProxySvr);
		AgentPort_ = ProxyPort;
	}
}

//向指定的URL发送请求，并得到返回信息
//该方法内部自动打开连接，并发送请求，然后得到返回结果，无须手工OpenConnection
//Method : 传入参数，Get还是Post请求，具体参见SendRequest方法说明
//pURL : 要请求的URL
//pData : 如果是Post请求，该参数表示Post的数据块
//DataLen : 如果是Post请求，该参数表示Post的数据块的长度，以字节为单位
//pResponseData : 请求成功，该参数返回服务器应答的数据（可能是一个html也可能是一个数据文件）
//pResponseLen : 请求成功，该参数返回服务器应答数据的长度。
//该方法成功返回0，失败返回负数
//-1表示参数不正确   ->  2
//-2表示URL格式不正确  -> 3
//-3表示连接到服务器失败  -> 4
//-4表示发送请求到服务器失败  -> 5
//-5表示得到response长度失败  -> 6
//-6表示得到response内容失败  -> 7
//  表示得到的response的状态错误 8
int SimpleHttpClient::Request(HTTPMethod Method, const char *pURL, char **pResponseData, long *pResponseLen, void *pData, long DataLen)
{
	if (NULL == pURL || NULL == pResponseData || NULL == pResponseLen)
	{
		return 2;
	}
	if (Method == PostMethod && (NULL == pData || 0 == DataLen) )
	{
		return 2;
	}
	*pResponseData = NULL;
	*pResponseLen = 0;
	
	SimpleHttpClient sc;
    HttpRequest hr;
    
    if (sc.ClipHttpRequest(pURL, &hr) < 0)
    {
    	return 3;
    }
    if (sc.OpenConnection(hr.pHost, hr.Port) < 0)
    {
        return 4;
    }
    if (sc.SendRequest(Method, hr.pObject, pData, DataLen) < 0)
    {
        return 5;
    }
	ResponseInfo Info;
	sc.GetResponseInfo(Info);

    if(Info.ResponseState != NULL)
    {
        long nState = atol(Info.ResponseState);
        if( nState >= 300)
        {
            return 8;
        }
    }

    long len = 0;
    if (NULL != Info.ContentLen)
    {
        len = atol(Info.ContentLen);
    }
    if (0 == len && NULL == Info.Transfer)
    {
    	return 6;
    }
    
	char *pbuf = NULL;
	if (len > 0)
	{
		pbuf = new char[len+1];
		int recvlen = 0;
		while (len >recvlen)
		{
			recvlen += sc.RecvData(pbuf+recvlen, len - recvlen);
		}
		if (recvlen <len)	
		{
			delete []pbuf;
			return 7;
		}
		pbuf[len] = 0;
		*pResponseData = pbuf;
		*pResponseLen = len;
	}
	else if (len == 0)
	{
		if (sc.RecvStreamData(pResponseData, pResponseLen) < 0 )
		{
			return 7;
		}
	}

	return 0;
}

void SimpleHttpClient::FreeResponse(char *pResponse)
{
	delete []pResponse;
}

/*
//设置直接使用IE代理
void SimpleHttpClient::UseIEProxy()
{
	string ProxySvr;
	unsigned short ProxyPort = 0;

	//得到IE代理服务器设置
	HKEY key;
	if (RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", &key) == ERROR_SUCCESS)
	{
		DWORD ProxyEnable = 0;
		DWORD Type = 0;
		DWORD Size = sizeof(ProxyEnable);
		RegQueryValueEx(key, "ProxyEnable", NULL, &Type, (LPBYTE)&ProxyEnable, &Size);
		if (ProxyEnable != 0)
		{
			char ProxyServer[1024] = "";
			Size = sizeof(ProxyServer);
			RegQueryValueEx(key, "ProxyServer", NULL, &Type, (LPBYTE)ProxyServer, &Size);
			char *pfind = strstr(ProxyServer, "http=");
			if (pfind == NULL)
			{	//没有http=表示所有代理用统一设置
				char *pfind1 = strstr(ProxyServer, ":");
				if (pfind1 != NULL)
				{
					ProxyPort = atol(pfind1 + 1);
					*pfind1 = '\0';
					ProxySvr = ProxyServer;
				}
			}
			else
			{	//有http=就取出http代理设置
				char *pfind1 = strstr(pfind, ":");
				if (pfind1 != NULL)
				{
					ProxyPort = atol(pfind1 + 1);
					*pfind1 = '\0';
					ProxySvr = pfind + 5;
				}
			}
		}
		RegCloseKey(key);
	}

	if (ProxySvr.length() < 1)
	{
		SetProxyServer(NULL, 0);
	}
	else
	{
		SetProxyServer(ProxySvr.c_str(), ProxyPort);
	}
}
*/
