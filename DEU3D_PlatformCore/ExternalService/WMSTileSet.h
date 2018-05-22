#pragma once
#include "ITileSet.h"

namespace deues
{
	class WMSTileSet : public ITileSet
	{
	public:
		WMSTileSet(void);
		~WMSTileSet(void);
	public:
		void initialize(std::map<std::string, DEULayerInfo>& layerSizeMap, std::string strURL, std::string strVersion, std::map<std::string,std::string> strLayerMap, std::string strCRS);
		virtual bool getTopInfo(void*& pBuffer,unsigned int& nLength) const;
		virtual bool queryData(const ID& id,void*& pBuffer,unsigned int& nLength,int& nError) const;
		virtual const ID&  getID() { return m_topID; }
		virtual unsigned __int64 getUniqueFlag() const { return m_nUniqueFlag; }
		virtual unsigned getDataSetCode() const { return EXTERNAL_DATASET_CODE; }
		virtual void setTileWidth(int nWidth);
		virtual void setTileHeight(int nHeight);
		virtual void setImageFormat(std::string strImgFormat);

		//bool queryTileData(DEULayerInfo& layerInfo,const unsigned& nRow,const unsigned& nCol,DEUTileInfo& tileInfo,int& nError) const;
	private:
		unsigned getLevel(double dScale,double& dOutScale) const;
		bool     getTileInfo(const ID& id,DEUTileInfo& tInfo) const;
	private:
		std::string					m_strImageFormat;
		int							m_nTileWidth;
		int							m_nTileHeight;
		std::string					m_strUrl;
		std::string					m_strVersion;
		unsigned __int64			m_nUniqueFlag;
		ID							m_topID;
		std::map<std::string, DEULayerInfo> m_pLayerSizeMap;
		std::map<std::string, std::string> m_strLayerMap;
		std::string					m_strCRS;
	};
}

