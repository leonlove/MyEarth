#include "DEURcdInfo.h"
#include <sstream>
#include <common/DEUBson.h>
#include <algorithm>

namespace deunw
{
    DEURcdInfo::DEURcdInfo(void)
    {
        m_strApachePort = m_strHost = m_strRcdInfo = m_strServerInfo = "";
    }



    DEURcdInfo::~DEURcdInfo(void)
    {
        m_strApachePort = m_strHost = m_strRcdInfo = m_strServerInfo = "";
    }

    struct UrlInfo
    {
        unsigned m_nStartIndex;
        unsigned m_nEndIndex;
        std::vector<std::string> strUrlVec;
    };

    struct RcdSorter
    {
        bool operator()(const UrlInfo &left, const UrlInfo &right)
        {
            if(left.m_nStartIndex < right.m_nStartIndex)   
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    };

    bool DEURcdInfo::Init(const std::string &strServer,const std::string& strApachePort)
    {
        m_strHost = strServer;
        m_strApachePort = strApachePort;

        std::string strErr = "";
        int nLen = 0;
        char *pBson = NULL;
#ifdef _SERVER_DEBUG_
        if(GetServerFile("DEUServerConfd","getRcdInfo", pBson, nLen, strErr))
#else
        if(GetServerFile("DEUServerConf","getRcdInfo", pBson, nLen, strErr))
#endif
        {
            if(pBson)
            {
                m_strRcdInfo = pBson;
                delete []pBson;
                pBson = NULL;
                nLen = 0;
            }
        }
        else
        {
            return false;
        }
#ifdef _SERVER_DEBUG_
        if(GetServerFile("DEUServerConfd","getServerInfo", pBson, nLen, strErr))
#else
        if(GetServerFile("DEUServerConf","getServerInfo", pBson, nLen, strErr))
#endif
        {
            if(pBson)
            {
                m_strServerInfo = pBson;
                delete []pBson;
            }
        }
        else
        {
            return false;
        }

        return ResolveServer();
    }

    std::vector<std::string> DEURcdInfo::GetDSUrl(int nDS)
    {
        std::vector<std::string>    vecUrls;

        std::map<int,ID2Url>::const_iterator itr = m_ID2UrlMap.find(nDS);
        if(itr == m_ID2UrlMap.cend())
        {
            return vecUrls;
        }

        ID2Url url = itr->second;
        std::string strUrl = "";
        unsigned nLength = url.GetRcdNum();
        for(unsigned n = 0;n < nLength;n++)
        {
            rcd* rd = url.GetRcdByIndex(n);
            if(rd == NULL)
                continue;
            for(unsigned n=0; n<rd->GetURLNum(); n++)
            {
                vecUrls.push_back(rd->GetURL(n));
            }
        }
        return vecUrls;
    }


    std::vector<std::string> DEURcdInfo::GetRcdUrl(const unsigned & nDs)
    {
        std::vector<std::string>    vecUrls;
        std::map<int,ID2Url>::const_iterator itr = m_ID2UrlMap.find(nDs);
        if(itr == m_ID2UrlMap.end())
            return vecUrls;

        ID2Url uu = itr->second;
        int nNum = uu.GetRcdNum();
        for(int m = 0;m < nNum;m++)
        {
            rcd* rd = uu.GetRcdByIndex(m);
            if(rd == NULL)
                continue;
            for(unsigned n = 0; n < rd->GetURLNum(); n++)
            {
                vecUrls.push_back(rd->GetURL(n));
            }
        }
        return vecUrls;
    };

    std::vector<std::string> DEURcdInfo::GetRcdUrl(const ID &ObjID)
    {
        std::vector<std::string>    vecUrls;
        unsigned int nDs = ObjID.ObjectID.m_nDataSetCode;
        std::map<int,ID2Url>::const_iterator itr = m_ID2UrlMap.find(nDs);
        if(itr == m_ID2UrlMap.end())
            return vecUrls;
        rcd* rd = GetRcdByGuid(ObjID,itr->second);
        if(rd == NULL)
            return vecUrls;

        for(unsigned n=0; n<rd->GetURLNum(); n++)
        {
            vecUrls.push_back(rd->GetURL(n));
        }

        return vecUrls;
    };


