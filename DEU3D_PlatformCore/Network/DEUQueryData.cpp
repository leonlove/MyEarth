#include "DEUQueryData.h"
#include <common/DEUBson.h>
#include <common/Common.h>
#include <sstream>
#include <zlib.h>
#include "DEUDefine.h"

struct DEUTransHeader
{
    unsigned char  m_szFlag[8];//DEUDATA+ 0/1
    UINT64         m_nLength;
};

const unsigned char g_szTransFlag[7] = {'D', 'E', 'U', 'D', 'A','T','A'};

namespace deunw
{
    void convertBsonDoc2Buffer(const bson::bsonDocument &bsonDoc, std::vector<char> &vecBuffer)
    {
        bson::bsonStream bsonSS;
        bsonDoc.Write(&bsonSS);

        const char *pStream = (const char *)bsonSS.Data();
        vecBuffer.assign(pStream, pStream + bsonSS.DataLen());
    }

    DEUQueryData::DEUQueryData(void)
    {
    }


    DEUQueryData::~DEUQueryData(void)
    {
    }

    //init host
    bool DEUQueryData::InitHost(const std::string& strHost,const std::string& strApachePort)
    {
        m_strHost = strHost;
        m_strApachePort = strApachePort;
        return m_rcdInfo.Init(m_strHost,m_strApachePort);
    }

    bool DEUQueryData::DelDatumFun(const std::string& strUrl,const std::vector<char> &vecPost, std::vector<ID>&errIds,int& nErrorCode)
    {
        std::vector<char>   vecRespBuf;
        SimpleHttpClient shc;
        const int nRet = shc.Request(PostMethod, strUrl, vecRespBuf, vecPost);
        if(vecRespBuf.empty())
        {
            nErrorCode = nRet;
            return false;
        }

        bson::bsonDocument bsonDoc;
        const bool bConv2Bson = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();
        if(!bConv2Bson)
        {
            return false;
        }

        bson::bsonElement* pElem = bsonDoc.GetElement("RetCode");
        if(pElem == NULL)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        const int nRetCode = pElem->Int32Value();
        if(nRetCode == 0)
        {
            pElem = bsonDoc.GetElement("ErrDisp");
            if(pElem == NULL)
            {
                nErrorCode = DEU_UNKNOWN;
            }
            else
            {
                nErrorCode = pElem->Int32Value();
            }
            return false;
        }

        pElem = bsonDoc.GetElement("ErrID");
        if(pElem == NULL)
        {
            return true;
        }

        if(pElem->GetType() == bson::bsonArrayType)
        {
            bson::bsonArrayEle* pArrayElem = (bson::bsonArrayEle*)pElem;
            errIds.resize(pArrayElem->ChildCount());
            for(unsigned int i = 0;i < pArrayElem->ChildCount();i++)
            {
                bson::bsonElement* pChildElem = pArrayElem->GetElement(i);
                if(pChildElem == NULL)
                {
                    continue;
                }
                errIds[i].fromString(pChildElem->StrValue());
            }
        }

        return true;
    }


    bool DEUQueryData::DeleteLayerChildrenFun(const std::string& strUrl,const std::vector<char> &vecPost, int& nErrorCode)
    {
        std::vector<char>   vecRespBuf;
        SimpleHttpClient shc;
        const int nRet = shc.Request(PostMethod, strUrl, vecRespBuf, vecPost);
        if(vecRespBuf.empty())
        {
            nErrorCode = nRet;
            return false;
        }

        bson::bsonDocument  bsonDoc;
        const bool bConv2Bson = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();
        if(!bConv2Bson)
        {
            return false;
        }

        bson::bsonElement* pElem = bsonDoc.GetElement("RetCode");
        if(pElem == NULL)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        const int nRetCode = pElem->Int32Value();
        if(nRetCode == 0)
        {
            pElem = bsonDoc.GetElement("ErrDisp");
            if(pElem == NULL)
            {
                nErrorCode = DEU_UNKNOWN;
            }
            else
            {
                nErrorCode = pElem->Int32Value();
            }
            return false;
        }

        return true;
    }


    bool DEUQueryData::DeleteLayerChildren(const std::vector<ID>& idVec,const std::string& strTicket,int& nErrorCode)
    {

        ID id = ID::getCultureLayerRootID();

        const std::vector<std::string> vecServers = m_rcdInfo.GetRcdUrl(id);
        if(vecServers.empty())
            return true;

        std::vector<char>   vecDeleteStream;
        {
            bson::bsonDocument bDoc;
            bson::bsonArrayEle* pArray = (bson::bsonArrayEle*)bDoc.AddArrayElement("ID");

            for(unsigned n = 0;n < idVec.size();n++)
            {
                pArray->AddStringElement(idVec[n].toString().c_str());
            }
            convertBsonDoc2Buffer(bDoc, vecDeleteStream);
        }

        for(unsigned n = 0;n < vecServers.size();n++)
        {
            const std::string &strHost = vecServers[n];
            std::ostringstream oss;
            oss << "http://" << strHost.c_str() << "/DEUDataPub?type=delLayerChildren&db=" << id.ObjectID.m_nDataSetCode
                << "&tid=" << strTicket << '\0';

            return DeleteLayerChildrenFun(oss.str(), vecDeleteStream, nErrorCode);
        }
        return true;
    }
    // 删除数据
    bool DEUQueryData::DelDatum(const std::string& strHost,const std::string& strPort,const std::vector<ID>& idVec,
                                const std::string& strTicket,std::vector<ID>& errIds,int& nErrorCode)
    {
        std::vector<char> vecStream;
        {
            bson::bsonDocument bDoc;
            bson::bsonArrayEle* pArray = (bson::bsonArrayEle*)bDoc.AddArrayElement("ID");
            for(unsigned n = 0;n < idVec.size();n++)
            {
                pArray->AddStringElement(idVec[n].toString().c_str());
            }

            convertBsonDoc2Buffer(bDoc, vecStream);
        }

        //url
        std::ostringstream oss;
        oss<<"http://"<<strHost<<":"<<strPort<<"/DEUDataPub?type=delDatum&db="<<idVec[0].ObjectID.m_nDataSetCode
        <<"&tid="<<strTicket<<'\0';

        return DelDatumFun(oss.str(), vecStream, errIds, nErrorCode);
    }


