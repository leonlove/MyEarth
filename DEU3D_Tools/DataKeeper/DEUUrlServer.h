#ifndef _DEUDATAKEEPER_DEUURLSERVER_H_
#define _DEUDATAKEEPER_DEUURLSERVER_H_ 

#include "export.h"
#include "DEUDefine.h"
#include <Network/IDEUNetwork.h>

class DATAKEEPER_API DEUUrlServer
{
public:
    DEUUrlServer(void);
    DEUUrlServer(OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork);
    ~DEUUrlServer(void);
    // 初始化根服务器
    void initialize(const std::string& strHostIP,const std::string& strApachePort, bool bInitForFetching);

    // 获取散列和数据服务信息
    bool getInfo(std::map<unsigned int,RcdInfo>& rcdMap,
                 std::map<std::string,HostInfo>& hostMap,
                 int* nErrorCode = NULL);

    bool getInfo(std::string& strRcd,std::string& strHost,int* nErrorCode = NULL);

    // 提交数据集对应的散列信息
    bool addUrlInfo(const unsigned int nDSCode,const RcdInfo rInfo);

    // 删除对应的数据集散列信息
    bool removeUrlInfo(const unsigned int nDSCode);

    // 删除所有散列信息
    bool removeAllUrl();

    // 添加数据服务器
    bool addDataServer(const std::string strIP,const HostInfo hInfo);

    // 删除数据服务器
    bool removeDataServer(const std::string strIP);

    // 删除所有数据服务器
    bool removeAllDataServer();

    // 提交修改
    bool submit(int* nErrorCode = NULL,std::map<std::string,std::vector<std::string>>& errMap = std::map<std::string,std::vector<std::string>>());

    //- 设置网络信息
    void setDEUNetWork(OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork);

    //- 获取网络信息
    OpenSP::sp<deunw::IDEUNetwork> getDEUNetWork();

private:
    bool                            m_bInit;
    std::string                     m_strHostIP;
    std::string                     m_strApachePort;
    std::string                     m_strRcdInfo;
    std::string                     m_strHostInfo;
    std::map<unsigned int,RcdInfo>  m_rcdMap;
    std::map<std::string,HostInfo>  m_hostMap;
    OpenSP::sp<deunw::IDEUNetwork>  m_pIUrlNetwork;

    std::map<std::string,std::map<std::string,std::vector<unsigned> > >  m_dsMap;

    void getRcdInfo(std::string& strRcd,int* nErrorCode = NULL);
    void getHostInfo(std::string& strHost,int* nErrorCode = NULL);
};
#endif //_DEUDATAKEEPER_DEUURLSERVER_H_

