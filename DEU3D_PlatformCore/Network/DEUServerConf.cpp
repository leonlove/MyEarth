#include "DEUServerConf.h"
#include <common/DEUBson.h>
#include "CSimpleHttpClient.h"
#include <algorithm>
#include <sstream>

namespace deunw
{

    DEUServerConf::DEUServerConf(void)
    {
        m_nRetry = 0;
    }


    DEUServerConf::~DEUServerConf(void)
    {
        m_nRetry = 0;
    }

    //post function
    bool DEUServerConf::postFun(const std::string& strUrl,const std::vector<char> &vecBuffer, std::string& strErr)
    {
        std::vector<char>   vecRespBuf;
        SimpleHttpClient shc;
        const int nRet = shc.Request(PostMethod, strUrl, vecRespBuf, vecBuffer);

        //return
        if(nRet == 0 || !vecRespBuf.empty())
        {
            bson::bsonDocument bsonDoc;
            const bool bConv2Bson = bsonDoc.FromBsonStream(vecRespBuf.data(), vecRespBuf.size());
            vecRespBuf.clear();
            vecRespBuf.shrink_to_fit();
            if(!bConv2Bson)
            {
                return false;
            }

            bson::bsonElement* pElem = bsonDoc.GetElement("RetCode");
            const int nRetCode = pElem->Int32Value();
            if(nRetCode == 0)
            {
                pElem = bsonDoc.GetElement("ErrDisp");
                strErr = pElem->StrValue();
                return false;
            }
            else//else return 1
            {
                return true;
            }
        }
        else
        {
            strErr = "提交访问失败";
            return false;
        }
    }

    //创建服务
    bool DEUServerConf::createService(const std::string& strHost,const std::string& strPath,const std::vector<std::string>& strPortVec,
                                      std::string& strErr,bool bIsMainSvr,const std::string& strMainPort)
    {
        if(strHost == "" || strPath == "" || strPortVec.empty()
            || (bIsMainSvr && strMainPort == "")
            || (bIsMainSvr && std::find(strPortVec.cbegin(),strPortVec.cend(),strMainPort) == strPortVec.cend()))
        {
            return false;
        }
        bson::bsonDocument bsonDoc;
        bsonDoc.AddStringElement("Path",strPath.c_str());
        bson::bsonElement* pElem = bsonDoc.AddArrayElement("Port");
        if(pElem->GetType() == bson::bsonArrayType)
        {
            bson::bsonArrayEle* pArrayElem = (bson::bsonArrayEle*)pElem;
            for(size_t i = 0;i < strPortVec.size();i++)
            {
                pArrayElem->AddStringElement(strPortVec[i].c_str());
            }
            bsonDoc.AddBoolElement("IsMainSvr",bIsMainSvr);
            bsonDoc.AddStringElement("MainPort",strMainPort.c_str());

            std::string strJson = "";
            bsonDoc.JsonString(strJson);

            const std::vector<char> vecJsonStream(strJson.begin(), strJson.end());

            //url
            std::ostringstream oss;
#ifdef _SERVER_DEBUG_
            oss<<"http://"<<strHost<<"/cgi-bin/DEUServerConfd?type=createService"<<'\0';
#else
            oss<<"http://"<<strHost<<"/cgi-bin/DEUServerConf?type=createService"<<'\0';
#endif
            
            for(unsigned n = 0;n <= m_nRetry;n++)
            {
                if(postFun(oss.str(), vecJsonStream, strErr))
                {
                    return true;

                }
            }
            return false;
        }
        else
        {
            strErr = "Fail to construct a bson document";
            return false;
        }
    }

  /*  bool DEUServerConf::createService()
    {
        
    }*/
}
