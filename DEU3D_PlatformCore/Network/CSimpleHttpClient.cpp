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
#ifdef    __WINDOWS__
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
#endif    //__WINDOWS__
    memset(&ResponseInfo_, '\0', sizeof(ResponseInfo_));
}

SimpleHttpClient::~SimpleHttpClient()
{
    if (INVALID_SOCKET != s_)
    {
        ::closesocket(s_);
        s_ = INVALID_SOCKET;
    }
#ifdef    __WINDOWS__
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
    if (NULL != ResponseInfo_.HttpVersion)        delete []ResponseInfo_.HttpVersion;
    if (NULL != ResponseInfo_.ResponseState)    delete []ResponseInfo_.ResponseState;    //服务器应答状态(如:返回200,302,404等)
    if (NULL != ResponseInfo_.ServerType)        delete []ResponseInfo_.ServerType;        //服务类型(IIS等)
    if (NULL != ResponseInfo_.Date)                delete []ResponseInfo_.Date;                //标准时间
    if (NULL != ResponseInfo_.Location)            delete []ResponseInfo_.Location;            //返回的目标(哪个网页,图片等)
    if (NULL != ResponseInfo_.ContentLen)        delete []ResponseInfo_.ContentLen;        //内容长度
    if (NULL != ResponseInfo_.ContentType)        delete []ResponseInfo_.ContentType;        //内容类型
    if (NULL != ResponseInfo_.Transfer)        delete []ResponseInfo_.Transfer;        //流编码，如果是流方式，必须为chunked

    memset(&ResponseInfo_, '\0', sizeof(ResponseInfo_));
}


//连接服务器,成功返回0,失败返回-1
//连接成功后才能发送请求和接受数据
//pHost:传入参数,要连接的主机,可以接受的类型有:IP地址,URL(www.sina.com等),机器名(如果可识别),LocalHost
//Port:传入参数,表示要打开的端口
int    SimpleHttpClient::OpenConnection(const char *pHost, unsigned short Port)
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

    int            err = 0;
    bool        connSuccess = false;
    sockaddr_in sd;

    memset(&sd, '\0', sizeof(sd));
    sd.sin_family = AF_INET;
