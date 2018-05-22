#include "DEUUtils.h"
#include <Common/DEUBson.h>
#include <sstream>


DEUUtils::DEUUtils(void)
{
}


DEUUtils::~DEUUtils(void)
{
}


void DEUUtils::getRcdFromBson(const std::string strRcd,std::map<unsigned int,RcdInfo>& rcdMap)
{
    rcdMap.clear();

    bson::bsonDocument rcdDoc;
    if(!rcdDoc.FromJsonString(strRcd))
    {
        return;
    }
    std::string strDS = "",strName = "";
    // 循环获取数据集信息
    for(unsigned n = 0; n < rcdDoc.ChildCount();n++)
    {
        bson::bsonDocumentEle* pDocElem = (bson::bsonDocumentEle*)rcdDoc.GetElement(n);
        if(pDocElem == NULL)
        {
            continue;
        }
        // 获取数据集编码
        const char * pstrDS = pDocElem->EName();
        if (pstrDS == NULL)
        {
            continue;
        }

        strDS = pstrDS;
        unsigned nDs = atoi(strDS.c_str());

        RcdInfo rInfo;
        // 获取名称 name
        bson::bsonDocument& docElem = pDocElem->GetDoc();
        bson::bsonStringEle* pNameElem = (bson::bsonStringEle*)docElem.GetElement("name");
        if(pNameElem != NULL)
        {
            strName = pNameElem->StrValue();
        }
        rInfo.m_strName = strName;

        // 获取url
        bson::bsonArrayEle* pUrlArray = (bson::bsonArrayEle*)docElem.GetElement("url");
        if(pUrlArray == NULL)
        {
            continue;
        }
        std::string strStart = "",strEnd = "";
        std::vector<UrlInfo> urlVec;
        for(unsigned m = 0;m < pUrlArray->ChildCount();m++)
        {
            bson::bsonDocumentEle* pUrlDocEle = (bson::bsonDocumentEle*)pUrlArray->GetElement(m);
            if(pUrlDocEle == NULL)
                continue;
            bson::bsonDocument& urlDocEle = pUrlDocEle->GetDoc();
            UrlInfo urlInfo;
            bson::bsonElement* pStartElem = urlDocEle.GetElement("si");
            bson::bsonElement* pEneElem   = urlDocEle.GetElement("ei");
            pStartElem->ValueString(strStart,false);
            pEneElem->ValueString(strEnd,false);
            urlInfo.m_nStart = atoi(strStart.c_str());
            urlInfo.m_nEnd   = atoi(strEnd.c_str());

            bson::bsonDocumentEle* pPortDocEle = (bson::bsonDocumentEle*)urlDocEle.GetElement("port");
            if(pPortDocEle != NULL)
            {
                bson::bsonDocument& portDocEle = pPortDocEle->GetDoc();
                for(unsigned k = 0;k < portDocEle.ChildCount();k++)
                {
                    bson::bsonArrayEle* pArrayPort = (bson::bsonArrayEle*)portDocEle.GetElement(k);
                    if(pArrayPort == NULL)
                        continue;
                    std::string strIP = pArrayPort->EName();
                    std::vector<std::string> strPortVec;
                    std::string strPort = "";
                    for(unsigned l = 0;l < pArrayPort->ChildCount();l++)
                    {
                        bson::bsonElement* pPortElem = pArrayPort->GetElement(l);
                        if (pPortElem == NULL)
                        {
                            continue;
                        }
                        pPortElem->ValueString(strPort,false);
                        strPortVec.push_back(strPort);
                    }
                    if(!strPortVec.empty())
                        urlInfo.m_urlMap[strIP] = strPortVec;
                }

            }
            urlVec.push_back(urlInfo);
        }
        rInfo.m_urlInfoVec = urlVec;
        rcdMap[nDs] = rInfo;
    }
}

void DEUUtils::getHostFromBson(const std::string strHost,std::map<std::string,HostInfo>& hostMap)
{
    hostMap.clear();

    bson::bsonDocument serverDoc;
    if(strcmp(strHost.c_str(), "{}") != 0){
        if(!serverDoc.FromJsonString(strHost))
        {
            return;
        }
    }

    for(unsigned n = 0;n < serverDoc.ChildCount();n++)
    {
        std::string strApachePort = "";
        bson::bsonElement* pElem = serverDoc.GetElement(n);
        if(pElem == NULL)
            continue;
        if(pElem->GetType() == bson::bsonDocType)
        {
             HostInfo hInfo;

            std::string strIP = pElem->EName();
            bson::bsonDocumentEle* pDocElem = (bson::bsonDocumentEle*)pElem;
            bson::bsonDocument& childDoc = pDocElem->GetDoc();
            bson::bsonElement* pApachePort = childDoc.GetElement("ApachePort");
            if(pApachePort == NULL)
                continue;
            pApachePort->ValueString(strApachePort,false);
            hInfo.m_strApachePort = strApachePort;
            bson::bsonArrayEle* pPortArray = (bson::bsonArrayEle*)childDoc.GetElement("Ports");
            std::vector<std::string> strPortVec;
            if(pPortArray != NULL)
            {
                std::string strPort = "";
                for(unsigned m = 0;m < pPortArray->ChildCount();m++)
                {
                    pPortArray->GetElement(m)->ValueString(strPort,false);
                    if(strPort == "")
                        continue;
                    strPortVec.push_back(strPort);
                }
            }
            hInfo.m_portVec = strPortVec;
            hostMap[strIP] = hInfo;
        }
    }
}


