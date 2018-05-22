#include <vector>

#include <osg/Node>
#include <osg/Image>
#include <osg/Shape>
#include <osgTerrain/Layer>
#include <osg/ValueObject>
#include <Common/Pyramid.h>
#include <osg/TexMat>
#include <sstream>
#include <osg/PagedLOD>
#include <osgDB/WriteFile>
#include <osg/ClusterCullingCallback>
#include <Common/deuImage.h>
#include <Common/deuMath.h>
#include <osg/CoordinateSystemNode>
#include <osgUtil/ClusterCullingCreator>
#include <IDProvider/Definer.h>
#include "TileObjBlender.h"
#include "Utility.h"

inline bool isValidElevation(double dblValue)
{
    return dblValue > -99999;
}

osgTerrain::TerrainTile *buildTerrainTile(const ID &id, std::vector<std::pair<osg::ref_ptr<osg::Texture2D>, osg::ref_ptr<osg::TexMat> > > &vecTexture, osg::Image *pDemImage)
{
    osg::ref_ptr<osgTerrain::Locator> pLocator = new osgTerrain::Locator;

    pLocator->setCoordinateSystemType(osgTerrain::Locator::GEOCENTRIC);
    pLocator->setTransformScaledByResolution(false);

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    pLocator->setEllipsoidModel(pEllipsoidModel);

    double dXmin,dXmax,dYmin,dYmax;
    const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
    pPyramid->getTilePos(id.TileID.m_nLevel, id.TileID.m_nRow, id.TileID.m_nCol, dXmin, dYmin, dXmax, dYmax);
    pLocator->setTransformAsExtents(dXmin, dYmin, dXmax, dYmax);

    osg::ref_ptr<osgTerrain::TerrainTile> pTerrainTile = new osgTerrain::TerrainTile;
    pTerrainTile->setID(id);
    pTerrainTile->setLocator(pLocator.get());
    pTerrainTile->setTileID(osgTerrain::TileID(id.TileID.m_nLevel, id.TileID.m_nCol, id.TileID.m_nRow));

    osg::StateSet *pStateSet = pTerrainTile->getOrCreateStateSet();

    if(vecTexture.size() > 1)
    {
        int a = 0;
    }

    if(!vecTexture.empty())
    {
        std::vector<std::pair<osg::ref_ptr<osg::Texture2D>, osg::ref_ptr<osg::TexMat> > >::const_reverse_iterator critor = vecTexture.crbegin();
        int i = 0;
        for(; critor != vecTexture.crend(); ++critor, i++)
        {
            osg::ref_ptr<osgTerrain::TextureLayer> pTextureLayer = new osgTerrain::TextureLayer;
            pTextureLayer->setLocator(pLocator.get());

            pTextureLayer->setTexture(critor->first.get());
            if(critor->second.valid())
            {
                pTextureLayer->setTexMat(critor->second.get());
            }

            pTerrainTile->setColorLayer(i, pTextureLayer.get());
            EarthLightModel::setSampleStatus(pStateSet, i, true);
        }
    }

    // 创建高程图层

    osg::ref_ptr<osg::HeightField>  pHeightField = new osg::HeightField;
    if(pDemImage)
    {
        pHeightField->allocate(pDemImage->s(), pDemImage->t());
        osg::FloatArray *pArray = pHeightField->getFloatArray();
        const float *pHeightData = (const float *)pDemImage->data();
        pArray->assign(pHeightData, pHeightData + pDemImage->s() * pDemImage->t());
        pHeightField->setXInterval((dXmax - dXmin) / (pDemImage->s() - 1));
        pHeightField->setYInterval((dYmax - dYmin) / (pDemImage->t() - 1));
    }
    else
    {
        pHeightField->allocate(8, 8);
        pHeightField->setXInterval((dXmax - dXmin) / 7.0);
        pHeightField->setYInterval((dYmax - dYmin) / 7.0);
    }
    pHeightField->setOrigin(osg::Vec3(dXmin, dYmin, 0.0f));

    osg::Vec3d point0;
    pEllipsoidModel->convertLatLongHeightToXYZ(dXmin, dYmin, 0.0, point0.x(), point0.y(), point0.z());

    osg::Vec3d point1;
    pEllipsoidModel->convertLatLongHeightToXYZ(dXmax, dYmax, 0.0, point1.x(), point1.y(), point1.z());

    const double dblLen = (point0 - point1).length();
    pHeightField->setSkirtHeight(dblLen * 0.05);

    osg::ref_ptr<osgTerrain::HeightFieldLayer> pHeightFieldLayer = new osgTerrain::HeightFieldLayer;
    pHeightFieldLayer->setHeightField(pHeightField);
    pHeightFieldLayer->setLocator(pLocator.get());

    pTerrainTile->setElevationLayer(pHeightFieldLayer.get());

    osg::ref_ptr<osg::ClusterCullingCallback> pClusterCallback = osgUtil::createClusterCullingCallbackByHeightField(pHeightField);
    pTerrainTile->addCullCallback(pClusterCallback.get());

    return pTerrainTile.release();
}

