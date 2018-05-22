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
    //- 输出日志信息
    void                                OutputLog(const ID* pID, LOGPACKAGE& _LOGPACKAGE, ea::IEventAdapter* pEventAdapter);
    //- 设置网络信息
    void                                setDEUNetWork(OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork);
    //- 获取网络信息
    OpenSP::sp<deunw::IDEUNetwork>      getDEUNetWork();
    //- 中上提交
    bool                                IsStop(unsigned int nIndex, std::string& TaskName, ea::IEventAdapter* pEventAdapter);
    //- 设置当前数据集文件日志信息
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

//添加瓦片数据集
class DEULocalTile : public DEULocalData
{
public:
    DEULocalTile(ID* pId);
    ~DEULocalTile();
    enum DATATYPE { DEM, DOM };
    // 初始化
    bool                            initialize(const std::string& strDBPath);        
    // 提交瓦片数据，返回值：0 - 成功；1 - 终止；2 - 失败
    unsigned                        submit(ea::IEventAdapter *pEventAdapter, std::vector<ID>& IDCollect);                            //全部提交
    unsigned                        submit(const ID& id, void* Buffer, const unsigned int nLen, void (*ErrorDesc)(const char*));    //单例提交buf

public: 
    DATATYPE m_DATATYPE;
};

//添加模型矢量数据集
class DEULocalModel : public DEULocalData
{
public:
    DEULocalModel(ID* pId);
    ~DEULocalModel();
    enum DATATYPE { MODAL, VECTOR_PT, VECTOR_LINE, VECTOR_FACE};
    // 初始化
    bool                            initialize(const std::string& strDBPath);
    // 提交，返回值：0 - 成功；1 - 终止；2 - 失败
     unsigned                       submit(ea::IEventAdapter *pEventAdapter, std::vector<ID>& IDCollect);    //全部提交
     unsigned                       submit(const ID id, ea::IEventAdapter *pEventAdapter);                    //单例提交

private:
    bool                            DeleteVirtualTileFile( std::string& strTilePath );
    unsigned                        CreateVirtualTile(ea::IEventAdapter *pEventAdapter, std::vector<ID>& IDCollect);   //返回值：0 - 成功；1 - 终止；2 - 失败
    unsigned                        PostDataBlock(ea::IEventAdapter *pEventAdapter, std::vector<ID>& IDCollect);       //返回值：0 - 成功；1 - 终止；2 - 失败

public: 
    DATATYPE     m_DATATYPE;
    std::set<ID>                    m_VirtualIDCollect;
};

#endif //_DEUDATAKEEPER_DEULOCALDATASET_H_
