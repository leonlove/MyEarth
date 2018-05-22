#ifndef I_SERVICE_OBJ_H_D814B2A1_AF02_47E2_B5FF_19EA22F05EAE_INCLUDE
#define I_SERVICE_OBJ_H_D814B2A1_AF02_47E2_B5FF_19EA22F05EAE_INCLUDE

#include <OpenSP/Ref.h>
#include <string>
#include <EventAdapter/IEventObject.h>
#include <EventAdapter/IEventAdapter.h>
#include "export.h"

namespace dk
{

class IServiceManager;
class IDataSetSegment;
class IDataSet;

class IService   : public OpenSP::Ref
{
public:
    //- 设置消息事件ID
    virtual void                                       setEventFeatureID(ID* pID)                                        = 0;

    //- 启动服务
    virtual bool                                       start(ea::IEventAdapter* p_EventAdapter)                          = 0;

    //- 停止服务
    virtual bool                                       stop(ea::IEventAdapter* p_EventAdapter)                           = 0;
    
    //- 获取IP信息
    virtual const std::string &                        getIP(void) const                                                 = 0;
    
    //- 设置IP信息
    virtual void                                       setIP(const std::string &strIP)                                   = 0;
    
    //- 获取端口号
    virtual unsigned                                   getPort(void) const                                               = 0;
    
    //- 设置端口号
    virtual void                                       setPort(unsigned nPort)                                           = 0;
    
    //- 判断当前服务是否正在运行
    virtual bool                                       isRunning(void)                                                   = 0;
    
    //- 增加数据集片段
    virtual bool                                       addDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg)        = 0;
    
    //- 删除数据集片段
    virtual bool                                       delDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg)        = 0;
    
    //- 根据指定的数据集编码获取数据集对象
    virtual OpenSP::sp<IDataSet>                       getDataSetByCode(unsigned nDataSetCode)                        = 0;
    
    //- 获取当前服务器节点下的数据集片段个数
    virtual unsigned                                   getDataSetSegmentCount()                                          = 0;
    
    //- 获取指定序号的数据集片段对象
    virtual OpenSP::sp<dk::IDataSetSegment>            getDataSetSegment(unsigned index)                                 = 0;
    
    //- 获取数据集片段列表
    virtual std::set<OpenSP::sp<dk::IDataSetSegment>>& getDataSetSegments()                                              = 0;

    //- 创建数据集片段
    virtual OpenSP::sp<dk::IDataSetSegment>            createDataSetSegment(void)                                        = 0;

    //- 设置当前服务器对象所属服务器管理器
    virtual void                                       setServiceManager(OpenSP::sp<IServiceManager> pServiceManager)    = 0;

    //- 获取当前服务器对象的服务器管理器
    virtual OpenSP::sp<IServiceManager>                getServiceManager()                                               = 0;
};

}

#endif



