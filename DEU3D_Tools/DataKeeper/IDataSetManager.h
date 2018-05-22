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
    //- ��ʼ�����ݼ�������
    virtual void                                init()                                                                                        = 0;

    //- ���ӱ�������
    virtual OpenSP::sp<IDataSet>                addLocalDataSet(const std::string &strDB)                                                     = 0;

    //- ɾ����������
    virtual bool                                removeLocalDataSet(const std::string &strDB)                                                  = 0;

    //- �������ݼ�������ӵ��������ݼ�������
    virtual OpenSP::sp<IDataSet>                createLogicObj(const unsigned DataSetID, const std::string &strName)                          = 0;

    //- �������ݼ�IDɾ�����ݼ�����
    virtual bool                                removeLogicObj(const unsigned DataSetID)                                                      = 0;

    //- ��ȡ���ݼ������������ݼ��ĸ���
    virtual unsigned                            getDataSetCount(void) const                                                                   = 0;

    //- �������ݼ�����ɾ�����ݼ�����
    virtual bool                                deleteDataSet(unsigned nDataSetCode)                                                       = 0;

    //- ���ݷ������������ݼ�����ɾ�����ݼ�����
    virtual bool                                deleteDataSetByCodeAndService(unsigned nCode, OpenSP::sp<IService> pServiceObj)            = 0;

    //- ������������ȡ���ݼ��������ϵ����ݼ����󣬷���ѭ����������ȡ�����ݼ�����
    virtual OpenSP::sp<IDataSet>                getDataSetByIndex(unsigned nIndex)                                                         = 0;

    //- ��ȡ���ݼ�������������ָ�����ݼ�����ĵ����ݼ�����
    virtual OpenSP::sp<IDataSet>                getDataSetByCode(unsigned nCode)                                                           = 0;

    //- ���ݷ������������ݼ������ȡָ�������ݼ�����
    virtual OpenSP::sp<IDataSet>                getDataSetByCodeAndService(unsigned nCode, OpenSP::sp<IService> pServiceObj)               = 0;

    //- �ύ���ݼ��������е�����
    virtual bool                                commit(void)                                                                                  = 0;

    //- �������ݼ��������е�����
    virtual void                                update(void)                                                                                  = 0;

    //- ���õ�ǰ���ݼ�����������DataKeeperʵ������
    virtual void                                setDataKeeper(OpenSP::sp<IDataKeeper> pDataKeeper)                                            = 0;

    //- ��ȡ��ǰ���ݼ���������DataKeeperʵ������
    virtual OpenSP::sp<IDataKeeper>             getDataKeeper()                                                                               = 0;

};



}

#endif
