#ifndef I_DATA_SET_MANAGER_H_E2911C70_A70A_4F79_9568_34541CB3537F_INCLUDE
#define I_DATA_SET_MANAGER_H_E2911C70_A70A_4F79_9568_34541CB3537F_INCLUDE

#include <OpenSP/Ref.h>
#include <string>
#include "DEUUrlServer.h"
#include "export.h"

namespace dk{

class IDataKeeper;
class IService;
class IDataSet;
class IDataSetSegment;

class IDataSetManager : public OpenSP::Ref
{
public:
    //- 初始化数据集管理器
    virtual void                                init()                                                                                        = 0;

    //- 增加本地数据
    virtual OpenSP::sp<IDataSet>                addLocalDataSet(const std::string &strDB)                                                     = 0;

    //- 删除本地数据
    virtual bool                                removeLocalDataSet(const std::string &strDB)                                                  = 0;

    //- 创建数据集对象并添加到本地数据集容器中
    virtual OpenSP::sp<IDataSet>                createLogicObj(const unsigned DataSetID, const std::string &strName)                          = 0;

    //- 根据数据集ID删除数据集对象
    virtual bool                                removeLogicObj(const unsigned DataSetID)                                                      = 0;

    //- 获取数据集管理器中数据集的个数
    virtual unsigned                            getDataSetCount(void) const                                                                   = 0;

    //- 根据数据集编码删除数据集对象
    virtual bool                                deleteDataSet(unsigned nDataSetCode)                                                       = 0;

    //- 根据服务器对象及数据集编码删除数据集对象
    virtual bool                                deleteDataSetByCodeAndService(unsigned nCode, OpenSP::sp<IService> pServiceObj)            = 0;

    //- 根据索引来获取数据集管理器上的数据集对象，方便循环遍历容器取出数据集对象
    virtual OpenSP::sp<IDataSet>                getDataSetByIndex(unsigned nIndex)                                                         = 0;

    //- 获取数据集管理器容器中指定数据集编码的的数据集对象
    virtual OpenSP::sp<IDataSet>                getDataSetByCode(unsigned nCode)                                                           = 0;

    //- 根据服务器对象及数据集编码获取指定的数据集对象
    virtual OpenSP::sp<IDataSet>                getDataSetByCodeAndService(unsigned nCode, OpenSP::sp<IService> pServiceObj)               = 0;

    //- 提交数据集管理器中的数据
    virtual bool                                commit(void)                                                                                  = 0;

    //- 更新数据集管理器中的数据
    virtual void                                update(void)                                                                                  = 0;

    //- 设置当前数据集管理器所属DataKeeper实例对象
    virtual void                                setDataKeeper(OpenSP::sp<IDataKeeper> pDataKeeper)                                            = 0;

    //- 获取当前数据集管理器的DataKeeper实例对象
    virtual OpenSP::sp<IDataKeeper>             getDataKeeper()                                                                               = 0;

};



}

#endif
