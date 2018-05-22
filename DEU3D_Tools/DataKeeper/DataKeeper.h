#ifndef DATA_KEEPER_H_3CDE89DE_C280_4F55_8B5D_E0B4A9425F07_INCLUDE
#define DATA_KEEPER_H_3CDE89DE_C280_4F55_8B5D_E0B4A9425F07_INCLUDE

#pragma warning(disable:4018)

#include "IDataKeeper.h"
#include <iostream>
#include <vector>

#include "IDProvider/ID.h"
#include <OpenSP/sp.h>
#include "DEUUrlServer.h"
#include "IDataSetManager.h"
#include "DataSetManager.h"

#include "IServiceManager.h"
#include "ServiceManager.h"


#include "IService.h"
#include "Service.h"

#include "IDataSetSegment.h"
#include "export.h"

#include <Network/IDEUNetwork.h>

namespace dk
{
    class DataKeeper : public IDataKeeper
    {
    public:
        explicit DataKeeper(void);
        virtual ~DataKeeper(void);

    protected:
        //- 登录服务器
        virtual bool                    login(const std::string& strHost, const std::string& strPort, const std::string& strUser, const std::string& strPwd);

        //- 退出服务器
        virtual bool                    logout();

        //- 初始化DataKeeper，创建数据集管理器、服务管理器，初始化和创建DEUNetwork网络连接类
        bool                            initialize(const std::string& strHost, const std::string& strPort, bool& bConnect);

        //- 增加服务器节点
        bool                            addServer(std::string& szHost, std::string& ApachePort, std::string& port);

        //- 删除服务器节点
        bool                            delServer(std::string& szHost, std::string& ApachePort, std::string& port);

        //- 获取服务器节点
        bool                            getServer(std::vector<std::string>& vecSrvHost);

    public:
        //- 获取主服务IP
        std::string                     getRootHost();

        //- 获取主服务端口
        std::string                     getRootPort();

        //- 获取网络信息
        OpenSP::sp<deunw::IDEUNetwork>  getDEUNetWork();

        //- 获取数据集管理器
        OpenSP::sp<dk::IDataSetManager> getDataSetManager(void);

        //- 获取服务器管理器
        OpenSP::sp<dk::IServiceManager> getServiceManager(void);

        //- 设置所有文件路径
        void                            setTotalFiles(const std::string& sTotalFiles);

        //- 提交开始时输出日志开始信息
        virtual void                        commitBegin();

        //- 提交结束时输出日志结束信息
        virtual void                        commitEnd();

        //- 传入配置文件路径，调用日志解析函数   
        virtual bool                        readConfigFile(const std::string& sConfigFile);

        //- 获取批量上传成功的文件路径容器 
        virtual std::vector<std::string>    getCommitSuccessFiles();

        //- 获取批量上传失败的文件路径容器 
        virtual std::vector<std::string>    getCommitFailedFiles();

        //- 获取批量没有上传的文件路径容器 
        virtual std::vector<std::string>    getCommitNoneFiles();

        //- 获取所有的文件路径容器 
        virtual std::vector<std::string>    getCommitAllFiles();

        //- 获取当前文件提交状态  失败->0，成功->1，未提交->2
        virtual unsigned int                getFileCommitState(const std::string& sCurCommitFile);

        //- 清空所有的文件路径容器 
        virtual void                        clearAllFiles();

        //-获取二进制文件的指针和长度
        virtual void*                       formJsonToBsonPtr(const std::string& sJsonString, unsigned int* pBuffLen);

        //- 导出散列配置文件
        virtual void                        exportRcdAndUrlFile(const std::string& sRcdAndUrlFile);

        //- 导入散列配置文件
        virtual void                        importRcdAndUrlFile(const std::string& sRcdAndUrlFile);

        //- 获取导入的url列表
        virtual void                        getImportUrl(std::map<std::string,HostInfo>& hostImportMap);

        //- 设置是否为批量上传
        void                                setBatchUpload(const bool bBatch);

        //- 获取是否批量上传
        bool                                getBatchUpload();

    protected:
        std::string                        m_strRootHost;
        std::string                        m_strRootPort;

        OpenSP::sp<deunw::IDEUNetwork>     m_pIUrlNetwork;
        OpenSP::sp<dk::IDataSetManager>    m_pIDataSetManager;
        OpenSP::sp<dk::IServiceManager>    m_pIServiceManager;

        std::map<std::string,HostInfo>     m_mapImportHost;
        bool                               m_bIsBatchUpload;

        void exportTempRcdFile(const std::string& sTempRcdFile, std::map<unsigned int,RcdInfo>& rcdMap);
    };
}

#endif

