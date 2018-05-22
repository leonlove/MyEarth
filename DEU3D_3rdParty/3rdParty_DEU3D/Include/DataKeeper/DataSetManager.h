#ifndef DATA_SET_MANAGER_H_EBD271D3_C14A_4B59_8132_6A9FCE9102B4_INCLUDE
#define DATA_SET_MANAGER_H_EBD271D3_C14A_4B59_8132_6A9FCE9102B4_INCLUDE

#pragma warning(disable:4018)


#include "IDataSetManager.h"
#include "DataSet.h"
#include "DataSetSegment.h"
#include "Service.h"
#include "export.h"


namespace dk
{
    class DataSetManager : public IDataSetManager
    {
        typedef std::set<OpenSP::sp<dk::IDataSet>> VECDATASET;

    public:
        explicit DataSetManager(void);
        virtual ~DataSetManager(void);

    protected:
        //- 初始化数据集管理器
        virtual void                            init();

        //- 增加本地数据
        virtual OpenSP::sp<IDataSet>            addLocalDataSet(const std::string &strDB);

        //- 删除本地数据
        virtual bool                            removeLocalDataSet(const std::string &strDB);

        //- 创建数据集对象并添加到本地数据集容器中
        virtual OpenSP::sp<IDataSet>            createLogicObj(const unsigned DataSetID, const std::string &strName);

        //- 根据数据集ID删除数据集对象
        virtual bool                            removeLogicObj(const unsigned DataSetID);

        //- 获取数据集管理器中数据集的个数
        virtual unsigned                        getDataSetCount(void) const;

        //- 根据数据集编码删除数据集对象
        virtual bool                            deleteDataSet(unsigned nDataSetCode);

        //- 根据服务器对象及数据集编码删除数据集对象
        virtual bool                            deleteDataSetByCodeAndService(unsigned nCode, OpenSP::sp<IService> pServiceObj);

        //- 根据索引来获取数据集管理器上的数据集对象，方便循环遍历容器取出数据集对象
        virtual OpenSP::sp<IDataSet>            getDataSetByIndex(unsigned nIndex);

        //- 获取数据集管理器容器中指定数据集编码的的数据集对象
        virtual OpenSP::sp<IDataSet>            getDataSetByCode(unsigned nCode);

        //- 根据服务器对象及数据集编码获取指定的数据集对象
        virtual OpenSP::sp<IDataSet>            getDataSetByCodeAndService(unsigned nCode, OpenSP::sp<IService> pServiceObj);

         //- 提交数据集管理器中的数据
        virtual bool                            commit(void);

        //- 更新数据集管理器中的数据
        virtual void                            update(void);

        //- 设置当前数据集管理器所属DataKeeper实例对象
        virtual void                            setDataKeeper(OpenSP::sp<IDataKeeper> pDataKeeper);

        //- 获取当前数据集管理器的DataKeeper实例对象
        virtual OpenSP::sp<IDataKeeper>         getDataKeeper();

    private:
        VECDATASET                     m_SetDataSetSrv;
        VECDATASET                     m_SetDataSetLocal;
        OpenSP::sp<IDataKeeper>        m_pDataKeeper;
    };

    dk::IDataSetManager* CreateDataSetManager(void);

}

#endif
