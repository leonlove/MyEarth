#ifndef I_SERVICE_OBJ_H_D814B2A1_AF02_47E2_B5FF_19EA22F05EAE_INCLUDE
#define I_SERVICE_OBJ_H_D814B2A1_AF02_47E2_B5FF_19EA22F05EAE_INCLUDE

#include <OpenSP/Ref.h>
#include <string>
#include <EventAdapter/IEventObject.h>
#include <EventAdapter/IEventAdapter.h>
#include "export.h"

namespace dk
{

class IServiceManager;
class IDataSetSegment;
class IDataSet;

class IService   : public OpenSP::Ref
{
public:
    //- ������Ϣ�¼�ID
    virtual void                                       setEventFeatureID(ID* pID)                                        = 0;

    //- ��������
    virtual bool                                       start(ea::IEventAdapter* p_EventAdapter)                          = 0;

    //- ֹͣ����
    virtual bool                                       stop(ea::IEventAdapter* p_EventAdapter)                           = 0;
    
    //- ��ȡIP��Ϣ
    virtual const std::string &                        getIP(void) const                                                 = 0;
    
    //- ����IP��Ϣ
    virtual void                                       setIP(const std::string &strIP)                                   = 0;
    
    //- ��ȡ�˿ں�
    virtual unsigned                                   getPort(void) const                                               = 0;
    
    //- ���ö˿ں�
    virtual void                                       setPort(unsigned nPort)                                           = 0;
    
    //- �жϵ�ǰ�����Ƿ���������
    virtual bool                                       isRunning(void)                                                   = 0;
    
    //- �������ݼ�Ƭ��
    virtual bool                                       addDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg)        = 0;
    
    //- ɾ�����ݼ�Ƭ��
    virtual bool                                       delDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg)        = 0;
    
    //- ����ָ�������ݼ������ȡ���ݼ�����
    virtual OpenSP::sp<IDataSet>                       getDataSetByCode(unsigned nDataSetCode)                        = 0;
    
    //- ��ȡ��ǰ�������ڵ��µ����ݼ�Ƭ�θ���
    virtual unsigned                                   getDataSetSegmentCount()                                          = 0;
    
    //- ��ȡָ����ŵ����ݼ�Ƭ�ζ���
    virtual OpenSP::sp<dk::IDataSetSegment>            getDataSetSegment(unsigned index)                                 = 0;
    
    //- ��ȡ���ݼ�Ƭ���б�
    virtual std::set<OpenSP::sp<dk::IDataSetSegment>>& getDataSetSegments()                                              = 0;

    //- �������ݼ�Ƭ��
    virtual OpenSP::sp<dk::IDataSetSegment>            createDataSetSegment(void)                                        = 0;

    //- ���õ�ǰ��������������������������
    virtual void                                       setServiceManager(OpenSP::sp<IServiceManager> pServiceManager)    = 0;

    //- ��ȡ��ǰ����������ķ�����������
    virtual OpenSP::sp<IServiceManager>                getServiceManager()                                               = 0;
};

}

#endif



