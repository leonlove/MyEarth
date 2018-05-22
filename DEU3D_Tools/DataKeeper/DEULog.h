#ifndef    __NEW_DEU_LOG_H__
#define    __NEW_DEU_LOG_H__

#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <fstream>
using namespace std;

//单个文件的日志信息
class LogInfo : public OpenSP::Ref
{
public:
    LogInfo()
    {
        sCurCommitFile = "";
        bFinishFlag = false;
        nTotalIDCount = 0;
    }

    ~LogInfo(){}

public:
    bool operator < (const LogInfo& rhs) const
    {
        return rhs.sCurCommitFile > sCurCommitFile;
    }

public:
    bool                    bFinishFlag;                //- 是否提交成功
    int                     nTotalIDCount;              //- 当前文件ID个数
    std::string             sCurCommitFile;             //- 当前提交文件
    std::set<std::string>   setLogInfo;                 //- 错误日志信息
    std::set<std::string>   setCommitFaildID;           //- 提交失败ID
};

class CDeuLog : public OpenSP::Ref
{
public:
    //- 根据日志文件全路径名初始化文件流
    CDeuLog(std::string sLogFilePath = "C:\\DeuDataKeeperLog.log");
    ~CDeuLog();

    //- 获取默认日志实例
    static OpenSP::sp<CDeuLog>          getDefaultInstance();

    //- 销毁默认日志实例
    static void                         clearDefaultInstance();

    //- 解析日志文件
    bool                                ReadLogFile(const string& sLogFilePath);

    //- 写所有文件路径
    bool                                WriteFilePath(const std::set<std::string>& setTotalFiles);

    //- 逐句写入日志信息
    bool                                WriteLogInfo(const string& sLogInfo);

    //- 单个文件处理完成后批量写入日志信息
    bool                                WriteLogInfo(OpenSP::sp<LogInfo> pCurLogInfo);

    //- 设置及获取日志文件路径
    void                                SetLogFilePath(const string& sLogFilePath);
    std::string                         GetLogFilePath();

    //- 设置及获取idx文件个数
    void                                SetFileCount(int nFileCount);
    int                                 GetFileCount();

    //- 设置及获取所有idx文件路径
    void                                SetTotalFile(const std::set<string>& setTotalFiles);
    std::set<string>                    GetTotalFile();

    //- 设置及获取所有文件的日志信息
    void                                SetLogInfo(const std::set<OpenSP::sp<LogInfo> >& setLogInfos);
    std::set<OpenSP::sp<LogInfo> >      GetLogInfo();

    //- 增加及获取单个文件的日志信息
    void                                AddLogInfo(const OpenSP::sp<LogInfo> pCurLogInfo);
    OpenSP::sp<LogInfo>                 GetLogInfo(const std::string& sFileName);

    //- 获取上传成功的idx文件路径
    std::vector<std::string>            GetCommitSuccessFiles();

    //- 获取上传失败的idx文件路径
    std::vector<std::string>            GetCommitFailedFiles();

    //- 获取没有上传的idx文件路径
    std::vector<std::string>            GetCommitNoneFiles();

    //- 获取全部文件路径
    std::vector<std::string>            GetCommitAllFiles();

    //- 获取当前文件提交状态  失败->0，成功->1，未提交->2
    unsigned int                        GetFileCommitState(const std::string& sCurCommitFile);

    //- 清空全部文件路径
    void                                ClearCommitAllFiles();

    //- 打开日志文件
    void                                OpenLogFile();

    //- 关闭日志文件
    void                                CloseLogFile();

private:
    //- file_path格式化输出
    bool                                WriteFilePath(const std::string& sFilePath);
    //- error_info格式化输出
    bool                                WriteErrorInfo(const std::string& sErrorInfo);
    //- error_id格式化输出
    bool                                WriteErrorID(const std::string& sErrorID);

private:
    static OpenSP::sp<CDeuLog>         m_pDefaultInstance;          //- 默认日志实例
    std::string                        m_sLogFilePath;              //- 日志文件路径
    std::ofstream                      m_oLogWriter;                //- 写日志文件流
    std::ifstream                      m_iLogReader;                //- 读日志文件流
    int                                m_nFileCount;                //- 所有idx文件个数
    std::set<std::string>              m_setTotalFiles;             //- 所有idx文件路径
    std::set<OpenSP::sp<LogInfo> >     m_setLogInfos;               //- 记录所有文件的日志信息

    std::vector<std::string>           m_vecAllFiles;               //- 从配置文件中读取的所有idx文件路径
    std::vector<std::string>           m_vecCommitSuccessFiles;     //- 上传成功的idx文件路径
    std::vector<std::string>           m_vecCommitFailedFiles;      //- 上传失败的idx文件路径
    std::vector<std::string>           m_vecCommitNoneFiles;        //- 没有失败的idx文件路径
};

#endif