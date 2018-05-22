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
    // ��ʼ����������
    void initialize(const std::string& strHostIP,const std::string& strApachePort, bool bInitForFetching);

    // ��ȡɢ�к����ݷ�����Ϣ
    bool getInfo(std::map<unsigned int,RcdInfo>& rcdMap,
                 std::map<std::string,HostInfo>& hostMap,
                 int* nErrorCode = NULL);

    bool getInfo(std::string& strRcd,std::string& strHost,int* nErrorCode = NULL);

    // �ύ���ݼ���Ӧ��ɢ����Ϣ
    bool addUrlInfo(const unsigned int nDSCode,const RcdInfo rInfo);

    // ɾ����Ӧ�����ݼ�ɢ����Ϣ
    bool removeUrlInfo(const unsigned int nDSCode);

    // ɾ������ɢ����Ϣ
    bool removeAllUrl();

    // ������ݷ�����
    bool addDataServer(const std::string strIP,const HostInfo hInfo);

    // ɾ�����ݷ�����
    bool removeDataServer(const std::string strIP);

    // ɾ���������ݷ�����
    bool removeAllDataServer();

    // �ύ�޸�
    bool submit(int* nErrorCode = NULL,std::map<std::string,std::vector<std::string>>& errMap = std::map<std::string,std::vector<std::string>>());

    //- ����������Ϣ
    void setDEUNetWork(OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork);

    //- ��ȡ������Ϣ
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

