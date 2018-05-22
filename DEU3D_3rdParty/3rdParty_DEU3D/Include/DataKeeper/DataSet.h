#ifndef DATA_SET_OBJ_H_6822561F_A9B5_49D6_91D9_9B5FA60B9C19_INCLUDE
#define DATA_SET_OBJ_H_6822561F_A9B5_49D6_91D9_9B5FA60B9C19_INCLUDE

#pragma warning(disable:4018)
#pragma warning(disable:4996)

#include <OpenSP/sp.h>
#include <DEUDB/IDEUDB.h>
#include <DEUDBProxy/IDEUDBProxy.h>
#include "IDataSet.h"
#include "DEUUrlServer.h"
#include "DEULocalDataSet.h"
#include "DataSetSegment.h"
#include "Service.h"
#include <EventAdapter/IEventObject.h>


namespace dk
{

class DataSet : public IDataSet
{
public:
    explicit DataSet(void);
    virtual ~DataSet(void);

private:
    OpenSP::sp<deudbProxy::IDEUDBProxy> m_pdeudb;
    OpenSP::sp<DEULocalTile>            m_pDEULocalTile;
    OpenSP::sp<DEULocalModel>            m_pDEULocalModel;

    ID* m_pEventFeature;
    OpenSP::sp<IDataSetManager> m_pDataSetManager;

    //- 删除远程数据
    void DelRemoteData(const std::string& strHost, const std::string& strPort, ea::IEventAdapter* pEventAdapter, const std::vector<ID>& IDCollect);

protected:
    //- 设置事件标识ID
    void                                       setEventFeatureID(ID* pID);

    //- 统计当前数据集记录项总和
    unsigned                                   getDataItemCount(void);
    
    //- 根据索引获取指定的数据集数据项
    ID                                         getDataItem(unsigned nIndex);
    
    //- 获取当前数据集对象的数据集记录项ID列表
    bool                                       getIndicesData(const std::string& strHost, const std::string& strPort, unsigned nDSCode, unsigned nOffset,unsigned nCount, std::vector<ID>& vecIds);
    
    //- 从列表中删除指定IP及端口的ID数据项
    bool                                       deleteData(const std::string& strHost, const std::string& strPort, ea::IEventAdapter* pEventAdapter, const IDList &listIDs);
    
    //- 根据ID，服务器IP地址和端口查看数据项是否存在
    bool                                       isDataItemExist(const ID &id, const std::string& strIP, const std::string& strPort);
    
    //- 局部提交
    unsigned                                   commitSection(ea::IEventAdapter* pEventAdapter, const IDList &listIDs);
    
    //- 全部提交
    unsigned                                   commit(ea::IEventAdapter* pEventAdapter);
    
    //- 终止提交
    void                                       commitBreak(ea::IEventAdapter* pEventAdapter, bool isFinish=true);
    
    //- 刷新
    void                                       refresh(void);
    
    //- 根据文件路径初始化本地数据集对象
    bool                                       initLocalData(const std::string& sFilePath);
    
    //- 删除本地文件
    void                                       removeLocalFile();
    
    //- 增加数据集片段
    bool                                       addDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg);
    
    //- 获取该数据集对象下散列区间容器的散列个数
    unsigned                                   getDataSetSegmentCount();
    
    //- 根据索引（容器序号）获取散列区间
    OpenSP::sp<dk::IDataSetSegment>            getDataSetSegment(unsigned index);
    
    //- 获取散列区间列表
    std::set<OpenSP::sp<dk::IDataSetSegment>>& getDataSetSegments();
    
    //- 创建数据集片段
    OpenSP::sp<dk::IDataSetSegment>            createDataSetSegment(void);

    //- 设置当前数据集对象所属数据集管理器
    void                                       setDataSetManager(OpenSP::sp<IDataSetManager> pDataSetManager);

    //- 获取当前数据集对象的数据集管理器
    OpenSP::sp<IDataSetManager>                getDataSetManager();

    //- 根据起始段查找散列区间
    OpenSP::sp<dk::IDataSetSegment>            findDataSetSegment(unsigned nMin, unsigned nMax);

    std::set<OpenSP::sp<dk::IDataSetSegment>> m_SetSegment;
};

dk::IDataSet* createDataSet();

}

#endif
