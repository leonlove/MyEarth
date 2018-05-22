#include "WMSTileSet.h"
#include <common/Common.h>
#include <common/Pyramid.h>
#include <common/DEUBson.h>
#include "CSimpleHttpClient.h"
#include <sstream>
#include "DEUUtils.h"

namespace deues
{
	WMSTileSet::WMSTileSet(void)
	{
		m_nTileWidth = 256;
		m_nTileHeight = 256;
		m_strImageFormat = "image/png";
	}

	WMSTileSet::~WMSTileSet(void)
	{
	}

	void WMSTileSet::initialize(std::map<std::string, DEULayerInfo>& layerSizeMap, std::string strURL, std::string strVersion, std::map<std::string,std::string> strLayerMap, std::string strCRS)
	{
		m_nUniqueFlag = cmm::genGlobeUniqueNumber();
		m_topID = ID::genNewID();
		m_topID.ObjectID.m_nDataSetCode = 6;
		m_topID.ObjectID.m_nType = TERRAIN_DOM_ID;
		m_pLayerSizeMap = layerSizeMap;
		m_strUrl = strURL;
		m_strVersion = strVersion;
		m_strLayerMap = strLayerMap;
		m_strCRS = strCRS;
	}

	bool WMSTileSet::getTopInfo(void*& pBuffer,unsigned int& nLength) const
	{
		//取需要创建图层的范围的并集作为顶层瓦片的大小
		double dXMin=180.0,dYMin=90.0,dXMax=-180.0,dYMax=-90.0;

		std::map<std::string,std::string>::const_iterator pItr = m_strLayerMap.cbegin();
		while(pItr != m_strLayerMap.cend())
		{
			std::string strLayerName = pItr->first;
			std::map<std::string, DEULayerInfo>::const_iterator pos = m_pLayerSizeMap.find(strLayerName);
			if(pos != m_pLayerSizeMap.end())
			{
				DEULayerInfo layerInfo = pos->second;

				if(layerInfo.m_dMinX < dXMin)
					dXMin = layerInfo.m_dMinX;
				if(layerInfo.m_dMinY < dYMin)
					dYMin = layerInfo.m_dMinY;

				if(layerInfo.m_dMaxX > dXMax)
					dXMax = layerInfo.m_dMaxX;
				if(layerInfo.m_dMaxY > dYMax)
					dYMax = layerInfo.m_dMaxY;
			}
			pItr++;
		}

		//和标准瓦片的大小做对比
		cmm::Pyramid pyd;
		unsigned nTopLevel = 0;
		unsigned nBottomLevel = 32;
		for(unsigned n = 0; n <= 32; n++)
		{
			if((dXMax-dXMin) > pyd.getLevelTileSize(n).x()*180.0/cmm::math::PI)
			{
				nTopLevel = n;
				break;
			}
		}

		for(unsigned n = 0; n <= 32; n++)
		{
			if(((dXMax - dXMin)/256.0) > (pyd.getLevelTileSize(n).x()*180.0/cmm::math::PI))
			{
				nBottomLevel = n;
				break;
			}
		}

		unsigned nFromRow,nFromCol,nToRow,nToCol;
		pyd.getTile(nTopLevel,dXMin*cmm::math::PI/180.0,dYMin*cmm::math::PI/180.0,dXMax*cmm::math::PI/180.0,dYMax*cmm::math::PI/180.0,nFromRow,nFromCol,nToRow,nToCol);

		bson::bsonDocument bDoc;
		bDoc.AddStringElement("ID",m_topID.toString().c_str());
		bDoc.AddStringElement("Name","外部影像");

		bson::bsonArrayEle* pBSElem = (bson::bsonArrayEle*)bDoc.AddArrayElement("BoundingSphere");
		pBSElem->AddDblElement((dXMin + dXMax)*0.5);
		pBSElem->AddDblElement((dYMin + dYMax)*0.5);
		pBSElem->AddDblElement(0.0);
		pBSElem->AddDblElement(dXMax - dXMin);

		bson::bsonArrayEle* pChildArrayElem = (bson::bsonArrayEle*)bDoc.AddArrayElement("ChildrenID");

		for(unsigned nr = nFromRow;nr <= nToRow;nr++)
		{
			for(unsigned nc = nFromCol;nc <= nToCol;nc++)
			{
				ID childID(0ui64, 0ui64, 0ui64);
				childID.TileID.m_nDataSetCode = EXTERNAL_DATASET_CODE;
				childID.TileID.m_nLevel = nTopLevel;
				childID.TileID.m_nRow = nr;
				childID.TileID.m_nCol = nc;
				childID.TileID.m_nUniqueID = m_nUniqueFlag;
				childID.TileID.m_nType = TERRAIN_TILE_IMAGE;

				bson::bsonDocumentEle* pChildDocElem = (bson::bsonDocumentEle*)pChildArrayElem->AddDocumentElement();
				bson::bsonDocument& bChildDoc = pChildDocElem->GetDoc();
				bChildDoc.AddInt32Element(childID.toString().c_str(),nBottomLevel);
			}
		}

		bson::bsonStream bss;
		bDoc.Write(&bss);
		nLength = bss.DataLen();
		pBuffer = malloc(nLength);
		memcpy(pBuffer,bss.Data(),nLength);
		return true;
	}

