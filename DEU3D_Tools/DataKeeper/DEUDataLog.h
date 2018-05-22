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
    int            m_nOperateType;  //操作类型
    int            m_nErrorType;    //错误类型
    std::string    m_strErrorDesp;  //错误描述
    std::string    m_strIP;            //IP
    std::string    m_strPort;        //端口
    std::string    m_strID;            //更新后的新ID； 默认使用此ID
    std::string    m_strSourceID;   //更新前使用的ID
    std::string    m_strDBPath;        //数据来源的DEUDB
        
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

