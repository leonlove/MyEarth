#include "export.h"

#ifndef I_DATA_KEEPER_H_7977E4E2_F8D5_44E7_AF8E_72D356E72DF9_INCLUDE
#define I_DATA_KEEPER_H_7977E4E2_F8D5_44E7_AF8E_72D356E72DF9_INCLUDE

#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include "IDProvider/ID.h"
#include "IDataSetManager.h"
#include "IServiceManager.h"
#include "IDataSetSegment.h"
#include "IDataSetSegmentCollection.h"
#include <Network/IDEUNetwork.h>

namespace dk{

    class IDataKeeper : public OpenSP::Ref
    {
    public:
        //- ��¼������
        virtual bool                                login(const std::string& strHost, const std::string& strPort, 
                                                          const std::string& strUser, const std::string& strPwd)                      = 0;
        //- �˳�������
        virtual bool                                logout()                                                                            = 0;
        //- ��ʼ��DataKeeper���������ݼ����������������������ʼ���ʹ���DEUNetwork����������
        virtual bool                                initialize( const std::string &strHost, const std::string &strPort, bool& bConnect) = 0;

        //- ��ȡ���ݼ�������
        virtual OpenSP::sp<dk::IDataSetManager>     getDataSetManager(void)                                                           = 0;

        //- ��ȡ������������
        virtual OpenSP::sp<dk::IServiceManager>     getServiceManager(void)                                                           = 0;

        //- ���ӷ������ڵ�
        virtual bool                                addServer(std::string& szHost, std::string& ApachePort, std::string& port)        = 0;

        //- ɾ���������ڵ�
        virtual bool                                delServer(std::string& szHost, std::string& ApachePort, std::string& port)        = 0;

        //- ��ȡ�������ڵ�
        virtual bool                                getServer(std::vector<std::string>& vecSrvHost)                                   = 0;

        //- ��ȡ������IP
        virtual std::string                         getRootHost()                                                                     = 0;

        //- ��ȡ������˿�
        virtual std::string                         getRootPort()                                                                     = 0;

        //- ��ȡ������Ϣ
        virtual OpenSP::sp<deunw::IDEUNetwork>      getDEUNetWork()                                                                   = 0;

        //- ���������ļ�·��
        virtual void                                setTotalFiles(const std::string& sTotalFiles)                                     = 0;

        //- �ύ��ʼʱ�����־��ʼ��Ϣ
        virtual void                                commitBegin()                                                                     = 0;

        //- �ύ����ʱ�����־������Ϣ
        virtual void                                commitEnd()                                                                       = 0;

        //- ���������ļ�·����������־��������
        virtual bool                                readConfigFile(const std::string& sConfigFile)                                    = 0;

        //- ��ȡ�����ϴ��ɹ����ļ�·������ 
        virtual std::vector<std::string>            getCommitSuccessFiles()                                                           = 0;

        //- ��ȡ�����ϴ�ʧ�ܵ��ļ�·������
        virtual std::vector<std::string>            getCommitFailedFiles()                                                            = 0;

        //- ��ȡ����û���ϴ����ļ�·������
        virtual std::vector<std::string>            getCommitNoneFiles()                                                              = 0;

        //- ��ȡ�����ϴ����ļ�·������
        virtual std::vector<std::string>            getCommitAllFiles()                                                               = 0;

        //- ��ȡ��ǰ�ļ��ύ״̬  ʧ��->0���ɹ�->1��δ�ύ->2
        virtual unsigned int                        getFileCommitState(const std::string& sCurCommitFile)                             = 0;

        //- ������е��ļ�·������
        virtual void                                clearAllFiles()                                                                   = 0;

        virtual void*                               formJsonToBsonPtr(const std::string& sJsonString, unsigned int* pBuffLen)         = 0;

        //- ����ɢ�������ļ�
        virtual void                                exportRcdAndUrlFile(const std::string& sRcdAndUrlFile)                            = 0;

        //- ����ɢ�������ļ�
        virtual void                                importRcdAndUrlFile(const std::string& sRcdAndUrlFile)                            = 0;

        //- ��ȡ�����url�б�
        virtual void                                getImportUrl(std::map<std::string,HostInfo>& hostImportMap)                       = 0;

        //- �����Ƿ�Ϊ�����ϴ�
        virtual void                                setBatchUpload(const bool bBatch)                                                 = 0;

        //- ��ȡ�Ƿ������ϴ�
        virtual bool                                getBatchUpload()                                                                  = 0;
    };

    DATAKEEPER_API dk::IDataKeeper *CreateDataKeeper(void);
}

#endif