osgTerrain::TerrainTile *buildTerrainTile(const ID &id, osg::Texture2D *pTexture, osg::Image *pDemImage)
{
    osg::ref_ptr<osgTerrain::Locator> pLocator = new osgTerrain::Locator;

    pLocator->setCoordinateSystemType(osgTerrain::Locator::GEOCENTRIC);
    pLocator->setTransformScaledByResolution(false);

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    pLocator->setEllipsoidModel(pEllipsoidModel);

    double dXmin,dXmax,dYmin,dYmax;
    const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
    pPyramid->getTilePos(id.TileID.m_nLevel, id.TileID.m_nRow, id.TileID.m_nCol, dXmin, dYmin, dXmax, dYmax);
    pLocator->setTransformAsExtents(dXmin, dYmin, dXmax, dYmax);

    osg::ref_ptr<osgTerrain::TerrainTile> pTerrainTile = new osgTerrain::TerrainTile;
    pTerrainTile->setID(id);
    pTerrainTile->setLocator(pLocator.get());
    pTerrainTile->setTileID(osgTerrain::TileID(id.TileID.m_nLevel, id.TileID.m_nCol, id.TileID.m_nRow));

    osg::StateSet *pStateSet = pTerrainTile->getOrCreateStateSet();

    if(pTexture != NULL)
    {
        osg::ref_ptr<osgTerrain::TextureLayer> pTextureLayer = new osgTerrain::TextureLayer;
        pTextureLayer->setLocator(pLocator.get());

        pTextureLayer->setTexture(pTexture);

        pTerrainTile->setColorLayer(0, pTextureLayer.get());
    }

    // 创建高程图层

    osg::ref_ptr<osg::HeightField>  pHeightField = new osg::HeightField;
    if(pDemImage)
    {
        pHeightField->allocate(pDemImage->s(), pDemImage->t());
        osg::FloatArray *pArray = pHeightField->getFloatArray();
        const float *pHeightData = (const float *)pDemImage->data();
        pArray->assign(pHeightData, pHeightData + pDemImage->s() * pDemImage->t());
        pHeightField->setXInterval((dXmax - dXmin) / (pDemImage->s() - 1));
        pHeightField->setYInterval((dYmax - dYmin) / (pDemImage->t() - 1));
    }
    else
    {
        pHeightField->allocate(8, 8);
        pHeightField->setXInterval((dXmax - dXmin) / 7.0);
        pHeightField->setYInterval((dYmax - dYmin) / 7.0);
    }
    pHeightField->setOrigin(osg::Vec3(dXmin, dYmin, 0.0f));

    osg::Vec3d point0;
    pEllipsoidModel->convertLatLongHeightToXYZ(dXmin, dYmin, 0.0, point0.x(), point0.y(), point0.z());

    osg::Vec3d point1;
    pEllipsoidModel->convertLatLongHeightToXYZ(dXmax, dYmax, 0.0, point1.x(), point1.y(), point1.z());

    const double dblLen = (point0 - point1).length();
    pHeightField->setSkirtHeight(dblLen * 0.05);

    osg::ref_ptr<osgTerrain::HeightFieldLayer> pHeightFieldLayer = new osgTerrain::HeightFieldLayer;
    pHeightFieldLayer->setHeightField(pHeightField);
    pHeightFieldLayer->setLocator(pLocator.get());

    pTerrainTile->setElevationLayer(pHeightFieldLayer.get());

    osg::ref_ptr<osg::ClusterCullingCallback> pClusterCallback = osgUtil::createClusterCullingCallbackByHeightField(pHeightField);
    pTerrainTile->addCullCallback(pClusterCallback.get());

    return pTerrainTile.release();
}