    bool DEUQueryData::DelAllData(const std::string& strHost,const std::string& strPort,const unsigned nDSCode,const std::string& strTicket,std::vector<ID>& errIds, int& nErrorCode)
    {
        //url
        std::ostringstream oss;
        oss<<"http://"<<strHost<<":"<<strPort<<"/DEUDataPub?type=delAllData&db="<<nDSCode
        <<"&tid="<<strTicket<<'\0';

        return DelAllDataFun(oss.str(),errIds,nErrorCode);
    }

    bool DEUQueryData::DelAllDataFun(const std::string& strUrl,std::vector<ID>&errIds,int& nErrorCode)
    {
        //variables
        std::vector<char>  vecRespBuf;
        //request
        SimpleHttpClient shc;
        const int nRet = shc.Request(GetMethod, strUrl, vecRespBuf);
        if(nRet != 0 && vecRespBuf.empty())
        {
            nErrorCode = nRet;
            return false;
        }

        bson::bsonDocument bsonDoc;
        const bool bConv2Bson = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();
        if(!bConv2Bson)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        bson::bsonElement* pElem = bsonDoc.GetElement("RetCode");
        if(pElem == NULL)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        const int nRetCode = pElem->Int32Value();
        if(nRetCode == 0)
        {
            pElem = bsonDoc.GetElement("ErrDisp");
            if(pElem == NULL)
            {
                nErrorCode = DEU_UNKNOWN;
            }
            else
            {
                nErrorCode = pElem->Int32Value();
            }
            return false;
        }

        pElem = bsonDoc.GetElement("ErrID");
        if(pElem == NULL)
        {
            return true;
        }

        if(pElem->GetType() == bson::bsonArrayType)
        {
            bson::bsonArrayEle* pArrayElem = (bson::bsonArrayEle*)pElem;
            errIds.resize(pArrayElem->ChildCount());
            for(unsigned int i = 0;i < pArrayElem->ChildCount();i++)
            {
                bson::bsonElement* pChildElem = pArrayElem->GetElement(i);
                if(pChildElem == NULL)
                {
                    continue;
                }
                errIds[i].fromString(pChildElem->StrValue());
            }
        }
        return true;
    }


    // 输出数据功能函数
    bool DEUQueryData::DelDataFun(const std::string& strUrl,int& nErrorCode)
    {
        //variables
        std::vector<char>   vecRespBuf;
        //request
        SimpleHttpClient shc;
        const int nRet = shc.Request(GetMethod, strUrl, vecRespBuf);
        if(vecRespBuf.empty())
        {
            nErrorCode = nRet;
            return false;
        }

        bson::bsonDocument bsonDoc;
        const bool bConv2Bson = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();
        if(!bConv2Bson)
        {
            return false;
        }

        bson::bsonElement* pElem = bsonDoc.GetElement("RetCode");
        if(pElem == NULL)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        const int nRetCode = pElem->Int32Value();
        if(nRetCode == 0)
        {
            pElem = bsonDoc.GetElement("ErrDisp");
            if(pElem == NULL)
            {
                nErrorCode = DEU_UNKNOWN;
            }
            else
            {
                nErrorCode = pElem->Int32Value();
            }
            return false;
        }

        return true;
    }

    // 删除属性数据
    bool DEUQueryData::DelProperty(const std::string& strHost,const std::string& strPort,const unsigned nDSCode,const std::vector<ID>& idVec,
                                   const std::string& strTicket,std::vector<ID>& errIds,int& nErrorCode)
    {
        std::vector<char> vecStream;
        {
            bson::bsonDocument bDoc;
            bson::bsonArrayEle* pArray = (bson::bsonArrayEle*)bDoc.AddArrayElement("ID");

            for(unsigned n = 0;n < idVec.size();n++)
            {
                pArray->AddStringElement(idVec[n].toString().c_str());
            }

            convertBsonDoc2Buffer(bDoc, vecStream);
        }

        //url
        std::ostringstream oss;
        oss<<"http://"<<strHost<<":"<<strPort<<"/DEUDataPub?type=delAttr&db="<<nDSCode
        <<"&tid="<<strTicket<<'\0';
        return DelDatumFun(oss.str(), vecStream, errIds, nErrorCode);
    }

    bool DEUQueryData::DelAllProperty(const std::string& strHost,const std::string& strPort,const unsigned nDSCode,const std::string& strTicket,int& nErrorCode)
    {
        //url
        std::ostringstream oss;
        oss<<"http://"<<strHost<<":"<<strPort<<"/DEUDataPub?type=delDataSetAttr&db="<<nDSCode
        <<"&tid="<<strTicket<<'\0';

        return DelDataFun(oss.str(),nErrorCode);
    }

    // 删除数据集
    bool DEUQueryData::DelDataSet(unsigned nDS,const std::string &strHost,const std::string &strPort,const std::string& strTicket,int& nErrorCode)
    {
        //url
        std::ostringstream oss;
        oss<<"http://"<<strHost<<":"<<strPort<<"/DEUDataPub?type=delDataSet&db="<<nDS
            <<"&tid="<<strTicket<<'\0';

        return DelDataFun(oss.str(),nErrorCode);
    }

    // 删除数据集属性
    bool DEUQueryData::DelDataSetAttr(unsigned nDS,const std::string &strHost,const std::string &strPort,const std::string& strTicket,int& nErrorCode)
    {
        //url
        std::ostringstream oss;
        oss<<"http://"<<strHost<<":"<<strPort<<"/DEUDataPub?type=delDataSetAttr&db="<<nDS
            <<"&tid="<<strTicket<<'\0';

        return DelDataFun(oss.str(),nErrorCode);
    }

    // 添加数据功能函数
    bool DEUQueryData::AddDataFun(const std::string& strUrl, const std::vector<char> &vecBuffer,int& nErrorCode)
    {
        std::vector<char>   vecRespBuf;
        //request
        SimpleHttpClient shc;
        const int nRet = shc.Request(PostMethod, strUrl, vecRespBuf, vecBuffer);
        if(vecRespBuf.empty())
        {
            nErrorCode = nRet;
            return false;
        }

        bson::bsonDocument bsonDoc;
        const bool bConv2Bson = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();
        if(!bConv2Bson)
        {
            return false;
        }

        bson::bsonElement* pElem = bsonDoc.GetElement("RetCode");
        if(pElem == NULL)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        const int nRetCode = pElem->Int32Value();
        if(nRetCode == 0)
        {
            pElem = bsonDoc.GetElement("ErrDisp");
            if(pElem == NULL)
            {
                nErrorCode = DEU_UNKNOWN;
            }
            else
            {
                nErrorCode = pElem->Int32Value();
            }
            return false;
        }

        return true;
    }

