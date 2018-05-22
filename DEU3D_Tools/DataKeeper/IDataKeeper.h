#include "export.h"

#ifndef I_DATA_KEEPER_H_7977E4E2_F8D5_44E7_AF8E_72D356E72DF9_INCLUDE
#define I_DATA_KEEPER_H_7977E4E2_F8D5_44E7_AF8E_72D356E72DF9_INCLUDE

#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include "IDProvider/ID.h"
#include "IDataSetManager.h"
#include "IServiceManager.h"
#include "IDataSetSegment.h"
#include "IDataSetSegmentCollection.h"
#include <Network/IDEUNetwork.h>

namespace dk{

    class IDataKeeper : public OpenSP::Ref
    {
    public:
        //- 登录服务器
        virtual bool                                login(const std::string& strHost, const std::string& strPort, 
                                                          const std::string& strUser, const std::string& strPwd)                      = 0;
        //- 退出服务器
        virtual bool                                logout()                                                                            = 0;
        //- 初始化DataKeeper，创建数据集管理器、服务管理器，初始化和创建DEUNetwork网络连接类
        virtual bool                                initialize( const std::string &strHost, const std::string &strPort, bool& bConnect) = 0;

        //- 获取数据集管理器
        virtual OpenSP::sp<dk::IDataSetManager>     getDataSetManager(void)                                                           = 0;

        //- 获取服务器管理器
        virtual OpenSP::sp<dk::IServiceManager>     getServiceManager(void)                                                           = 0;

        //- 增加服务器节点
        virtual bool                                addServer(std::string& szHost, std::string& ApachePort, std::string& port)        = 0;

        //- 删除服务器节点
        virtual bool                                delServer(std::string& szHost, std::string& ApachePort, std::string& port)        = 0;

        //- 获取服务器节点
        virtual bool                                getServer(std::vector<std::string>& vecSrvHost)                                   = 0;

        //- 获取主服务IP
        virtual std::string                         getRootHost()                                                                     = 0;

        //- 获取主服务端口
        virtual std::string                         getRootPort()                                                                     = 0;

        //- 获取网络信息
        virtual OpenSP::sp<deunw::IDEUNetwork>      getDEUNetWork()                                                                   = 0;

        //- 设置所有文件路径
        virtual void                                setTotalFiles(const std::string& sTotalFiles)                                     = 0;

        //- 提交开始时输出日志开始信息
        virtual void                                commitBegin()                                                                     = 0;

        //- 提交结束时输出日志结束信息
        virtual void                                commitEnd()                                                                       = 0;

        //- 传入配置文件路径，调用日志解析函数
        virtual bool                                readConfigFile(const std::string& sConfigFile)                                    = 0;

        //- 获取批量上传成功的文件路径容器 
        virtual std::vector<std::string>            getCommitSuccessFiles()                                                           = 0;

        //- 获取批量上传失败的文件路径容器
        virtual std::vector<std::string>            getCommitFailedFiles()                                                            = 0;

        //- 获取批量没有上传的文件路径容器
        virtual std::vector<std::string>            getCommitNoneFiles()                                                              = 0;

        //- 获取所有上传的文件路径容器
        virtual std::vector<std::string>            getCommitAllFiles()                                                               = 0;

        //- 获取当前文件提交状态  失败->0，成功->1，未提交->2
        virtual unsigned int                        getFileCommitState(const std::string& sCurCommitFile)                             = 0;

        //- 清空所有的文件路径容器
        virtual void                                clearAllFiles()                                                                   = 0;

        virtual void*                               formJsonToBsonPtr(const std::string& sJsonString, unsigned int* pBuffLen)         = 0;

        //- 导出散列配置文件
        virtual void                                exportRcdAndUrlFile(const std::string& sRcdAndUrlFile)                            = 0;

        //- 导入散列配置文件
        virtual void                                importRcdAndUrlFile(const std::string& sRcdAndUrlFile)                            = 0;

        //- 获取导入的url列表
        virtual void                                getImportUrl(std::map<std::string,HostInfo>& hostImportMap)                       = 0;

        //- 设置是否为批量上传
        virtual void                                setBatchUpload(const bool bBatch)                                                 = 0;

        //- 获取是否批量上传
        virtual bool                                getBatchUpload()                                                                  = 0;
    };

    DATAKEEPER_API dk::IDataKeeper *CreateDataKeeper(void);
}

#endif