osg::Node *buildTerrainNode(const ID &id, osgTerrain::TerrainTile *pTerrainTile)
{
    if(!pTerrainTile)   return NULL;

    if(id.TileID.m_nLevel >= 21u)
    {
        return pTerrainTile;
    }

    const float fRadius = pTerrainTile->getBound().radius();

    osg::ref_ptr<osg::PagedLOD> pPagedLOD = new osg::PagedLOD;
    pPagedLOD->addChild(pTerrainTile);

    const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
    double dblmax_X, dblmin_X, dblmax_Y, dblmin_Y;
    pPyramid->getTilePos(id.TileID.m_nLevel, id.TileID.m_nRow, id.TileID.m_nCol, dblmin_X, dblmin_Y, dblmax_X, dblmax_Y);

    if((id.TileID.m_nLevel > 4u) &&
        (cmm::math::floatEqual(dblmin_Y, -cmm::math::PI_2) || cmm::math::floatEqual(dblmax_Y, cmm::math::PI_2)))
    {
        pPagedLOD->setFileID(0u, ID::ID());
        pPagedLOD->setRange(0u, 0.0f, 1e10f);
    }
    else
    {
        const float fltDistance = osg::clampAbove(10.0f - (id.TileID.m_nLevel - 1.0f) * 0.4f, 4.0f);
        pPagedLOD->setFileID(0u, ID::ID());
        pPagedLOD->setRange(0u, fRadius * fltDistance, 1e10f);

        pPagedLOD->setFileID(1u, id);
        pPagedLOD->setRange(1u, 0.0f, fRadius * fltDistance);
    }

    const osg::BoundingSphere &bound = pPagedLOD->getBound();
    pPagedLOD->setCenter(bound.center());
    return pPagedLOD.release();
}


bool doesImageHaveAlpha(const osg::Image *pImage)
{
    if(!pImage) return false;

    if(pImage->getPixelFormat() == GL_RGBA)
    {
        cmm::image::Image image;
        image.attach((void *)pImage->data(), pImage->s(), pImage->t(), cmm::image::PF_RGBA);
        return image.hasAlpha();
    }
    else if(pImage->getPixelFormat() == GL_LUMINANCE)
    {
        cmm::image::Image   image;
        image.attach((void *)pImage->data(), pImage->s(), pImage->t(), cmm::image::PF_LUMINANCE);
        return image.hasAlpha();
    }

    return false;
}


void OutImage(const osg::Image *pImage, const ID &id)
{
    std::ostringstream oss;
    oss << "e:\\outTile\\"
        << "L_" << (unsigned)id.TileID.m_nLevel
        << "_R_" << (unsigned)id.TileID.m_nRow
        << "_C_" << (unsigned)id.TileID.m_nCol
        << ".txt";
    const std::string strFilePath = oss.str();

    std::ofstream file(strFilePath);

    for(int y = 0; y < pImage->t(); y++)
    {
        for(int x = 0; x < pImage->s(); x++)
        {
            const unsigned char *pData = pImage->data(x, y);
            const float *flt = (float *)pData;
            file << *flt << '\t';
        }
        file << '\n';
    }
    file.close();
}


