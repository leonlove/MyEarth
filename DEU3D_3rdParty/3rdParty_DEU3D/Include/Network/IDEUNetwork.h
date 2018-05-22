#ifndef I_DEUNETWORK_H_4B258213_B7DA_42AA_890A_E49B9FA93C37_INCLUDE
#define I_DEUNETWORK_H_4B258213_B7DA_42AA_890A_E49B9FA93C37_INCLUDE

#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include <IDProvider/ID.h>
#include <Common\IDEUException.h>
#include <Common\ErrorCode.h>
#include "Export.h"
#include <ExternalService/ITileSet.h>

namespace deunw
{
    class IDEUNetwork : public OpenSP::Ref
    {
    public:
        //��ʼ��������Ϣ
        virtual bool initialize(
            const std::string& strHost,                         // ָ�������IP��ַ�Ͷ˿ں�
            const std::string& strApachePort,                   // ָ������Ķ˿ں�
            const bool bInitForFetching = true,                 // �Ƿ�򵥵ĳ�ʼ��Ϊ���ء���Ϊfalse��������ӿڽ���ʼ��Ϊ����ά���������߱��޸ķ�������ݵ�����
            const std::string& strLocalCache = ""              // ָ�����ػ����·��
        ) = 0;
        // �������ݼ�ID��ȡ���� 
        virtual bool queryData(const ID& id, void* &pBuffer, unsigned &nBufLen,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL, unsigned nVersion = 0) = 0;
        virtual bool queryData(const ID& id,const std::string &strHost,const std::string &strPort,void* &pBuffer, unsigned &nBufLen,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL, unsigned nVersion = 0) = 0;
        // ��ȡ���ݸ���
        virtual unsigned queryBlockCount(const std::string& strHost,const std::string& strPort,unsigned nDSCode,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool     queryVersion(const ID& id,std::vector<unsigned>& versionList,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // ��ȡָ����Χ��ID
        virtual bool queryIndices(const std::string& strHost,const std::string& strPort,unsigned nDSCode,
                                           unsigned nOffset,unsigned nCount,std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // �������
        virtual bool addData(const ID& id,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool updateData(const ID& id,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool replaceData(const ID& id,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool addVirtTile(const void* pBuffer,unsigned nBufLen,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool addProperty(const ID& id,const std::string& strProperty,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool updateProperty(const ID& id,const std::string& strProperty,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // ���ͼ��
        virtual bool addLayer(const ID& id,const ID& idParent,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // ����ͼ��
        virtual bool updateLayer(const ID& id,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // ��ӷ��ſ�
        virtual bool addCategory(const ID& id,const ID& idParent,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // ���·��ſ�
        virtual bool updateCategory(const ID& id,const void* pBuffer,unsigned nLength,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // ɾ������
        virtual bool deleteData(const ID& id, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool deleteData(const std::string& strHost,const std::string& strPort,const std::vector<ID>& idVec,
            std::vector<ID>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool deleteAllData(const std::string& strHost,const std::string& strPort, unsigned nDSCode,std::vector<ID>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool deleteLayerChildren(const std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // ɾ������
        virtual bool deleteProperty(const std::string& strHost,const std::string& strPort, unsigned nDSCode,const std::vector<ID>& idVec,
                         std::vector<ID>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool deleteAllProperty(const std::string& strHost,const std::string& strPort, unsigned nDSCode,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // ɾ�����ݼ�
        virtual bool deleteDataSet(unsigned nDataSet,const std::string &strHost,const std::string &strPort,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool deleteDataSetAttr(unsigned nDataSet,const std::string &strHost,const std::string &strPort,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // �������Ի�ȡID
        virtual bool queryIdByProperty(const std::string& strProperty, std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool queryIdByProperty(const std::string& strProperty,const std::string &strHost,const std::string &strPort,std::vector<ID>& idVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // ��ȡ���ݼ���ɢ����Ϣ
        virtual std::string getDSInfo(const std::string &strType,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // �������ݼ�\ɢ����Ϣ�ļ�
        virtual bool setDSInfo(const std::string &strText,const std::string &strType,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // ��������
        virtual bool startService(const std::string &strHost,const std::string &strApachePort,const std::vector<std::string>& strPortVec,const std::string& strType,std::vector<std::string>& errVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool startService(const std::string &strHost,const std::string &strApachePort,const std::string &strPort,const std::string& strType,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        // ��ѯ�����Ķ˿�
        virtual bool getActivePorts(const std::string& strHost,const std::string& strApachePort,std::vector<std::string>& strPortVec,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;

        virtual void addTileSet(deues::ITileSet* pTileSet) = 0;
        virtual void removeTileSet(deues::ITileSet* pTileSet) = 0;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual bool login(const std::string& strHost,const std::string& strPort,const std::string& strUser,const std::string& strPwd,OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool logout(OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //���ӡ�ɾ���������� liubo 20151116
        virtual bool addServer(const std::string& strHost, const std::string& strApachePort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool delServer(const std::string& strHost, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;

        virtual bool addService(const std::string& strHost, const std::string& strApachePort, const std::string& strSerPort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        virtual bool delService(const std::string& strHost, const std::string& strApachePort, const std::string& strSerPort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        
        //���apache�������Ƿ����� liubo 20151116
        virtual bool checkApache(const std::string& strHost, const std::string& strApachePort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;
        //������ݶ˿��Ƿ�����  liubo 20151118
        virtual bool checkPortIsActive(const std::string& strHost, const std::string& strApachePort, const std::string& strSerPort, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL) = 0;

    };

    DEUNW_EXPORT IDEUNetwork *createDEUNetwork(void);
    DEUNW_EXPORT void    freeMemory(void *pData);

    //��ȡ��������
    DEUNW_EXPORT std::string GetErrDesc(int ErrCode);
}
#endif //I_DEUNETWORK_H_4B258213_B7DA_42AA_890A_E49B9FA93C37_INCLUDE