    // 添加虚拟瓦片
    bool DEUQueryData::AddVirtTile(const std::vector<char> &vecBuffer, const std::string& strTicket,std::vector<std::string>& errVec,int& nErrorCode)
    {
        std::vector<std::string> strVec = m_rcdInfo.GetRcdUrl(0);
        if(strVec.empty())
        {
            nErrorCode = DEU_FAIL_GET_RCD;
            return false;
        }

        for(unsigned k = 0;k < strVec.size();k++)
        {
            std::string strHost = strVec[k];
            //url
            std::ostringstream oss;
            oss<<"http://"<<strHost<<"/DEUDataPub?type=addVirtTile&db=0"<<"&tid="<<strTicket<<'\0';
            if(!AddDataFun(oss.str(), vecBuffer, nErrorCode))
            {
                errVec.push_back(strHost);
                continue;
            }
        }
        if(errVec.size() == strVec.size())
        {
            return false;
        }
        return true;
    }

    bool DEUQueryData::AddLayer(const ID& id,const ID& idParent,const std::string& strTicket,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,int& nErrorCode)
    {
        std::vector<std::string> strVec = m_rcdInfo.GetRcdUrl(id);
        if(strVec.empty())
        {
            nErrorCode = DEU_FAIL_GET_RCD;
            return false;
        }
        //add alias in utf-8
        bson::bsonStream bSS;
        bSS.Write(pBuffer,nBufLen);
        bSS.Reset();
        bson::bsonDocument bDoc;
        bDoc.Read(&bSS);

        std::string strName = "";
        bson::bsonStringEle* pNameElem = (bson::bsonStringEle*)bDoc.GetElement("Name");
        if(pNameElem != NULL)
        {
            strName = pNameElem->StrValue();
        }

        std::wstring wstrName = cmm::ANSIToUnicode(strName);
        std::string strUtfName = cmm::UnicodeToUTF8(wstrName);
        bDoc.AddStringElement("Alias",strUtfName.c_str());

        bSS.Clear();
        bDoc.Write(&bSS);

        std::vector<char>   vecData;
        {
            bson::bsonDocument bsonDoc;
            bsonDoc.AddBinElement("Data",(void*)bSS.Data(),bSS.DataLen());
            convertBsonDoc2Buffer(bsonDoc, vecData);
        }

        for(unsigned k = 0;k < strVec.size();k++)
        {
            std::string strHost = strVec[k];
            //url
            std::ostringstream oss;
            oss<<"http://"<<strHost<<"/DEUDataPub?type=addLayer&id="<<id.toString()<<"&pid="<<idParent.toString()<<"&tid="<<strTicket<<'\0';
            if(!AddDataFun(oss.str(), vecData, nErrorCode))
            {
                errVec.push_back(strVec[k]);
            }
        }

        if(errVec.size() == strVec.size())
        {
            return false;
        }
        return true;
    }

    bool DEUQueryData::AddCategory(const ID& id,const ID& idParent,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,int& nErrorCode)
    {
        std::vector<std::string> strVec = m_rcdInfo.GetRcdUrl(id);
        if(strVec.empty())
        {
            nErrorCode = DEU_FAIL_GET_RCD;
            return false;
        }

        std::vector<char>   vecData;
        {
            bson::bsonDocument bsonDoc;
            bsonDoc.AddBinElement("Data",(void*)pBuffer,nBufLen);
            convertBsonDoc2Buffer(bsonDoc, vecData);
        }


        for(unsigned k = 0;k < strVec.size();k++)
        {
            std::string strHost = strVec[k];
            //url
            std::ostringstream oss;
            oss<<"http://"<<strHost<<"/DEUDataPub?type=addCategory&id="<<id.toString()<<"&pid="<<idParent.toString()<<'\0';
            if(!AddDataFun(oss.str(), vecData, nErrorCode))
            {
                errVec.push_back(strVec[k]);
            }
        }

        if(errVec.size() == strVec.size())
        {
            return false;
        }
        return true;
    }

    // 添加数据
    bool DEUQueryData::AddData(const ID& id,const std::string& strTicket,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,int& nErrorCode)
    {
        std::vector<std::string> strVec = m_rcdInfo.GetRcdUrl(id);
        if(strVec.empty())
        {
            nErrorCode = DEU_FAIL_GET_RCD;
            return false;
        }

        std::vector<char>   vecData;
        {
            bson::bsonDocument bsonDoc;
            bsonDoc.AddBinElement("Data",(void*)pBuffer,nBufLen);
            convertBsonDoc2Buffer(bsonDoc, vecData);
        }

        for(unsigned k = 0;k < strVec.size();k++)
        {
            std::string strHost = strVec[k];
            //url
            std::ostringstream oss;
            oss<<"http://"<<strHost<<"/DEUDataPub?type=addData&db="<<id.ObjectID.m_nDataSetCode<<"&id="<<id.toString()<<"&tid="<<strTicket<<'\0';
            if(!AddDataFun(oss.str(), vecData, nErrorCode))
            {
                errVec.push_back(strVec[k]);
            }
        }

        if(errVec.size() == strVec.size())
        {
            return false;
        }
        return true;
    }

