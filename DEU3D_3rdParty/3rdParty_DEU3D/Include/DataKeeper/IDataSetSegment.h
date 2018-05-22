#ifndef I_DATA_SET_SEGMENT_H_7626DF22_20F8_47BA_B057_AF7F17AAEA05_INCLUDE
#define I_DATA_SET_SEGMENT_H_7626DF22_20F8_47BA_B057_AF7F17AAEA05_INCLUDE

#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include "export.h"
#include <IDProvider/ID.h>
#include <set>

namespace dk
{
class IService;
class IDataSet;
class IDataSetSegment : public OpenSP::Ref
{
public:
    //- 为当前数据集片段关联服务器对象
    virtual bool                            addService(OpenSP::sp<IService> pServiceObj)            = 0;

    //- 获取当前数据集片段关联的服务器对象个数
    virtual unsigned                        getServicesCount(void) const                            = 0;

    //- 删除指定的服务器对象
    virtual bool                            deleteService(unsigned nIndex)                          = 0;

    //- 删除所有的服务器对象
    virtual bool                            deleteAllService()                                      = 0;

    //- 获取指定的服务器对象
    virtual OpenSP::sp<IService>            getService(unsigned nIndex)                             = 0;

    //- 指定当前数据集片段属于哪一个数据集对象
    virtual void                            setParent(OpenSP::sp<IDataSet> pParent)                    = 0;

    //- 获取当前数据集片段所属的数据集对象
    virtual OpenSP::sp<IDataSet>            getParent()                                                = 0;

    //- 统计当前数据集片段的记录项总和
    virtual unsigned                        getDataItemCount(void)                                     = 0;

    //- 获取当前数据集片段的数据集记录项ID列表
    virtual bool                            getIndicesData( const std::string& strHost, 
                                                            const std::string& strPort, 
                                                            unsigned nDSCode, 
                                                            unsigned nOffset,
                                                            unsigned nCount, 
                                                            std::vector<ID>& vecIds)                   = 0;
public:
    unsigned m_Min;
    unsigned m_Max;

};

}
#endif

