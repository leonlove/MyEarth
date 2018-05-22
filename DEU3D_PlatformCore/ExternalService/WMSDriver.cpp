#include "WMSDriver.h"
#include "WMSTileSet.h"
#include "CSimpleHttpClient.h"
#include <sstream>
#include "DEUUtils.h"

namespace deues
{
	WMSDriver::WMSDriver(void)
	{
	}
	
	WMSDriver::~WMSDriver(void)
	{
		for(int i=0; i<m_arrLayerInfo.size(); i++)
		{
			m_arrLayerInfo[i].m_vecCRS.clear();
			m_arrLayerInfo[i].m_vecStyleInfo.clear();
		}
		m_arrLayerInfo.clear();
		m_LayerSizeMap.clear();
	}

	bool WMSDriver::initialize(const std::string& strUrl, const std::string& strVersion)
	{
		int nFind = strUrl.rfind('?');
		if(nFind == -1)
		{
			m_strUrl = strUrl;
		}
		else
		{
			m_strUrl = strUrl.substr(0,nFind);
		}
		m_strVersion = strVersion;
		std::string strXML = getMetaData();
		if(!DEUUtils::getWMSMetaData(strXML.c_str(), &m_arrLayerInfo))
		{
			return false;
		}
		return true;
	}

	ITileSet* WMSDriver::createTileSet(const std::map<std::string,std::string>& strLayerMap,const std::string& strCRS)
	{
		OpenSP::sp<WMSTileSet> pWMSTileSet = new WMSTileSet;
		
		m_LayerSizeMap.clear();
		for(int i=0; i<m_arrLayerInfo.size(); i++)
		{
			DEULayerInfo layer = m_arrLayerInfo[i];
			m_LayerSizeMap[layer.m_strLayerName] = layer;
		}

		pWMSTileSet->initialize(m_LayerSizeMap, m_strUrl, m_strVersion, strLayerMap, strCRS);

		return pWMSTileSet.release();
	}

	void WMSDriver::getLayerInfo(std::vector<DEULayerInfo> &arrLayerInfo)
	{		
		arrLayerInfo = m_arrLayerInfo;
	}

	std::string WMSDriver::getMetaData(void) const
	{
		int nError = 0;
		std::string strMetaData = "";
		bool bRes = QueryMetaData(strMetaData,nError);
		return strMetaData;
	}

	bool WMSDriver::QueryMetaData(std::string& strMetaData,int& nError) const
	{
		//variables
		char* szRespBuf = NULL;
		long nResplen = 0;
		//request
		SimpleHttpClient shc;
		std::ostringstream oss;
		oss<<m_strUrl<<"?SERVICE=WMS&VERSION="<<m_strVersion<<"&REQUEST=GetCapabilities";
		std::string strUrl = DEUUtils::urlEncode(oss.str());
		int nRet = shc.Request(GetMethod, strUrl.c_str(), &szRespBuf, &nResplen);
		//return
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
			strMetaData = szRespBuf;
			shc.FreeResponse(szRespBuf);
			return true;
		}
		else
		{
			nError = nRet;
			return false;
		}
	}

	IWMSDriver *createWMSDriver(void)
	{
		OpenSP::sp<WMSDriver> pDriver = new WMSDriver;
		return pDriver.release();
	}

// 	void freeMemory(void *p)
// 	{
// 		free(p);
// 	}
}
