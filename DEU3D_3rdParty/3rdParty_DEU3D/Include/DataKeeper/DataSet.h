#ifndef DATA_SET_OBJ_H_6822561F_A9B5_49D6_91D9_9B5FA60B9C19_INCLUDE
#define DATA_SET_OBJ_H_6822561F_A9B5_49D6_91D9_9B5FA60B9C19_INCLUDE

#pragma warning(disable:4018)
#pragma warning(disable:4996)

#include <OpenSP/sp.h>
#include <DEUDB/IDEUDB.h>
#include <DEUDBProxy/IDEUDBProxy.h>
#include "IDataSet.h"
#include "DEUUrlServer.h"
#include "DEULocalDataSet.h"
#include "DataSetSegment.h"
#include "Service.h"
#include <EventAdapter/IEventObject.h>


namespace dk
{

class DataSet : public IDataSet
{
public:
    explicit DataSet(void);
    virtual ~DataSet(void);

private:
    OpenSP::sp<deudbProxy::IDEUDBProxy> m_pdeudb;
    OpenSP::sp<DEULocalTile>            m_pDEULocalTile;
    OpenSP::sp<DEULocalModel>            m_pDEULocalModel;

    ID* m_pEventFeature;
    OpenSP::sp<IDataSetManager> m_pDataSetManager;

    //- ɾ��Զ������
    void DelRemoteData(const std::string& strHost, const std::string& strPort, ea::IEventAdapter* pEventAdapter, const std::vector<ID>& IDCollect);

protected:
    //- �����¼���ʶID
    void                                       setEventFeatureID(ID* pID);

    //- ͳ�Ƶ�ǰ���ݼ���¼���ܺ�
    unsigned                                   getDataItemCount(void);
    
    //- ����������ȡָ�������ݼ�������
    ID                                         getDataItem(unsigned nIndex);
    
    //- ��ȡ��ǰ���ݼ���������ݼ���¼��ID�б�
    bool                                       getIndicesData(const std::string& strHost, const std::string& strPort, unsigned nDSCode, unsigned nOffset,unsigned nCount, std::vector<ID>& vecIds);
    
    //- ���б���ɾ��ָ��IP���˿ڵ�ID������
    bool                                       deleteData(const std::string& strHost, const std::string& strPort, ea::IEventAdapter* pEventAdapter, const IDList &listIDs);
    
    //- ����ID��������IP��ַ�Ͷ˿ڲ鿴�������Ƿ����
    bool                                       isDataItemExist(const ID &id, const std::string& strIP, const std::string& strPort);
    
    //- �ֲ��ύ
    unsigned                                   commitSection(ea::IEventAdapter* pEventAdapter, const IDList &listIDs);
    
    //- ȫ���ύ
    unsigned                                   commit(ea::IEventAdapter* pEventAdapter);
    
    //- ��ֹ�ύ
    void                                       commitBreak(ea::IEventAdapter* pEventAdapter, bool isFinish=true);
    
    //- ˢ��
    void                                       refresh(void);
    
    //- �����ļ�·����ʼ���������ݼ�����
    bool                                       initLocalData(const std::string& sFilePath);
    
    //- ɾ�������ļ�
    void                                       removeLocalFile();
    
    //- �������ݼ�Ƭ��
    bool                                       addDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg);
    
    //- ��ȡ�����ݼ�������ɢ������������ɢ�и���
    unsigned                                   getDataSetSegmentCount();
    
    //- ����������������ţ���ȡɢ������
    OpenSP::sp<dk::IDataSetSegment>            getDataSetSegment(unsigned index);
    
    //- ��ȡɢ�������б�
    std::set<OpenSP::sp<dk::IDataSetSegment>>& getDataSetSegments();
    
    //- �������ݼ�Ƭ��
    OpenSP::sp<dk::IDataSetSegment>            createDataSetSegment(void);

    //- ���õ�ǰ���ݼ������������ݼ�������
    void                                       setDataSetManager(OpenSP::sp<IDataSetManager> pDataSetManager);

    //- ��ȡ��ǰ���ݼ���������ݼ�������
    OpenSP::sp<IDataSetManager>                getDataSetManager();

    //- ������ʼ�β���ɢ������
    OpenSP::sp<dk::IDataSetSegment>            findDataSetSegment(unsigned nMin, unsigned nMax);

    std::set<OpenSP::sp<dk::IDataSetSegment>> m_SetSegment;
};

dk::IDataSet* createDataSet();

}

#endif