    // 添加数据功能函数
    bool DEUQueryData::UpdateLayerFun(const std::string& strUrl, const std::vector<char> &vecBuffer,std::vector<std::string>& errIds,int& nErrorCode)
    {
        std::vector<char>   vecRespBuf;
        SimpleHttpClient shc;
        const int nRet = shc.Request(PostMethod, strUrl, vecRespBuf, vecBuffer);
        if(vecRespBuf.empty())
        {
            nErrorCode = nRet;
            return false;
        }

        bson::bsonDocument bsonDoc;
        const bool bConv2Bson = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();
        if(!bConv2Bson)
        {
            return false;
        }

        bson::bsonElement* pElem = bsonDoc.GetElement("RetCode");
        if(pElem == NULL)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        const int nRetCode = pElem->Int32Value();
        if(nRetCode == 0)
        {
            pElem = bsonDoc.GetElement("ErrDisp");
            if(pElem == NULL)
            {
                nErrorCode = DEU_UNKNOWN;
            }
            else
            {
                nErrorCode = pElem->Int32Value();
            }
            return false;
        }

        //"ErrAddID","ErrDelID"
        pElem = bsonDoc.GetElement("ErrAddID");
        bson::bsonElement* pDelElem = bsonDoc.GetElement("ErrDelID");
        if(pElem == NULL && pDelElem == NULL)
        {
            return true;
        }

        if(pElem->GetType() == bson::bsonArrayType)
        {
            bson::bsonArrayEle* pArrayElem = (bson::bsonArrayEle*)pElem;
            for(unsigned int i = 0;i < pArrayElem->ChildCount();i++)
            {
                bson::bsonElement* pChildElem = pArrayElem->GetElement(i);
                if(pChildElem == NULL)
                {
                    continue;
                }
                errIds.push_back(pChildElem->StrValue());
            }
        }

        if(pDelElem->GetType() == bson::bsonArrayType)
        {
            bson::bsonArrayEle* pArrayElem = (bson::bsonArrayEle*)pDelElem;
            for(unsigned int i = 0;i < pArrayElem->ChildCount();i++)
            {
                bson::bsonElement* pChildElem = pArrayElem->GetElement(i);
                if(pChildElem == NULL)
                {
                    continue;
                }
                errIds.push_back(pChildElem->StrValue());
            }
        }

        return true;
    }

    bool DEUQueryData::UpdateLayer(const ID& id,const std::string& strTicket,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,int& nErrorCode)
    {
        std::vector<std::string> strVec = m_rcdInfo.GetRcdUrl(id);
        if(strVec.empty())
        {
            nErrorCode = DEU_FAIL_GET_RCD;
            return false;
        }

        /*
        std::vector<char>   vecBuffer;
        {
            bson::bsonDocument bsonDoc;
            bsonDoc.AddBinElement("Data",(void*)pBuffer,nBufLen);
            convertBsonDoc2Buffer(bsonDoc, vecBuffer);
        }
        */

        //liubo 20151204 mod begin

        bson::bsonDocument bsonDoc,outDoc;
        bsonDoc.FromBsonStream(pBuffer, nBufLen);
        bson::bsonElement *eleName = bsonDoc.GetElement("Name");

        std::string strName = "";
        std::wstring wstrName;
        std::string strUtfName;

        if(eleName)
        {
            eleName->ValueString(strName, false);
            wstrName = cmm::ANSIToUnicode(strName);
            strUtfName = cmm::UnicodeToUTF8(wstrName);
            bsonDoc.AddStringElement("Alias", strUtfName.c_str());
        }

        bson::bsonStream outBSS;
        bsonDoc.Write(&outBSS);
        outDoc.AddBinElement("Data",(void*)outBSS.Data(),outBSS.DataLen());

        std::vector<char>   vecBuffer;
        convertBsonDoc2Buffer(outDoc, vecBuffer);
        //end


        for(unsigned k = 0;k < strVec.size();k++)
        {
            const std::string &strHost = strVec[k];
            std::ostringstream oss;
            oss<<"http://"<<strHost<<"/DEUDataPub?type=updateLayer&db="<<id.ObjectID.m_nDataSetCode<<"&id="<<id.toString()<<"&tid="<<strTicket<<'\0';
            if(!UpdateLayerFun(oss.str(), vecBuffer, errVec, nErrorCode))
            {
                errVec.push_back(strHost);
            }
        }

        if(errVec.size() == strVec.size())
        {
            return false;
        }
        return true;
    }

