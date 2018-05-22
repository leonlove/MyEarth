#include "DEULog.h"
#include <string>
#include <algorithm>
#include <sys/stat.h>
#include <io.h>

OpenSP::sp<CDeuLog> CDeuLog::m_pDefaultInstance = NULL;

void trim(string& str)
{
    static const string delims = " \t\r";
    str.erase(str.find_last_not_of(delims)+1);
    str.erase(0, str.find_first_not_of(delims));
}

CDeuLog::CDeuLog(std::string sLogFilePath)
{
    m_sLogFilePath = sLogFilePath;
    //m_oLogWriter.open(m_sLogFilePath.c_str(), ios_base::trunc);
}

CDeuLog::~CDeuLog()
{
    m_nFileCount = 0;
    m_setTotalFiles.clear();
}

OpenSP::sp<CDeuLog> CDeuLog::getDefaultInstance()
{
    if (NULL == m_pDefaultInstance)
    {
        m_pDefaultInstance = new CDeuLog;
    }

    return m_pDefaultInstance;
}

void CDeuLog::clearDefaultInstance()
{
    m_pDefaultInstance = NULL;
}

bool CDeuLog::ReadLogFile(const string& sLogFilePath)
{
    m_vecAllFiles.clear();
    m_vecCommitSuccessFiles.clear();
    m_vecCommitFailedFiles.clear();
    m_vecCommitNoneFiles.clear();

    m_iLogReader.open(sLogFilePath.c_str());
    if (!m_iLogReader.is_open())
    {
        return false;
    }

    string sLine, sSubLine;
    while(getline(m_iLogReader, sLine))
    {
        //解析path
        trim(sLine);
        sSubLine = "Path：";
        int nIndex1 = sLine.find(sSubLine);
        if (nIndex1 != -1)
        {
            nIndex1 += sSubLine.length();
            string sPath = sLine.substr(nIndex1);
            m_vecAllFiles.push_back(sPath);
            continue;
        }

        //解析curfile
        trim(sLine);
        sSubLine = "CurFile："; 
        int nIndex2 = sLine.find(sSubLine);
        if (nIndex2 != -1)
        {
            nIndex2 += sSubLine.length();
            string sCurFile = sLine.substr(nIndex2);
            sCurFile += ".idx";

            //解析finishflag
            getline(m_iLogReader, sLine);
            trim(sLine);
            sSubLine = "FinishFlag："; 
            string sFinishFlag = sLine.substr(sLine.find(sSubLine) + sSubLine.length());
            //提交成功的文件
            if (sFinishFlag == "1")
            {
                m_vecCommitSuccessFiles.push_back(sCurFile);
            }
            //提交失败的文件
            else
            {
                m_vecCommitFailedFiles.push_back(sCurFile);
            }
            continue;
        }
    }
    m_iLogReader.close();

    //没有提交的文件
    std::vector<std::string>::iterator itr_success, itr_failed;
    for (size_t i=0; i<m_vecAllFiles.size(); i++)
    {
        itr_success = find(m_vecCommitSuccessFiles.begin(), m_vecCommitSuccessFiles.end(), m_vecAllFiles[i]);
        itr_failed = find(m_vecCommitFailedFiles.begin(), m_vecCommitFailedFiles.end(), m_vecAllFiles[i]);
        if (itr_success==m_vecCommitSuccessFiles.end() && itr_failed==m_vecCommitFailedFiles.end())
        {
            m_vecCommitNoneFiles.push_back(m_vecAllFiles[i]);
        }
    }

    return true;
}

bool CDeuLog::WriteFilePath(const std::set<std::string>& setTotalFiles)
{
    m_oLogWriter << "    <FilePath>" << endl;

    std::set<std::string>::iterator itr_path = setTotalFiles.begin();
    for (; itr_path!=setTotalFiles.end(); itr_path++)
    {
        WriteFilePath(*itr_path);
    }

    m_oLogWriter << "    <FilePath>" << endl;
    m_oLogWriter.flush();

    return true;
}

bool CDeuLog::WriteLogInfo(const string& sLogInfo)
{
    m_oLogWriter << sLogInfo << endl;
    m_oLogWriter.flush();

    return true;
}

bool CDeuLog::WriteFilePath(const std::string& sFilePath)
{
    m_oLogWriter << "        Path：" << sFilePath << endl;

    return true;
}

bool CDeuLog::WriteErrorInfo(const std::string& sErrorInfo)
{
    m_oLogWriter << "        sErrorInfo：" << sErrorInfo << endl;

    return true;
}

bool CDeuLog::WriteErrorID(const std::string& sErrorID)
{
    m_oLogWriter << "        sErrorID：" << sErrorID << endl;

    return true;
}

