#ifndef DATA_SET_SEGMENT_H_235586A6_4090_4AD1_B22E_3EEBF5B46244_INCLUDE
#define DATA_SET_SEGMENT_H_235586A6_4090_4AD1_B22E_3EEBF5B46244_INCLUDE

#include "IDataSetSegment.h"
#include "IService.h"
#include "IDataSet.h"
#include <set>

namespace dk
{
    class DataSetSegment : public IDataSetSegment
    {

    public:
        explicit DataSetSegment(void);
        virtual ~DataSetSegment(void);

    protected:
        //- 为当前数据集片段关联服务器对象
        virtual bool                        addService(OpenSP::sp<IService> pServiceObj);

        //- 获取当前数据集片段关联的服务器对象个数
        virtual unsigned                    getServicesCount(void) const;

        //- 删除指定的服务器对象
        virtual bool                        deleteService(unsigned nIndex);

        //- 删除所有的服务器对象
        virtual bool                        deleteAllService();

        //- 获取指定的服务器对象
        virtual OpenSP::sp<IService>        getService(unsigned nIndex);

        //- 指定当前数据集片段属于哪一个数据集对象
        virtual void                        setParent(OpenSP::sp<IDataSet> pParent);

        //- 获取当前数据集片段所属的数据集对象
        virtual OpenSP::sp<IDataSet>        getParent();

        //- 统计当前数据集片段的记录项总和
        virtual unsigned                    getDataItemCount(void);

        //- 获取当前数据集片段的数据集记录项ID列表
        virtual bool                        getIndicesData( const std::string& strHost, 
                                                            const std::string& strPort, 
                                                            unsigned nDSCode, 
                                                            unsigned nOffset,
                                                            unsigned nCount, 
                                                            std::vector<ID>& vecIds);

    private:
        OpenSP::sp<IDataSet>               m_pParent;
        std::set<OpenSP::sp<dk::IService>> m_SetIService;
    };

    dk::IDataSetSegment* CreateDataSetSegment();
}

#endif