std::string DEUUtils::getHostBsonString(const std::map<std::string,HostInfo>& hostMap)
{
    std::string strHost = "";

    std::map<std::string,HostInfo>::const_iterator itr =  hostMap.cbegin();
    bson::bsonDocument hostDoc;
    while (itr != hostMap.cend())
    {
        bson::bsonDocumentEle* pHostElem = (bson::bsonDocumentEle*)hostDoc.AddDocumentElement(itr->first.c_str());
        if (pHostElem == NULL)
        {
            return strHost;
        }

        bson::bsonDocument& childHost = pHostElem->GetDoc();

        HostInfo hInfo = itr->second;
        childHost.AddStringElement("ApachePort",hInfo.m_strApachePort.c_str());

        bson::bsonArrayEle* pPortArray = (bson::bsonArrayEle*)childHost.AddArrayElement("Ports");
        if (pPortArray == NULL)
        {
            return strHost;
        }

        for(unsigned n = 0;n < hInfo.m_portVec.size();n++)
        {
            pPortArray->AddStringElement(hInfo.m_portVec[n].c_str());
        }
        itr++;
    }

    hostDoc.JsonString(strHost);
    return strHost;
}


std::string DEUUtils::getRcdBsonString(const std::map<unsigned int,RcdInfo>& rcdMap)
{
    std::string strRcd = "";

    std::map<unsigned int,RcdInfo>::const_iterator itr = rcdMap.cbegin();
    bson::bsonDocument rcdDoc;
    while(itr != rcdMap.cend())
    {
        unsigned nDSCode = itr->first;
        RcdInfo rInfo = itr->second;
        std::ostringstream oss;
        oss<<nDSCode;
        bson::bsonDocumentEle* pUrlDoc = (bson::bsonDocumentEle*)rcdDoc.AddDocumentElement(oss.str().c_str());
        if(pUrlDoc == NULL)
        {
            itr++;
            continue;
        }
        bson::bsonDocument& urlDoc = pUrlDoc->GetDoc();
        urlDoc.AddStringElement("name",rInfo.m_strName.c_str());

        bson::bsonArrayEle* pUrlArray = (bson::bsonArrayEle*)urlDoc.AddArrayElement("url");
        if (pUrlArray == NULL)
        {
            return strRcd;
        }

        for(unsigned n = 0;n < rInfo.m_urlInfoVec.size();n++)
        {
            UrlInfo uInfo = rInfo.m_urlInfoVec[n];
            bson::bsonDocumentEle* pChildDoc = (bson::bsonDocumentEle*)pUrlArray->AddDocumentElement();
            if (pChildDoc == NULL)
            {
                return strRcd;
            }

            bson::bsonDocument& childDoc = pChildDoc->GetDoc();

            childDoc.AddInt32Element("si",uInfo.m_nStart);
            childDoc.AddInt32Element("ei",uInfo.m_nEnd);

            if(!uInfo.m_urlMap.empty())
            {
                bson::bsonDocumentEle* pPortElem = (bson::bsonDocumentEle*)childDoc.AddDocumentElement("port");
                if (pPortElem == NULL)
                {
                    return strRcd;
                }

                bson::bsonDocument& portDoc = pPortElem->GetDoc();
                std::map<std::string,std::vector<std::string>>::const_iterator tempItr = uInfo.m_urlMap.cbegin();
                while(tempItr != uInfo.m_urlMap.cend())
                {
                    bson::bsonArrayEle* pPortArray = (bson::bsonArrayEle*)portDoc.AddArrayElement(tempItr->first.c_str());
                    if (pPortArray == NULL)
                    {
                        return strRcd;
                    }

                    for(unsigned k = 0;k < tempItr->second.size();k++)
                    {
                        pPortArray->AddStringElement(tempItr->second[k].c_str());
                    }
                    tempItr++;
                }
            }
        }

        itr++;
    }

    rcdDoc.JsonString(strRcd);
    return strRcd;
}
