#ifndef _DEUDATAKEEPER_DEULOCALDATASET_H_
#define _DEUDATAKEEPER_DEULOCALDATASET_H_ 

#include <sstream>
#include <DEUDBProxy/IDEUDBProxy.h>
#include <IDProvider/ID.h>
#include <DEUDB/IDEUDB.h>
#include "DEUDefine.h"
#include "export.h"    
#include <EventAdapter/IEventAdapter.h>
#include <EventAdapter/IEventObject.h>
#include <Network/IDEUNetwork.h>
#include "DEULog.h"

typedef struct
{
    ID id;
    int nError;
    std::string strErr;
    std::string m_strDBPath;
    std::vector<std::string> vecErr;
    int nOperateType;
}LOGPACKAGE;

class DEULocalData : public OpenSP::Ref
{
public:
    DEULocalData(ID* pId);
    virtual ~DEULocalData();

public:
    virtual bool                        initialize(const std::string& strDBPath) = 0;
    //- �����־��Ϣ
    void                                OutputLog(const ID* pID, LOGPACKAGE& _LOGPACKAGE, ea::IEventAdapter* pEventAdapter);
    //- ����������Ϣ
    void                                setDEUNetWork(OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork);
    //- ��ȡ������Ϣ
    OpenSP::sp<deunw::IDEUNetwork>      getDEUNetWork();
    //- �����ύ
    bool                                IsStop(unsigned int nIndex, std::string& TaskName, ea::IEventAdapter* pEventAdapter);
    //- ���õ�ǰ���ݼ��ļ���־��Ϣ
    void                                setLogInfo(OpenSP::sp<LogInfo> logInfo);

public:
    bool                            m_bIsFinish;
    ID*                             m_pEventFeature;
    OpenSP::sp<deunw::IDEUNetwork>  m_pIUrlNetwork;
    std::set<ID>                    m_setIDCollection;
    deudbProxy::IDEUDBProxy*        m_pProxy;
    std::string                     m_strDBPath;
    OpenSP::sp<LogInfo>             m_pLogInfo;
};

//�����Ƭ���ݼ�
class DEULocalTile : public DEULocalData
{
public:
    DEULocalTile(ID* pId);
    ~DEULocalTile();
    enum DATATYPE { DEM, DOM };
    // ��ʼ��
    bool                            initialize(const std::string& strDBPath);        
    // �ύ��Ƭ���ݣ�����ֵ��0 - �ɹ���1 - ��ֹ��2 - ʧ��
    unsigned                        submit(ea::IEventAdapter *pEventAdapter, std::vector<ID>& IDCollect);                            //ȫ���ύ
    unsigned                        submit(const ID& id, void* Buffer, const unsigned int nLen, void (*ErrorDesc)(const char*));    //�����ύbuf

public: 
    DATATYPE m_DATATYPE;
};

//���ģ��ʸ�����ݼ�
class DEULocalModel : public DEULocalData
{
public:
    DEULocalModel(ID* pId);
    ~DEULocalModel();
    enum DATATYPE { MODAL, VECTOR_PT, VECTOR_LINE, VECTOR_FACE};
    // ��ʼ��
    bool                            initialize(const std::string& strDBPath);
    // �ύ������ֵ��0 - �ɹ���1 - ��ֹ��2 - ʧ��
     unsigned                       submit(ea::IEventAdapter *pEventAdapter, std::vector<ID>& IDCollect);    //ȫ���ύ
     unsigned                       submit(const ID id, ea::IEventAdapter *pEventAdapter);                    //�����ύ

private:
    bool                            DeleteVirtualTileFile( std::string& strTilePath );
    unsigned                        CreateVirtualTile(ea::IEventAdapter *pEventAdapter, std::vector<ID>& IDCollect);   //����ֵ��0 - �ɹ���1 - ��ֹ��2 - ʧ��
    unsigned                        PostDataBlock(ea::IEventAdapter *pEventAdapter, std::vector<ID>& IDCollect);       //����ֵ��0 - �ɹ���1 - ��ֹ��2 - ʧ��

public: 
    DATATYPE     m_DATATYPE;
    std::set<ID>                    m_VirtualIDCollect;
};

#endif //_DEUDATAKEEPER_DEULOCALDATASET_H_
