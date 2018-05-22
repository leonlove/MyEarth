
#include "Source.h"
#include "Common/Pyramid.h"
#include "TileBuilderImpl.h"
#include <fstream>

template<typename T>
inline T minimum(T lhs,T rhs) { return lhs<rhs?lhs:rhs; }

template<typename T>
inline T maximum(T lhs,T rhs) { return lhs>rhs?lhs:rhs; }

template<typename T>
inline T clampTo(T v,T minimum,T maximum)
{ return v<minimum?minimum:v>maximum?maximum:v; }

template<typename T>
inline T clampAbove(T v,T minimum) { return v<minimum?minimum:v; }

template<typename T>
inline T clampBelow(T v,T maximum) { return v>maximum?maximum:v; }

template<typename T>
inline T clampBetween(T v,T minimum, T maximum)
{ return clampBelow(clampAbove(v,minimum),maximum); }

Source::Source(void)
: m_pGDALDataSet(NULL)
, m_nMinLevel(1)
, m_nMaxLevel(31)
, m_nMinInterval(1)
, m_nMaxInterval(31)
, m_pTileBuilderImpl(NULL)
, m_GDALDataType(GDT_Unknown)
, m_RangeMin(NULL)
, m_RangeMax(NULL)
{
    m_strSourceFile = "";
    m_RangeMin = new char[8];
    m_RangeMax = new char[8];
    memset(m_RangeMin, 1, 8);
    memset(m_RangeMax, 0, 8);

	m_pMetadata = new double[2];
	memset(m_pMetadata, -1000000.0,sizeof(double)*2);
}

Source::~Source(void)
{
    if (m_pGDALDataSet != NULL)
    {
        GDALClose(m_pGDALDataSet);
        m_pGDALDataSet = NULL;
    }

    delete [] m_RangeMin;
	delete [] m_RangeMax;
	delete [] m_pMetadata;
}

bool Source::computeStretchRange()
{
    int  nBland = m_pGDALDataSet->GetRasterCount();
    bool hasRGB = nBland >= 3;

    if (hasRGB)
    {
        //取出像素最大最小范围，以供线性拉伸成byte使用
        //首先需要过滤掉源图中的错误值，过滤算法需要研究，在此暂时挂起，等待补充
        if (m_GDALDataType != GDT_Byte)
        {
            for(int i = 1; i <= nBland; i++)
            {
                GDALRasterBand* pBand = m_pGDALDataSet->GetRasterBand(i);
                if (pBand == NULL)
                {
                    return false;
                }

                const int nMaxXSize = m_pGDALDataSet->GetRasterXSize();
                const int nMaxYSize = m_pGDALDataSet->GetRasterYSize();

                for (int m = 0; m < nMaxYSize; m++)
                {
                    switch (m_GDALDataType)
                    {
                    case GDT_Unknown:
                        return false;
                        break;
                    case GDT_Byte:
                        break;
                    case GDT_UInt16:
                        {
                            unsigned short* pafScanline  = new unsigned short[nMaxXSize];

                            m_mtxRasterIO.lock();
                            pBand->RasterIO(GF_Read, 0, m, nMaxXSize, 1, pafScanline, nMaxXSize, 1, GDT_UInt16, 0, 0);
                            m_mtxRasterIO.unlock();

                            unsigned short fMin = pafScanline[0];
                            unsigned short fMax = pafScanline[0];
                            for (int n = 1; n < nMaxXSize; n++)
                            {
                                fMin = fMin < pafScanline[n] ? fMin : pafScanline[n];
                                fMax = fMax > pafScanline[n] ? fMax : pafScanline[n];
                            }

                            if (*(unsigned short*)m_RangeMin > fMin)
                            {
                                memcpy_s(m_RangeMin, sizeof(unsigned short), &fMin, sizeof(unsigned short));
                            }
                            if (*(unsigned short*)m_RangeMax < fMax)
                            {
                                memcpy_s(m_RangeMax, sizeof(unsigned short), &fMax, sizeof(unsigned short));
                            }

                            delete [] pafScanline;
                            pafScanline = NULL;
                        }
                        break;
                    case GDT_Int16:
                        break;
                    case GDT_UInt32:
                        {
                            unsigned int* pafScanline  = new unsigned int[nMaxXSize];

                            m_mtxRasterIO.lock();
                            pBand->RasterIO(GF_Read, 0, m, nMaxXSize, 1, pafScanline, nMaxXSize, 1, GDT_UInt32, 0, 0);
                            m_mtxRasterIO.unlock();

                            unsigned int fMin = pafScanline[0];
                            unsigned int fMax = pafScanline[0];
                            for (int n = 1; n < nMaxXSize; n++)
                            {
                                fMin = fMin < pafScanline[n] ? fMin : pafScanline[n];
                                fMax = fMax > pafScanline[n] ? fMax : pafScanline[n];
                            }

                            if (*(unsigned int*)m_RangeMin > fMin)
                            {
                                memcpy_s(m_RangeMin, sizeof(unsigned int), &fMin, sizeof(unsigned int));
                            }
                            if (*(unsigned int*)m_RangeMax < fMax)
                            {
                                memcpy_s(m_RangeMax, sizeof(unsigned int), &fMax, sizeof(unsigned int));
                            }

                            delete [] pafScanline;
                            pafScanline = NULL;
                        }
                        break;
                    case GDT_Int32:
                    case GDT_Float32:
                    case GDT_Float64:
                    case GDT_CInt16:
                    case GDT_CInt32:
                    case GDT_CFloat32:
                    case GDT_CFloat64:
                        return false;
                        break;
                    }
                }
            }
        }
    }

    return true;
}

