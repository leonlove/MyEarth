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
        //- ��ʼ�����ݼ�������
        virtual void                            init();

        //- ���ӱ�������
        virtual OpenSP::sp<IDataSet>            addLocalDataSet(const std::string &strDB);

        //- ɾ����������
        virtual bool                            removeLocalDataSet(const std::string &strDB);

        //- �������ݼ�������ӵ��������ݼ�������
        virtual OpenSP::sp<IDataSet>            createLogicObj(const unsigned DataSetID, const std::string &strName);

        //- �������ݼ�IDɾ�����ݼ�����
        virtual bool                            removeLogicObj(const unsigned DataSetID);

        //- ��ȡ���ݼ������������ݼ��ĸ���
        virtual unsigned                        getDataSetCount(void) const;

        //- �������ݼ�����ɾ�����ݼ�����
        virtual bool                            deleteDataSet(unsigned nDataSetCode);

        //- ���ݷ������������ݼ�����ɾ�����ݼ�����
        virtual bool                            deleteDataSetByCodeAndService(unsigned nCode, OpenSP::sp<IService> pServiceObj);

        //- ������������ȡ���ݼ��������ϵ����ݼ����󣬷���ѭ����������ȡ�����ݼ�����
        virtual OpenSP::sp<IDataSet>            getDataSetByIndex(unsigned nIndex);

        //- ��ȡ���ݼ�������������ָ�����ݼ�����ĵ����ݼ�����
        virtual OpenSP::sp<IDataSet>            getDataSetByCode(unsigned nCode);

        //- ���ݷ������������ݼ������ȡָ�������ݼ�����
        virtual OpenSP::sp<IDataSet>            getDataSetByCodeAndService(unsigned nCode, OpenSP::sp<IService> pServiceObj);

         //- �ύ���ݼ��������е�����
        virtual bool                            commit(void);

        //- �������ݼ��������е�����
        virtual void                            update(void);

        //- ���õ�ǰ���ݼ�����������DataKeeperʵ������
        virtual void                            setDataKeeper(OpenSP::sp<IDataKeeper> pDataKeeper);

        //- ��ȡ��ǰ���ݼ���������DataKeeperʵ������
        virtual OpenSP::sp<IDataKeeper>         getDataKeeper();

    private:
        VECDATASET                     m_SetDataSetSrv;
        VECDATASET                     m_SetDataSetLocal;
        OpenSP::sp<IDataKeeper>        m_pDataKeeper;
    };

    dk::IDataSetManager* CreateDataSetManager(void);

}

#endif
