#ifndef _WMS_DRIVER_H_F1A6168A_2182_4FF3_80DC_B3A050017C6C_
#define _WMS_DRIVER_H_F1A6168A_2182_4FF3_80DC_B3A050017C6C_

#include "IWMSDriver.h"

namespace deues
{
	class WMSDriver : public IWMSDriver
	{
	public:
		explicit WMSDriver(void);
		virtual ~WMSDriver(void);
		virtual bool initialize(const std::string& strUrl, const std::string& strVersion);
		virtual std::string getVersion() const { return m_strUrl; }
		virtual std::string getUrl() const { return m_strVersion; }
		
		ITileSet* createTileSet(const std::map<std::string,std::string>& strLayerMap,const std::string& strCRS);
		void getLayerInfo(std::vector<DEULayerInfo> &arrLayerInfo);
	protected:
		std::string getMetaData(void) const;
		bool QueryMetaData(std::string& strMetaData,int& nError) const;
	private:
		std::string m_strUrl;
		std::string m_strVersion;	
		std::vector<DEULayerInfo> m_arrLayerInfo;
		std::map<std::string,DEULayerInfo> m_LayerSizeMap;
	};
}

#endif