    bool DEUQueryData::UpdateCategory(const ID& id,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,int& nErrorCode)
    {
        std::vector<std::string> strVec = m_rcdInfo.GetRcdUrl(id);
        if(strVec.empty())
        {
            nErrorCode = DEU_FAIL_GET_RCD;
            return false;
        }

        std::vector<char>   vecBsonBuffer;
        {
            bson::bsonDocument bsonDoc;
            bsonDoc.AddBinElement("Data",(void*)pBuffer,nBufLen);
            convertBsonDoc2Buffer(bsonDoc, vecBsonBuffer);
        }

        for(unsigned k = 0;k < strVec.size();k++)
        {
            const std::string &strHost = strVec[k];
            //url
            std::ostringstream oss;
            oss<<"http://"<<strHost<<"/DEUDataPub?type=updateCategory&db="<<id.ObjectID.m_nDataSetCode<<"&id="<<id.toString()<<'\0';
            if(!UpdateLayerFun(oss.str(), vecBsonBuffer, errVec, nErrorCode))
            {
                errVec.push_back(strHost);
            }
        }
        if(errVec.size() == strVec.size())
        {
            return false;
        }
        return true;
    }
    // 更新数据
    bool DEUQueryData::UpdateData(const ID& id,const std::string& strTicket,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,int& nErrorCode)
    {
        std::vector<std::string> strVec = m_rcdInfo.GetRcdUrl(id);
        if(strVec.empty())
        {
            nErrorCode = DEU_FAIL_GET_RCD;
            return false;
        }

        std::vector<char>   vecBsonBuffer;
        {
            bson::bsonDocument bsonDoc;
            bsonDoc.AddBinElement("Data",(void*)pBuffer,nBufLen);
            convertBsonDoc2Buffer(bsonDoc, vecBsonBuffer);
        }

        for(unsigned k = 0;k < strVec.size();k++)
        {
            const std::string &strHost = strVec[k];
            //url
            std::ostringstream oss;
            oss<<"http://"<<strHost<<"/DEUDataPub?type=updateData&db="<<id.ObjectID.m_nDataSetCode<<"&id="<<id.toString()<<"&tid="<<strTicket<<'\0';
            if(!AddDataFun(oss.str(), vecBsonBuffer, nErrorCode))
            {
                errVec.push_back(strVec[k]);
            }
        }
        if(errVec.size() == strVec.size())
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    // 替换数据
    bool DEUQueryData::ReplaceData(const ID& id,const std::string& strTicket,const void *pBuffer, unsigned nBufLen,std::vector<std::string>& errVec,int& nErrorCode)
    {
        std::vector<std::string> strVec = m_rcdInfo.GetRcdUrl(id);
        if(strVec.empty())
        {
            nErrorCode = DEU_FAIL_GET_RCD;
            return false;
        }

        std::vector<char>   vecBsonBuffer;
        {
            bson::bsonDocument bsonDoc;
            bsonDoc.AddBinElement("Data",(void*)pBuffer,nBufLen);
            convertBsonDoc2Buffer(bsonDoc, vecBsonBuffer);
        }

        for(unsigned k = 0;k < strVec.size();k++)
        {
            const std::string &strHost = strVec[k];
            //url
            std::ostringstream oss;
            oss<<"http://"<<strHost<<"/DEUDataPub?type=replaceData&db="<<id.ObjectID.m_nDataSetCode<<"&id="<<id.toString()<<"&tid="<<strTicket<<'\0';
            if(!AddDataFun(oss.str(), vecBsonBuffer, nErrorCode))
            {
                errVec.push_back(strVec[k]);
            }
        }
        if(errVec.size() == strVec.size())
        {
            return false;
        }
        return true;
    }

    // 更新属性数据
    bool DEUQueryData::UpdateProperty(const ID& id,const std::string& strProperty,const std::string& strTicket,std::vector<std::string>& errVec,int& nErrorCode)
    {
        std::vector<std::string> strVec = m_rcdInfo.GetRcdUrl(id);
        if(strVec.empty())
        {
            nErrorCode = DEU_FAIL_GET_RCD;
            return false;
        }

        const std::vector<char>   vecProperty(strProperty.begin(), strProperty.end());
        for(unsigned k = 0;k < strVec.size();k++)
        {
            const std::string &strHost = strVec[k];
            //url
            std::ostringstream oss;
            oss<<"http://"<<strHost<<"/DEUDataPub?type=updateAttr&id="<<id.toString()<<"&tid="<<strTicket<<'\0';
            if(!AddDataFun(oss.str(), vecProperty, nErrorCode))
            {
                errVec.push_back(strVec[k]);
            }
        }
        if(errVec.size() == strVec.size())
        {
            return false;
        }
        return true;
    }
    // 添加属性数据
    bool DEUQueryData::AddProperty(const ID& id,const std::string& strProperty,const std::string& strTicket,std::vector<std::string>& errVec,int& nErrorCode)
    {
        std::vector<std::string> strVec = m_rcdInfo.GetRcdUrl(id);
        if(strVec.empty())
        {
            nErrorCode = DEU_FAIL_GET_RCD;
            return false;
        }

        const std::vector<char>   vecProperty(strProperty.begin(), strProperty.end());
        for(unsigned k = 0;k < strVec.size();k++)
        {
            const std::string &strHost = strVec[k];
            //url
            std::ostringstream oss;
            oss<<"http://"<<strHost<<"/DEUDataPub?type=addAttr&id="<<id.toString()<<"&tid="<<strTicket<<'\0';
            if(!AddDataFun(oss.str(), vecProperty, nErrorCode))
            {
                errVec.push_back(strVec[k]);
            }
        }

        if(errVec.size() == strVec.size())
        {
            return false;
        }
        return true;
    }
    // 获取数据
    bool DEUQueryData::QueryData(const ID &id, unsigned nVersion,const std::string &strHost,const std::string &strPort, std::vector<char> &vecBuffer, int& nErrorCode)
    {
        //url
        std::ostringstream oss;
        oss<<"http://"<<strHost<<":"<<strPort
            <<"/DEUDataPub?type=queryData&version="
            <<nVersion
            <<"&id="
            <<id.toString()<<'\0';

        return QueryDataFun(oss.str(), vecBuffer, nErrorCode);
    }

    bool DEUQueryData::QueryDataFun(const std::string& strUrl, std::vector<char> &vecBuffer, int& nErrorCode)
    {
        //variables
        std::vector<char>   vecRespBuf;
        //request
        SimpleHttpClient shc;
        const int nRet = shc.Request(GetMethod, strUrl, vecRespBuf);
        if(vecRespBuf.empty())
        {
            nErrorCode = nRet;
            return false;
        }

        if(vecRespBuf.size() < sizeof(DEUTransHeader))
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        DEUTransHeader header;
        memcpy(&header, vecRespBuf.data(), sizeof(DEUTransHeader));
        if(memcmp(header.m_szFlag,g_szTransFlag,7) != 0 || header.m_nLength == 0)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }
        bool bZip = true;
        if(header.m_szFlag[7] == '0')
        {
            bZip = false;
        }

        bson::bsonDocument bsonDoc;
        if(bZip)
        {
            std::vector<char>   vecDest(header.m_nLength + 1);
            uLongf nDestLen = vecDest.size();
            const int nRes = uncompress((Bytef*)vecDest.data(), &nDestLen, (Bytef*)(vecRespBuf.data()+sizeof(DEUTransHeader)),vecRespBuf.size()-sizeof(DEUTransHeader));
            if(nRes != Z_OK)
            {
                nErrorCode = DEU_UNKNOWN;
                return false;
            }

            bsonDoc.FromBsonStream(vecDest.data(), nDestLen);
        }
        else
        {
            bsonDoc.FromBsonStream(vecRespBuf.data() + sizeof(DEUTransHeader), vecRespBuf.size() - sizeof(DEUTransHeader));
        }

        //get return code
        bson::bsonElement* pElem = bsonDoc.GetElement("RetCode");
        if(pElem == NULL)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        const int nRetCode = pElem->Int32Value();
        if(nRetCode == 0)
        {
            pElem = bsonDoc.GetElement("ErrDisp");
            if(pElem == NULL)
            {
                nErrorCode = DEU_UNKNOWN;
            }
            else
            {
                nErrorCode = pElem->Int32Value();
            }
            return false;
        }

        bson::bsonElement* pElemData = bsonDoc.GetElement("Data");
        if(pElemData != NULL && pElemData->GetType() == bson::bsonBinType)
        {
            const char* tmpBson = (const char *)pElemData->BinData();
            vecBuffer.assign(tmpBson, tmpBson + pElemData->BinDataLen());
        }
        return true;
    }

