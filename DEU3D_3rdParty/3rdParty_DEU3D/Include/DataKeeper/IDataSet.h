#ifndef I_DATA_SET_OBJ_H_36F54912_F421_40B0_B477_43B0728FECDB_INCLUDE
#define I_DATA_SET_OBJ_H_36F54912_F421_40B0_B477_43B0728FECDB_INCLUDE

#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include <IDProvider/ID.h>
#include <EventAdapter/IEventAdapter.h>
#include "DEUUrlServer.h"
#include "export.h"
#include <set>
#include "IDataSetManager.h"

namespace dk
{

enum DataState
{
    DS_ADD,
    DS_REMOVE,
    DS_UPDATE
};

enum DATASOURCE {
    LOCAL, //本地数据
    REMOTE, //远程数据
    EMPTY    //空数据
};

enum DATATYPE {
    TILE  = 0, //瓦片
    MODEL = 1,  //模型
    NOTHING = -1
};


class IDataSetSegment;
class IDataSet : public OpenSP::Ref
{
public:
    //- 设置事件标识ID
    virtual void                                       setEventFeatureID(ID* pID)                                                                   = 0;

    //- 统计当前数据集记录项总和
    virtual unsigned                                   getDataItemCount(void)                                                                       = 0;

    //- 根据索引获取指定的数据集数据项
    virtual ID                                         getDataItem(unsigned nIndex)                                                                 = 0;

    //- 获取当前数据集对象的数据集记录项ID列表
    virtual bool                                       getIndicesData(const std::string& strHost, const std::string& strPort, unsigned nDSCode,
                                                                      unsigned nOffset,unsigned nCount, std::vector<ID>& vecIds)                    = 0;
    //- 从列表中删除指定IP及端口的ID数据项
    virtual bool                                       deleteData(const std::string& strHost, const std::string& strPort, 
                                                                  ea::IEventAdapter* pEventAdapter, const IDList &listIDs)                          = 0;
    //- 根据ID，服务器IP地址和端口查看数据项是否存在
    virtual bool                                       isDataItemExist(const ID &id, const std::string& strIP, const std::string& strPort)          = 0;
    
    //- 局部提交
    virtual unsigned                                   commitSection(ea::IEventAdapter* pEventAdapter, const IDList &listIDs)                       = 0;

    //- 全部提交
    virtual unsigned                                   commit(ea::IEventAdapter* pEventAdapter)                                                     = 0;

    //- 终止提交
    virtual void                                       commitBreak(ea::IEventAdapter* pEventAdapter, bool isFinish)                                 = 0;

    //- 刷新
    virtual void                                       refresh(void)                                                                                = 0;

    //- 根据文件路径初始化本地数据集对象
    virtual bool                                       initLocalData(const std::string& sFilePath)                                                  = 0;

    //- 删除本地文件
    virtual void                                       removeLocalFile()                                                                            = 0;

    //- 增加数据集片段
    virtual bool                                       addDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg)                                   = 0;

    //- 获取该数据集对象下散列区间容器的散列个数
    virtual unsigned                                   getDataSetSegmentCount()                                                                     = 0;
    
    //- 根据索引（容器序号）获取散列区间
    virtual OpenSP::sp<dk::IDataSetSegment>            getDataSetSegment(unsigned index)                                                            = 0;
    
    //- 获取散列区间列表
    virtual std::set<OpenSP::sp<dk::IDataSetSegment>>& getDataSetSegments()                                                                         = 0;

    //- 创建数据集片段
    virtual OpenSP::sp<dk::IDataSetSegment>            createDataSetSegment(void)                                                                   = 0;

    //- 设置当前数据集对象所属数据集管理器
    virtual void                                       setDataSetManager(OpenSP::sp<IDataSetManager> pDataSetManager)                               = 0;

    //- 获取当前数据集对象的数据集管理器
    virtual OpenSP::sp<IDataSetManager>                getDataSetManager()                                                                          = 0;

    //- 根据起始段查找散列区间
    virtual OpenSP::sp<dk::IDataSetSegment>            findDataSetSegment(unsigned nMin, unsigned nMax)                                             = 0;

public:
    std::string     m_strFilePath;
    std::string     m_strName;
    unsigned        m_DataSetID;
    DATASOURCE      m_DataSource;
    DATATYPE        m_DataType;
};

}

#endif
