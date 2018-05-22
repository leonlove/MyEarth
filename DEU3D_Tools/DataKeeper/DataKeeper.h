#ifndef DATA_KEEPER_H_3CDE89DE_C280_4F55_8B5D_E0B4A9425F07_INCLUDE
#define DATA_KEEPER_H_3CDE89DE_C280_4F55_8B5D_E0B4A9425F07_INCLUDE

#pragma warning(disable:4018)

#include "IDataKeeper.h"
#include <iostream>
#include <vector>

#include "IDProvider/ID.h"
#include <OpenSP/sp.h>
#include "DEUUrlServer.h"
#include "IDataSetManager.h"
#include "DataSetManager.h"

#include "IServiceManager.h"
#include "ServiceManager.h"


#include "IService.h"
#include "Service.h"

#include "IDataSetSegment.h"
#include "export.h"

#include <Network/IDEUNetwork.h>

namespace dk
{
    class DataKeeper : public IDataKeeper
    {
    public:
        explicit DataKeeper(void);
        virtual ~DataKeeper(void);

    protected:
        //- ��¼������
        virtual bool                    login(const std::string& strHost, const std::string& strPort, const std::string& strUser, const std::string& strPwd);

        //- �˳�������
        virtual bool                    logout();

        //- ��ʼ��DataKeeper���������ݼ����������������������ʼ���ʹ���DEUNetwork����������
        bool                            initialize(const std::string& strHost, const std::string& strPort, bool& bConnect);

        //- ���ӷ������ڵ�
        bool                            addServer(std::string& szHost, std::string& ApachePort, std::string& port);

        //- ɾ���������ڵ�
        bool                            delServer(std::string& szHost, std::string& ApachePort, std::string& port);

        //- ��ȡ�������ڵ�
        bool                            getServer(std::vector<std::string>& vecSrvHost);

    public:
        //- ��ȡ������IP
        std::string                     getRootHost();

        //- ��ȡ������˿�
        std::string                     getRootPort();

        //- ��ȡ������Ϣ
        OpenSP::sp<deunw::IDEUNetwork>  getDEUNetWork();

        //- ��ȡ���ݼ�������
        OpenSP::sp<dk::IDataSetManager> getDataSetManager(void);

        //- ��ȡ������������
        OpenSP::sp<dk::IServiceManager> getServiceManager(void);

        //- ���������ļ�·��
        void                            setTotalFiles(const std::string& sTotalFiles);

        //- �ύ��ʼʱ�����־��ʼ��Ϣ
        virtual void                        commitBegin();

        //- �ύ����ʱ�����־������Ϣ
        virtual void                        commitEnd();

        //- ���������ļ�·����������־��������   
        virtual bool                        readConfigFile(const std::string& sConfigFile);

        //- ��ȡ�����ϴ��ɹ����ļ�·������ 
        virtual std::vector<std::string>    getCommitSuccessFiles();

        //- ��ȡ�����ϴ�ʧ�ܵ��ļ�·������ 
        virtual std::vector<std::string>    getCommitFailedFiles();

        //- ��ȡ����û���ϴ����ļ�·������ 
        virtual std::vector<std::string>    getCommitNoneFiles();

        //- ��ȡ���е��ļ�·������ 
        virtual std::vector<std::string>    getCommitAllFiles();

        //- ��ȡ��ǰ�ļ��ύ״̬  ʧ��->0���ɹ�->1��δ�ύ->2
        virtual unsigned int                getFileCommitState(const std::string& sCurCommitFile);

        //- ������е��ļ�·������ 
        virtual void                        clearAllFiles();

        //-��ȡ�������ļ���ָ��ͳ���
        virtual void*                       formJsonToBsonPtr(const std::string& sJsonString, unsigned int* pBuffLen);

        //- ����ɢ�������ļ�
        virtual void                        exportRcdAndUrlFile(const std::string& sRcdAndUrlFile);

        //- ����ɢ�������ļ�
        virtual void                        importRcdAndUrlFile(const std::string& sRcdAndUrlFile);

        //- ��ȡ�����url�б�
        virtual void                        getImportUrl(std::map<std::string,HostInfo>& hostImportMap);

        //- �����Ƿ�Ϊ�����ϴ�
        void                                setBatchUpload(const bool bBatch);

        //- ��ȡ�Ƿ������ϴ�
        bool                                getBatchUpload();

    protected:
        std::string                        m_strRootHost;
        std::string                        m_strRootPort;

        OpenSP::sp<deunw::IDEUNetwork>     m_pIUrlNetwork;
        OpenSP::sp<dk::IDataSetManager>    m_pIDataSetManager;
        OpenSP::sp<dk::IServiceManager>    m_pIServiceManager;

        std::map<std::string,HostInfo>     m_mapImportHost;
        bool                               m_bIsBatchUpload;

        void exportTempRcdFile(const std::string& sTempRcdFile, std::map<unsigned int,RcdInfo>& rcdMap);
    };
}

#endif

