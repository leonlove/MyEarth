#ifndef _DEUNETWORKQUERYDATA_H_
#define _DEUNETWORKQUERYDATA_H_

#include "DEURcdInfo.h"
#include "IDProvider/ID.h"
#include "DEUDefine.h"
#include <vector>

namespace deunw
{
    class DEUQueryData
    {
    public:
        DEUQueryData(void);
        ~DEUQueryData(void);
    public:
        // ��ʼ��
        bool InitHost(const std::string& strHost,const std::string& strApachePort);
        // �������ݼ�ID��ȡ���� 
        bool QueryData(const ID &id, unsigned nVersion,const std::string &strHost,const std::string &strPort, std::vector<char> &vecBuffer, int& nErrorCode);
        bool QueryDatum(const std::string& strHost,const std::vector<ID>& idVec,const std::string& strTicket,std::vector<char> &vecBuffer, int& nErrorCode);
        // ��ȡ���ݸ���
        unsigned QueryBlockCount(const std::string& strHost,const std::string& strPort,const unsigned nDSCode, const std::string& strTicket,int& nErrorCode);
        bool     QueryVersion(const ID& id,std::vector<unsigned>& vList,int& nErrorCode);
        // �������
        bool AddData(const ID& id,const std::string& strTicket, const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,int& nErrorCode);
        bool AddLayer(const ID& id,const ID& idParent,const std::string& strTicket,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,int& nErrorCode);
        bool AddCategory(const ID& id,const ID& idParent,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,int& nErrorCode);
        bool UpdateData(const ID& id,const std::string& strTicket,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,int& nErrorCode);
        bool UpdateLayer(const ID& id,const std::string& strTicket,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,int& nErrorCode);
        bool UpdateCategory(const ID& id,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,int& nErrorCode);
        bool ReplaceData(const ID& id,const std::string& strTicket,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,int& nErrorCode);
        // ���������Ƭ
        bool AddVirtTile(const std::vector<char> &vecBuffer,const std::string& strTicket,std::vector<std::string>& errVec,int& nErrorCode);
        // �����������
        bool AddProperty(const ID& id,const std::string& strProperty,const std::string& strTicket,std::vector<std::string>& errVec, int& nErrorCode);
        bool UpdateProperty(const ID& id,const std::string& strProperty,const std::string& strTicket,std::vector<std::string>& errVec,int& nErrorCode);
        // ɾ������
        bool DelDatum(const std::string& strHost,const std::string& strPort,const std::vector<ID>& idVec,const std::string& strTicket,std::vector<ID>& errIds,int& nErrorCode);
        bool DelAllData(const std::string& strHost,const std::string& strPort, unsigned nDSCode,const std::string& strTicket,std::vector<ID>& errIds, int& nErrorCode);
        bool DeleteLayerChildren(const std::vector<ID>& idVec,const std::string& strTicket,int& nErrorCode);
        // ɾ����������
        bool DelProperty(const std::string& strHost,const std::string& strPort, unsigned nDSCode,const std::vector<ID>& idVec,
                         const std::string& strTicket,std::vector<ID>& errIds,int& nErrorCode);
        bool DelAllProperty(const std::string& strHost,const std::string& strPort, unsigned nDSCode,const std::string& strTicket,int& nErrorCode);
        // ɾ�����ݼ�
        bool DelDataSet(unsigned nDS,const std::string &strHost,const std::string &strPort, const std::string& strTicket,int& nErrorCode);
        // ɾ�����ݼ�����
        bool DelDataSetAttr(unsigned nDS,const std::string &strHost,const std::string &strPort, const std::string& strTicket,int& nErrorCode);
        // �������Ի�ȡID
        bool QueryIdByProperty(const std::string& strProperty,const std::string& strTicket, std::vector<ID>& idVec,  int& nErrorCode);
        // �������Բ�ID
        bool QueryIdByProperty(const std::string& strProperty,const std::string &strHost,const std::string &strPort,const std::string& strTicket,std::vector<ID>& idVec, int& nErrorCode);
        // ��ȡָ����Χ��ID
        bool QueryIndices(const std::string& strHost,const std::string& strPort, unsigned nDSCode,const std::string& strTicket,
                        unsigned int nOffset,unsigned int nCount,std::vector<ID>& idVec, int& nErrorCode);
        std::vector<std::string> GetRcdUrl(int nDS);
        std::vector<std::string> GetRcdUrl(const ID &ObjID);

        bool Login(const std::string& strHost,const std::string& strPort,const std::string& strUser,const std::string& strPwd,
            std::vector<std::string>& strPermVec,std::string& strTicket,int& nErrorCode);
        bool Logout(const std::string& strHost,const std::string& strPort,const std::string& strTicket,int& nErrorCode);
        bool AuthPerm(const std::string& strHost,const std::string& strPort,const std::string& strTicket,const std::string& strPerm,int& nErrorCode);
        bool AuthRes(const std::string& strHost,const std::string& strPort,const std::string& strTicket,const unsigned nDB,int& nErrorCode);
        bool UpdateCacheVersion();
        bool GetCacheVersion(unsigned __int64& nVersion,int& nErrorCode);

    private:
        bool AddDataFun(const std::string& strUrl, const std::vector<char> &vecBuffer,int& nErrorCode);
        bool AddPropertyFun(const std::string& strUrl,const std::string& strProperty, int& nErrorCode);
        bool DelDataFun(const std::string& strUrl,int& nErrorCode);
        bool QueryDataFun(const std::string& strUrl, std::vector<char> &vecBuffer, int& nErrorCode);
        bool QueryBlockCountFun(const std::string& strUrl,unsigned& nBlockCount, int& nErrorCode);
        bool QueryIndicesFun(const std::string& strUrl, std::vector<ID>& idVec,  int& nErrorCode);
        bool QueryVersionFun(const std::string& strUrl,std::vector<unsigned>& vList,int& nErrorCode);
        bool DelDatumFun(const std::string& strUrl,const std::vector<char> &vecPost,std::vector<ID>&errIds,int& nErrorCode);
        bool DelAllDataFun(const std::string& strUrl,std::vector<ID>&errIds,int& nErrorCode);
        bool DeleteLayerChildrenFun(const std::string& strUrl, const std::vector<char> &vecPost, int& nErrorCode);
        bool UpdateLayerFun(const std::string& strUrl, const std::vector<char> &vecBuffer, std::vector<std::string>& errIds,int& nErrorCode);

        //members
        std::string m_strHost;
        std::string m_strApachePort;
        DEURcdInfo m_rcdInfo;

    };
}

#endif //_DEUNETWORKQUERYDATA_H_