//    sd.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
//    sd.sin_addr.S_un.S_un_b.s_b1 = 127;
//    sd.sin_addr.S_un.S_un_b.s_b2 = 0;
//    sd.sin_addr.S_un.S_un_b.s_b3 = 0;
//    sd.sin_addr.S_un.S_un_b.s_b4 = 1;

    const char *pTmpHost = NULL;
    unsigned TmpPort = 0;
    if (NULL != AgentHost_)
    {    //如果设置了代理服务器就连接代理服务
        pTmpHost = AgentHost_;
        TmpPort = AgentPort_;
    }
    else
    {    //否则就直接连接主机
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
        {    //连接成功
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
    {    //连接主机失败
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
int SimpleHttpClient::SendRequest(HTTPMethod Method, const std::string &strObj, const std::vector<char> &vecPostData)
{
    if (s_ == -1 || Host_ == NULL)
    {
        return -1;
    }

    //创建Http请求协议包:Get方法
    std::string httprequest;
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
        httprequest += strObj;
        httprequest += " HTTP/1.1\r\n";
        break;
    case PostMethod:
        //第一行,请求路径和协议版本
//        httprequest += "POST * HTTP/1.1\r\n";
        httprequest += "POST ";
        httprequest += tmpstr;
        httprequest += strObj;
        httprequest += " HTTP/1.1\r\n";
        memset(tmpstr, '\0', sizeof(tmpstr));

        //Content-Length是Post必须具备的参数
        httprequest += "Content-Length: ";
        sprintf(tmpstr, "%d", vecPostData.size());
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
    //最后一行,空行
    httprequest += "\r\n";

    std::vector<char>   vecSendData(httprequest.begin(), httprequest.end());
    int err = Send(vecSendData);
    if (err == -1)
    {
        return -3;
    }

    if (Method == PostMethod)
    {
        err = Send(vecPostData);
        if (err == -1)
        {
            return -3;
        }
    }

    //接受Response头
    char c[2] = "";
    char end[] = "\r\n\r\n";
    bool getresponse = false;
    std::string Response;
    while (true)
    {
        err = recv(s_, c, 1, 0);
        if (err == SOCKET_ERROR)
        {
            return -4;
        }
        if (err == 0)
        {    //连接中断,传输结束
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

int SimpleHttpClient::Send(const std::vector<char> &vecData)
{
    if (s_ == -1)
    {
        return -1;
    }

    char *ptmp = (char *)vecData.data();
    int num = 0;
    int sum = 0;
    while ((int)vecData.size() > sum)
    {
        num = send(s_, ptmp + sum, vecData.size() - sum, 0);
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


//将一个URL解析成几个部分,具体解析结果参见struct HttpRequest描述
//成功返回0, 失败返回负数, -1表示参数无效
int SimpleHttpClient::ClipHttpRequest(const std::string &strURL, HttpRequest *phr)
{
    if (strURL.empty() || NULL == phr)
    {
        return -1;
    }
    const char *pfind = NULL, *pfind1 = NULL;
    pfind = strURL.c_str();

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
        {    //没有指定端口号,端口号为80
            phr->Port = 80;
            phr->pHost = new char[pfind1 - pfind + 1];
            strncpy(phr->pHost, pfind, pfind1 - pfind);
            phr->pHost[pfind1 - pfind] = 0;
            pfind = pfind1;
        }
        else
        {    //有就取出端口号
            if (pfind2 > pfind1)
            {    //冒号在斜线后面表示没有指定端口号,端口号为80
                phr->Port = 80;
                phr->pHost = new char[pfind1 - pfind + 1];
                strncpy(phr->pHost, pfind, pfind1 - pfind);
                phr->pHost[pfind1 - pfind] = 0;
                pfind = pfind1;
            }
            else
            {    //指定了端口号
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
    {    //全部内容可能就是host和端口号
        phr->pObject = new char[1];
        phr->pObject[0] = 0;

        //找到端口号和host
        const char *pfind2 = strstr(pfind, ":");
        if (pfind2 == NULL)
        {    //没有指定端口号,端口号为80
            phr->Port = 80;
            phr->pHost = new char[strlen(pfind) + 1];
            strcpy(phr->pHost, pfind);
//            phr->pObject[strlen(pfind)] = 0;
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
int SimpleHttpClient::Request(HTTPMethod Method, const std::string &strURL, std::vector<char> &vecResponseData, const std::vector<char> &vecData)
{
    if (strURL.empty())
    {
        return 2;
    }
    if (Method == PostMethod && vecData.empty())
    {
        return 2;
    }

    vecResponseData.clear();

    HttpRequest hr;
    if (ClipHttpRequest(strURL, &hr) < 0)
    {
        return 3;
    }
    if (OpenConnection(hr.pHost, hr.Port) < 0)
    {
        return 4;
    }
    if (SendRequest(Method, hr.pObject, vecData) < 0)
    {
        return 5;
    }

    if(ResponseInfo_.ResponseState != NULL)
    {
        long nState = atol(ResponseInfo_.ResponseState);
        if( nState >= 300)
        {
            return 8;
        }
    }

    long len = 0;
    if (NULL != ResponseInfo_.ContentLen)
    {
        len = atol(ResponseInfo_.ContentLen);
    }
    if (0 == len && NULL == ResponseInfo_.Transfer)
    {
        return 6;
    }
    
    if(len <= 0)
    {
        return 7;
    }

    std::vector<char>   vecBuffer(unsigned(len + 1));
    int recvlen = 0;
    while (len >recvlen)
    {
        recvlen += RecvData(vecBuffer.data()+recvlen, len - recvlen);
    }
    if (recvlen <len)
    {
        return 7;
    }

    vecResponseData.swap(vecBuffer);

    return 0;
}

