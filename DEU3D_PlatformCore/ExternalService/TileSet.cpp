#include "TileSet.h"
#include <common/Common.h>
#include <common/Pyramid.h>
#include <common/DEUBson.h>
#include "DEUUtils.h"
#include "CSimpleHttpClient.h"
#include <sstream>
#include <IDProvider/Definer.h>
#include <osgDB/ReadFile>
#include <common/deuImage.h>

namespace deues
{
    TileSet::TileSet(void)
    {
        m_bInit = false;
    }


    TileSet::~TileSet(void)
    {
    }
    //初始化
    bool TileSet::initialize(const char* pXML,const std::string& strUrl,const std::string& strVersion)
    {
        //get metadata
        int nError = false;
        if(!DEUUtils::getWMTSMetaInfo(pXML,m_metaData,nError))
        {
            return false;
        }
        m_strUrl  = strUrl;
        m_strVersion = strVersion;
        m_nUniqueFlag = cmm::genGlobeUniqueNumber();
        m_topID = ID::genNewID();
        m_topID.ObjectID.m_nDataSetCode = 6;
        m_topID.ObjectID.m_nType = TERRAIN_DOM_ID;
        m_bInit = true;
        return true;
    }
    //获取顶层瓦片信息
    bool TileSet::getTopInfo(void*& pBuffer,unsigned int& nLength) const
    {
        //获取最大、最小比例尺
        double dBottomScale = m_metaData.m_matrixMap.begin()->first;
        double dTopScale    = m_metaData.m_matrixMap.rbegin()->first;

        double dMinScale = 0.0, dMaxScale = 0.0;
        unsigned nMinLevel = getLevel(dTopScale,dMinScale);
        unsigned nMaxLevel = getLevel(dBottomScale,dMaxScale);
        
        double dLeftX = -180.0,dLeftY = -90.0;
        double dEps = 1e-6;

        unsigned nFromCol = floor(abs(m_metaData.m_dMinX - dLeftX)/(dMinScale*256) + dEps);
        unsigned nToCol   = floor(abs(m_metaData.m_dMaxX - dLeftX)/(dMinScale*256) - dEps);

        unsigned nFromRow = floor(abs(m_metaData.m_dMinY - dLeftY)/(dMinScale*256) + dEps);
        unsigned nToRow   = floor(abs(m_metaData.m_dMaxY - dLeftY)/(dMinScale*256) - dEps);

        bson::bsonDocument bDoc;
        bDoc.AddStringElement("ID",m_topID.toString().c_str());
        bDoc.AddStringElement("Name","外部影像");
        
        bson::bsonArrayEle* pBSElem = (bson::bsonArrayEle*)bDoc.AddArrayElement("BoundingSphere");
        pBSElem->AddDblElement((m_metaData.m_dMinX + m_metaData.m_dMaxX)*0.5);
        pBSElem->AddDblElement((m_metaData.m_dMinY + m_metaData.m_dMaxY)*0.5);
        pBSElem->AddDblElement(0.0);
        pBSElem->AddDblElement(m_metaData.m_dMaxX - m_metaData.m_dMinX);

        bson::bsonArrayEle*    pChildArrayElem = (bson::bsonArrayEle*)bDoc.AddArrayElement("ChildrenID");

        for(unsigned nr = nFromRow;nr <= nToRow;nr++)
        {
            for(unsigned nc = nFromCol;nc <= nToCol;nc++)
            {
                ID childID(0ui64, 0ui64, 0ui64);
                childID.TileID.m_nDataSetCode = EXTERNAL_DATASET_CODE;
                childID.TileID.m_nLevel = nMinLevel;
                childID.TileID.m_nRow = nr;
                childID.TileID.m_nCol = nc;
                childID.TileID.m_nUniqueID = m_nUniqueFlag;
                childID.TileID.m_nType = TERRAIN_TILE_IMAGE;

                bson::bsonDocumentEle* pChildDocElem = (bson::bsonDocumentEle*)pChildArrayElem->AddDocumentElement();
                bson::bsonDocument& bChildDoc = pChildDocElem->GetDoc();
                bChildDoc.AddInt32Element(childID.toString().c_str(),nMaxLevel);
            }
        }

        bson::bsonStream bss;
        bDoc.Write(&bss);
        nLength = bss.DataLen();
        pBuffer = malloc(nLength);
        memcpy(pBuffer,bss.Data(),nLength);
        return true;
    }
    unsigned TileSet::getLevel(double dScale,double& dOutScale) const
    {
        cmm::Pyramid pyd;
        for(unsigned n = 1;n < 31;n++)
        {
            double dSize1 = pyd.getLevelTileSize(n).x()*180.0/cmm::math::PI/256.0;
            double dSize2 = pyd.getLevelTileSize(n+1).x()*180.0/cmm::math::PI/256.0;
            if(dScale > dSize1)
            {
                dOutScale = dSize1;
                return n;
            }
            else if(dSize1 > dScale && dScale > dSize2)
            {
                dOutScale = dSize2;
                return n+1;
            }
            else if(dScale < dSize2)
            {
                continue;
            }
        }
        return 32;
    }
    //获取数据
    bool TileSet::queryData(const ID& id,void*& pBuffer,unsigned int& nLength,int& nError) const
    {
        //1. 获取源瓦片范围
        DEUTileInfo tInfo;
        if(!getTileInfo(id,tInfo))
        {
            return false;
        }
        //2. 计算目标瓦片范围
        unsigned nFromRow = 0,nToRow = 0,nFromCol = 0,nToCol = 0;
        DEUMatrixInfo matrix;
        bool bMerge = true;
        if(!calcTileRange(tInfo,matrix,nFromRow,nToRow,nFromCol,nToCol,bMerge,nError))
        {
            return false;
        }
        //3. 请求瓦片
        std::vector<DEUTileInfo> tInfoVec;
        for(unsigned nRow = nFromRow;nRow <= nToRow;nRow++)
        {
            for(unsigned nCol = nFromCol;nCol <= nToCol;nCol++)
            {
                DEUTileInfo tInfo;
                if(!queryTileData(matrix,nRow,nCol,tInfo,nError))
                {
                    continue;
                }
                tInfo.m_dMinX = matrix.m_dTopLeftX + matrix.m_dScale*matrix.m_nCol*nCol;
                tInfo.m_dMaxX = matrix.m_dTopLeftX + (nCol+1)*matrix.m_dScale*matrix.m_nCol;
                tInfo.m_dMaxY = matrix.m_dTopLeftY - nRow*matrix.m_nRow*matrix.m_dScale;
                tInfo.m_dMinY = matrix.m_dTopLeftY - (nRow+1)*matrix.m_nRow*matrix.m_dScale;
                tInfo.m_nCol = 256;
                tInfo.m_nRow = 256;

                tInfoVec.push_back(tInfo);
            }
        }
        //4. 如果请求瓦片失败，返回
        if(tInfoVec.empty())
        {
            return false;
        }
        //5. 融合瓦片
        bool bRes = false;
        if(bMerge)
        {
            bRes = jointTiles(tInfo,tInfoVec,pBuffer,nLength);
        }
        else
        {
            nLength = tInfoVec[0].m_nLength;
            pBuffer = malloc(nLength);
            memcpy(pBuffer,tInfoVec[0].m_pData,nLength);
            bRes = true;
        }
        for(unsigned i = 0;i < tInfoVec.size();i++)
        {
            delete tInfoVec[i].m_pData;
        }
        return bRes;
    }

