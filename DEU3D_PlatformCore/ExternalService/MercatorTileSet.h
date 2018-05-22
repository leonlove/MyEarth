#pragma once
#include "ITileSet.h"
#include <common/DEUBson.h>
#include <DEUDBProxy/IDEUDBProxy.h>
namespace deues
{
	class MercatorTileSet :	public ITileSet
	{
	public:
		MercatorTileSet(void);
		~MercatorTileSet(void);
	public:
		void initialize(const std::string& strUrl,const std::string& strService, const std::string&  strMapStyle, const std::string&  strDBPath);
        void unInitialize(void);
		virtual bool getTopInfo(void*& pBuffer,unsigned int& nLength) const;
		virtual bool queryData(const ID& id,void*& pBuffer,unsigned int& nLength,int& nError) const;
		virtual const ID&  getID();
		virtual unsigned __int64 getUniqueFlag() const;
		virtual unsigned getDataSetCode() const { return EXTERNAL_DATASET_CODE; }
	private:
// 		unsigned getLevel(double dScale,double& dOutScale) const;
// 		bool getTileInfo(const ID& id,DEUTileInfo& tInfo) const;
        bool OpenDB(const std::string& strDBPath);

	private:
 		std::string					m_strUrl;
		std::string					m_strService;
		std::string                 m_strMapStyle;
		ID							m_topID;
        OpenSP::sp<deudbProxy::IDEUDBProxy>  m_pDBProxy;
	};
}


