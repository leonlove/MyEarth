#ifndef _SOURCE_H
#define _SOURCE_H

#include <string>
#include "OpenSP/Ref.h"
#include "Common/deuMath.h"
#include "gdal_priv.h"

#include <vector>

class TileBuilderImpl;

typedef struct OffsetData
{
    int nSourceX;       //相交区域相对于源图中像素的x偏移
    int nSourceY;       //相交区域相对于源图中像素的y偏移
    int nOffsetX;       //相交区域相在源图中的像素大小
    int nOffsetY;       //相交区域相在源图中的像素大小
    int nTargetX;       //相交区域相对于目标切片中的像素的x偏移
    int nTargetY;       //相交区域相对于目标切片中的像素的y偏移
    int nTargetOffsetX; //相交区域在目标切片中的像素大小
    int nTargetOffsetY; //相交区域在目标切片中的像素大小
}OFFSETDATA;

class Source : public OpenSP::Ref
{
public:
    Source(void);
    ~Source(void);

public:
    bool create(const std::string &strFileName, const bool bHeightField, TileBuilderImpl* pTileBuilderImpl);
    bool setLevel(const unsigned int nMinLevel, const unsigned int nMaxLevel);
    bool readTileForImage(const cmm::math::Box2d &box2d, const unsigned int nTileSize, std::vector<char*> &RasterDataVec);
    bool readTileForHeightField(const cmm::math::Box2d &box2d, const unsigned int nTileSize, float** pRasterData);
    bool isAreaInBoud(const cmm::math::Box2d &box2d) const;

    unsigned int              getMinInterval() const;
    unsigned int              getMaxInterval() const;
    const cmm::math::Box2d&   getBound() const;
    const cmm::math::Point2d& getPrecision() const;

    unsigned int computeMinInterval();
    unsigned int computeMaxInterval();

    bool getFileCoordinates(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY);
    bool setFileCoordinates(double dMinX, double dMinY, double dMaxX, double dMaxY);
    std::string getSourceFileName(){ return m_strSourceFile; }

	typedef struct vect{
		float value;
		int index;
		int row;
		int col;
	}vect;

private:
    bool computeOffsetData(const cmm::math::Box2d &box2d, const unsigned int nTileSize, OFFSETDATA &offsetData);
    bool computeStretchRange();

private:
    unsigned int        m_nMinLevel;   //用户选择最小层级
    unsigned int        m_nMaxLevel;   //用户选择最大层级
    unsigned int        m_nMinInterval;//自动计算可达到最小层级
    unsigned int        m_nMaxInterval;//自动计算可达到最大层级
    GDALDataset*        m_pGDALDataSet;
    GDALDataType        m_GDALDataType;
    std::string         m_strSourceFile;
    cmm::math::Box2d    m_AreaBound;
    cmm::math::Point2d  m_vecPrecision;
    OpenThreads::Mutex  m_mtxRasterIO;
    TileBuilderImpl*    m_pTileBuilderImpl;

	double* m_pMetadata;

    char* m_RangeMin;
    char* m_RangeMax;
};

#endif
