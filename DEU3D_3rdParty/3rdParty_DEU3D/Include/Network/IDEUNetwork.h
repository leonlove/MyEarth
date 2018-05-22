#ifndef I_DEUNETWORK_H_4B258213_B7DA_42AA_890A_E49B9FA93C37_INCLUDE
#define I_DEUNETWORK_H_4B258213_B7DA_42AA_890A_E49B9FA93C37_INCLUDE

#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include <IDProvider/ID.h>
#include <Common\IDEUException.h>
#include <Common\ErrorCode.h>
#include "Export.h"
#include <ExternalService/ITileSet.h>

namespace deunw
{
    class IDEUNetwork : public OpenSP::Ref
    {
    public:
        //初始化服务信息
        virtual bool initialize(
            const std::string& strHost,                         // 指定服务的IP地址和端口号
            const std::string& strApachePort,                   // 指定服务的端口号
            const bool bInitForFetching = true,                 // 是否简单的初始化为下载。若为false，则网络接口将初始化为网络维护，它将具备修改服务端数据的能力
            const std::string& strLocalCache = ""              // 指定本地缓存的路径
        ) = 0;
        // 根据数据集ID读取数据 
        virtual bool queryData(const ID& id, void* &pBuffer, unsigned &nBufLen,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL, unsigned nVersion = 0) = 0;
        virtual bool queryData(const ID& id,const std::string &strHost,const std::string &strPort,void* &pBuffer, unsigned &nBufLen,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL, unsigned nVersion = 0) = 0;
        // 获取数据个数
        virtual unsigned queryBlockCount(const std::string& strHost,const std::string& strPort,unsigned nDSCode,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool     queryVersion(const ID& id,std::vector<unsigned>& versionList,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // 获取指定范围的ID
        virtual bool queryIndices(const std::string& strHost,const std::string& strPort,unsigned nDSCode,
                                           unsigned nOffset,unsigned nCount,std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // 添加数据
        virtual bool addData(const ID& id,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool updateData(const ID& id,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool replaceData(const ID& id,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool addVirtTile(const void* pBuffer,unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool addProperty(const ID& id,const std::string& strProperty,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool updateProperty(const ID& id,const std::string& strProperty,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // 添加图层
        virtual bool addLayer(const ID& id,const ID& idParent,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // 更新图层
        virtual bool updateLayer(const ID& id,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // 添加符号库
        virtual bool addCategory(const ID& id,const ID& idParent,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // 更新符号库
        virtual bool updateCategory(const ID& id,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // 删除数据
        virtual bool deleteData(const ID& id, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool deleteData(const std::string& strHost,const std::string& strPort,const std::vector<ID>& idVec,
            std::vector<ID>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool deleteAllData(const std::string& strHost,const std::string& strPort, unsigned nDSCode,std::vector<ID>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool deleteLayerChildren(const std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // 删除属性
        virtual bool deleteProperty(const std::string& strHost,const std::string& strPort, unsigned nDSCode,const std::vector<ID>& idVec,
                         std::vector<ID>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool deleteAllProperty(const std::string& strHost,const std::string& strPort, unsigned nDSCode,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // 删除数据集
        virtual bool deleteDataSet(unsigned nDataSet,const std::string &strHost,const std::string &strPort,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool deleteDataSetAttr(unsigned nDataSet,const std::string &strHost,const std::string &strPort,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // 根据属性获取ID
        virtual bool queryIdByProperty(const std::string& strProperty, std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool queryIdByProperty(const std::string& strProperty,const std::string &strHost,const std::string &strPort,std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // 获取数据集、散列信息
        virtual std::string getDSInfo(const std::string &strType,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // 创建数据集\散列信息文件
        virtual bool setDSInfo(const std::string &strText,const std::string &strType,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // 启动服务
        virtual bool startService(const std::string &strHost,const std::string &strApachePort,const std::vector<std::string>& strPortVec,const std::string& strType,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool startService(const std::string &strHost,const std::string &strApachePort,const std::string &strPort,const std::string& strType,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // 查询开启的端口
        virtual bool getActivePorts(const std::string& strHost,const std::string& strApachePort,std::vector<std::string>& strPortVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;

        virtual void addTileSet(deues::ITileSet* pTileSet) = 0;
        virtual void removeTileSet(deues::ITileSet* pTileSet) = 0;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual bool login(const std::string& strHost,const std::string& strPort,const std::string& strUser,const std::string& strPwd,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool logout(OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //增加、删除服务配置 liubo 20151116
        virtual bool addServer(const std::string& strHost, const std::string& strApachePort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool delServer(const std::string& strHost, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;

        virtual bool addService(const std::string& strHost, const std::string& strApachePort, const std::string& strSerPort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool delService(const std::string& strHost, const std::string& strApachePort, const std::string& strSerPort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        
        //检测apache主服务是否启动 liubo 20151116
        virtual bool checkApache(const std::string& strHost, const std::string& strApachePort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        //检测数据端口是否启动  liubo 20151118
        virtual bool checkPortIsActive(const std::string& strHost, const std::string& strApachePort, const std::string& strSerPort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;

    };

    DEUNW_EXPORT IDEUNetwork *createDEUNetwork(void);
    DEUNW_EXPORT void    freeMemory(void *pData);

    //获取错误描述
    DEUNW_EXPORT std::string GetErrDesc(int ErrCode);
}
#endif //I_DEUNETWORK_H_4B258213_B7DA_42AA_890A_E49B9FA93C37_INCLUDE