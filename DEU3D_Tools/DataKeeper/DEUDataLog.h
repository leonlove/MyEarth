#ifndef _DEUDATAKEEPER_DEUDATALOG_H_
#define _DEUDATAKEEPER_DEUDATALOG_H_

#include "DEULog/CDEULog.h"
#include "export.h"

enum DEUOperateType
{
    DEU_ADD_DATA    = 1,
    DEU_UPDATE_DATA = 2,
    DEU_DELETE_DATA = 3
};


struct DEUDataLogInfo 
{
    int            m_nOperateType;  //��������
    int            m_nErrorType;    //��������
    std::string    m_strErrorDesp;  //��������
    std::string    m_strIP;            //IP
    std::string    m_strPort;        //�˿�
    std::string    m_strID;            //���º����ID�� Ĭ��ʹ�ô�ID
    std::string    m_strSourceID;   //����ǰʹ�õ�ID
    std::string    m_strDBPath;        //������Դ��DEUDB
        
    DEUDataLogInfo()
    {
        m_nErrorType = m_nOperateType = -1;
        m_strErrorDesp = m_strIP = m_strPort = m_strID = m_strSourceID = m_strDBPath = "";
    }
    ~DEUDataLogInfo()
    {

    }
};

class DATAKEEPER_API DEUDataLog
{
public:
    DEUDataLog(void);
    ~DEUDataLog(void);
    bool initPath(const std::string strCfgPath,const std::string strRootPath,const std::string strCategoryDir);
    bool AddDataLog(const DEUDataLogInfo logInfo);
    deulog::DEULog* GetDEULog() {return m_deuLog;}

private:
    deulog::DEULog* m_deuLog;
    void AddDataItem(const std::string strCategoryDir);

};

#endif //_DEUDATAKEEPER_DEUDATALOG_H_

