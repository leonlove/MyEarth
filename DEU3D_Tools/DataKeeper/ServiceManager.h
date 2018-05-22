#ifndef SERVICE_MANAGER_H_EBD271D3_C14A_4B59_8132_6A9FCE9102B4_INCLUDE
#define SERVICE_MANAGER_H_EBD271D3_C14A_4B59_8132_6A9FCE9102B4_INCLUDE

#pragma warning(disable:4018)

#include "export.h"
#include "IServiceManager.h"
#include "IService.h"
#include "Service.h"
#include "OpenSP/sp.h"
#include <Common/variant.h>
#include "DEUUrlServer.h"
#include <string>
#include <set>
#include <map>



namespace dk
{
    class IService;
    class ServiceManager : public IServiceManager
    {
    
    public:
        ServiceManager();
        ~ServiceManager();

    private:
        OpenSP::sp<IDataKeeper>                             m_pDataKeeper;
        std::set<OpenSP::sp<dk::IService>>                  m_SetServices;
        std::map<std::string, std::string>                  m_mapApacheHost;
        std::map<std::string, std::vector<std::string>>     m_mapHost;

    protected:
        //- 初始化服务器管理器
        void                            init();

        //- 根据服务器IP和端口增加服务器节点
        bool                            addService(const std::string &strIP, unsigned nPort, OpenSP::sp<dk::IService>& pServiceObj);

        //- 增加服务器节点
        bool                            addService(OpenSP::sp<dk::IService> pServiceObj);

        //- 删除服务器节点
        bool                            delService(const std::string &strIP, unsigned nPort);

        //- 获取服务器节点个数
        unsigned                        getServiceCount(void) const;

        //- 根据索引获取服务器节点
        OpenSP::sp<IService>            getService(unsigned nIndex);

        //- 根据服务器IP和端口查找服务器节点对象
        OpenSP::sp<IService>            findService(const std::string &strIP, unsigned nPort);

        //- 根据指定的数据集编码查找服务器列表
        bool                            findServiceByCode(unsigned nCode, cmm::variant_data &varServiceList);

        //- 开启所有服务
        bool                            start(void);

        //- 关闭所有服务
        bool                            stop(void);

        //- 更新服务器管理器
        bool                            update(void);

        //- 提交所有服务器节点
        bool                            commit(void);

        //- 获取服务器状态信息
        void                            stateInfo(std::vector<std::string>& vecServiceState);

        //- 增加服务器节点
        bool                            addServer(std::string& szHost, std::string& ApachePort, std::string& port);

        //- 删除服务器节点
        bool                            delServer(std::string& szHost, std::string& ApachePort, std::string& port);

        //- 获取服务器节点
        void                            getServer(std::vector<std::string>& vecSrvHost);

        //- 创建服务器对象
        OpenSP::sp<dk::IService>        createService(void);

        //- 设置当前服务器管理器所属DataKeeper实例对象
        void                            setDataKeeper(OpenSP::sp<IDataKeeper> pDataKeeper);

        //- 获取当前服务器管理器的DataKeeper实例对象
        OpenSP::sp<IDataKeeper>         getDataKeeper();

        //- 根据IP查找Apache端口
        std::string                     findApache(const std::string& strIP);
    };
}

#endif