    bool DEURcdInfo::GetIndex(bson::bsonElement* pElem,unsigned& nValue)
    {
        if(pElem == NULL)
        {
            return false;
        }

        if(pElem->GetType() == bson::bsonDoubleType)
        {
            bson::bsonDoubleEle * pDblElem = (bson::bsonDoubleEle*)pElem;
            double dValue = pDblElem->DblValue();
            nValue = dValue;
            return true;
        }
        else if(pElem->GetType() == bson::bsonInt32Type)
        {
            bson::bsonInt32Ele* pIntElem = (bson::bsonInt32Ele*)pElem;
            nValue = pIntElem->Int32Value();
            return true;
        }
        else if(pElem->GetType() == bson::bsonInt64Type)
        {
            bson::bsonInt64Ele* pIntElem = (bson::bsonInt64Ele*)pElem;
            nValue = pIntElem->Int64Value();
            return true;
        }
        else if(pElem->GetType() == bson::bsonBoolType)
        {
            bson::bsonBoolEle* pBoolElem = (bson::bsonBoolEle*)pElem;
            nValue = pBoolElem->BoolValue();
            return true;
        }
        else if(pElem->GetType() == bson::bsonStringType)
        {
            bson::bsonStringEle* pStrElem = (bson::bsonStringEle*)pElem;
            const char* chStr = pStrElem->StrValue();
            nValue = atoi(chStr);
            return true;
        }
        else 
            return false;
    }

    bool DEURcdInfo::ResolveServer()
    {
       bson::bsonDocument rcdDoc;
       bson::bsonDocument serverDoc;
      
       if (strcmp(m_strServerInfo.c_str(), "") == 0)
       {
           if (strcmp(m_strRcdInfo.c_str(), "") == 0)
           {
               return true;
           }
           else
           {
               return false;
           }
       }
       else
       {
           if(strcmp(m_strRcdInfo.c_str(), "") != 0)
           {
               if(strcmp(m_strRcdInfo.c_str(), "{}") != 0)
               {
                   if(!rcdDoc.FromJsonString(m_strRcdInfo))
                   {
                       return false;
                   }
               }
           }
       }

       //增加对空数据引起的异常的判断 add by hsc 2013.04.24
       
       if(strcmp(m_strServerInfo.c_str(), "{}") != 0 )
       {
            
            if(!serverDoc.FromJsonString(m_strServerInfo))
            {
                return false;
            }
       }
       
       m_serverMap.clear();
       m_ID2UrlMap.clear();
       // 循环获取数据集信息
       for(unsigned n = 0; n < rcdDoc.ChildCount();n++)
       {
           bson::bsonDocumentEle* pDocElem = (bson::bsonDocumentEle*)rcdDoc.GetElement(n);
           if(pDocElem == NULL)
           {
               continue;
           }
           // 获取数据集编码
           std::string strDS = pDocElem->EName();
           unsigned nDs = atoi(strDS.c_str());

           bson::bsonDocument& docElem = pDocElem->GetDoc();
           // 获取url
           bson::bsonArrayEle* pUrlArray = (bson::bsonArrayEle*)docElem.GetElement("url");
           if(pUrlArray == NULL)
           {
               continue;
           }
           std::string strStart = "",strEnd = "";
       
           unsigned nUrlCount = pUrlArray->ChildCount();
          /* ID2Url _ID2Url;
           _ID2Url.SetInit(nUrlCount);*/
           std::vector<UrlInfo> urlVec;

           for(unsigned m = 0;m < nUrlCount;m++)
           {
               bson::bsonDocumentEle* pUrlDocEle = (bson::bsonDocumentEle*)pUrlArray->GetElement(m);
               if(pUrlDocEle == NULL)
                   continue;
               bson::bsonDocument& urlDocEle = pUrlDocEle->GetDoc();
              
               bson::bsonElement* pStartElem = urlDocEle.GetElement("si");
               bson::bsonElement* pEneElem   = urlDocEle.GetElement("ei");
               pStartElem->ValueString(strStart,false);
               pEneElem->ValueString(strEnd,false);
               unsigned nStartIndex = atoi(strStart.c_str());
               unsigned nEndIndex = atoi(strEnd.c_str());
               UrlInfo uInfo;
               uInfo.m_nStartIndex = nStartIndex;
               uInfo.m_nEndIndex = nEndIndex;
               //_ID2Url.AddRcd(m,nStartIndex,nEndIndex);

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
                           pPortElem->ValueString(strPort,false);
                           std::string strUrl = strIP + ":" + strPort;
                           strPortVec.push_back(strPort);
                           uInfo.strUrlVec.push_back(strUrl);
                           //_ID2Url.SetRcd(m,strUrl.c_str());
                           
                       }
                       std::map<std::string,std::vector<std::string>>::iterator tempItr = m_serverMap.find(strIP);
                       if(tempItr == m_serverMap.end())
                       {
                            m_serverMap[strIP] = strPortVec;
                       }
                       else
                       {
                            std::vector<std::string> tempVec = m_serverMap[strIP];
                            for(unsigned mm = 0;mm < strPortVec.size();mm++)
                            {
                                std::vector<std::string>::iterator portItr = std::find(tempVec.begin(),tempVec.end(),strPortVec[mm]);
                                if(portItr == tempVec.end())
                                    tempVec.push_back(strPortVec[mm]);
                            }
                            m_serverMap[strIP] = tempVec;
                       }
                   }
               }