bool Source::create(const std::string &strFileName, const bool bHeightField, TileBuilderImpl* pTileBuilderImpl)
{
    if (pTileBuilderImpl == NULL)
    {
        return false;
    }

    m_strSourceFile    = strFileName;
    m_pTileBuilderImpl = pTileBuilderImpl;

    m_pGDALDataSet = (GDALDataset*)GDALOpen(m_strSourceFile.c_str(), GA_ReadOnly);
    if (m_pGDALDataSet == NULL)
    {
        return false;
    }

//     if (bHeightField && (m_pGDALDataSet->GetRasterCount() > 1 || m_pGDALDataSet->GetRasterBand(1)->GetColorTable()))
//     {
//         GDALClose(m_pGDALDataSet);
//         m_pGDALDataSet = NULL;
//         return false;
//     }

    int nXSize = m_pGDALDataSet->GetRasterXSize();
    int nYSize = m_pGDALDataSet->GetRasterYSize();

    m_GDALDataType = m_pGDALDataSet->GetRasterBand(1)->GetRasterDataType();

    //(xMin, yMin) (xMax, yMax)
    cmm::math::Point2d ptMin(-180.0, -90.0);
    cmm::math::Point2d ptMax(180.0, 90.0);

    std::string strType = strFileName.substr(strFileName.find_last_of('.'), strFileName.length());
    if (strType == ".jpg" || strType == ".bmp")
    {
        std::string strFileDOM = strFileName.substr(0, strFileName.find_last_of('.')) + ".dom";
        std::ifstream file(strFileDOM, std::ios::_Nocreate);
        if (file)
        {
            std::string strUnit;
            double dXMin = 0.0;
            double dYMin = 0.0;
            double dXPrecision = 0.0;
            double dYPrecision = 0.0;

            char buf[256];
            while (file.getline(buf, 256))
            {
                std::string strLine = buf;
                std::string strLeft = strLine.substr(0, strLine.find(':'));
                std::string strRight = strLine.substr(strLine.find(':')+1, strLine.length());
                strLeft = strLeft.substr(strLeft.find_first_not_of(' '), strLeft.find_last_not_of(' ') + 1);
                strRight = strRight.substr(strRight.find_first_not_of(' '), strRight.find_last_not_of(' ') + 1);

                if (strLeft == "Unit")
                {
                    strUnit = strRight;
                }
                else if (strLeft == "Xr")
                {
                    dXMin = atof(strRight.c_str());
                }
                else if (strLeft == "Yc")
                {
                    dYMin = atof(strRight.c_str());
                }
                else if (strLeft == "Dr")
                {
                    dXPrecision = atof(strRight.c_str());
                }
                else if (strLeft == "Dc")
                {
                    dYPrecision = atof(strRight.c_str());
                }
            }

            file.close();

            if (strUnit == "D")
            {
                m_vecPrecision.x() = cmm::math::Degrees2Radians(fabs(dXPrecision));
                m_vecPrecision.y() = cmm::math::Degrees2Radians(fabs(dYPrecision));

                ptMin.x() = dXMin;
                ptMin.y() = dYMin;
                ptMax.x() = dXMin + fabs(dXPrecision) * nXSize;
                ptMax.y() = dYMin + fabs(dYPrecision) * nYSize;
            }
        }
    }
    else
    {
        //(GT0,GT3) - 左上角 ; GT2和GT4为0 ; GT1 - 象元宽 ; GT5 - 象元高
        double dGeoTransform[6] = {0};
        m_pGDALDataSet->GetGeoTransform(dGeoTransform);
        m_vecPrecision.x() = cmm::math::Degrees2Radians(fabs(dGeoTransform[1]));
        m_vecPrecision.y() = cmm::math::Degrees2Radians(fabs(dGeoTransform[5]));

		if (bHeightField)
		{
			//获取当前文件高程值的正常范围
			int n1 = 0;
			int n2 = 0;
			m_pMetadata[0] = m_pGDALDataSet->GetRasterBand(1)->GetMinimum(&n1);
			m_pMetadata[1] = m_pGDALDataSet->GetRasterBand(1)->GetMaximum(&n2);
			if (!(n1&&n2))
			{
				GDALComputeRasterMinMax(m_pGDALDataSet->GetRasterBand(1), FALSE, m_pMetadata);
			}
		}

        //(xMin, yMin) (xMax, yMax)
        ptMin.x() = (dGeoTransform[0]);
        ptMin.y() = (dGeoTransform[3] + nYSize * dGeoTransform[5]);
        ptMax.x() = (dGeoTransform[0] + nXSize * dGeoTransform[1]);
        ptMax.y() = (dGeoTransform[3]);
    }

    ptMin.x() = clampBetween(ptMin.x(), -180.0, 180.0);
    ptMin.y() = clampBetween(ptMin.y(), -90.0, 90.0);
    ptMax.x() = clampBetween(ptMax.x(), -180.0, 180.0);
    ptMax.y() = clampBetween(ptMax.y(), -90.0, 90.0);

    ptMin.x() = cmm::math::Degrees2Radians(ptMin.x());
    ptMin.y() = cmm::math::Degrees2Radians(ptMin.y());
    ptMax.x() = cmm::math::Degrees2Radians(ptMax.x());
    ptMax.y() = cmm::math::Degrees2Radians(ptMax.y());
    m_AreaBound.set(ptMin, ptMax);

    computeMinInterval();
    computeMaxInterval();

    computeStretchRange();

    return true;
}

