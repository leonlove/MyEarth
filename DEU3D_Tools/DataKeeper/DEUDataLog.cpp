#include "DEUDataLog.h"


DEUDataLog::DEUDataLog(void)
{

}


DEUDataLog::~DEUDataLog(void)
{
    if(m_deuLog != NULL)
        delete m_deuLog;
}

void DEUDataLog::AddDataItem(const std::string strCategoryDir)
{
    deulog::ConfigItem dataItem;
    dataItem.TypeID = "DEUDataLog";

    deulog::FormatItem dataFormat;
    dataFormat.Name = "OperateType";
    dataFormat.Type = INTEGER_TYPE;
    dataFormat.Size = sizeof(int);
    dataItem.Format.push_back(dataFormat);

    dataFormat.Name = "ErrorType";
    dataFormat.Type = INTEGER_TYPE;
    dataFormat.Size = sizeof(int);
    dataItem.Format.push_back(dataFormat);

    dataFormat.Name = "ErrorDesp";
    dataFormat.Type = STRING_TYPE;
    dataFormat.Size = 100;
    dataItem.Format.push_back(dataFormat);

    dataFormat.Name = "IP";
    dataFormat.Type = STRING_TYPE;
    dataFormat.Size = sizeof("192.168.200.108");
    dataItem.Format.push_back(dataFormat);

    dataFormat.Name = "Port";
    dataFormat.Type = STRING_TYPE;
    dataFormat.Size = 10;
    dataItem.Format.push_back(dataFormat);

    dataFormat.Name = "ID";
    dataFormat.Type = STRING_TYPE;
    dataFormat.Size = 48;
    dataItem.Format.push_back(dataFormat);

    dataFormat.Name = "SourceID";
    dataFormat.Type = STRING_TYPE;
    dataFormat.Size = 48;
    dataItem.Format.push_back(dataFormat);

    dataFormat.Name = "DBPath";
    dataFormat.Type = STRING_TYPE;
    dataFormat.Size = 256;
    dataItem.Format.push_back(dataFormat);

    m_deuLog->AddCategory(dataItem,strCategoryDir.c_str());
}

bool DEUDataLog::initPath(const std::string strCfgPath,const std::string strRootPath,const std::string strCategoryDir)
{
    m_deuLog = new deulog::DEULog();
    if(strCfgPath.empty() || strRootPath.empty() || strCategoryDir.empty())
    {
        return false;
    }
    if(m_deuLog->init(strCfgPath.c_str()))
    {
        return true;
    }
    else
    {
        m_deuLog->GetLogCfg()->SetRoot(strRootPath.c_str());
        AddDataItem(strCategoryDir);
    }
    return true;
}

bool DEUDataLog::AddDataLog(const DEUDataLogInfo logInfo)
{
    if(m_deuLog == NULL)
        return false;
    
    deulog::DEULogCategory* pcat = m_deuLog->GetCategory("DEUDataLog");
    if(pcat == NULL)
        return false;
    deulog::ConfigItem item = m_deuLog->GetLogCfg()->GetConfigItem("DEUDataLog"); //日志路径未设置可能会挂掉，放在后面 yubo

    deulog::DEULogRecord logrcd(&item);
    logrcd.IntVal(0u) = logInfo.m_nOperateType;
    logrcd.IntVal(1u) = logInfo.m_nErrorType;
    memcpy(logrcd.StrVal(2u),logInfo.m_strErrorDesp.c_str(),logInfo.m_strErrorDesp.size());
    memcpy(logrcd.StrVal(3u),logInfo.m_strIP.c_str(),logInfo.m_strIP.size());
    memcpy(logrcd.StrVal(4u),logInfo.m_strPort.c_str(),logInfo.m_strPort.size());
    memcpy(logrcd.StrVal(5u),logInfo.m_strID.c_str(),logInfo.m_strID.size());
    memcpy(logrcd.StrVal(6u),logInfo.m_strSourceID.c_str(),logInfo.m_strSourceID.size());
    memcpy(logrcd.StrVal(7u),logInfo.m_strDBPath.c_str(),logInfo.m_strDBPath.size());

    return pcat->WriteLog(logrcd);

    return true;

}