	bool WMSTileSet::queryData(const ID& id,void*& pBuffer,unsigned int& nLength,int& nError) const
	{
		std::string strLayerNames;
		std::string strStyle;
		cmm::Pyramid pyd;
		double dMinX = 0.0;
		double dMinY = 0.0;
		double dMaxX = 0.0;
		double dMaxY = 0.0;
		pyd.getTilePos(id.TileID.m_nLevel,id.TileID.m_nRow,id.TileID.m_nCol,dMinX,dMinY,dMaxX,dMaxY);
		dMinX = dMinX*180.0/cmm::math::PI;
		dMinY = dMinY*180.0/cmm::math::PI;
		dMaxX = dMaxX*180.0/cmm::math::PI;
		dMaxY = dMaxY*180.0/cmm::math::PI;
		std::map<std::string,std::string>::const_iterator it = m_strLayerMap.begin();
		while(it != m_strLayerMap.end())
		{
			strLayerNames += it->first;
			strStyle += it->second;
			it++;
			if (it != m_strLayerMap.end())
			{
				strLayerNames += ",";
				strStyle += ",";
			}		
		}

		SimpleHttpClient shc;
		std::ostringstream oss;

		if(m_strVersion == "1.1.1")
		{
			oss<<m_strUrl<<"?service=WMS&version="<<m_strVersion<<"&request=GetMap&TRANSPARENT=TRUE&SRS="<<m_strCRS
				<<"&BBOX="<<dMinX<<","<<dMinY<<","<<dMaxX<<","<<dMaxY<<"&layers="<<strLayerNames<<"&styles="
				<<strStyle<<"&width="<<m_nTileWidth<<"&height="<<m_nTileHeight<<"&format="<<m_strImageFormat;
		}
		else
		{
			oss<<m_strUrl<<"?service=WMS&version="<<m_strVersion<<"&request=GetMap&TRANSPARENT=TRUE&CRS="<<m_strCRS
				<<"&BBOX="<<dMinX<<","<<dMinY<<","<<dMaxX<<","<<dMaxY<<"&layers="<<strLayerNames<<"&styles="
				<<strStyle<<"&width="<<m_nTileWidth<<"&height="<<m_nTileHeight<<"&format="<<m_strImageFormat;
		}
		

		char* szRespBuf = NULL;
		long nResplen = 0;
		std::string strUrl = DEUUtils::urlEncode(oss.str());
		int nRet = shc.Request(GetMethod, strUrl.c_str(), &szRespBuf, &nResplen);
		//return
		if(nRet == 0 || szRespBuf != NULL)
		{
			nLength = nResplen;
			pBuffer = (char*)malloc(nResplen);
			memcpy(pBuffer,szRespBuf,nResplen);
			shc.FreeResponse(szRespBuf);
			return true;
		}
		else
		{
			nError = nRet;
			return false;
		}
	}

	void WMSTileSet::setTileHeight(int nHeight)
	{
		m_nTileHeight = nHeight;
	}

	void WMSTileSet::setTileWidth(int nWidth)
	{
		m_nTileWidth = nWidth;
	}

	void WMSTileSet::setImageFormat(std::string strImgFormat)
	{
		m_strImageFormat = strImgFormat;
	}
}