bool Source::getFileCoordinates(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY)
{
    dMinX = cmm::math::Radians2Degrees(m_AreaBound.point0().x());
    dMinY = cmm::math::Radians2Degrees(m_AreaBound.point0().y());
    dMaxX = cmm::math::Radians2Degrees(m_AreaBound.point1().x());
    dMaxY = cmm::math::Radians2Degrees(m_AreaBound.point1().y());

    return true;
}

bool Source::setFileCoordinates(double dMinX, double dMinY, double dMaxX, double dMaxY)
{
    cmm::math::Point2d ptMin, ptMax;

    m_vecPrecision.x() = cmm::math::Degrees2Radians((dMaxX - dMinX) / m_pGDALDataSet->GetRasterXSize());
    m_vecPrecision.y() = cmm::math::Degrees2Radians((dMaxY - dMinY) / m_pGDALDataSet->GetRasterYSize());

    ptMin.x() = cmm::math::Degrees2Radians(dMinX);
    ptMin.y() = cmm::math::Degrees2Radians(dMinY);
    ptMax.x() = cmm::math::Degrees2Radians(dMaxX);
    ptMax.y() = cmm::math::Degrees2Radians(dMaxY);
    m_AreaBound.set(ptMin, ptMax);

    computeMinInterval();
    computeMaxInterval();

    computeStretchRange();

    return true;
}

bool Source::setLevel(const unsigned int nMinLevel, const unsigned int nMaxLevel)
{
    if (nMinLevel >= m_nMinInterval && nMinLevel <= m_nMaxInterval && nMaxLevel >= m_nMinInterval && nMaxLevel <= m_nMaxInterval && nMinLevel <= nMaxLevel)
    {
        m_nMinLevel = nMinLevel;
        m_nMaxLevel = nMaxLevel;

        return true;
    }

    if (nMinLevel == 0xFFFFFFFF && nMaxLevel == 0xFFFFFFFF)
    {
        m_nMinLevel = computeMinInterval();
        m_nMaxLevel = computeMaxInterval();

        return true;
    }
    else if (nMinLevel == 0xFFFFFFFF)
    {
        unsigned int nMin = computeMinInterval();
        if (nMin >= 1 && nMin <= 31 && nMin <= nMaxLevel)
        {
            m_nMinLevel = nMin;
            m_nMaxLevel = nMaxLevel;

            return true;
        }
    }
    else if (nMaxLevel == 0xFFFFFFFF)
    {
        unsigned int nMax = computeMaxInterval();
        if (nMax >= 1 && nMax <= 31 && nMinLevel <= nMax)
        {
            m_nMinLevel = nMinLevel;
            m_nMaxLevel = nMax;

            return true;
        }
    }

    return false;
}

