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

//�����ļ�����־��Ϣ
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
    bool                    bFinishFlag;                //- �Ƿ��ύ�ɹ�
    int                     nTotalIDCount;              //- ��ǰ�ļ�ID����
    std::string             sCurCommitFile;             //- ��ǰ�ύ�ļ�
    std::set<std::string>   setLogInfo;                 //- ������־��Ϣ
    std::set<std::string>   setCommitFaildID;           //- �ύʧ��ID
};

class CDeuLog : public OpenSP::Ref
{
public:
    //- ������־�ļ�ȫ·������ʼ���ļ���
    CDeuLog(std::string sLogFilePath = "C:\\DeuDataKeeperLog.log");
    ~CDeuLog();

    //- ��ȡĬ����־ʵ��
    static OpenSP::sp<CDeuLog>          getDefaultInstance();

    //- ����Ĭ����־ʵ��
    static void                         clearDefaultInstance();

    //- ������־�ļ�
    bool                                ReadLogFile(const string& sLogFilePath);

    //- д�����ļ�·��
    bool                                WriteFilePath(const std::set<std::string>& setTotalFiles);

    //- ���д����־��Ϣ
    bool                                WriteLogInfo(const string& sLogInfo);

    //- �����ļ�������ɺ�����д����־��Ϣ
    bool                                WriteLogInfo(OpenSP::sp<LogInfo> pCurLogInfo);

    //- ���ü���ȡ��־�ļ�·��
    void                                SetLogFilePath(const string& sLogFilePath);
    std::string                         GetLogFilePath();

    //- ���ü���ȡidx�ļ�����
    void                                SetFileCount(int nFileCount);
    int                                 GetFileCount();

    //- ���ü���ȡ����idx�ļ�·��
    void                                SetTotalFile(const std::set<string>& setTotalFiles);
    std::set<string>                    GetTotalFile();

    //- ���ü���ȡ�����ļ�����־��Ϣ
    void                                SetLogInfo(const std::set<OpenSP::sp<LogInfo> >& setLogInfos);
    std::set<OpenSP::sp<LogInfo> >      GetLogInfo();

    //- ���Ӽ���ȡ�����ļ�����־��Ϣ
    void                                AddLogInfo(const OpenSP::sp<LogInfo> pCurLogInfo);
    OpenSP::sp<LogInfo>                 GetLogInfo(const std::string& sFileName);

    //- ��ȡ�ϴ��ɹ���idx�ļ�·��
    std::vector<std::string>            GetCommitSuccessFiles();

    //- ��ȡ�ϴ�ʧ�ܵ�idx�ļ�·��
    std::vector<std::string>            GetCommitFailedFiles();

    //- ��ȡû���ϴ���idx�ļ�·��
    std::vector<std::string>            GetCommitNoneFiles();

    //- ��ȡȫ���ļ�·��
    std::vector<std::string>            GetCommitAllFiles();

    //- ��ȡ��ǰ�ļ��ύ״̬  ʧ��->0���ɹ�->1��δ�ύ->2
    unsigned int                        GetFileCommitState(const std::string& sCurCommitFile);

    //- ���ȫ���ļ�·��
    void                                ClearCommitAllFiles();

    //- ����־�ļ�
    void                                OpenLogFile();

    //- �ر���־�ļ�
    void                                CloseLogFile();

private:
    //- file_path��ʽ�����
    bool                                WriteFilePath(const std::string& sFilePath);
    //- error_info��ʽ�����
    bool                                WriteErrorInfo(const std::string& sErrorInfo);
    //- error_id��ʽ�����
    bool                                WriteErrorID(const std::string& sErrorID);

private:
    static OpenSP::sp<CDeuLog>         m_pDefaultInstance;          //- Ĭ����־ʵ��
    std::string                        m_sLogFilePath;              //- ��־�ļ�·��
    std::ofstream                      m_oLogWriter;                //- д��־�ļ���
    std::ifstream                      m_iLogReader;                //- ����־�ļ���
    int                                m_nFileCount;                //- ����idx�ļ�����
    std::set<std::string>              m_setTotalFiles;             //- ����idx�ļ�·��
    std::set<OpenSP::sp<LogInfo> >     m_setLogInfos;               //- ��¼�����ļ�����־��Ϣ

    std::vector<std::string>           m_vecAllFiles;               //- �������ļ��ж�ȡ������idx�ļ�·��
    std::vector<std::string>           m_vecCommitSuccessFiles;     //- �ϴ��ɹ���idx�ļ�·��
    std::vector<std::string>           m_vecCommitFailedFiles;      //- �ϴ�ʧ�ܵ�idx�ļ�·��
    std::vector<std::string>           m_vecCommitNoneFiles;        //- û��ʧ�ܵ�idx�ļ�·��
};

#endif