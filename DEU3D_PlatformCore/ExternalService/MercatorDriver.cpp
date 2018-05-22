#include "MercatorDriver.h"
#include "MercatorTileSet.h"
#include "CSimpleHttpClient.h"
#include <sstream>
#include <common/DEUBson.h>

namespace deues
{
	MercatorDriver::MercatorDriver(void)
	{
	}

	MercatorDriver::~MercatorDriver(void)
	{
	}

	bool MercatorDriver::initialize(const std::string& strUrl, const std::string& strVersion)
	{
		if (strUrl.empty())
			return false;
		m_strUrl = strUrl;
		return true;
	}

	ITileSet* MercatorDriver::createTileSet(const std::string& strMapService, const std::string& strMapStyle, const std::string& strDBPath)
	{
		OpenSP::sp<MercatorTileSet> pMercatorTileSet = new MercatorTileSet;
		pMercatorTileSet->initialize(m_strUrl,strMapService,strMapStyle, strDBPath);
		return pMercatorTileSet.release();
	}

	bool MercatorDriver::getMapServices(std::vector<DEUMorcatorInfo>& arrServices)
	{
		std::string strUrl="";
		std::ostringstream oss;
#ifdef _DEBUG
		oss<<strUrl<<m_strUrl<<"/DEUSMercatorSrvd?type=queryServices"<<'\0';
#else
		oss<<strUrl<<m_strUrl<<"/DEUSMercatorSrv?type=queryServices"<<'\0';
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

			for (int j=0; j<doc.ChildCount(); j++)
			{
				DEUMorcatorInfo obj;
				bson::bsonDocumentEle *pDocEle = (bson::bsonDocumentEle*)doc.GetElement(j);
				obj.m_strMapService = pDocEle->EName();
				bson::bsonDocument &docEle = pDocEle->GetDoc();
				bson::bsonDocumentEle *pStyle = (bson::bsonDocumentEle*)docEle.GetElement("MapStyle");
				if (pStyle)
				{
					std::string strMapStyle = "";
					bson::bsonDocument &styleDoc = pStyle->GetDoc();
					for (unsigned i=0; i<styleDoc.ChildCount(); i++)
					{
						bson::bsonElement* pChild = styleDoc.GetElement(i);
						strMapStyle = pChild->EName();
						obj.m_vecMapStyle.push_back(strMapStyle);
					}
				}
				arrServices.push_back(obj);
			}
			shc.FreeResponse(szRespBuf);
			if (arrServices.size() > 0)
				return true;
		}
		return false;
	}
		
	IMercatorDriver* createMercatorDriver(void)
	{
		OpenSP::sp<MercatorDriver> pDriver = new MercatorDriver;
		return pDriver.release();
	}
}