bool Source::computeOffsetData(const cmm::math::Box2d &box2d, const unsigned int nTileSize, OFFSETDATA &offsetData)
{
    if (m_pGDALDataSet == NULL)
    {
        m_pGDALDataSet = (GDALDataset*)GDALOpen(m_strSourceFile.c_str(), GA_ReadOnly);
        if (m_pGDALDataSet == NULL)
        {
            return false;
        }
    }

    const int nMaxXSize = m_pGDALDataSet->GetRasterXSize();
    const int nMaxYSize = m_pGDALDataSet->GetRasterYSize();

    //计算出相交区域
    cmm::math::Point2d ptMin, ptMax;
    ptMax.x() = std::max(box2d.point0().x(), m_AreaBound.point0().x());
    ptMax.y() = std::max(box2d.point0().y(), m_AreaBound.point0().y());
    ptMin.x() = std::min(box2d.point1().x(), m_AreaBound.point1().x());
    ptMin.y() = std::min(box2d.point1().y(), m_AreaBound.point1().y());
    const cmm::math::Box2d intersectBox(ptMin, ptMax);

    //取出相交区域相对于源图中像素的x偏移和y偏移
    double dOffsetX = fabs((intersectBox.point0().x() - m_AreaBound.point0().x()) / m_vecPrecision.x());
    double dOffsetY = fabs((intersectBox.point1().y() - m_AreaBound.point1().y()) / m_vecPrecision.y());
    int nSourceX = (int)dOffsetX;
    int nSourceY = (int)dOffsetY;
    if (int(dOffsetX * 10.0) % 10 >= 5)
    {
        nSourceX++;
    }
    if (int(dOffsetY * 10.0) % 10 >= 5)
    {
        nSourceY++;
    }

    //取出相交区域在源图中的像素大小, 四舍五入
    dOffsetX = fabs((intersectBox.point0().x() - intersectBox.point1().x()) / m_vecPrecision.x());
    dOffsetY = fabs((intersectBox.point0().y() - intersectBox.point1().y()) / m_vecPrecision.y());
    int nOffsetX = (int)dOffsetX;
    int nOffsetY = (int)dOffsetY;
    if (int(dOffsetX * 10.0) % 10 >= 5)
    {
        nOffsetX++;
    }
    if (int(dOffsetY * 10.0) % 10 >= 5)
    {
        nOffsetY++;
    }

    if (nOffsetX == 0 || nOffsetY == 0)
    {
        offsetData.nSourceX = nSourceX;
        offsetData.nSourceY = nSourceY;
        offsetData.nOffsetX = nOffsetX;
        offsetData.nOffsetY = nOffsetY;
        offsetData.nTargetX = 0;
        offsetData.nTargetY = 0;
        offsetData.nTargetOffsetX = 0;
        offsetData.nTargetOffsetY = 0;

        return true;
    }

    //防止获取光栅数据时越界
    if ((nSourceX + nOffsetX) > nMaxXSize)
    {
        nOffsetX--;
    }
    if ((nSourceY + nOffsetY) > nMaxYSize)
    {
        nOffsetY--;
    }

    //取出相交区域相对于目标切片中的像素的x偏移和y偏移, 四舍五入
    dOffsetX = fabs((intersectBox.point0().x() - box2d.point0().x())) / box2d.width()  * nTileSize;
    dOffsetY = fabs((intersectBox.point1().y() - box2d.point1().y())) / box2d.height() * nTileSize;
    int nTargetX = (int)dOffsetX;
    int nTargetY = (int)dOffsetY;
    if (int(dOffsetX * 10.0) % 10 >= 5)
    {
        nTargetX++;
    }
    if (int(dOffsetY * 10.0) % 10 >= 5)
    {
        nTargetY++;
    }

    //取出相交区域在目标切片中的像素大小，四舍五入
    const double dXSize = (intersectBox.width()  / box2d.width())  * nTileSize;
    const double dYSize = (intersectBox.height() / box2d.height()) * nTileSize;
    int nTargetOffsetX  = (int)dXSize;
    int nTargetOffsetY  = (int)dYSize;
    if (int(dXSize * 10.0) % 10 >= 5)
    {
        nTargetOffsetX++;
    }
    if (int(dYSize * 10.0) % 10 >= 5)
    {
        nTargetOffsetY++;
    }

    offsetData.nSourceX = nSourceX;
    offsetData.nSourceY = nSourceY;
    offsetData.nOffsetX = nOffsetX;
    offsetData.nOffsetY = nOffsetY;
    offsetData.nTargetX = nTargetX;
    offsetData.nTargetY = nTargetY;
    offsetData.nTargetOffsetX = nTargetOffsetX;
    offsetData.nTargetOffsetY = nTargetOffsetY;

    return true;
}

