#include <common/Common.h>
#include <sstream>
#include  <io.h>
#include "MercatorTileSet.h"
#include "CSimpleHttpClient.h"


namespace deues
{

    const UINT_64       g_nMctRBuffSize   = 128ui64 * 1024ui64 * 1024ui64;
    const UINT_64       g_nMctWBuffSize   = 64ui64 * 1024ui64 * 1024ui64;

	MercatorTileSet::MercatorTileSet(void)
	{
        m_strUrl = "";
		m_strService = "";
		m_strMapStyle = "";
        m_pDBProxy = NULL;
	}

	MercatorTileSet::~MercatorTileSet(void)
	{
        unInitialize();
	}

    void MercatorTileSet::unInitialize(void)
    {
        m_strUrl = "";
        m_strService = "";
        m_strMapStyle = "";
        if (m_pDBProxy)
        {
            m_pDBProxy->closeDB();
            m_pDBProxy = NULL;
        }
    }

	void MercatorTileSet::initialize(const std::string& strUrl,const std::string& strService,const std::string&  strMapStyle, const std::string&  strDBPath)
	{
		m_strUrl = strUrl;
		m_strService = strService;
		m_strMapStyle = strMapStyle;

        OpenDB(strDBPath);

	}

	bool MercatorTileSet::getTopInfo(void*& pBuffer,unsigned int& nLength) const
	{
		std::string strUrl="";
		std::ostringstream oss;
#ifdef _DEBUG
		oss<<strUrl<<m_strUrl<<"/DEUSMercatorSrvd?type=getTopInfo&service="<<m_strService
			<<"&mapstyle="<<m_strMapStyle<<'\0';
#else
		oss<<strUrl<<m_strUrl<<"/DEUSMercatorSrv?type=getTopInfo&service="<<m_strService
			<<"&mapstyle="<<m_strMapStyle<<'\0';
#endif
		bson::bsonDocument doc;
		SimpleHttpClient shc;
		char* szRespBuf = NULL;
		long nResplen = 0;
		int nRet = shc.Request(GetMethod, oss.str().c_str(), &szRespBuf, &nResplen);
		if(nRet == 0 || szRespBuf != NULL)
		{
			unsigned i = 0;
			for(i = nResplen - 1;i >= 0;i--)
			{
				if( (szRespBuf[i] != ' ') && (szRespBuf[i]!=0x0a)&&(szRespBuf[i]!=0x0d) )
				{
					break ;
				}
			}
			szRespBuf[i+1] = 0;
			bson::bsonStream bs;
			bs.Write((const void*)szRespBuf,nResplen);
			nLength = bs.DataLen();
			pBuffer = malloc(nLength);
			memcpy(pBuffer,bs.Data(),nLength);
		}
		else
		{
			return false;
		}
		return true;
	}

	unsigned __int64 MercatorTileSet::getUniqueFlag() const
	{
		unsigned __int64 nUniqueFlag = 0;
		std::string strUrl="";
		std::ostringstream oss;
#ifdef _DEBUG
		oss<<strUrl<<m_strUrl<<"/DEUSMercatorSrvd?type=getUniqueFlag&service="<<m_strService
			<<"&mapstyle="<<m_strMapStyle<<'\0';
#else
		oss<<strUrl<<m_strUrl<<"/DEUSMercatorSrv?type=getUniqueFlag&service="<<m_strService
			<<"&mapstyle="<<m_strMapStyle<<'\0';
#endif
		bson::bsonDocument doc;
		SimpleHttpClient shc;
		char* szRespBuf = NULL;
		long nResplen = 0;
		int nRet = shc.Request(GetMethod, oss.str().c_str(), &szRespBuf, &nResplen);
		if(nRet == 0 || szRespBuf != NULL)
		{
			unsigned i = 0;
			for(i = nResplen - 1;i >= 0;i--)
			{
				if( (szRespBuf[i] != ' ') && (szRespBuf[i]!=0x0a)&&(szRespBuf[i]!=0x0d) )
				{
					break ;
				}
			}
			szRespBuf[i+1] = 0;
			bson::bsonStream bs;
			bs.Write((const void*)szRespBuf,nResplen);
			bs.Reset();
			doc.Read(&bs);
			bson::bsonElement *pUniqueFlag = (bson::bsonElement*)doc.GetElement("UniqueFlag");
			if (pUniqueFlag != NULL && pUniqueFlag->GetType() == bson::bsonInt64Type) 
				nUniqueFlag = pUniqueFlag->Int64Value();
		}
		return nUniqueFlag;
	}

