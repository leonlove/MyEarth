#ifndef I_SERVICE_MANAGER_H_815F9E0B_9F93_44AA_AACD_551587A37D06_INCLUDE
#define I_SERVICE_MANAGER_H_815F9E0B_9F93_44AA_AACD_551587A37D06_INCLUDE

#include <OpenSP/Ref.h>
#include <string>
#include <Common/variant.h>


namespace dk
{

class IDataKeeper;
class IService;
class IServiceManager : public OpenSP::Ref
{
public:
    //- 初始化服务器管理器
    virtual void                        init()                                                                                         = 0;
    
    //- 根据服务器IP和端口增加服务器节点
    virtual bool                        addService(const std::string &strIP, unsigned nPort, OpenSP::sp<dk::IService>& pServiceObj)    = 0;

    //- 增加服务器节点
    virtual bool                        addService(OpenSP::sp<dk::IService> pServiceObj)                                               = 0;

    //- 删除服务器节点
    virtual bool                        delService(const std::string &strIP, unsigned nPort)                                           = 0;

    //- 获取服务器节点个数
    virtual unsigned                    getServiceCount(void) const                                                                    = 0;

    //- 根据索引获取服务器节点
    virtual OpenSP::sp<IService>        getService(unsigned nIndex)                                                                    = 0;

    //- 根据服务器IP和端口查找服务器节点对象
    virtual OpenSP::sp<IService>        findService(const std::string &strIP, unsigned nPort)                                          = 0;

    //- 根据指定的数据集编码查找服务器列表
    virtual bool                        findServiceByCode(unsigned nCode, cmm::variant_data &varServiceList)                           = 0;

    //- 开启所有服务
    virtual bool                        start(void)                                                                                    = 0;

    //- 关闭所有服务
    virtual bool                        stop(void)                                                                                     = 0;

    //- 更新服务器管理器
    virtual bool                        update(void)                                                                                   = 0;

    //- 提交所有服务器节点
    virtual bool                        commit(void)                                                                                   = 0;

    //- 获取服务器状态信息
    virtual void                        stateInfo(std::vector<std::string>& vecServiceState)                                           = 0;

    //- 增加服务器节点
    virtual bool                        addServer(std::string& szHost, std::string& ApachePort, std::string& port)                     = 0;
    
    //- 获取服务器节点
    virtual void                        getServer(std::vector<std::string>& vecSrvHost)                                                = 0;

    //- 删除服务器节点
    virtual bool                        delServer(std::string& szHost, std::string& ApachePort, std::string& port)                     = 0;

    //- 创建服务器对象
    virtual OpenSP::sp<dk::IService>    createService(void)                                                                            = 0;

    //- 设置当前服务器管理器所属DataKeeper实例对象
    virtual void                        setDataKeeper(OpenSP::sp<IDataKeeper> pDataKeeper)                                             = 0;

    //- 获取当前服务器管理器的DataKeeper实例对象
    virtual OpenSP::sp<IDataKeeper>     getDataKeeper()                                                                                = 0;

    //- 根据IP查找Apache端口
    virtual std::string                 findApache(const std::string& strIP)                                                                 = 0;

};

DATAKEEPER_API dk::IServiceManager* CreateServiceManager(void);

}

#endif