    bool DEUQueryData::QueryDatum(const std::string& strHost,const std::vector<ID>& idVec,const std::string& strTicket,std::vector<char> &vecBuffer, int& nErrorCode)
    {
        if(strHost.empty())
        {
            nErrorCode = DEU_FAIL_GET_RCD;
            return false;
        }

        std::ostringstream oss;
        oss << "http://"<< strHost<<"/DEUDataPub?type=queryData3&tid="<<strTicket<<'\0';

        std::vector<char>   vecPostBuffer;
        {
            bson::bsonDocument bDoc;
            bson::bsonArrayEle* pArray = dynamic_cast<bson::bsonArrayEle*>(bDoc.AddArrayElement("ID"));

            for(unsigned n = 0;n < idVec.size();n++)
            {

                ID nId = idVec[n];
                std::string IdString = nId.toString();
                const char *pzString = IdString.c_str();
                
                if (pzString && pzString[0] != 0)
                {
                    pArray->AddStringElement(pzString);
                }
                
            }

            convertBsonDoc2Buffer(bDoc, vecPostBuffer);
        }

        std::vector<char>   vecRespBuf;
        SimpleHttpClient shc;
        const int nRet = shc.Request(PostMethod, oss.str(), vecRespBuf, vecPostBuffer);
        if(vecRespBuf.empty())
        {
            nErrorCode = nRet;
            return false;
        }

        if(vecRespBuf.size() < sizeof(DEUTransHeader))
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        DEUTransHeader header;
        memcpy(&header, vecRespBuf.data(), sizeof(DEUTransHeader));
        if(memcmp(header.m_szFlag,g_szTransFlag,7) != 0 || header.m_nLength == 0)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        bool bZip = true;
        if(header.m_szFlag[7] == '0')
        {
            bZip = false;
        }

        bson::bsonStream bs;
        bson::bsonDocument bsonDoc;
        if(bZip)
        {
            std::vector<char>   vecDest(header.m_nLength+1);
            unsigned long nDestLen = vecDest.size();
            int nRes = uncompress((Bytef*)vecDest.data(), &nDestLen,(Bytef*)(vecRespBuf.data()+sizeof(DEUTransHeader)),vecRespBuf.size()-sizeof(DEUTransHeader));
            if(nRes != Z_OK)
            {
                nErrorCode = DEU_UNKNOWN;
                return false;
            }

            bsonDoc.FromBsonStream(vecDest.data(),nDestLen);
        }
        else
        {
            bsonDoc.FromBsonStream(vecRespBuf.data()+sizeof(DEUTransHeader),vecRespBuf.size()-sizeof(DEUTransHeader));
        }

        //get return code
        bson::bsonElement* pElem = bsonDoc.GetElement("RetCode");
        if(pElem == NULL)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        const int nRetCode = pElem->Int32Value();
        if(nRetCode == 0)
        {
            pElem = bsonDoc.GetElement("ErrDisp");
            if(pElem == NULL)
            {
                nErrorCode = DEU_UNKNOWN;
            }
            else
            {
                nErrorCode = pElem->Int32Value();
            }
            return false;
        }

        bson::bsonElement* pElemData = bsonDoc.GetElement("Data");
        if(pElemData && pElemData->GetType() == bson::bsonBinType)
        {
            const unsigned nSize = pElemData->BinDataLen();
            if (nSize > 0u)
            {
                vecBuffer.resize(nSize);
                memcpy(vecBuffer.data(),pElemData->BinData(),nSize);
            }
        }
        return true;
    }

    //query id 
    bool DEUQueryData::QueryIdByProperty(const std::string& strProperty, const std::string& strTicket,std::vector<ID>& idVec,  int& nErrorCode)
    {
        std::map<std::string,std::vector<std::string>> serverMap = m_rcdInfo.GetServerMap();
        if(serverMap.empty())
        {
            nErrorCode = DEU_FAIL_GET_RCD;
            return false;
        }

        std::map<std::string,std::vector<std::string>>::const_iterator itr = serverMap.begin();
        while(itr != serverMap.end())
        {
            std::vector<std::string> strPortVec = itr->second;
            if(strPortVec.empty())
            {
                itr++;
                continue;
            }
            //url
            std::ostringstream oss;
            oss<<"http://"<<itr->first<<":"<<strPortVec[0]<<"/DEUDataPub?type=queryID&attr="<< strProperty
            <<"&tid="<<strTicket<<'\0';

            std::vector<ID> tempVec;
            QueryIndicesFun(oss.str(),tempVec,nErrorCode);
            for(unsigned n = 0;n < tempVec.size();n++)
            {
                idVec.push_back(tempVec[n]);
            }
            itr++;
        }

        return true;
    }

    bool DEUQueryData::QueryIdByProperty(const std::string& strProperty,const std::string &strHost,const std::string &strPort,const std::string& strTicket,std::vector<ID>& idVec, int& nErrorCode)
    {
        //url
        std::ostringstream oss;
        oss<<"http://"<<strHost<<":"<<strPort<<"/DEUDataPub?type=queryID&attr="<< strProperty
        <<"&tid="<<strTicket<<'\0';

        if(QueryIndicesFun(oss.str(),idVec,nErrorCode))
        {
            return true;
        }

        return false;
    }
    // 获取指定范围的ID
    bool DEUQueryData::QueryIndicesFun(const std::string& strUrl, std::vector<ID>& idVec, int& nErrorCode)
    {
        //variables
        std::vector<char>   vecRespBuf;
        //request
        SimpleHttpClient shc;
        const int nRet = shc.Request(GetMethod, strUrl.c_str(), vecRespBuf);
        if(vecRespBuf.empty())
        {
            nErrorCode = nRet;
            return false;
        }

        if(vecRespBuf.size() < sizeof(DEUTransHeader))
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        DEUTransHeader header;
        memcpy(&header, vecRespBuf.data(), sizeof(DEUTransHeader));
        if(memcmp(header.m_szFlag, g_szTransFlag, 7) != 0 || header.m_nLength == 0)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        bool bZip = true;
        if(header.m_szFlag[7] == '0')
        {
            bZip = false;
        }

        //get return code,1 for true,0 for false
        bson::bsonDocument bsonDoc;
        if(bZip)
        {
            std::vector<char>   vecDest(header.m_nLength+1);
            unsigned long nDestLen = vecDest.size();

            const int nRes = uncompress((Bytef*)vecDest.data(), &nDestLen, (Bytef*)(vecRespBuf.data() + sizeof(DEUTransHeader)), vecRespBuf.size()-sizeof(DEUTransHeader));
            if(nRes != Z_OK)
            {
                nErrorCode = DEU_UNKNOWN;
                return false;
            }

            bsonDoc.FromBsonStream(vecDest.data(), nDestLen);
        }
        else
        {
            bsonDoc.FromBsonStream(vecRespBuf.data()+sizeof(DEUTransHeader),vecRespBuf.size()-sizeof(DEUTransHeader));
        }

        bson::bsonElement* pElem = bsonDoc.GetElement("RetCode");
        if(pElem == NULL)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }
        const int nRetCode = pElem->Int32Value();
        if(nRetCode == 0)
        {
            pElem = bsonDoc.GetElement("ErrDisp");
            if(pElem == NULL)
            {
                nErrorCode = DEU_UNKNOWN;
            }
            else
            {
                nErrorCode = pElem->Int32Value();
            }
            return false;
        }

