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
    if (NULL != ResponseInfo_.ResponseState)    delete []ResponseInfo_.ResponseState;    //������Ӧ��״̬(��:����200,302,404��)
    if (NULL != ResponseInfo_.ServerType)        delete []ResponseInfo_.ServerType;        //��������(IIS��)
    if (NULL != ResponseInfo_.Date)                delete []ResponseInfo_.Date;                //��׼ʱ��
    if (NULL != ResponseInfo_.Location)            delete []ResponseInfo_.Location;            //���ص�Ŀ��(�ĸ���ҳ,ͼƬ��)
    if (NULL != ResponseInfo_.ContentLen)        delete []ResponseInfo_.ContentLen;        //���ݳ���
    if (NULL != ResponseInfo_.ContentType)        delete []ResponseInfo_.ContentType;        //��������
    if (NULL != ResponseInfo_.Transfer)        delete []ResponseInfo_.Transfer;        //�����룬���������ʽ������Ϊchunked

    memset(&ResponseInfo_, '\0', sizeof(ResponseInfo_));
}


//���ӷ�����,�ɹ�����0,ʧ�ܷ���-1
//���ӳɹ�����ܷ�������ͽ�������
//pHost:�������,Ҫ���ӵ�����,���Խ��ܵ�������:IP��ַ,URL(www.sina.com��),������(�����ʶ��),LocalHost
//Port:�������,��ʾҪ�򿪵Ķ˿�
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
    {    //��������˴�������������Ӵ������
        pTmpHost = AgentHost_;
        TmpPort = AgentPort_;
    }
    else
    {    //�����ֱ����������
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
        {    //���ӳɹ�
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
    {    //��������ʧ��
        ::closesocket(s_);
        s_ = -1;
        return -3;
    }

    return 0;
}

//���ø÷���ǰ����ɹ�����OpenConnection����
//�����Get����,pObj����ָ��Ҫ��ȡ������,����:
//   http://www.geostar.com.cn/index.htm/?t=a
//   1      2                 3
//����1ΪЭ������(http://)
//    2Ϊ����(www.geostar.com.cn)
//    3�Ժ������ΪҪ��ȡ������(/Index.htm/?t=a)
//��Get����pPostData��DataLen��Ч��
//�����Post����,pObj������Ч,pPostData����ΪPost������,DataLen������ʾPost���ݵ��ֽڳ���
//�÷����ɹ�����0,ʧ�ܷ��ظ���
//-1��ʾδ����OpenConnection����
//-2��ʾmethod������Ч
//-3��ʾ��������ʧ��
//-4��ʾ����Responseʧ��
int SimpleHttpClient::SendRequest(HTTPMethod Method, const std::string &strObj, const std::vector<char> &vecPostData)
{
    if (s_ == -1 || Host_ == NULL)
    {
        return -1;
    }

    //����Http����Э���:Get����
    std::string httprequest;
    char tmpstr[256] = "";

    if (AgentHost_ != NULL)
    {
        sprintf(tmpstr, "HTTP://%s:%d", Host_, Port_);
    }

    switch (Method)
    {
    case GetMethod:
        //��һ��,����·����Э��汾
        httprequest += "GET ";
        httprequest += tmpstr;
        httprequest += strObj;
        httprequest += " HTTP/1.1\r\n";
        break;
    case PostMethod:
        //��һ��,����·����Э��汾
//        httprequest += "POST * HTTP/1.1\r\n";
        httprequest += "POST ";
        httprequest += tmpstr;
        httprequest += strObj;
        httprequest += " HTTP/1.1\r\n";
        memset(tmpstr, '\0', sizeof(tmpstr));

        //Content-Length��Post����߱��Ĳ���
        httprequest += "Content-Length: ";
        sprintf(tmpstr, "%d", vecPostData.size());
        httprequest += tmpstr;
        httprequest += "\r\n";
        memset(tmpstr, '\0', sizeof(tmpstr));
        break;
    default:
        return -2;
    }
    //�ڶ���,����
    httprequest += "Host: ";
    httprequest += Host_;
    if (Port_ != 0)
    {
        sprintf(tmpstr, ":%d", Port_);
        httprequest += tmpstr;
        memset(tmpstr, '\0', sizeof(tmpstr));
    }
    httprequest += "\r\n";
    //������,������������
    httprequest += "Accept: *.*\r\n";
    //������,���������
    httprequest += "User-Agent: SimpleHttpClient\r\n";//MSIE6.00; Windows 2000\r\n";
    //Cache����
    httprequest += "Cache-Control:no-cache\r\n";
    //������,��������,����
    httprequest += "Connection: Keep-Alive\r\n";
    //������
    httprequest += "Accept-Language: zh-cn\r\n";
    //���һ��,����
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

    //����Responseͷ
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
        {    //�����ж�,�������
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

    //�ҵ���һ���ո�
    const char *pfind = Response;
    while ((*pfind != ' ' && *pfind != '\t') && pfind < pfindrt)
        ++pfind;
    if (*pfind == '\0' || pfind >= pfindrt)
        return;

    //�ӵ�һ���ո����ҵ���һ�����ǿո��λ��
    while ((*pfind == ' ' || *pfind == '\t') && pfind < pfindrt) 
        ++pfind;
    if (pfind >= pfindrt)
        return;

    //�ҵ���һ���ո�
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

//�ڷ�������(SendRequest����)�ɹ���,���ø÷�����÷��������ص�����(��ҳ,ͼƬ,�ļ���)
//pBuf,�������,���ڽ������ݵĻ�������ַ
//BufLen,�������,���ڴ��뻺������С,�ֽ���
//����:�ɹ�����ʵ�ʽ������ݵ��ֽ���,��������С��Buflen
//ʧ�ܷ���-1
//����0��ʾ��������ֹ���ӡ�
int SimpleHttpClient::RecvData(char *pBuf, long BufLen)
{
    if (s_ == -1)
    {
        return -1;
    }
    return recv(s_, pBuf, BufLen, 0);
}

//�����յ�������������ƴ������,�ɹ�����0,ʧ�ܷ���-1
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


//��һ��URL�����ɼ�������,�����������μ�struct HttpRequest����
//�ɹ�����0, ʧ�ܷ��ظ���, -1��ʾ������Ч
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
        {    //û��ָ���˿ں�,�˿ں�Ϊ80
            phr->Port = 80;
            phr->pHost = new char[pfind1 - pfind + 1];
            strncpy(phr->pHost, pfind, pfind1 - pfind);
            phr->pHost[pfind1 - pfind] = 0;
            pfind = pfind1;
        }
        else
        {    //�о�ȡ���˿ں�
            if (pfind2 > pfind1)
            {    //ð����б�ߺ����ʾû��ָ���˿ں�,�˿ں�Ϊ80
                phr->Port = 80;
                phr->pHost = new char[pfind1 - pfind + 1];
                strncpy(phr->pHost, pfind, pfind1 - pfind);
                phr->pHost[pfind1 - pfind] = 0;
                pfind = pfind1;
            }
            else
            {    //ָ���˶˿ں�
                phr->pHost = new char[pfind2 - pfind + 1];
                strncpy(phr->pHost, pfind, pfind2 - pfind);
                phr->pHost[pfind2 - pfind] = 0;
                phr->Port = (unsigned short)atol(pfind2 + 1);
                pfind = pfind1;
            }
        }
        //������Ŀ��
        phr->pObject = new char[strlen(pfind) + 1];
        strcpy(phr->pObject, pfind);
    }
    else
    {    //ȫ�����ݿ��ܾ���host�Ͷ˿ں�
        phr->pObject = new char[1];
        phr->pObject[0] = 0;

        //�ҵ��˿ںź�host
        const char *pfind2 = strstr(pfind, ":");
        if (pfind2 == NULL)
        {    //û��ָ���˿ں�,�˿ں�Ϊ80
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

//��ָ����URL�������󣬲��õ�������Ϣ
//�÷����ڲ��Զ������ӣ�����������Ȼ��õ����ؽ���������ֹ�OpenConnection
//Method : ���������Get����Post���󣬾���μ�SendRequest����˵��
//pURL : Ҫ�����URL
//pData : �����Post���󣬸ò�����ʾPost�����ݿ�
//DataLen : �����Post���󣬸ò�����ʾPost�����ݿ�ĳ��ȣ����ֽ�Ϊ��λ
//pResponseData : ����ɹ����ò������ط�����Ӧ������ݣ�������һ��htmlҲ������һ�������ļ���
//pResponseLen : ����ɹ����ò������ط�����Ӧ�����ݵĳ��ȡ�
//�÷����ɹ�����0��ʧ�ܷ��ظ���
//-1��ʾ��������ȷ   ->  2
//-2��ʾURL��ʽ����ȷ  -> 3
//-3��ʾ���ӵ�������ʧ��  -> 4
//-4��ʾ�������󵽷�����ʧ��  -> 5
//-5��ʾ�õ�response����ʧ��  -> 6
//-6��ʾ�õ�response����ʧ��  -> 7
//  ��ʾ�õ���response��״̬���� 8
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

