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
    //- Ϊ��ǰ���ݼ�Ƭ�ι�������������
    virtual bool                            addService(OpenSP::sp<IService> pServiceObj)            = 0;

    //- ��ȡ��ǰ���ݼ�Ƭ�ι����ķ������������
    virtual unsigned                        getServicesCount(void) const                            = 0;

    //- ɾ��ָ���ķ���������
    virtual bool                            deleteService(unsigned nIndex)                          = 0;

    //- ɾ�����еķ���������
    virtual bool                            deleteAllService()                                      = 0;

    //- ��ȡָ���ķ���������
    virtual OpenSP::sp<IService>            getService(unsigned nIndex)                             = 0;

    //- ָ����ǰ���ݼ�Ƭ��������һ�����ݼ�����
    virtual void                            setParent(OpenSP::sp<IDataSet> pParent)                    = 0;

    //- ��ȡ��ǰ���ݼ�Ƭ�����������ݼ�����
    virtual OpenSP::sp<IDataSet>            getParent()                                                = 0;

    //- ͳ�Ƶ�ǰ���ݼ�Ƭ�εļ�¼���ܺ�
    virtual unsigned                        getDataItemCount(void)                                     = 0;

    //- ��ȡ��ǰ���ݼ�Ƭ�ε����ݼ���¼��ID�б�
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