               urlVec.push_back(uInfo);
           }

           ID2Url _ID2Url;
           unsigned nSize = urlVec.size();
           _ID2Url.SetInit(nSize);

           if(nSize == 1)
           {
                UrlInfo urlInfo = urlVec[0];
                _ID2Url.AddRcd(0,urlInfo.m_nStartIndex,urlInfo.m_nEndIndex);
                for(unsigned n = 0;n < urlInfo.strUrlVec.size();n++)
                    _ID2Url.SetRcd(0,urlInfo.strUrlVec[n].c_str());
           }
           else if(nSize > 1)
           {
                std::sort(urlVec.begin(),urlVec.end(),RcdSorter());
                for(unsigned m = 0;m < urlVec.size();m++)
                {
                    UrlInfo urlInfo = urlVec[m];
                    _ID2Url.AddRcd(m,urlInfo.m_nStartIndex,urlInfo.m_nEndIndex);
                    for(unsigned n = 0;n < urlInfo.strUrlVec.size();n++)
                        _ID2Url.SetRcd(m,urlInfo.strUrlVec[n].c_str());
                }
           }
           else 
           {
                continue;
           }
           m_ID2UrlMap[nDs] = _ID2Url;
       }
        return true;
    }

    rcd* DEURcdInfo::GetRcdByGuid(const ID &id,const ID2Url& _ID2Url ) const
    {
        if(_ID2Url.GetRcdNum() == 0)
            return NULL;

        unsigned int nNum;
        const std::string strID = id.toString();
        _ID2Url.GetNum((unsigned char *)strID.c_str(), strID.length(), nNum);

        ID2Url &convert = const_cast<ID2Url &>(_ID2Url);
        return convert.GetRcdByNum(nNum);
    }

    bool DEURcdInfo::GetServerFile(const std::string& strServer,const std::string& strType, char* &szBson, int &nlen, std::string &strErr)
    {
        //url
        std::ostringstream oss;
        oss<<"http://"<<m_strHost<<":"<<m_strApachePort<<"/cgi-bin/"<<strServer<<"?type="<<strType<<'\0';

        std::vector<char>   vecRespBuf;
        SimpleHttpClient shc;
        const int nRet = shc.Request(GetMethod, oss.str(), vecRespBuf);
        if(vecRespBuf.empty())
        {
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

        bson::bsonElement* bElem = bsonDoc.GetElement("RetCode");
        if(!bElem) return false;
        const int nRetCode = bElem->Int32Value();
        if(nRetCode == 0)
        {
            bElem = bsonDoc.GetElement("ErrDisp");
            if(bElem)
            {
                strErr = bElem->ValueString(strErr,false);
            }
            return false;
        }

        bElem = bsonDoc.GetElement("Data");
        if(bElem == NULL)
        {
            return true;
        }

        if(bElem->GetType() == bson::bsonBinType)
        {
            const void* tmpBson = bElem->BinData();
            nlen = bElem->BinDataLen();
            szBson = (char *)malloc(nlen + 1);
            memcpy(szBson, tmpBson, nlen);
            szBson[nlen] = '\0';
        }
        return true;
    }

}