osg::Image *floodImage(const ID &idSrc, const ID &idDes, const osg::Image *pImage)
{
    if(!pImage)
    {
        return NULL;
    }
    if(idSrc.TileID.m_nLevel > idDes.TileID.m_nLevel)
    {
        return NULL;
    }

    osg::ref_ptr<osg::Image>  pDesImage = dynamic_cast<osg::Image *>(pImage->clone(osg::CopyOp::DEEP_COPY_ALL));

    if(idSrc.TileID.m_nLevel == idDes.TileID.m_nLevel)
    {
        return pDesImage.release();
    }

    const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();
    cmm::math::Point2d ptDesMin, ptDesMax;
    pPyramid->getTilePos(idDes.TileID.m_nLevel, idDes.TileID.m_nRow, idDes.TileID.m_nCol, ptDesMin.x(), ptDesMin.y(), ptDesMax.x(), ptDesMax.y());

    cmm::math::Point2d ptSrcMin, ptSrcMax;
    pPyramid->getTilePos(idSrc.TileID.m_nLevel, idSrc.TileID.m_nRow, idSrc.TileID.m_nCol, ptSrcMin.x(), ptSrcMin.y(), ptSrcMax.x(), ptSrcMax.y());

    cmm::math::Box2d bbDes(ptDesMin, ptDesMax);
    cmm::math::Box2d bbSrc(ptSrcMin, ptSrcMax);


    if(idDes.TileID.m_nType != TERRAIN_TILE_HEIGHT_FIELD)
    {
        //if(bUseTexMat)
        //{
        //    osg::Matrix mtx = computeTexMat(idSrc, idDes);
        //    osg::ref_ptr<osg::TexMat> pTexMat = new osg::TexMat;
        //    pTexMat->setMatrix(mtx);
        //    pImage->setUserData(pTexMat);
        //    return true;
        //}
        if(!scaleImageByTileInfo(pDesImage.get(), bbSrc, bbDes))
            return NULL;
        else
            return pDesImage.release();
    }

    const osg::Vec2s vecSize1(pImage->s(), pImage->t());
    osg::Vec2s vecSize2;
    findNearestImageSize(vecSize1, vecSize2);
    if(vecSize1 == vecSize2)
    {
        if(!scaleImageByTileInfo(pDesImage.get(), bbSrc, bbDes))
            return NULL;
        else
            return pDesImage.release();
    }

    const osg::Vec2s  vecDelta(vecSize1.x() - vecSize2.x(), vecSize1.y() - vecSize2.y());
    if(vecDelta.x() < 0 || vecDelta.y() < 0)
    {
        return NULL;
    }

    const osg::Vec2d vecIntervalSrc(bbSrc.width() / (vecSize2.x() - 1), bbSrc.height() / (vecSize2.y() - 1));
    const osg::Vec2d vecBiasSrc(vecIntervalSrc.x() * vecDelta.x() * 0.5, vecIntervalSrc.y() * vecDelta.y() * 0.5);
    ptSrcMin.x() -= vecBiasSrc.x();
    ptSrcMax.x() += vecBiasSrc.x();
    ptSrcMin.y() -= vecBiasSrc.y();
    ptSrcMax.y() += vecBiasSrc.y();
    bbSrc.set(ptSrcMin, ptSrcMax);

    const osg::Vec2d vecIntervalDes(bbDes.width() / (vecSize2.x() - 1), bbDes.height() / (vecSize2.y() - 1));
    const osg::Vec2d vecBiasDes(vecIntervalDes.x() * vecDelta.x() * 0.5, vecIntervalDes.y() * vecDelta.y() * 0.5);
    ptDesMin.x() -= vecBiasDes.x();
    ptDesMax.x() += vecBiasDes.x();
    ptDesMin.y() -= vecBiasDes.y();
    ptDesMax.y() += vecBiasDes.y();
    bbDes.set(ptDesMin, ptDesMax);

    if(!scaleImageByTileInfo(pDesImage.get(), bbSrc, bbDes))
    {
        return NULL;
    }
    else
    {
        return pDesImage.release();
    }
}