	const ID&  MercatorTileSet::getID()
	{
		std::string strUrl="";
		std::ostringstream oss;
#ifdef _DEBUG
		oss<<strUrl<<m_strUrl<<"/DEUSMercatorSrvd?type=getID&service="<<m_strService
			<<"&mapstyle="<<m_strMapStyle<<'\0';
#else
		oss<<strUrl<<m_strUrl<<"/DEUSMercatorSrv?type=getID&service="<<m_strService
			<<"&mapstyle="<<m_strMapStyle<<'\0';
#endif
		bson::bsonDocument doc;
		SimpleHttpClient shc;
		char* szRespBuf = NULL;
		long nResplen = 0;
		int nRet = shc.Request(GetMethod, oss.str().c_str(), &szRespBuf, &nResplen);
		if(nRet == 0 || szRespBuf != NULL)
		{
			unsigned i = 0;
			for(i = nResplen - 1;i >= 0;i--)
			{
				if( (szRespBuf[i] != ' ') && (szRespBuf[i]!=0x0a)&&(szRespBuf[i]!=0x0d) )
				{
					break ;
				}
			}
			szRespBuf[i+1] = 0;
			bson::bsonStream bs;
			bs.Write((const void*)szRespBuf,nResplen);
			bs.Reset();
			doc.Read(&bs);
			std::string strTemp = "";
			doc.JsonString(strTemp);
			std::string strID = "";
			bson::bsonElement *pTopId = (bson::bsonElement*)doc.GetElement("TopID");
			if (pTopId != NULL && pTopId->GetType() == bson::bsonStringType) 
			{
				strID = pTopId->StrValue();
				if (!strID.empty())
					m_topID = ID::genIDfromString(strID);
				std::string str = m_topID.toString();
			}
		}
		return m_topID;
	}

	bool MercatorTileSet::queryData(const ID& id,void*& pBuffer,unsigned int& nLength,int& nError) const
	{

        if(m_pDBProxy != NULL)
        {
            if(m_pDBProxy->readBlock(id, pBuffer, nLength))
            {
                return true;
            }
        }

		std::string strUrl="";
		std::ostringstream oss;
#ifdef _DEBUG
		oss<<strUrl<<m_strUrl<<"/DEUSMercatorSrvd?type=queryTileData&id="<<id.toString().c_str()<<'\0';
#else
		oss<<strUrl<<m_strUrl<<"/DEUSMercatorSrv?type=queryTileData&id="<<id.toString().c_str()<<'\0';
#endif
		SimpleHttpClient shc;
		char* szRespBuf = NULL;
		long nResplen = 0;
		int nRet = shc.Request(GetMethod, oss.str().c_str(), &szRespBuf, &nResplen);
		if(nRet == 0 || szRespBuf != NULL)
		{
			unsigned i = 0;
			for(i = nResplen - 1;i >= 0;i--)
			{
				if( (szRespBuf[i] != ' ') && (szRespBuf[i]!=0x0a)&&(szRespBuf[i]!=0x0d) )
				{
					break ;
				}
			}
			szRespBuf[i+1] = 0;
			bson::bsonDocument doc;
			bson::bsonStream bs;
			bs.Write((const char*)szRespBuf,nResplen);
			bs.Reset();
			doc.Read(&bs);
			bson::bsonBinaryEle *pTileEle = (bson::bsonBinaryEle*)doc.GetElement("Data");
			if (pTileEle != NULL)
			{
				nLength = pTileEle->BinDataLen();
				pBuffer = (char*)malloc(nLength);
				memcpy(pBuffer,pTileEle->BinData(),nLength);

                if(m_pDBProxy != NULL)
                {
                    m_pDBProxy->replaceBlock(id, pBuffer, nLength);
                }

			}
			shc.FreeResponse(szRespBuf);
			return true;
		}
		else
		{
			nError = nRet;
			return false;
		}
	}


    bool MercatorTileSet::OpenDB(const std::string& p_strDBPath)
    {
        std::string strDBPath = p_strDBPath;

        if (strDBPath == "")
        {
            strDBPath = "C:\\MctCache";
        }

        int nPos = strDBPath.rfind("/");
        int nPos2 = strDBPath.rfind("\\");
        if(nPos == -1 && nPos2 == -1)
        {
            return false;
        }

        nPos = nPos > nPos2 ? nPos : nPos2;

        std::string strPath = "";
        strPath = strDBPath.substr(0,nPos);
        int nRes = _access(strPath.c_str(),0);
        if(nRes == -1)
        {
            return false;
        }

        m_pDBProxy = deudbProxy::createDEUDBProxy();
        if(!m_pDBProxy.valid())
        {
            return false;
        }

        if(!m_pDBProxy->openDB(strDBPath, g_nMctRBuffSize, g_nMctWBuffSize))
        {
            m_pDBProxy = NULL;
            return false;
        }

        return true;
    }

// 	unsigned MercatorTileSet::getLevel(double dScale,double& dOutScale) const
// 	{
// 		return 0;
// 	}
// 
// 	bool MercatorTileSet::getTileInfo(const ID& id,DEUTileInfo& tInfo) const
// 	{
// 		return true;
// 	}
// 

}