        pElem = bsonDoc.GetElement("ID");
        if(pElem != NULL && pElem->GetType() == bson::bsonArrayType)
        {
            bson::bsonArrayEle* pArrayElem = (bson::bsonArrayEle*)pElem;
            idVec.resize(pArrayElem->ChildCount());
            for(unsigned int i = 0;i < pArrayElem->ChildCount();i++)
            {
                bson::bsonElement* pChildElem = pArrayElem->GetElement(i);
                if(pChildElem == NULL)
                {
                    continue;
                }
                idVec[i].fromString(pChildElem->StrValue());
            }
        }
        return true;
    }

    bool DEUQueryData::QueryIndices(const std::string& strHost,const std::string& strPort,const unsigned nDSCode,const std::string& strTicket,
        unsigned int nOffset,unsigned int nCount,std::vector<ID>& idVec,int& nErrorCode)
    {
        //url
        std::ostringstream oss;
        oss<<"http://"<<strHost<<":"<<strPort<<"/DEUDataPub?type=queryIndices&db="<<nDSCode<<"&offset="<<nOffset<<"&count="<<nCount
            <<"&tid="<<strTicket<<'\0';

        return QueryIndicesFun(oss.str(),idVec,nErrorCode);
    }

    // 获取数据个数
    unsigned DEUQueryData::QueryBlockCount(const std::string& strHost,const std::string& strPort,const unsigned nDSCode, const std::string& strTicket,int& nErrorCode)
    {
        unsigned nBlockCount = 0;
        //url
        std::ostringstream oss;
        oss<<"http://"<<strHost<<":"<<strPort<<"/DEUDataPub?type=getBlockCount&db="<<nDSCode
            <<"&tid="<<strTicket<<'\0';

        QueryBlockCountFun(oss.str(),nBlockCount,nErrorCode);
        return nBlockCount;
    }

    bool DEUQueryData::QueryBlockCountFun(const std::string& strUrl,unsigned& nBlockCount,int& nErrorCode)
    {
        nBlockCount = 0;
        //variables
        std::vector<char>   vecRespBuf;
        //request
        SimpleHttpClient shc;
        const int nRet = shc.Request(GetMethod, strUrl, vecRespBuf);
        if(vecRespBuf.empty())
        {
            nErrorCode = nRet;
            return false;
        }

        //get return code
        bson::bsonDocument bsonDoc;
        const bool bConv2Bson = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();
        if(!bConv2Bson)
        {
            return false;
        }

        bson::bsonElement* pElem = bsonDoc.GetElement("RetCode");
        if(pElem == NULL)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        const int nRetCode = pElem->Int32Value();
        if(nRetCode == 0)
        {
            pElem = bsonDoc.GetElement("ErrDisp");
            if(pElem == NULL)
            {
                nErrorCode = DEU_UNKNOWN;
            }
            else
            {
                nErrorCode = pElem->Int32Value();
            }
            return false;
        }

        pElem = bsonDoc.GetElement("BlockCount");
        if(pElem == NULL)
        {
            nErrorCode = DEU_RECEIVE_NO_PARAM;
            return false;
        }
        if(pElem->GetType() == bson::bsonInt32Type)
        {
            bson::bsonInt32Ele* pInt = (bson::bsonInt32Ele*)pElem;
            nBlockCount = pInt->Int32Value();
        }
        else if(pElem->GetType() == bson::bsonInt64Type)
        {
            bson::bsonInt64Ele* pInt = (bson::bsonInt64Ele*)pElem;
            nBlockCount = pInt->Int64Value();
        }
        return true;
    }

    // 获取版本
    bool DEUQueryData::QueryVersion(const ID& id,std::vector<unsigned>& vList,int& nErrorCode)
    {
        const std::vector<std::string> vecServers = m_rcdInfo.GetRcdUrl(id);
        if(vecServers.empty())
        {
            nErrorCode = DEU_FAIL_GET_RCD;
            return false;
        }

        const unsigned nServer = rand() % vecServers.size();
        const std::string strHost = vecServers[nServer];
        //url
        std::ostringstream oss;
        oss << "http://"
            << strHost.c_str()
            << "/DEUDataPub?type=queryVersion&db="
            << id.ObjectID.m_nDataSetCode
            << "&id="
            << id.toString()
            << '\0';

        return QueryVersionFun(oss.str(),vList,nErrorCode);
    }

    std::vector<std::string> DEUQueryData::GetRcdUrl(int nDS)
    {
        return m_rcdInfo.GetRcdUrl(nDS);
    }

    std::vector<std::string> DEUQueryData::GetRcdUrl(const ID &ObjID)
    {
        return m_rcdInfo.GetRcdUrl(ObjID);
    }

    bool DEUQueryData::QueryVersionFun(const std::string& strUrl,std::vector<unsigned>& vList,int& nErrorCode)
    {
        //variables
        std::vector<char>   vecRespBuf;
        //request
        SimpleHttpClient shc;
        const int nRet = shc.Request(GetMethod, strUrl, vecRespBuf);
        if(vecRespBuf.empty())
        {
            nErrorCode = nRet;
            return false;
        }

        //get return code
        bson::bsonDocument bsonDoc;
        const bool bConv2Bson = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();
        if(!bConv2Bson)
        {
            return false;
        }

        bson::bsonElement* pElem = bsonDoc.GetElement("RetCode");
        if(pElem == NULL)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }
        const int nRetCode = pElem->Int32Value();
        if(nRetCode == 0)
        {
            pElem = bsonDoc.GetElement("ErrDisp");
            if(pElem == NULL)
            {
                nErrorCode = DEU_UNKNOWN;
            }
            else
            {
                nErrorCode = pElem->Int32Value();
            }
            return false;
        }

        pElem = bsonDoc.GetElement("Version");
        if(pElem == NULL)
        {
            nErrorCode = DEU_RECEIVE_NO_PARAM;
            return false;
        }
        if(pElem->GetType() == bson::bsonArrayType)
        {
            bson::bsonArrayEle* pArrayElem = (bson::bsonArrayEle*)pElem;
            vList.resize(pArrayElem->ChildCount());
            for(unsigned int i = 0;i < pArrayElem->ChildCount();i++)
            {
                bson::bsonElement* pChildElem = pArrayElem->GetElement(i);
                vList[i] = pChildElem->Int32Value();
            }
        }
        return true;
    }

    bool DEUQueryData::Login(const std::string& strHost,const std::string& strPort,const std::string& strUser,const std::string& strPwd,
                             std::vector<std::string>& strPermVec,std::string& strTicket,int& nErrorCode)
    {
        //url
        std::ostringstream oss;
        oss<<"http://"<<strHost<<":"<<strPort<<"/DEUSMServer?type=login"<<'\0';

        std::vector<char> vecPostBuffer;
        {
            bson::bsonDocument bDoc;
            bDoc.AddStringElement("UserID",strUser.c_str());
            bDoc.AddStringElement("UserPwd",strPwd.c_str());

            convertBsonDoc2Buffer(bDoc, vecPostBuffer);
        }

        std::vector<char>   vecRespBuf;
        SimpleHttpClient shc;
        const int nRet = shc.Request(PostMethod, oss.str(), vecRespBuf, vecPostBuffer);
        if(vecRespBuf.empty())
        {
            nErrorCode = nRet;
            return false;
        }

        //get return code
        bson::bsonDocument bsonDoc;
        const bool bConv2Bson = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();
        if(!bConv2Bson)
        {
            return false;
        }

        bson::bsonElement* pElem = bsonDoc.GetElement("RetCode");
        if(!pElem)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }
        const int nRetCode = pElem->Int32Value();
        if(nRetCode == 0)
        {
            pElem = bsonDoc.GetElement("ErrDisp");
            if(pElem == NULL)
            {
                nErrorCode = DEU_UNKNOWN;
            }
            else
            {
                nErrorCode = pElem->Int32Value();
            }
            return false;
        }

        bson::bsonElement* pTicketElem = bsonDoc.GetElement("Ticket");
        if(pTicketElem && pTicketElem->GetType() == bson::bsonStringType)
        {
            bson::bsonStringEle* pStrElem = (bson::bsonStringEle*)pTicketElem;
            strTicket = pStrElem->StrValue();
        }
        else
        {
            nErrorCode = DEU_RECEIVE_NO_PARAM;
            return false;
        }
        return true;
    }

    bool DEUQueryData::Logout(const std::string& strHost,const std::string& strPort,const std::string& strTicket,int& nErrorCode)
    {
        //url
        std::ostringstream oss;
        oss<<"http://"<<strHost<<":"<<strPort<<"/DEUSMServer?type=logout&tid="<<strTicket<<'\0';

        return DelDataFun(oss.str(),nErrorCode);
    }

    bool DEUQueryData::AuthPerm(const std::string& strHost,const std::string& strPort,const std::string& strTicket,const std::string& strPerm,int& nErrorCode)
    {
        std::ostringstream oss;
        oss<<"http://"<<strHost<<":"<<strPort<<"/DEUSMServer?type=authWritePerm&tid="<<strTicket<<"&perm="<<strPerm<<'\0';

        return DelDataFun(oss.str(),nErrorCode);
    }

    bool DEUQueryData::AuthRes(const std::string& strHost,const std::string& strPort,const std::string& strTicket,const unsigned nDB,int& nErrorCode)
    {
        std::ostringstream oss;
        oss<<"http://"<<strHost<<":"<<strPort<<"/DEUSMServer?type=authWriteRes&tid="<<strTicket<<"&db="<<nDB<<'\0';

        return DelDataFun(oss.str(),nErrorCode);
    }
    bool DEUQueryData::UpdateCacheVersion()
    {
        std::vector<std::string> portVec = m_rcdInfo.GetRcdUrl(6);
        if(portVec.empty())
        {
            return false;
        }
        const std::string strHost = portVec[0];
        //url
        std::ostringstream oss;
        oss << "http://"<< m_strHost<<":"<<m_strApachePort<<"/cgi-bin/DEUServerConf?type=updateCacheVersion"<<'\0';
        //variables
        std::vector<char>  vecRespBuf;
        //request
        SimpleHttpClient shc;
        int nRet = shc.Request(GetMethod, oss.str().c_str(), vecRespBuf);
        //return
        if(nRet == 0 || !vecRespBuf.empty())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    bool DEUQueryData::GetCacheVersion(unsigned __int64& nVersion,int& nErrorCode)
    {
        std::vector<std::string> portVec = m_rcdInfo.GetRcdUrl(6);
        if(portVec.empty())
        {
            return false;
        }
        const std::string strHost = portVec[0];
        //url
        std::ostringstream oss;
        oss << "http://"<< m_strHost<<":"<<m_strApachePort<<"/cgi-bin/DEUServerConf?type=getCacheVersion"<<'\0';
        //variables
        std::vector<char>  vecRespBuf;
        //request
        SimpleHttpClient shc;
        int nRet = shc.Request(GetMethod, oss.str().c_str(), vecRespBuf);
        //return
        if(nRet != 0 && vecRespBuf.empty())
        {
            nErrorCode = nRet;
            return false;
        }
        
        //get return code
        bson::bsonDocument bsonDoc;
        const bool bConv2Bson = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
        vecRespBuf.clear();
        vecRespBuf.shrink_to_fit();
        if(!bConv2Bson)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }

        bson::bsonElement* pElem = bsonDoc.GetElement("RetCode");
        if(pElem == NULL)
        {
            nErrorCode = DEU_UNKNOWN;
            return false;
        }
        const int nRetCode = pElem->Int32Value();
        if(nRetCode == 0)
        {
            pElem = bsonDoc.GetElement("ErrDisp");
            nErrorCode = pElem->Int32Value();
            return false;
        }
        else//else return 1
        {
            pElem = bsonDoc.GetElement("CacheVersion");
            if(pElem == NULL)
            {
                nErrorCode = DEU_RECEIVE_NO_PARAM;
                return false;
            }
            if(pElem->GetType() == bson::bsonInt64Type)
            {
                bson::bsonInt64Ele* pInt = (bson::bsonInt64Ele*)pElem;
                nVersion = pInt->Int64Value();
                return true;
            }
            else
            {
                nErrorCode = DEU_RECEIVE_NO_PARAM;
                return false;
            }
        }
        
    }
}