    bool TileSet::jointTiles(DEUTileInfo srcInfo,const std::vector<DEUTileInfo>& tInfoVec,void*& pBuffer,unsigned& nLength) const
    {
       OpenSP::sp<cmm::image::IDEUImage> pTargetImage = cmm::image::createDEUImage();
       for(unsigned n = 0;n < tInfoVec.size();n++)
       {
           DEUTileInfo tInfo = tInfoVec[n];
           OpenSP::sp<cmm::image::IDEUImage> pImage =  osgDB::parseImageFromStream(tInfo.m_pData,tInfo.m_nLength);
           if(!pImage->isValid())
           {
                continue;
           }

           //pImage->saveToFile("G:\\wmts\\wmts2.png");

           double dMinX = srcInfo.m_dMinX > tInfo.m_dMinX ? srcInfo.m_dMinX : tInfo.m_dMinX;
           double dMinY = srcInfo.m_dMinY > tInfo.m_dMinY ? srcInfo.m_dMinY : tInfo.m_dMinY;
           double dMaxX = srcInfo.m_dMaxX < tInfo.m_dMaxX ? srcInfo.m_dMaxX : tInfo.m_dMaxX;
           double dMaxY = srcInfo.m_dMaxY < tInfo.m_dMaxY ? srcInfo.m_dMaxY : tInfo.m_dMaxY;

           unsigned nSrcWidth =  (dMinX - tInfo.m_dMinX)*tInfo.m_nCol/(tInfo.m_dMaxX - tInfo.m_dMinX);
           unsigned nSrcHeight = (dMinY - tInfo.m_dMinY)*tInfo.m_nRow/(tInfo.m_dMaxY - tInfo.m_dMinY);
           unsigned nDesWidth =  (dMinX - srcInfo.m_dMinX)*srcInfo.m_nCol/(srcInfo.m_dMaxX - srcInfo.m_dMinX);
           unsigned nDesHeight = (dMinY - srcInfo.m_dMinY)*srcInfo.m_nRow/(srcInfo.m_dMaxY - srcInfo.m_dMinY);

           unsigned nRangeWidth =  (dMaxX - dMinX)*tInfo.m_nCol/(tInfo.m_dMaxX - tInfo.m_dMinX);
           unsigned nRangeHeight = (dMaxY - dMinY)*tInfo.m_nRow/(tInfo.m_dMaxY - tInfo.m_dMinY);

           if(n == 0)
           {
                if(!pTargetImage->allocImage(srcInfo.m_nCol,srcInfo.m_nRow,pImage->getPixelFormat()))
                {
                    return false;
                }
           }
           
           pTargetImage->jointImage(pImage,nSrcWidth,nSrcHeight,nDesWidth,nDesHeight,nRangeWidth,nRangeHeight);
       }
       //pTargetImage->saveToFile("G:\\wmts\\wmts.png");
       const void* pData = pTargetImage->data();
       nLength = strlen((char*)pData);
       pBuffer = malloc(nLength);
       memcpy(pBuffer,pData,nLength);
       return true;
    }

