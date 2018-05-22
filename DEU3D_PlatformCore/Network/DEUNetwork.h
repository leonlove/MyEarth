#ifndef _DEUNETWORK_H_
#define _DEUNETWORK_H_

#include "IDEUNetwork.h"
#include "DEUQueryData.h"
#include <DEUDBProxy\IDEUDBProxy.h>
#include "DEURcdInfo.h"
#include <OpenThreads/Thread>
#include <OpenThreads/Block>
#include <OpenThreads/Atomic>

namespace deunw
{
    class  DEUNetwork : public IDEUNetwork
    {
    public:
        explicit DEUNetwork(void);
        virtual ~DEUNetwork(void);
    public:
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // ��ʼ����������Ϣ

        virtual bool initialize(
            const std::string& strHost,                         // ָ�������IP��ַ�Ͷ˿ں�
            const std::string& strApachePort,                   // ָ������Ķ˿ں�
            const bool bInitForFetching = true,                 // �Ƿ�򵥵ĳ�ʼ��Ϊ���ء���Ϊfalse��������ӿڽ���ʼ��Ϊ����ά���������߱��޸ķ�������ݵ�����
            const std::string& strLocalCache = ""              // ָ�����ػ����·��
        );

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // ��ȡ���ݼ���ɢ����Ϣ
        std::string getDSInfo(const std::string &strType,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // �������ݼ�\ɢ����Ϣ�ļ�
        bool setDSInfo(const std::string &strText,const std::string &strType,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // ��������
        bool startService(const std::string &strHost,const std::string &strApachePort,const std::vector<std::string>& strPortVec,const std::string& strType,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool startService(const std::string &strHost,const std::string &strApachePort,const std::string &strPort,const std::string& strType,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // ��ѯ�����Ķ˿�
        bool getActivePorts(const std::string& strHost,const std::string& strApachePort,std::vector<std::string>& strPortVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // �������ݼ�ID��ȡ���� 
        bool queryData(const ID &id, void* &pBuffer, unsigned &nBufLen,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL, unsigned nVersion = 0);
        bool queryData(const ID& id,const std::string &strHost,const std::string &strPort,void* &pBuffer, unsigned &nBufLen,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL, unsigned nVersion = 0);
        // ��ȡ���ݸ���
        unsigned queryBlockCount(const std::string& strHost,const std::string& strPort, unsigned nDSCode,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool queryVersion(const ID& id,std::vector<unsigned>& versionList,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // �������
        bool addData(const ID& id,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool updateData(const ID& id,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool replaceData(const ID& id,const void* pBuffer,unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // ���ͼ��
        bool addLayer(const ID& id,const ID& idParent,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // ����ͼ��
        bool updateLayer(const ID& id,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // ���ͼ��
        bool addCategory(const ID& id,const ID& idParent,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // ����ͼ��
        bool updateCategory(const ID& id,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // ���������Ƭ
        bool addVirtTile(const void* pBuffer,unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // �������
        bool addProperty(const ID& id,const std::string& strProperty,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // ��������
        bool updateProperty(const ID& id,const std::string& strProperty,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // ɾ������
        bool deleteData(const ID& id, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool deleteData(const std::string& strHost,const std::string& strPort,const std::vector<ID>& idVec,
                        std::vector<ID>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool deleteAllData(const std::string& strHost,const std::string& strPort, unsigned nDSCode,std::vector<ID>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool deleteLayerChildren(const std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // ɾ������
        bool deleteProperty(const std::string& strHost,const std::string& strPort, unsigned nDSCode,const std::vector<ID>& idVec,
                            std::vector<ID>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool deleteAllProperty(const std::string& strHost,const std::string& strPort, unsigned nDSCode,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // ɾ�����ݼ�
        bool deleteDataSet(unsigned nDataSet,const std::string &strHost,const std::string &strPort,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool deleteDataSetAttr(unsigned nDataSet,const std::string &strHost,const std::string &strPort,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // �������Ի�ȡID
        bool queryIdByProperty(const std::string& strProperty, std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool queryIdByProperty(const std::string& strProperty,const std::string &strHost,const std::string &strPort,std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        // ��ȡָ����Χ��ID
        bool queryIndices(const std::string& strHost,const std::string& strPort, unsigned nDSCode,
                          unsigned nOffset,unsigned nCount,std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);

        void     setOffLineMode(bool bOffLine);
        bool     getOffLineMode(void) const;

        bool     login(const std::string& strHost,const std::string& strPort,const std::string& strUser,const std::string& strPwd,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool     logout(OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool     authPerm(const std::string& strPerm,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool     authRes(const unsigned nDB,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);

        void     addTileSet(deues::ITileSet* pTileSet);
        void     removeTileSet(deues::ITileSet* pTileSet);

        //���ӡ�ɾ���������� liubo 20151116
        bool addServer(const std::string& strHost, const std::string& strApachePort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool delServer(const std::string& strHost, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);

        bool addService(const std::string& strHost, const std::string& strApachePort, const std::string& strSerPort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        bool delService(const std::string& strHost, const std::string& strApachePort, const std::string& strSerPort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
        
        //���apache�������Ƿ����� liubo 20151116
        bool checkApache(const std::string& strHost, const std::string& strApachePort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);

        //������ݶ˿��Ƿ����� liubo 20151118
        bool checkPortIsActive(const std::string& strHost, const std::string& strApachePort, const std::string& strSerPort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);

    private:
        bool OpenDB(const std::string& strDBPath);

        bool startServiceFun(const std::string &strUrl,std::vector<std::string>& errVec,int& nErrorCode);
        bool queryDatum(const std::string& strHost,const std::vector<ID> &idVec,std::vector<char> &vecBuffer);
        bool removeCache(const std::string& strDBPath);
    private:
        //Ȩ�޷���
        std::string                        m_strTicket;
        std::string                        m_strUser;
        bool                               m_bLogin;
        std::string                        m_strAuthHost;
        std::string                        m_strAuthPort;
        //������Ϣmap
        std::string                        m_strError;
        std::string                        m_strHost;
        std::string                        m_strApachePort;
        std::string                        m_strDBPath;
        DEUQueryData                       m_queryData;
        std::map<unsigned __int64,deues::ITileSet*> m_tileSetMap;

        OpenSP::sp<deudbProxy::IDEUDBProxy>  m_pDBProxy;

        friend class NetworkCheckerThread;
        class NetworkCheckerThread : public OpenThreads::Thread
        {
        public:
            explicit NetworkCheckerThread(DEUNetwork* pNetwork) :
            m_pNetwork(pNetwork)
            {
                setStackSize(16u * 1024u);
                setSchedulePriority(OpenThreads::Thread::THREAD_PRIORITY_LOW);
                m_block.set(false);
            }

            void finishMission(void)
            {
                m_MissionFinished.exchange(1);
                m_block.set(true);
                 join();
            }

        protected:
            virtual void run(void);
            OpenThreads::Atomic     m_MissionFinished;
            OpenThreads::Block      m_block;
            DEUNetwork*             m_pNetwork;
        };
    protected:
        NetworkCheckerThread *m_pNetworkCheckerThread;
        volatile bool         m_bOffLineMode;
        volatile bool         m_bNetworkHealthy;
        bool                  canUseNetwork(void) const;
    protected:
        typedef std::pair<ID, unsigned>     RequestItem;
        std::list<RequestItem>              m_listRequestQueue;
        OpenThreads::Mutex                  m_mtxRequestQueue;

        struct DownloadResult : public OpenSP::Ref
        {
            std::vector<char>       m_vecBuffer;
            bool                    m_bSuccess;
            int                     m_nErrorCode;
            OpenThreads::Block      m_blockFinished;
        };
        std::map<unsigned, OpenSP::sp<DownloadResult> >     m_mapDownloadResult;
        OpenThreads::Mutex  m_mtxDownloadResult;

        unsigned    putRequestIntoList(const ID &id);
        void        fetchRequest(std::list<RequestItem> &listRequests,std::string& strHost);

    protected:
        class DownloadingThread : public OpenThreads::Thread
        {
        public:
            explicit DownloadingThread(DEUNetwork *pNetwork)
            {
                setStackSize(128u * 1024u);
                m_pThis = pNetwork;
                m_MissionFinished.exchange(0u);
                m_nSuspendCount = 0;
                m_block.release();
            }
        public:
            void suspend(bool bSuspend)
            {
                if((unsigned)m_MissionFinished == 1u)
                {
                    return;
                }

                OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxSuspendCount);
                if(bSuspend)
                {
                    ++m_nSuspendCount;
                }
                else
                {
                    --m_nSuspendCount;
                }

                m_block.set(m_nSuspendCount <= 0);
            }

            void finishMission(void)
            {
                m_MissionFinished.exchange(1u);
                m_block.set(true);
                join();
            }

        protected:
            virtual void run(void);

        protected:
            void    downloadingResultFailed(const std::list<RequestItem> &listCurrentReqs);

        protected:
            DEUNetwork             *m_pThis;
            OpenThreads::Block      m_block;
            OpenThreads::Atomic     m_MissionFinished;

            int                     m_nSuspendCount;
            OpenThreads::Mutex      m_mtxSuspendCount;
        };

        std::vector<DownloadingThread*>    m_vecDownloadingThread;
        volatile unsigned  m_nThread;
    };
}

#endif //_DEUNETWORK_H_

