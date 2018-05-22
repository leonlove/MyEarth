#ifndef _TILESET_H_1442128D_6AEC_4A7D_9BED_661AE1EB8C8A_
#define _TILESET_H_1442128D_6AEC_4A7D_9BED_661AE1EB8C8A_

#include "ITileSet.h"
#include <IDProvider/ID.h>
#include "DEUDefine.h"

namespace deues
{
    class TileSet : public ITileSet
    {
    public:
        TileSet(void);
        ~TileSet(void);
    public:
        bool initialize(const char* pXML,const std::string& strUrl,const std::string& strVersion);
        virtual bool getTopInfo(void*& pBuffer,unsigned int& nLength) const;
        virtual bool queryData(const ID& id,void*& pBuffer,unsigned int& nLength,int& nError) const;
        virtual const ID&  getID() { return m_topID; }
        virtual unsigned __int64 getUniqueFlag() const { return m_nUniqueFlag; }
         virtual unsigned getDataSetCode() const { return EXTERNAL_DATASET_CODE; }
    private:
        bool                  m_bInit;
        std::string           m_strUrl;
        std::string           m_strVersion;
        unsigned __int64      m_nUniqueFlag;
        ID                    m_topID;
        DEUMetaData           m_metaData;
    private:
        unsigned getLevel(double dScale,double& dOutScale) const;
        bool     getTileInfo(const ID& id,DEUTileInfo& tInfo) const;
        bool     calcTileRange(const DEUTileInfo& srcTileInfo,DEUMatrixInfo& mInfo,unsigned& nFromRow,unsigned& nToRow,
                               unsigned& nFromCol,unsigned& nToCol,bool& bMerge,int& nError) const;
        bool     queryTileData(DEUMatrixInfo mInfo,const unsigned& nRow,const unsigned& nCol,DEUTileInfo& tileInfo,int& nError) const;
        bool     jointTiles(DEUTileInfo srcInfo,const std::vector<DEUTileInfo>& tInfoVec,void*& pBuffer,unsigned& nLength) const;
    };

}

#endif

