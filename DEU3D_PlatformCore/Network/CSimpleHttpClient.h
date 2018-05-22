 //////////////////////////////////////////////////////////////////////
// 文件名：CSimpleHttpClient.h
// 功能：定义SimpleHttpClient类，该类用于实现简单的Http客户端

#ifndef    __SIMPLE_HTTP_CLIENT_H__
#define    __SIMPLE_HTTP_CLIENT_H__

//#define    __LINUX__
#include "stdio.h"
#include <vector>
#include <string>

#pragma warning( disable : 4996 )

#ifdef    _WINDOWS
  #if _MSC_VER > 1000
  #pragma once
  #endif // _MSC_VER > 1000

  #ifndef _WINSOCKAPI_
    #ifndef _WINSOCK2API_
      #include "winsock2.h"
    #endif
  #endif

#endif

#ifdef    __LINUX__
    #include "sys/types.h"
    #include <netinet/in.h>
    #include <netdb.h>
    #include "sys/socket.h"

    #define SOCKET int
    #define INVALID_SOCKET (SOCKET)(~0)
    #define SOCKET_ERROR           (-1)
    
    #define closesocket(s)  close(s)
  
#endif

struct ResponseInfo
{
    char *HttpVersion;        //服务器Http协议版本
    char *ResponseState;    //服务器应答状态（如：返回200，302，404等）
    char *ServerType;        //服务类型（IIS等）
    char *Date;                //标准时间
    char *Location;            //返回的目标（哪个网页，图片等）
    char *ContentLen;        //内容长度
    char *ContentType;        //内容类型
    char *Transfer;            //流编码，如果是流方式，必须为 chunked
};

struct HttpRequest
{
public:
    HttpRequest();
    ~HttpRequest();
public:
    char *pProtocol;        //协议段（如：Http://）
    char *pHost;            //主机 （如：172.17.1.139；www.sohu.com等）
    unsigned short Port;    //端口
    char *pObject;            //请求目标（如：/a.htm/?t=cfgfile）
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
class SimpleHttpClient  
{
public:
    SimpleHttpClient();
    virtual ~SimpleHttpClient();

public:
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
    int Request(HTTPMethod Method, const std::string &strURL, std::vector<char> &vecResponseData, const std::vector<char> &vecData = std::vector<char>());

protected:
    //连接服务器，成功返回0，失败返回负数，-1表示创建socket失败，-2表示主机无效，-3表示连接失败
    //连接成功后才能发送请求和接受数据
    //pHost ： 传入参数，要连接的主机，可以接受的类型有：IP地址，Url，机器名，LocalHost
    //Port ： 传入参数，表示要打开的端口
    //该函数可以重复调用，每次成功的调用会关闭上一次的连接打开一个新连接
    int    OpenConnection(const char *pHost, unsigned short Port);

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
    int SendRequest(HTTPMethod Method, const std::string &strObj, const std::vector<char> &vecPostData);

    //在发送请求（SendRequest方法）成功后，调用该方法获得服务器返回的内容（网页，图片，文件等）
    //pBuf，传入参数，用于接收数据的缓冲区地址
    //BufLen，传入参数，用于传入缓冲区大小，字节数
    //返回：成功返回实际接收数据的字节数，该数可能小于Buflen
    //失败返回-1
    //返回0表示服务器终止连接。
    int RecvData(char *pBuf, long BufLen);

    //将一个URL解析成几个部分，具体解析结果参见struct HttpRequest描述
    //成功返回0, 失败返回负数, -1表示参数无效
    static int ClipHttpRequest(const std::string &strURL, HttpRequest *phr);

private:
    //发送数据，成功返回0，失败返回-1
    int Send(const std::vector<char> &vecData);

    //下列函数用于辅助解释Response
    void GetField(const char *Response, const char *pName, char **Val);
    void GetHttpVersion(const char *Response, char **Val);
    void GetResponseState(const char *Response, char **Val);

    //下面函数用于释放ResponseInfo内存
    void SafeReleaseInfo();

private:
    SOCKET                s_;
    ResponseInfo        ResponseInfo_;
#ifdef    __WINDOWS__
    int                    SockInit_;
#endif    //__WINDOWS__
    char                *Host_;
    unsigned short        Port_;
    char                *AgentHost_;
    unsigned short        AgentPort_;
};

#ifdef    __WINDOWS__
  #ifndef    SIMPLEHTTPCLIENT_EXPORTS
      #ifndef    _DEBUG
          #pragma comment(lib, "ws2_32.lib")
      #else
          #pragma comment(lib, "ws2_32.lib")
      #endif
  #endif
#endif

#endif    //__SIMPLE_HTTP_CLIENT_H__
