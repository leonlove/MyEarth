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
        //- Ϊ��ǰ���ݼ�Ƭ�ι�������������
        virtual bool                        addService(OpenSP::sp<IService> pServiceObj);

        //- ��ȡ��ǰ���ݼ�Ƭ�ι����ķ������������
        virtual unsigned                    getServicesCount(void) const;

        //- ɾ��ָ���ķ���������
        virtual bool                        deleteService(unsigned nIndex);

        //- ɾ�����еķ���������
        virtual bool                        deleteAllService();

        //- ��ȡָ���ķ���������
        virtual OpenSP::sp<IService>        getService(unsigned nIndex);

        //- ָ����ǰ���ݼ�Ƭ��������һ�����ݼ�����
        virtual void                        setParent(OpenSP::sp<IDataSet> pParent);

        //- ��ȡ��ǰ���ݼ�Ƭ�����������ݼ�����
        virtual OpenSP::sp<IDataSet>        getParent();

        //- ͳ�Ƶ�ǰ���ݼ�Ƭ�εļ�¼���ܺ�
        virtual unsigned                    getDataItemCount(void);

        //- ��ȡ��ǰ���ݼ�Ƭ�ε����ݼ���¼��ID�б�
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
