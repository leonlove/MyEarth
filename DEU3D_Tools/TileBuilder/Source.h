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
    int nSourceX;       //�ཻ���������Դͼ�����ص�xƫ��
    int nSourceY;       //�ཻ���������Դͼ�����ص�yƫ��
    int nOffsetX;       //�ཻ��������Դͼ�е����ش�С
    int nOffsetY;       //�ཻ��������Դͼ�е����ش�С
    int nTargetX;       //�ཻ���������Ŀ����Ƭ�е����ص�xƫ��
    int nTargetY;       //�ཻ���������Ŀ����Ƭ�е����ص�yƫ��
    int nTargetOffsetX; //�ཻ������Ŀ����Ƭ�е����ش�С
    int nTargetOffsetY; //�ཻ������Ŀ����Ƭ�е����ش�С
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
    unsigned int        m_nMinLevel;   //�û�ѡ����С�㼶
    unsigned int        m_nMaxLevel;   //�û�ѡ�����㼶
    unsigned int        m_nMinInterval;//�Զ�����ɴﵽ��С�㼶
    unsigned int        m_nMaxInterval;//�Զ�����ɴﵽ���㼶
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