bool CDeuLog::WriteLogInfo(OpenSP::sp<LogInfo> pCurLogInfo)
{
    AddLogInfo(pCurLogInfo);
    m_oLogWriter << "    <begin>" << endl;
    m_oLogWriter << "        CurFile：" << pCurLogInfo->sCurCommitFile << endl;
    m_oLogWriter << "        FinishFlag：" << pCurLogInfo->bFinishFlag << endl;
    m_oLogWriter << "        TotalIDCount：" << pCurLogInfo->nTotalIDCount << endl;

    std::set<std::string>::iterator itr_info = pCurLogInfo->setLogInfo.begin();
    for (; itr_info!=pCurLogInfo->setLogInfo.end(); itr_info++)
    {
        WriteErrorInfo(*itr_info);
    }

    std::set<std::string>::iterator itr_id = pCurLogInfo->setCommitFaildID.begin();
    for (; itr_id!=pCurLogInfo->setCommitFaildID.end(); itr_id++)
    {
        WriteErrorID(*itr_id);
    }
    m_oLogWriter << "    <end>" << endl;
    m_oLogWriter.flush();

    return true;
}

void CDeuLog::SetLogFilePath(const string& sLogFilePath)
{
    m_sLogFilePath = sLogFilePath;

    if (m_oLogWriter.is_open())
    {
        m_oLogWriter.close();
    }
    m_oLogWriter.open(m_sLogFilePath.c_str(), ios_base::app);
}

std::string CDeuLog::GetLogFilePath()
{
    return m_sLogFilePath;
}


void CDeuLog::SetFileCount(int nFileCount)
{
    m_nFileCount = nFileCount;
}

int CDeuLog::GetFileCount()
{
    return m_nFileCount;
}

void CDeuLog::SetTotalFile(const std::set<string>& setTotalFiles)
{
    m_setTotalFiles = setTotalFiles;
}

std::set<string> CDeuLog::GetTotalFile()
{
    return m_setTotalFiles;
}

void CDeuLog::SetLogInfo(const std::set<OpenSP::sp<LogInfo> >& setLogInfos)
{
    m_setLogInfos = setLogInfos;
}

std::set<OpenSP::sp<LogInfo> > CDeuLog::GetLogInfo()
{
    return m_setLogInfos;
}

void CDeuLog::AddLogInfo(const OpenSP::sp<LogInfo> pCurLogInfo)
{
    m_setLogInfos.insert(pCurLogInfo);
}

OpenSP::sp<LogInfo> CDeuLog::GetLogInfo(const std::string& sFileName)
{
    //OpenSP::sp<LogInfo> pLogInfo;

    std::set<OpenSP::sp<LogInfo> >::iterator itr = m_setLogInfos.begin();
    for (; itr!=m_setLogInfos.end(); ++itr)
    {
        if ((*itr)->sCurCommitFile == sFileName)
        {
            //pLogInfo = *itr;
            return *itr;
        }
    }

    //return pLogInfo;
    return NULL;
}

std::vector<std::string> CDeuLog::GetCommitSuccessFiles()
{
    return m_vecCommitSuccessFiles;
}

std::vector<std::string> CDeuLog::GetCommitFailedFiles()
{
    return m_vecCommitFailedFiles;
}

std::vector<std::string> CDeuLog::GetCommitNoneFiles()
{
    return m_vecCommitNoneFiles;
}

std::vector<std::string> CDeuLog::GetCommitAllFiles()
{
    return m_vecAllFiles;
}

unsigned int CDeuLog::GetFileCommitState(const std::string& sCurCommitFile)
{
    OpenSP::sp<LogInfo> pLogInfo = GetLogInfo(sCurCommitFile);
    //- 未获取到信息，表示未提交
    if (NULL == pLogInfo)
    {
        return 2;
    }

    //- 成功
    if (pLogInfo->bFinishFlag)
    {
        return 0;
    }

    return 1;   //- 失败
}

void CDeuLog::ClearCommitAllFiles()
{
    m_vecAllFiles.clear();
    m_vecCommitNoneFiles.clear();
    m_vecCommitFailedFiles.clear();
    m_vecCommitSuccessFiles.clear();
}

void CDeuLog::OpenLogFile()
{
    chmod(m_sLogFilePath.c_str(), S_IWRITE);
    m_oLogWriter.open(m_sLogFilePath.c_str(), ios_base::trunc);
}

void CDeuLog::CloseLogFile()
{
    m_oLogWriter.close();
    chmod(m_sLogFilePath.c_str(), S_IREAD);
    m_setLogInfos.clear();
}