#ifndef SERVICE_OBJ_H_8F49F812_6CAA_481D_AC0D_387CF3050FE0_INCLUDE
#define SERVICE_OBJ_H_8F49F812_6CAA_481D_AC0D_387CF3050FE0_INCLUDE

#include "IService.h"
#include "DataSetSegment.h"

namespace dk
{

class Service : public IService
{
private:
    std::string                                 m_strIP;
    unsigned                                    m_nPort;
    bool                                        m_bState;
    ID*                                         m_pEventFeature;

    std::set<OpenSP::sp<dk::IDataSetSegment>>   m_SetSegment;
    OpenSP::sp<IServiceManager>                 m_pServiceManager;

public:
    explicit Service(void);
    virtual ~Service(void);

public:
    //- 设置消息事件ID
    virtual void                                       setEventFeatureID(ID* pID);
    
    //- 启动服务
    virtual bool                                       start(ea::IEventAdapter* p_EventAdapter);
    
    //- 停止服务
    virtual bool                                       stop(ea::IEventAdapter* p_EventAdapter);
    
    //- 获取IP信息
    virtual const std::string &                        getIP(void) const;
    
    //- 设置IP信息
    virtual void                                       setIP(const std::string &strIP);
    
    //- 获取端口号
    virtual unsigned                                   getPort(void) const;
    
    //- 设置端口号
    virtual void                                       setPort(unsigned nPort);
    
    //- 判断当前服务是否正在运行
    virtual bool                                       isRunning(void);
    
    //- 增加数据集片段
    virtual bool                                       addDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg);
    
    //- 删除数据集片段
    virtual bool                                       delDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg);
    
    //- 根据指定的数据集编码获取数据集对象
    virtual OpenSP::sp<IDataSet>                       getDataSetByCode(unsigned nDataSetCode);
    
    //- 获取当前服务器节点下的数据集片段个数
    virtual unsigned                                   getDataSetSegmentCount();
    
    //- 获取指定序号的数据集片段对象
    virtual OpenSP::sp<dk::IDataSetSegment>            getDataSetSegment(unsigned index);
    
    //- 获取数据集片段列表
    virtual std::set<OpenSP::sp<dk::IDataSetSegment>>& getDataSetSegments();

    //- 创建数据集片段
    OpenSP::sp<dk::IDataSetSegment>                    createDataSetSegment(void);
    
    //- 设置当前服务器对象所属服务器管理器
    virtual void                                       setServiceManager(OpenSP::sp<IServiceManager> pServiceManager);
    
    //- 获取当前服务器对象的服务器管理器
    virtual OpenSP::sp<IServiceManager>                getServiceManager();
};

dk::IService* createService();

}
#endif