bool Source::readTileForImage(const cmm::math::Box2d &box2d, const unsigned int nTileSize, std::vector<char*> &RasterDataVec)
{
    if (m_pGDALDataSet == NULL)
    {
        m_pGDALDataSet = (GDALDataset*)GDALOpen(m_strSourceFile.c_str(), GA_ReadOnly);
        if (m_pGDALDataSet == NULL)
        {
            return false;
        }
    }

    OFFSETDATA offsetData;
    if (!computeOffsetData(box2d, nTileSize, offsetData))
    {
        return false;
    }
    else if (offsetData.nOffsetX == 0 || offsetData.nOffsetY == 0)
    {
        return true;
    }

    std::vector<INVALIDCOLOR> vecInvalidColor = m_pTileBuilderImpl->getVecInvalidColor();

    //取出相交区域的像素数据
    int  nBland        = m_pGDALDataSet->GetRasterCount();
    bool hasRGB        = nBland >= 3;
    bool hasColorTable = nBland >= 1 && m_pGDALDataSet->GetRasterBand(1)->GetColorTable();

    if (hasRGB)
    {
        for(int i=1; i<=nBland; i++)
        {
            GDALRasterBand* pBand = m_pGDALDataSet->GetRasterBand(i);
            if (pBand == NULL)
            {
                return false;
            }

            unsigned int numBytesPerPixel = 1;
            switch (m_GDALDataType)
            {
            case GDT_Unknown:
                return false;
                break;
            case GDT_Byte:
                numBytesPerPixel = 1;
                break;
            case GDT_UInt16:
                numBytesPerPixel = 2;
                break;
            case GDT_UInt32:
                numBytesPerPixel = 4;
                break;
            case GDT_Int16:
            case GDT_Int32:
            case GDT_Float32:
            case GDT_Float64:
            case GDT_CInt16:
            case GDT_CInt32:
            case GDT_CFloat32:
            case GDT_CFloat64:
                return false;
                break;
            }

            unsigned char*  pafScanline  = new unsigned char[offsetData.nTargetOffsetX * offsetData.nTargetOffsetY * numBytesPerPixel];

            m_mtxRasterIO.lock();
            pBand->RasterIO(GF_Read, offsetData.nSourceX, offsetData.nSourceY, offsetData.nOffsetX, offsetData.nOffsetY, pafScanline, offsetData.nTargetOffsetX, offsetData.nTargetOffsetY, m_GDALDataType, 0, 0);
            m_mtxRasterIO.unlock();

            //将源像素数据融合到目标切片中
            for(int nY=0; nY<offsetData.nTargetOffsetY; nY++)
            {
                for (int nX=0; nX<offsetData.nTargetOffsetX; nX++)
                {
                    if(*(RasterDataVec[3]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) != 0)
                    {
                        continue;
                    }

                    switch (m_GDALDataType)
                    {
                    case GDT_Byte:
                        memcpy_s(RasterDataVec[i-1]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX, 1, pafScanline+offsetData.nTargetOffsetX*nY+nX, 1);

                        if (i == nBland)
                        {
                            if (nBland == 3)
                            {
                                *(RasterDataVec[3]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)255;
                            }

                            for (unsigned int j = 0; j < vecInvalidColor.size(); j++)
                            {
                                if ((*(RasterDataVec[0]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].r &&
                                    (*(RasterDataVec[1]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].g &&
                                    (*(RasterDataVec[2]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].b)
                                {
                                    *(RasterDataVec[3] +nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)0;
                                    break;
                                }
                            }
                        }
                        break;
                    case GDT_UInt16:
                        {
                            unsigned short* pValue = (unsigned short*)(pafScanline+(offsetData.nTargetOffsetX*nY+nX)*numBytesPerPixel);
                            *(RasterDataVec[i-1]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)(*pValue * 255 / (*(unsigned short*)m_RangeMax - *(unsigned short*)m_RangeMin));

                            if (i == nBland)
                            {
                                if (nBland == 3)
                                {
                                    *(RasterDataVec[3]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)255;
                                }

                                for (unsigned int j = 0; j < vecInvalidColor.size(); j++)
                                {
                                    if ((*(RasterDataVec[0]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].r &&
                                        (*(RasterDataVec[1]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].g &&
                                        (*(RasterDataVec[2]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].b)
                                    {
                                        *(RasterDataVec[3] +nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)0;
                                        break;
                                    }
                                }
                            }
                        }
                        break;
                    case GDT_Int16:
                        break;
                    case GDT_UInt32:
                        {
                            unsigned int* pValue = (unsigned int*)(pafScanline+(offsetData.nTargetOffsetX*nY+nX)*numBytesPerPixel);
                            *(RasterDataVec[i-1]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)(*pValue * 255 / (*(unsigned int*)m_RangeMax - *(unsigned int*)m_RangeMin));

                            if (i == nBland)
                            {
                                if (nBland == 3)
                                {
                                    *(RasterDataVec[3]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)255;
                                }

                                for (unsigned int j = 0; j < vecInvalidColor.size(); j++)
                                {
                                    if ((*(RasterDataVec[0]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].r &&
                                        (*(RasterDataVec[1]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].g &&
                                        (*(RasterDataVec[2]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].b)
                                    {
                                        *(RasterDataVec[3] +nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)0;
                                        break;
                                    }
                                }
                            }
                        }
                        break;
                    case GDT_Int32:
                        break;
                    case GDT_Float32:
                        break;
                    case GDT_Float64:
                        break;
                    case GDT_CInt16:
                        break;
                    case GDT_CInt32:
                        break;
                    case GDT_CFloat32:
                        break;
                    case GDT_CFloat64:
                        break;
                    }
                }
            }

            delete [] pafScanline;
        }
    }
    else if (hasColorTable)
    {
        GDALRasterBand* pBand = m_pGDALDataSet->GetRasterBand(1);
        if (pBand == NULL)
        {
            return false;
        }

        GDALColorTable* pColorTable = pBand->GetColorTable();
        if (pColorTable == NULL)
        {
            return false;
        }

        unsigned char*  pafScanline  = new unsigned char[offsetData.nTargetOffsetX * offsetData.nTargetOffsetY];
        unsigned char*  pafScanlineR = new unsigned char[offsetData.nTargetOffsetX * offsetData.nTargetOffsetY];
        unsigned char*  pafScanlineG = new unsigned char[offsetData.nTargetOffsetX * offsetData.nTargetOffsetY];
        unsigned char*  pafScanlineB = new unsigned char[offsetData.nTargetOffsetX * offsetData.nTargetOffsetY];
        unsigned char*  pafScanlineA = new unsigned char[offsetData.nTargetOffsetX * offsetData.nTargetOffsetY];

        m_mtxRasterIO.lock();
        pBand->RasterIO(GF_Read, offsetData.nSourceX, offsetData.nSourceY, offsetData.nOffsetX, offsetData.nOffsetY, pafScanline, offsetData.nTargetOffsetX, offsetData.nTargetOffsetY, GDT_Byte, 0, 0);
        m_mtxRasterIO.unlock();

        for(int i=0; i<offsetData.nTargetOffsetX*offsetData.nTargetOffsetY; i++ )
        {
            GDALColorEntry sEntry;
            pColorTable->GetColorEntryAsRGB(pafScanline[i], &sEntry);

            //用RGB填充目标图片
            pafScanlineR[i] = sEntry.c1;
            pafScanlineG[i] = sEntry.c2;
            pafScanlineB[i] = sEntry.c3;
            pafScanlineA[i] = sEntry.c4;
        }

        //将源像素数据融合到目标切片中
        for(int nY=0; nY<offsetData.nTargetOffsetY; nY++)
        {
            for (int nX=0; nX<offsetData.nTargetOffsetX; nX++)
            {
                if(*(RasterDataVec[3]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) != 0)
                {
                    continue;
                }

                memcpy_s(RasterDataVec[0]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX, 1, pafScanlineR+offsetData.nTargetOffsetX*nY+nX, 1);
                memcpy_s(RasterDataVec[1]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX, 1, pafScanlineG+offsetData.nTargetOffsetX*nY+nX, 1);
                memcpy_s(RasterDataVec[2]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX, 1, pafScanlineB+offsetData.nTargetOffsetX*nY+nX, 1);
                memcpy_s(RasterDataVec[3]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX, 1, pafScanlineA+offsetData.nTargetOffsetX*nY+nX, 1);

                for (unsigned int j = 0; j < vecInvalidColor.size(); j++)
                {
                    if ((*(RasterDataVec[0]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].r &&
                        (*(RasterDataVec[1]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].g &&
                        (*(RasterDataVec[2]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].b)
                    {
                        *(RasterDataVec[3]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)0;
                        break;
                    }
                }
            }
        }

        delete [] pafScanline;
        delete [] pafScanlineR;
        delete [] pafScanlineG;
        delete [] pafScanlineB;
    }
    else
    {
        GDALRasterBand* pBand = m_pGDALDataSet->GetRasterBand(1);
        if (pBand == NULL)
        {
            return false;
        }

        GDALDataType    type = pBand->GetRasterDataType() == GDT_Byte ? GDT_Byte : GDT_Float32;
        unsigned int    numBytesPerPixel = pBand->GetRasterDataType() == GDT_Byte ? 1 : 4;
        unsigned char*  pafScanline  = new unsigned char[offsetData.nTargetOffsetX * offsetData.nTargetOffsetY * numBytesPerPixel];

        m_mtxRasterIO.lock();
        pBand->RasterIO(GF_Read, offsetData.nSourceX, offsetData.nSourceY, offsetData.nOffsetX, offsetData.nOffsetY, pafScanline, offsetData.nTargetOffsetX, offsetData.nTargetOffsetY, type, 0, 0);
        m_mtxRasterIO.unlock();

        //将源像素数据融合到目标切片中
        for(int nY=0; nY<offsetData.nTargetOffsetY; nY++)
        {
            for (int nX=0; nX<offsetData.nTargetOffsetX; nX++)
            {
                if(*(RasterDataVec[3]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) != 0)
                {
                    continue;
                }

                if (type == GDT_Byte)
                {
                    memcpy_s(RasterDataVec[0]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX, 1, pafScanline+offsetData.nTargetOffsetX*nY+nX, 1);
                    memcpy_s(RasterDataVec[1]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX, 1, pafScanline+offsetData.nTargetOffsetX*nY+nX, 1);
                    memcpy_s(RasterDataVec[2]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX, 1, pafScanline+offsetData.nTargetOffsetX*nY+nX, 1);
                    *(RasterDataVec[3]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)255;
                }
                else
                {
                    if (type == GDT_Byte)
                    {
                        memcpy_s(RasterDataVec[0]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX, 1, pafScanline+offsetData.nTargetOffsetX*nY+nX, 1);
                        memcpy_s(RasterDataVec[1]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX, 1, pafScanline+offsetData.nTargetOffsetX*nY+nX, 1);
                        memcpy_s(RasterDataVec[2]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX, 1, pafScanline+offsetData.nTargetOffsetX*nY+nX, 1);
                        *(RasterDataVec[3]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)255;
                    }
                    else
                    {
                        float* pValue = (float*)(pafScanline+(offsetData.nTargetOffsetX*nY+nX)*numBytesPerPixel);
                        int nValue = (int)(*pValue);
                        *(RasterDataVec[0]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)(nValue%256);
                        *(RasterDataVec[1]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)(nValue%256);
                        *(RasterDataVec[2]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)(nValue%256);
                        *(RasterDataVec[3]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)255;
                    }
                }

                for (unsigned int j = 0; j < vecInvalidColor.size(); j++)
                {
                    if ((*(RasterDataVec[0]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].r &&
                        (*(RasterDataVec[1]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].g &&
                        (*(RasterDataVec[2]+nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX)) == vecInvalidColor[j].b)
                    {
                        *(RasterDataVec[3] +nTileSize*(offsetData.nTargetY+nY)+offsetData.nTargetX+nX) = (char)0;
                        break;
                    }
                }
            }
        }

        delete [] pafScanline;
    }

    return true;
}

bool Source::readTileForHeightField(const cmm::math::Box2d &box2d, const unsigned int nTileSize, float** pRasterData)
{
    if (m_pGDALDataSet == NULL)
    {
        m_pGDALDataSet = (GDALDataset*)GDALOpen(m_strSourceFile.c_str(), GA_ReadOnly);
        if (m_pGDALDataSet == NULL)
        {
            return false;
        }
    }

    OFFSETDATA offsetData;
    if (!computeOffsetData(box2d, nTileSize, offsetData))
    {
        return false;
    }
    else if (offsetData.nOffsetX == 0 || offsetData.nOffsetY == 0)
    {
        return true;
    }

    //取出相交区域的像素数据
    const int  nBand         = m_pGDALDataSet->GetRasterCount();
    const bool hasRGB        = nBand >= 3;
    const bool hasColorTable = (nBand >= 1 && m_pGDALDataSet->GetRasterBand(1)->GetColorTable());

    if (hasRGB || hasColorTable)
    {
        return false;
    }

    GDALRasterBand* pBand = m_pGDALDataSet->GetRasterBand(1);
    if (pBand == NULL)
    {
        return false;
    }

    GDALDataType    type  = pBand->GetRasterDataType() == GDT_Byte ? GDT_Byte : GDT_Float32;
    if (type != GDT_Float32)
    {
        return false;
    }

    double dGeoTransform[6] = {0};
    double invTransform[6]  = {0};
    m_pGDALDataSet->GetGeoTransform(dGeoTransform);
    GDALInvGeoTransform(dGeoTransform, invTransform);

    cmm::math::Point2d ptMin, ptMax;
    ptMin.x() = cmm::math::Radians2Degrees(box2d.point0().x());
    ptMin.y() = cmm::math::Radians2Degrees(box2d.point0().y());
    ptMax.x() = cmm::math::Radians2Degrees(box2d.point1().x());
    ptMax.y() = cmm::math::Radians2Degrees(box2d.point1().y());

    cmm::math::Box2d tmpBox2d;
    tmpBox2d.set(ptMin, ptMax);

    double xPixel = fabs(tmpBox2d.point0().x() - tmpBox2d.point1().x())/(nTileSize - 1);
    double yPixel = fabs(tmpBox2d.point0().y() - tmpBox2d.point1().y())/(nTileSize - 1);

	//定义一个临时的vector保存当前文件在当前瓦片中的数据信息
	std::vector<vect> tmpRasterData;

    //将源像素数据融合到目标切片中
    for (int nX = offsetData.nTargetX; nX < offsetData.nTargetX+offsetData.nTargetOffsetX; nX++)
    {
        double geoX = tmpBox2d.point0().x() + (xPixel * nX);
        for (int nY = offsetData.nTargetY; nY < offsetData.nTargetY+offsetData.nTargetOffsetY; nY++)
        {
            if (*(*pRasterData+nTileSize*nY+nX) > -900000.0)
            {
                continue;
            }

            double geoY = tmpBox2d.point0().y() + (yPixel * (nTileSize - nY));

            double r, c;
            GDALApplyGeoTransform(invTransform, geoX, geoY, &c, &r);

            int rowMin = maximum((int)floor(r), 0);
            int rowMax = maximum(minimum((int)ceil(r), (int)(m_pGDALDataSet->GetRasterYSize()-1)), 0);
            int colMin = maximum((int)floor(c), 0);
            int colMax = maximum(minimum((int)ceil(c), (int)(m_pGDALDataSet->GetRasterXSize()-1)), 0);

            if (rowMin > rowMax) rowMin = rowMax;
            if (colMin > colMax) colMin = colMax;

            float urHeight = 0.0;
            float llHeight = 0.0;
            float ulHeight = 0.0;
            float lrHeight = 0.0;

            m_mtxRasterIO.lock();
            pBand->RasterIO(GF_Read, colMin, rowMin, 1, 1, &llHeight, 1, 1, GDT_Float32, 0, 0);
            pBand->RasterIO(GF_Read, colMin, rowMax, 1, 1, &ulHeight, 1, 1, GDT_Float32, 0, 0);
            pBand->RasterIO(GF_Read, colMax, rowMin, 1, 1, &lrHeight, 1, 1, GDT_Float32, 0, 0);
            pBand->RasterIO(GF_Read, colMax, rowMax, 1, 1, &urHeight, 1, 1, GDT_Float32, 0, 0);
            m_mtxRasterIO.unlock();

            double x_rem = c - (int)c;
            double y_rem = r - (int)r;

            double w00 = (1.0 - y_rem) * (1.0 - x_rem) * (double)llHeight;
            double w01 = (1.0 - y_rem) * x_rem * (double)lrHeight;
            double w10 = y_rem * (1.0 - x_rem) * (double)ulHeight;
            double w11 = y_rem * x_rem * (double)urHeight;

            if ((w00 + w01 + w10 + w11) < -12000.0 || (w00 + w01 + w10 + w11) > 10000.0)
            {
                continue;
            }

			vect vec;
			vec.value = (float)(w00 + w01 + w10 + w11);
			vec.index = nTileSize*nY+nX;
			vec.row = nY - offsetData.nTargetY;;
			vec.col = nX - offsetData.nTargetX;
			tmpRasterData.push_back(vec);

            *(*pRasterData+nTileSize*nY+nX) = (float)(w00 + w01 + w10 + w11);
        }
    }

	//定义两个vector分别保存合法的值和噪声点
	std::vector<vect> legalRasterData, illegalRasterData;
	for (int i=0; i<tmpRasterData.size(); i++)
	{

		if ((tmpRasterData[i].value - (float)m_pMetadata[0])<-0.00000001 || (tmpRasterData[i].value - (float)m_pMetadata[1]) > 0.00000001)
		{
			illegalRasterData.push_back(tmpRasterData[i]);
		}
		else
		{
			legalRasterData.push_back(tmpRasterData[i]);
		}
	}

	//在合法的值中找到距离噪声点最近的位置并将其值赋值给噪声点所在的位置
	for (int i=0; i<illegalRasterData.size(); i++)
	{
		if (legalRasterData.size() != 0)
		{
			int n=10000, num=0;
			for (int j=0; j<legalRasterData.size(); j++)
			{
				int k = (int)sqrt(pow((double)(legalRasterData[j].row - illegalRasterData[i].row), 2) + pow((double)(legalRasterData[j].col - illegalRasterData[i].col),2));
				//int k = sqrt(abs(legalRasterData[j].row - illegalRasterData[i].row) + abs(legalRasterData[j].col - illegalRasterData[i].col));
				if (n > k)
				{
					n = k;
					num = j;
				}
			}
			*(*pRasterData+illegalRasterData[i].index) = legalRasterData[num].value;
		}
		else
		{
			*(*pRasterData+illegalRasterData[i].index) = (float)((m_pMetadata[0]+m_pMetadata[1])/2);
		}
	}

    return true;
}

bool Source::isAreaInBoud(const cmm::math::Box2d &box2d) const
{
    if (box2d.point0().x() > m_AreaBound.point1().x() ||
        box2d.point1().x() < m_AreaBound.point0().x() ||
        box2d.point0().y() > m_AreaBound.point1().y() ||
        box2d.point1().y() < m_AreaBound.point0().y())
    {
        return false;
    }

    return true;
}

unsigned int Source::getMinInterval() const
{
    return m_nMinInterval;
}

unsigned int Source::getMaxInterval() const
{
    return m_nMaxInterval;
}

const cmm::math::Box2d& Source::getBound() const
{
    return m_AreaBound;
}

const cmm::math::Point2d& Source::getPrecision() const
{
    return m_vecPrecision;
}

unsigned int Source::computeMinInterval()
{
    //以默认TileSize = 256为粗略计算值，每个图片必须保持在切片中占2*2以上的像素
    const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
    if (pPyramid == NULL)
    {
        return -1;
    }

    cmm::math::Vector2d vecTileSize(0.0, 0.0);
    int    nTileSize  = m_pTileBuilderImpl->getTileSize();

    for (int nLevel=1; nLevel<31; nLevel++)
    {
        vecTileSize = pPyramid->getLevelTileSize(nLevel);

        if (((m_AreaBound.width())/vecTileSize.x())  * nTileSize > 2 &&
            ((m_AreaBound.height())/vecTileSize.x()) * nTileSize > 2)
        {
            m_nMinInterval = nLevel;
            break;
        }
    }

    return 0;
}

unsigned int Source::computeMaxInterval()
{
    if (!m_pTileBuilderImpl->getAutoComputeLevel())
    {
        m_nMaxInterval = 31;
        return 0;
    }

    int nTileSize = m_pTileBuilderImpl->getTileSize();
    const double dbl2PI = cmm::math::PI * 2.0;
    for (int nLevel=1; nLevel<31; nLevel++)
    {
        double Precision1 = dbl2PI / (1<<nLevel);
        double Precision2 = dbl2PI / (1<<(nLevel+1));

//         if ((m_vecPrecision.x() * nTileSize) < Precision1 &&
//             (m_vecPrecision.x() * nTileSize) > Precision2)
//         {
//             m_nMaxInterval = nLevel + 1;
//             break;
//         }

        double dTargetPrecision = m_vecPrecision.x() * nTileSize;
        if (dTargetPrecision-Precision1 < 1e-9
            && dTargetPrecision-Precision2 > 1e-9)
        {
            m_nMaxInterval = nLevel + 1;
            break;
        }
    }

    return 0;
}
