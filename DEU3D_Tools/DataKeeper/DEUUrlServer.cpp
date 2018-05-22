#include "DEUUrlServer.h"
#include <Common/DEUBson.h>
#include "DEUUtils.h"
#include <algorithm>
#include <Network/IDEUNetwork.h>

DEUUrlServer::DEUUrlServer(void)
{
    m_bInit = false;
    m_pIUrlNetwork = NULL;
}

DEUUrlServer::DEUUrlServer(OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork)
{
    m_bInit = false;
    m_pIUrlNetwork = pIUrlNetwork;
}

DEUUrlServer::~DEUUrlServer(void)
{
}

// 初始化根服务器
void DEUUrlServer::initialize(const std::string& strHostIP,const std::string& strApachePort, bool bInitForFetching)
{
    m_strHostIP = strHostIP;
    m_strApachePort = strApachePort;

    if (m_pIUrlNetwork)
    {
        m_pIUrlNetwork->initialize(m_strHostIP, m_strApachePort, bInitForFetching);
    }
}


bool DEUUrlServer::getInfo(std::string& strRcd,std::string& strHost,int* nErrorCode)
{
    if(m_bInit)
    {
        m_strRcdInfo = DEUUtils::getRcdBsonString(m_rcdMap);
        m_strHostInfo = DEUUtils::getHostBsonString(m_hostMap);
        strRcd = m_strRcdInfo;
        strHost = m_strHostInfo;
        return true;
    }
    else
    {
        getRcdInfo(strRcd,nErrorCode);
        getHostInfo(strHost,nErrorCode);
    }
    return true;
}


// 获取数据集对应的url信息
bool DEUUrlServer::getInfo(std::map<unsigned int,RcdInfo>& rcdMap,std::map<std::string,HostInfo>& hostMap,int* nErrorCode)
{
    rcdMap.clear();
    hostMap.clear();
    if(m_bInit)
    {
        rcdMap = m_rcdMap;
        hostMap = m_hostMap;
        return true;
    }
    else
    {
        getRcdInfo(m_strRcdInfo,nErrorCode);
        getHostInfo(m_strHostInfo,nErrorCode);
        rcdMap = m_rcdMap;
        hostMap = m_hostMap;
    }
    return true;
}


// 添加数据服务器
bool DEUUrlServer::addDataServer(const std::string strIP,const HostInfo hInfo)
{
    m_hostMap[strIP] = hInfo;
    return true;
}


// 删除数据服务器
bool DEUUrlServer::removeDataServer(const std::string strIP)
{
    m_hostMap.erase(strIP);
    return true;
}


// 删除所有数据服务器
bool DEUUrlServer::removeAllDataServer()
{
    m_hostMap.clear();
    return true;
}


// 提交修改
bool DEUUrlServer::submit(int* nErrorCode,std::map<std::string,std::vector<std::string>>& errMap)
{
    if (!m_pIUrlNetwork)
    {
        return false;
    }

    errMap.clear();
    //sub rcd info 
    m_strRcdInfo = DEUUtils::getRcdBsonString(m_rcdMap);
    bool bRcd = m_pIUrlNetwork->setDSInfo(m_strRcdInfo,"setRcdInfo");

    //sub host info
    m_strHostInfo = DEUUtils::getHostBsonString(m_hostMap);
    bool bHost = m_pIUrlNetwork->setDSInfo(m_strHostInfo,"setServerInfo");

    m_bInit = false;

    return (bRcd && bHost);
}

//- 设置网络信息
void DEUUrlServer::setDEUNetWork(OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork)
{
    m_pIUrlNetwork = pIUrlNetwork;
}

//- 获取网络信息
OpenSP::sp<deunw::IDEUNetwork> DEUUrlServer::getDEUNetWork()
{
    return m_pIUrlNetwork;
}

// 获取散列信息
/*{
    "1":
    {"name":"VirtualTile",
        "url":
    [
    {"si":1,
    "ei":50,
    "port":
    {
        "192.168.200.108":["8080","8081"],
            "192.168.200.201":["8083"]
    }
    }
    ]
    }
}*/
void DEUUrlServer::getRcdInfo(std::string& strRcd,int* nErrorCode)
{
    if (!m_pIUrlNetwork)
    {
        return;
    }

    strRcd = "";
    m_rcdMap.clear();
    strRcd = m_pIUrlNetwork->getDSInfo("getRcdInfo");
    if(strRcd == "")
    {
        return;
    }

    if(strcmp(strRcd.c_str(), "{}") == 0) { strRcd=""; }
    DEUUtils::getRcdFromBson(strRcd,m_rcdMap);
    return;
}


/*{
"192.168.200.100":
{
"ApachePort": "80",
"Services":
[
{
"Path":"Deu3DServer2",
"Ports":[8081,8082]
},
{
......
}
]
}
}*/
void DEUUrlServer::getHostInfo(std::string& strHost,int* nErrorCode)
{
    if (!m_pIUrlNetwork)
    {
        return;
    }

    strHost = "";
    m_hostMap.clear();

    strHost = m_pIUrlNetwork->getDSInfo("getServerInfo");
    if(strHost == "")
        return ;
    DEUUtils::getHostFromBson(strHost,m_hostMap);
}


// 提交数据集对应的散列信息
bool DEUUrlServer::addUrlInfo(const unsigned int nDSCode,const RcdInfo rInfo)
{
    m_rcdMap[nDSCode] = rInfo;
    return true;
}


// 删除对应的数据集散列信息
bool DEUUrlServer::removeUrlInfo(const unsigned int nDSCode)
{
    m_rcdMap.erase(nDSCode);
    return true;
}


// 删除所有散列信息
bool DEUUrlServer::removeAllUrl()
{
    m_rcdMap.clear();
    return true;
}