    //计算瓦片范围
    bool TileSet::getTileInfo(const ID& id,DEUTileInfo& tInfo) const
    {
        const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
        double dXMin = 0.0,dYMin = 0.0,dXMax = 0.0,dYMax = 0.0;
        if(!pPyramid->getTilePos(id.TileID.m_nLevel, id.TileID.m_nRow, id.TileID.m_nCol, dXMin, dYMin, dXMax, dYMax))
        {
            return false;
        }
        
        tInfo.m_dMinX = dXMin*180.0/cmm::math::PI;
        tInfo.m_dMaxX = dXMax*180.0/cmm::math::PI;
        tInfo.m_dMinY = dYMin*180.0/cmm::math::PI;
        tInfo.m_dMaxY = dYMax*180.0/cmm::math::PI;
        tInfo.m_nLevel = id.TileID.m_nLevel;
        tInfo.m_nCurRow = id.TileID.m_nRow;
        tInfo.m_nCurCol = id.TileID.m_nCol;
        tInfo.m_nRow = 256;
        tInfo.m_nCol = 256;

        return true;
    }
    
    bool TileSet::calcTileRange(const DEUTileInfo& srcTileInfo,DEUMatrixInfo& mInfo,unsigned& nFromRow,unsigned& nToRow,
        unsigned& nFromCol,unsigned& nToCol,bool& bMerge,int& nError) const
    {
        //1. 如果请求的瓦片范围和数据范围没有交集，返回

        if(srcTileInfo.m_dMaxX < m_metaData.m_dMinX ||
            srcTileInfo.m_dMinX > m_metaData.m_dMaxX ||
            srcTileInfo.m_dMaxY < m_metaData.m_dMinY ||
            srcTileInfo.m_dMinY > m_metaData.m_dMaxY)
        {
            return false;
        }
            
        double dXMin = srcTileInfo.m_dMinX > m_metaData.m_dMinX ? srcTileInfo.m_dMinX : m_metaData.m_dMinX;
        double dXMax = srcTileInfo.m_dMaxX < m_metaData.m_dMaxX ? srcTileInfo.m_dMaxX : m_metaData.m_dMaxX;
        double dYMin = srcTileInfo.m_dMinY > m_metaData.m_dMinY ? srcTileInfo.m_dMinY : m_metaData.m_dMinY;
        double dYMax = srcTileInfo.m_dMaxY < m_metaData.m_dMaxY ? srcTileInfo.m_dMaxY : m_metaData.m_dMaxY;

        double dSrcRes = (srcTileInfo.m_dMaxX-srcTileInfo.m_dMinX) / (srcTileInfo.m_nCol*1.0);
        //2. 获取比例尺和层
        std::map<double,DEUMatrixInfo> matrixMap = m_metaData.m_matrixMap;
        int nLength = matrixMap.size();
        if(nLength < 1)
        {
            return false;
        }
        else if(nLength == 1)
        {
            mInfo = matrixMap.begin()->second;
        }
        else
        {
            std::map<double,DEUMatrixInfo>::const_reverse_iterator pItr = matrixMap.crbegin();
            while(pItr != matrixMap.crend())
            {
                double dRes1 = pItr->first;
                DEUMatrixInfo m1 = pItr->second;
                pItr++;
                if(pItr == matrixMap.crend())
                {
                    break;
                }
                double dRes2 = pItr->first;
                DEUMatrixInfo m2 = pItr->second;
                if(dRes1 < dSrcRes && dRes2 < dSrcRes)
                {
                    mInfo = m1;
                    break;
                }
                else if(dRes1 > dSrcRes && dRes2 > dSrcRes)
                {
                    mInfo = m2;
                }
                else if(dRes1 > dSrcRes && dSrcRes > dRes2)
                {
                    if((dRes1 - dSrcRes) < (dRes1 - dRes2)*0.5)
                    {
                        mInfo = m1;
                    }
                    else
                    {
                        mInfo = m2;
                    }
                    break;
                }
            }
        }
        double dTemp = mInfo.m_dScale / 10.0;
        if(abs(mInfo.m_dScale - dSrcRes) <= dTemp)
        {
            nFromCol = nToCol = srcTileInfo.m_nCurCol;
            nFromRow = nToRow = mInfo.m_nHeight - srcTileInfo.m_nCurRow - 1;
            bMerge = false;
            return true;
        }
        bMerge = true;
        //3. 获取行、列号
        double dEps = 1e-6;
        nFromCol = floor(abs(dXMin - mInfo.m_dTopLeftX)/(mInfo.m_dScale*mInfo.m_nCol) + dEps);
        nToCol = ceil(abs(dXMax - mInfo.m_dTopLeftX)/(mInfo.m_dScale*mInfo.m_nCol) - dEps);

        nFromRow = floor(abs(mInfo.m_dTopLeftY - dYMax)/(mInfo.m_dScale*mInfo.m_nRow) + dEps);
        nToRow = ceil(abs(mInfo.m_dTopLeftY -dYMin)/(mInfo.m_dScale*mInfo.m_nRow) - dEps);

        return true;

    }