osg::Image *getSubImage(const osg::Image *pImage, const osg::Vec2s &ptOffset, const osg::Vec2s &vecSize)
{
    if(!pImage) return NULL;
    if(ptOffset.x() < 0 || ptOffset.y() < 0)
    {
        return NULL;
    }
    if(vecSize.x() <= 0 || vecSize.y() <= 0)
    {
        return NULL;
    }
    const osg::Vec2s ptCorner = ptOffset + vecSize;
    if(ptCorner.x() >= pImage->s() || ptCorner.y() >= pImage->t())
    {
        return NULL;
    }

    osg::ref_ptr<osg::Image>    pSubImage = new osg::Image;
    pSubImage->allocateImage(vecSize.x(), vecSize.y(), 1, pImage->getPixelFormat(), pImage->getDataType());

    const unsigned char *pData  = pImage->data();
    unsigned char *pData1       = pSubImage->data();

    const unsigned nRowSize     = pImage->getRowSizeInBytes();
    const unsigned nRowSize1    = pSubImage->getRowSizeInBytes();
    const unsigned nPixelSize   = pSubImage->getPixelSizeInBits() / 8u;

    pData += nRowSize * ptOffset.y();
    const unsigned nOffset = nPixelSize * ptOffset.x();
    for(int y = 0; y < pSubImage->t(); y++)
    {
        memcpy(pData1, pData + nOffset, nRowSize1);
        pData1 += nRowSize1;
        pData  += nRowSize;
    }

    return pSubImage.release();
}


osg::Image *combineImage(osg::Image *pImageDes, osg::Image *pImageSrc, bool bImageTile)
{
    if(!pImageDes)  return NULL;
    if(!pImageSrc)  return pImageDes;

    if(!bImageTile)
    {
        const osg::Vec2s vecSizeDes1(pImageDes->s(), pImageDes->t());
        osg::Vec2s vecSizeDes2;
        findNearestImageSize(vecSizeDes1, vecSizeDes2);
        const bool bDesExtended = (vecSizeDes1 != vecSizeDes2);

        const osg::Vec2s vecSizeSrc1(pImageSrc->s(), pImageSrc->t());
        osg::Vec2s vecSizeSrc2;
        findNearestImageSize(vecSizeSrc1, vecSizeSrc2);
        const bool bSrcExtended = (vecSizeSrc1 != vecSizeSrc2);

        if(bDesExtended)
        {
            if(!bSrcExtended)
            {
                // 增加扩边
                osg::ref_ptr<osg::Image> pImageSrc1 = new osg::Image;
                pImageSrc1->allocateImage(vecSizeSrc1.x() + 2, vecSizeSrc1.y() + 2, 1, pImageSrc->getPixelFormat(), pImageSrc->getDataType());
                float *pData = (float *)pImageSrc1->data();
                for(int y = 0u; y < pImageSrc1->t(); y++)
                {
                    for(int x = 0u; x < pImageSrc1->s(); x++)
                    {
                        *pData = -999999.9f;
                        pData++;
                    }
                }
                pImageSrc1->copySubImage(1, 1, 0, pImageSrc);
                pImageSrc = pImageSrc1.release();
            }
        }
        else
        {
            if(bSrcExtended)
            {
                // 剪掉扩边
                const osg::Vec2s  vecDelta((vecSizeSrc1.x() - vecSizeSrc2.x()) / 2, (vecSizeSrc1.y() - vecSizeSrc2.y()) / 2);
                osg::ref_ptr<osg::Image> pImageSrc1 = getSubImage(pImageSrc, vecDelta, vecSizeSrc2);
                pImageSrc  = pImageSrc1.release();
            }
        }
    }

    const osg::Vec2s vecResultSize(std::max(pImageDes->s(), pImageSrc->s()), std::max(pImageDes->t(), pImageSrc->t()));
    if(pImageDes->s() != vecResultSize.x() || pImageDes->t() != vecResultSize.y())
    {
        pImageDes->scaleImage(vecResultSize.x(), vecResultSize.y(), 1);
    }
    if(pImageSrc->s() != vecResultSize.x() || pImageSrc->t() != vecResultSize.y())
    {
        pImageSrc->scaleImage(vecResultSize.x(), vecResultSize.y(), 1);
    }

    blendImage(pImageSrc, pImageDes);
    pImageDes = pImageSrc;

    return pImageDes;
}


