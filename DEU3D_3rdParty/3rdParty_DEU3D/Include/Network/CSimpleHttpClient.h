 //////////////////////////////////////////////////////////////////////
// �ļ�����CSimpleHttpClient.h
// ���ܣ�����SimpleHttpClient�࣬��������ʵ�ּ򵥵�Http�ͻ���

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
    char *HttpVersion;        //������HttpЭ��汾
    char *ResponseState;    //������Ӧ��״̬���磺����200��302��404�ȣ�
    char *ServerType;        //�������ͣ�IIS�ȣ�
    char *Date;                //��׼ʱ��
    char *Location;            //���ص�Ŀ�꣨�ĸ���ҳ��ͼƬ�ȣ�
    char *ContentLen;        //���ݳ���
    char *ContentType;        //��������
    char *Transfer;            //�����룬���������ʽ������Ϊ chunked
};

struct HttpRequest
{
public:
    HttpRequest();
    ~HttpRequest();
public:
    char *pProtocol;        //Э��Σ��磺Http://��
    char *pHost;            //���� ���磺172.17.1.139��www.sohu.com�ȣ�
    unsigned short Port;    //�˿�
    char *pObject;            //����Ŀ�꣨�磺/a.htm/?t=cfgfile��
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
// ����: SimpleHttpClient
// ���ܣ�ʵ�ּ򵥵�Http�ͻ���Э���װ����ɷ������󣬽���Ӧ��Ĺ���
// �������: HttpRequest, ResponseInfo
/////////////////////////////////////////////////////////////////////////////
class SimpleHttpClient  
{
public:
    SimpleHttpClient();
    virtual ~SimpleHttpClient();

public:
    //��ָ����URL�������󣬲��õ�������Ϣ
    //�÷����ڲ��Զ������ӣ�����������Ȼ��õ����ؽ���������ֹ�OpenConnection
    //Method : ���������Get����Post���󣬾���μ�SendRequest����˵��
    //pURL : Ҫ�����URL
    //pData : �����Post���󣬸ò�����ʾPost�����ݿ飬�������Post���󣬸ò�������ΪNULL
    //DataLen : �����Post���󣬸ò�����ʾPost�����ݿ�ĳ��ȣ����ֽ�Ϊ��λ
    //pResponseData : ����ɹ����ò������ط�����Ӧ������ݣ�������һ��htmlҲ����ʹһ�������ļ������ò�������ΪNULL
    //�����ص�����ʹ��FreeResponse�ͷ�
    //pResponseLen : ����ɹ����ò������ط�����Ӧ�����ݵĳ��ȣ��ò�������ΪNULL
    //�÷����ɹ�����0��ʧ�ܷ��ظ���
    //2��ʾ��������ȷ
    //3��ʾURL��ʽ����ȷ
    //4��ʾ���ӵ�������ʧ��
    //5��ʾ�������󵽷�����ʧ��
    //6��ʾ�õ�response����ʧ��
    //7��ʾ�õ�response����ʧ��
    int Request(HTTPMethod Method, const std::string &strURL, std::vector<char> &vecResponseData, const std::vector<char> &vecData = std::vector<char>());

protected:
    //���ӷ��������ɹ�����0��ʧ�ܷ��ظ�����-1��ʾ����socketʧ�ܣ�-2��ʾ������Ч��-3��ʾ����ʧ��
    //���ӳɹ�����ܷ�������ͽ�������
    //pHost �� ���������Ҫ���ӵ����������Խ��ܵ������У�IP��ַ��Url����������LocalHost
    //Port �� �����������ʾҪ�򿪵Ķ˿�
    //�ú��������ظ����ã�ÿ�γɹ��ĵ��û�ر���һ�ε����Ӵ�һ��������
    int    OpenConnection(const char *pHost, unsigned short Port);

    //���ø÷���ǰ����ɹ�����OpenConnection����
    //�����Get������pObj����ָ��Ҫ��ȡ�����ݣ����磺
    //   http://www.geostar.com.cn/index.htm/?t=a
    //   1      2                 3
    //����1ΪЭ�����ͣ�http://��
    //    2Ϊ������www.geostar.com.cn��
    //    3�Ժ������ΪҪ��ȡ�����ݣ�/Index.htm/?t=a��
    //��Get����pPostData��DataLen��Ч��
    //�����Post������pObj������Ч��pPostData����ΪPost�����ݣ�DataLen������ʾPost���ݵ��ֽڳ���
    //�÷����ɹ�����0��ʧ�ܷ��ظ���
    //-1��ʾδ����OpenConnection����
    //-2��ʾmethod������Ч
    //-3��ʾ��������ʧ��
    //-4��ʾ����Responseʧ��
    int SendRequest(HTTPMethod Method, const std::string &strObj, const std::vector<char> &vecPostData);

    //�ڷ�������SendRequest�������ɹ��󣬵��ø÷�����÷��������ص����ݣ���ҳ��ͼƬ���ļ��ȣ�
    //pBuf��������������ڽ������ݵĻ�������ַ
    //BufLen��������������ڴ��뻺������С���ֽ���
    //���أ��ɹ�����ʵ�ʽ������ݵ��ֽ�������������С��Buflen
    //ʧ�ܷ���-1
    //����0��ʾ��������ֹ���ӡ�
    int RecvData(char *pBuf, long BufLen);

    //��һ��URL�����ɼ������֣������������μ�struct HttpRequest����
    //�ɹ�����0, ʧ�ܷ��ظ���, -1��ʾ������Ч
    static int ClipHttpRequest(const std::string &strURL, HttpRequest *phr);

private:
    //�������ݣ��ɹ�����0��ʧ�ܷ���-1
    int Send(const std::vector<char> &vecData);

    //���к������ڸ�������Response
    void GetField(const char *Response, const char *pName, char **Val);
    void GetHttpVersion(const char *Response, char **Val);
    void GetResponseState(const char *Response, char **Val);

    //���溯�������ͷ�ResponseInfo�ڴ�
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
