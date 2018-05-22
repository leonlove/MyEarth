#ifndef _DEUNETWORKRCDINFO_H_
#define _DEUNETWORKRCDINFO_H_

#include <string>
#include <map>
#include "IDProvider/ID.h"
#include "CSimpleHttpClient.h"
#include "define.h"

namespace bson
{
    class bsonElement;
}

namespace deunw
{
    class DEURcdInfo
    {
    public:
        DEURcdInfo(void);
        ~DEURcdInfo(void);

    public:
        bool Init(const std::string &strServer,const std::string& strApachePort);
        std::vector<std::string> GetRcdUrl(const ID &ObjID);
        std::vector<std::string> GetRcdUrl(const unsigned & nDs);
        std::vector<std::string> GetDSUrl(int nDS);
        std::map<std::string,std::vector<std::string>> GetServerMap(){return m_serverMap;}
  
    private:
        bool ResolveServer();
        rcd* GetRcdByGuid(const ID &id,const ID2Url& _ID2Url) const;
        bool GetServerFile(const std::string& strServer,const std::string& strType,char* &szBson, int &nlen, std::string &strErr);
        bool GetIndex(bson::bsonElement* pElem,unsigned& nIndex);

    private:
        std::map<std::string,std::vector<std::string>> m_serverMap;
        std::string m_strHost;
        std::string m_strApachePort;
        std::string m_strRcdInfo; 
        std::string m_strServerInfo;
        std::map<int,ID2Url> m_ID2UrlMap;
    };
}
#endif //_DEUNETWORKRCDINFO_H_

