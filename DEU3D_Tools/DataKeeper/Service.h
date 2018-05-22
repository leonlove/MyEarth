#ifndef SERVICE_OBJ_H_8F49F812_6CAA_481D_AC0D_387CF3050FE0_INCLUDE
#define SERVICE_OBJ_H_8F49F812_6CAA_481D_AC0D_387CF3050FE0_INCLUDE

#include "IService.h"
#include "DataSetSegment.h"

namespace dk
{

class Service : public IService
{
private:
    std::string                                 m_strIP;
    unsigned                                    m_nPort;
    bool                                        m_bState;
    ID*                                         m_pEventFeature;

    std::set<OpenSP::sp<dk::IDataSetSegment>>   m_SetSegment;
    OpenSP::sp<IServiceManager>                 m_pServiceManager;

public:
    explicit Service(void);
    virtual ~Service(void);

public:
    //- ������Ϣ�¼�ID
    virtual void                                       setEventFeatureID(ID* pID);
    
    //- ��������
    virtual bool                                       start(ea::IEventAdapter* p_EventAdapter);
    
    //- ֹͣ����
    virtual bool                                       stop(ea::IEventAdapter* p_EventAdapter);
    
    //- ��ȡIP��Ϣ
    virtual const std::string &                        getIP(void) const;
    
    //- ����IP��Ϣ
    virtual void                                       setIP(const std::string &strIP);
    
    //- ��ȡ�˿ں�
    virtual unsigned                                   getPort(void) const;
    
    //- ���ö˿ں�
    virtual void                                       setPort(unsigned nPort);
    
    //- �жϵ�ǰ�����Ƿ���������
    virtual bool                                       isRunning(void);
    
    //- �������ݼ�Ƭ��
    virtual bool                                       addDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg);
    
    //- ɾ�����ݼ�Ƭ��
    virtual bool                                       delDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg);
    
    //- ����ָ�������ݼ������ȡ���ݼ�����
    virtual OpenSP::sp<IDataSet>                       getDataSetByCode(unsigned nDataSetCode);
    
    //- ��ȡ��ǰ�������ڵ��µ����ݼ�Ƭ�θ���
    virtual unsigned                                   getDataSetSegmentCount();
    
    //- ��ȡָ����ŵ����ݼ�Ƭ�ζ���
    virtual OpenSP::sp<dk::IDataSetSegment>            getDataSetSegment(unsigned index);
    
    //- ��ȡ���ݼ�Ƭ���б�
    virtual std::set<OpenSP::sp<dk::IDataSetSegment>>& getDataSetSegments();

    //- �������ݼ�Ƭ��
    OpenSP::sp<dk::IDataSetSegment>                    createDataSetSegment(void);
    
    //- ���õ�ǰ��������������������������
    virtual void                                       setServiceManager(OpenSP::sp<IServiceManager> pServiceManager);
    
    //- ��ȡ��ǰ����������ķ�����������
    virtual OpenSP::sp<IServiceManager>                getServiceManager();
};

dk::IService* createService();

}
#endif

