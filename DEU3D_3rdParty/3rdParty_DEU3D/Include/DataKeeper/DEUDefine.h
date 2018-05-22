#ifndef _DEUDATAKEEPER_DEUDEFINE_H_
#define _DEUDATAKEEPER_DEUDEFINE_H_ 

#include <string>
#include <vector>
#include <map>
#include <OpenSP/sp.h>
#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>
#include "export.h"
#include <EventAdapter/IEventAdapter.h>
#include <EventAdapter/IEventObject.h>

struct DATAKEEPER_API HostInfo
{
    std::string m_strApachePort;
    std::vector<std::string> m_portVec;

    HostInfo()
    {
        m_strApachePort = "";
    }
    ~HostInfo()
    {
        m_strApachePort = "";
        m_portVec.clear();
    }
};

struct DATAKEEPER_API UrlInfo
{
    unsigned m_nStart;
    unsigned m_nEnd;
    std::map<std::string,std::vector<std::string>> m_urlMap;

    UrlInfo()
    {
        m_nStart = m_nEnd = 0u;
    }
    ~UrlInfo()
    {
        m_urlMap.clear();
        m_nStart = m_nEnd = 0u;
    }
};

struct DATAKEEPER_API RcdInfo
{

    std::string m_strName;
    std::vector<UrlInfo> m_urlInfoVec;

    RcdInfo()
    {
        m_strName = "";
    }
    ~RcdInfo()
    {
        m_strName = "";
        m_urlInfoVec.clear();
    }
};


enum DEURECORDFLAG
{
    DEU_DEFAULT_FLAG            = 0,                // Ĭ�ϲ�������
    DEU_ADD_FLAG                = 1,                // ���
    DEU_UPDATE_FLAG             = 2,                // ����
    DEU_DELETE_FLAG             = 3                 // ɾ��
};

//- ����һ����CDeuIDString���з�װ����Щ�ӿ�ԭ����DataKeeper��
class DATAKEEPER_API CDeuIDString
{
public:
    bool getModelIDByIDString(const std::string &strID, std::string &modelID);
    bool getDataSetCodeByIDString(const std::string &strID, unsigned int &nDataSetCode);
    bool getLevelByIDString(const std::string &strID, unsigned int &nLevel);
    bool getRowByIDString  (const std::string &strID, unsigned int &nRow);
    bool getColByIDString  (const std::string &strID, unsigned int &nCol);
    bool getTypeByIDString (const std::string &strID, int &nType);
};

//- ����һ����CDeuCallEvent����Ϣ���ú������з�װ
class DATAKEEPER_API CDeuCallEvent
{
public:
    static void CallEventState(const int nIndex, const char* strDesc, const ID* pID, ea::IEventAdapter *pEventAdapter);
    static void CallBreakEventState(const char* strDesc, const ID* pID, ea::IEventAdapter *pEventAdapter);
};

#endif //_DEUDATAKEEPER_DEUDEFINE_H_