    //http://t0.tianditu.com/vec_c/wmts?service=WMTS&request=GetTile&version=1.0.0
    //&layer=vec&style=default&format=tiles&TileMatrixSet=c&TileMatrix=1&TileRow=0&TileCol=0
    bool TileSet::queryTileData(DEUMatrixInfo mInfo,const unsigned& nRow,const unsigned& nCol,DEUTileInfo& tileInfo,int& nError) const
    {
        //variables
        char* szRespBuf = NULL;
        long nResplen = 0;
        //request
        SimpleHttpClient shc;
        std::ostringstream oss;
        oss<<m_strUrl<<"?service=WMTS&request=GetTile&version="<<m_strVersion<<"&layer="<<m_metaData.m_strLayer
        <<"&style="<<m_metaData.m_strStyle<<"&format="<<m_metaData.m_strFormat<<"&TileMatrixSet="<<m_metaData.m_strMatrixSet
        <<"&TileMatrix="<<mInfo.m_strMatrix<<"&TileRow="<<nRow<<"&TileCol="<<nCol;

        int nRet = shc.Request(GetMethod, oss.str().c_str(), &szRespBuf, &nResplen);
        //return
        if(nRet == 0 || szRespBuf != NULL)
        {
            char* chData = (char*)malloc(nResplen);
            memcpy(chData,szRespBuf,nResplen);
            tileInfo.m_nLength = nResplen;
            tileInfo.m_pData = chData;
            shc.FreeResponse(szRespBuf);
            return true;
        }
        else
        {
            nError = nRet;
            return false;
        }
    }
}

