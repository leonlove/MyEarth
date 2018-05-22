#ifndef I_DATA_SET_OBJ_H_36F54912_F421_40B0_B477_43B0728FECDB_INCLUDE
#define I_DATA_SET_OBJ_H_36F54912_F421_40B0_B477_43B0728FECDB_INCLUDE

#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include <IDProvider/ID.h>
#include <EventAdapter/IEventAdapter.h>
#include "DEUUrlServer.h"
#include "export.h"
#include <set>
#include "IDataSetManager.h"

namespace dk
{

enum DataState
{
    DS_ADD,
    DS_REMOVE,
    DS_UPDATE
};

enum DATASOURCE {
    LOCAL, //��������
    REMOTE, //Զ������
    EMPTY    //������
};

enum DATATYPE {
    TILE  = 0, //��Ƭ
    MODEL = 1,  //ģ��
    NOTHING = -1
};


class IDataSetSegment;
class IDataSet : public OpenSP::Ref
{
public:
    //- �����¼���ʶID
    virtual void                                       setEventFeatureID(ID* pID)                                                                   = 0;

    //- ͳ�Ƶ�ǰ���ݼ���¼���ܺ�
    virtual unsigned                                   getDataItemCount(void)                                                                       = 0;

    //- ����������ȡָ�������ݼ�������
    virtual ID                                         getDataItem(unsigned nIndex)                                                                 = 0;

    //- ��ȡ��ǰ���ݼ���������ݼ���¼��ID�б�
    virtual bool                                       getIndicesData(const std::string& strHost, const std::string& strPort, unsigned nDSCode,
                                                                      unsigned nOffset,unsigned nCount, std::vector<ID>& vecIds)                    = 0;
    //- ���б���ɾ��ָ��IP���˿ڵ�ID������
    virtual bool                                       deleteData(const std::string& strHost, const std::string& strPort, 
                                                                  ea::IEventAdapter* pEventAdapter, const IDList &listIDs)                          = 0;
    //- ����ID��������IP��ַ�Ͷ˿ڲ鿴�������Ƿ����
    virtual bool                                       isDataItemExist(const ID &id, const std::string& strIP, const std::string& strPort)          = 0;
    
    //- �ֲ��ύ
    virtual unsigned                                   commitSection(ea::IEventAdapter* pEventAdapter, const IDList &listIDs)                       = 0;

    //- ȫ���ύ
    virtual unsigned                                   commit(ea::IEventAdapter* pEventAdapter)                                                     = 0;

    //- ��ֹ�ύ
    virtual void                                       commitBreak(ea::IEventAdapter* pEventAdapter, bool isFinish)                                 = 0;

    //- ˢ��
    virtual void                                       refresh(void)                                                                                = 0;

    //- �����ļ�·����ʼ���������ݼ�����
    virtual bool                                       initLocalData(const std::string& sFilePath)                                                  = 0;

    //- ɾ�������ļ�
    virtual void                                       removeLocalFile()                                                                            = 0;

    //- �������ݼ�Ƭ��
    virtual bool                                       addDataSetSegment(OpenSP::sp<IDataSetSegment> pDataSetSeg)                                   = 0;

    //- ��ȡ�����ݼ�������ɢ������������ɢ�и���
    virtual unsigned                                   getDataSetSegmentCount()                                                                     = 0;
    
    //- ����������������ţ���ȡɢ������
    virtual OpenSP::sp<dk::IDataSetSegment>            getDataSetSegment(unsigned index)                                                            = 0;
    
    //- ��ȡɢ�������б�
    virtual std::set<OpenSP::sp<dk::IDataSetSegment>>& getDataSetSegments()                                                                         = 0;

    //- �������ݼ�Ƭ��
    virtual OpenSP::sp<dk::IDataSetSegment>            createDataSetSegment(void)                                                                   = 0;

    //- ���õ�ǰ���ݼ������������ݼ�������
    virtual void                                       setDataSetManager(OpenSP::sp<IDataSetManager> pDataSetManager)                               = 0;

    //- ��ȡ��ǰ���ݼ���������ݼ�������
    virtual OpenSP::sp<IDataSetManager>                getDataSetManager()                                                                          = 0;

    //- ������ʼ�β���ɢ������
    virtual OpenSP::sp<dk::IDataSetSegment>            findDataSetSegment(unsigned nMin, unsigned nMax)                                             = 0;

public:
    std::string     m_strFilePath;
    std::string     m_strName;
    unsigned        m_DataSetID;
    DATASOURCE      m_DataSource;
    DATATYPE        m_DataType;
};

}

#endif