bool scaleImageByTileInfo(osg::Image *pSrcImage, const cmm::math::Box2d &bbFrom, const cmm::math::Box2d &bbTo)
{
    cmm::image::Image   imageSrc;
    if(!attachOsgImage(pSrcImage, &imageSrc))
    {
        return false;
    }

    if(!imageSrc.scaleImageByArea(bbFrom, bbTo))
    {
        return false;
    }

    return true;
}


bool attachOsgImage(osg::Image *pOsgImage, cmm::image::Image *pImage)
{
    if(!pOsgImage || !pImage)    return false;

    cmm::image::PixelFormat eFormat = cmm::image::PF_RGBA;
    if(pOsgImage->getPixelFormat() == GL_RGBA || pOsgImage->getPixelFormat() == GL_BGRA)
    {
        eFormat = cmm::image::PF_RGBA;
    }
    else if(pOsgImage->getPixelFormat() == GL_RGB || pOsgImage->getPixelFormat() == GL_BGR)
    {
        eFormat = cmm::image::PF_RGB;
    }
    else if(pOsgImage->getPixelFormat() == GL_LUMINANCE)
    {
        eFormat = cmm::image::PF_LUMINANCE;
    }
    else    return false;

    pImage->attach(pOsgImage->data(), pOsgImage->s(), pOsgImage->t(), eFormat);
    return true;
}


bool blendImage(osg::Image *pDesImage, const osg::Image *pSrcImage)
{
    if(!pDesImage)  return false;
    if(!pSrcImage)  return true;


    cmm::image::Image   imageDes;
    if(!attachOsgImage(pDesImage, &imageDes))
    {
        return false;
    }

    cmm::image::Image   imageSrc;
    if(!attachOsgImage(const_cast<osg::Image *>(pSrcImage), &imageSrc))
    {
        return false;
    }

    return imageDes.blendImage(imageSrc);
}


void clearAlphaAsColor(osg::Image *pImage, unsigned char r, unsigned char g, unsigned char b)
{
    if(!pImage) return;

    if(pImage->getPixelFormat() == GL_RGBA || pImage->getPixelFormat() == GL_BGRA)
    {
        cmm::image::Image   image;
        image.attach(pImage->data(), pImage->s(), pImage->t(), cmm::image::PF_RGBA);
        image.clearAlphaAsColor(r, g, b);
    }
}


void clearAlphaAsColor(osg::Image *pImage, float fltValue)
{
    if(!pImage) return;

    const GLenum eFormat = pImage->getPixelFormat();
    const GLenum eType   = pImage->getDataType();
    if(eFormat != GL_LUMINANCE || eType != GL_FLOAT)
    {
        return;
    }

    cmm::image::Image   image;
    image.attach(pImage->data(), pImage->s(), pImage->t(), cmm::image::PF_LUMINANCE);
    image.clearAlphaAsColor(fltValue);
}


void smoothHeightField(osg::Image *pImage, unsigned nCount)
{
    if(!pImage) return;

    const GLenum eFormat = pImage->getPixelFormat();
    const GLenum eType   = pImage->getDataType();
    if(eFormat != GL_LUMINANCE || eType != GL_FLOAT)
    {
        return;
    }

    cmm::image::Image   image;
    image.attach(pImage->data(), pImage->s(), pImage->t(), cmm::image::PF_LUMINANCE);

    const double dbl = 1.0 / 9.0;
    const double dblKernel[3][3] = {dbl, dbl, dbl, dbl, dbl, dbl, dbl, dbl, dbl};
    for(unsigned n = 0u; n < nCount; n++)
    {
        image.convoluteImage(dblKernel);
    }
}


void findNearestImageSize(const osg::Vec2s &sizeOrg, osg::Vec2s &sizeResult)
{
    sizeResult = sizeOrg;
    if(sizeOrg.x() <= 4 || sizeOrg.y() <= 4)
    {
        return;
    }

    short x = 1;
    while(x <= sizeOrg.x())
    {
        x += x;
    }
    if(x != sizeOrg.x())
    {
        sizeResult.x() = x / 2;
    }

    short y = 1;
    while(y <= sizeOrg.y())
    {
        y += y;
    }
    if(y != sizeOrg.y())
    {
        sizeResult.y() = y / 2;
    }
}


