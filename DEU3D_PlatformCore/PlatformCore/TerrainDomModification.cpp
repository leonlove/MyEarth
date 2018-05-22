#include "TerrainDomModification.h"
#include <algorithm>
#include <osg/TexEnv>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <IDProvider/Definer.h>
#include <Common/deuMath.h>
#include <Common/deuImage.h>
#include <assert.h>
#include "Utility.h"
#include <osgShadow/SoftShadowMap>

#include "Registry.h"

TerrainDomModification::TerrainDomModification(const std::string &strType, ea::IEventAdapter *pEventAdapter)
    : TerrainModification(strType, pEventAdapter)
{
}


TerrainDomModification::~TerrainDomModification(void)
{
}


bool TerrainDomModification::setDomFile(const std::string &strFilePath)
{
    if(isApply())   return false;
    if(strFilePath.empty()) return false;

    std::string strFile = strFilePath;
    std::transform(strFile.begin(), strFile.end(), strFile.begin(), ::toupper);
    if(strFile == m_strDomFile)
    {
        return true;
    }

    osg::ref_ptr<osg::Image>    pImage = osgDB::readImageFile(strFilePath);
    if(!pImage.valid())
    {
        return false;
    }
    const unsigned nPixelSize = pImage->getPixelSizeInBits();
    if(nPixelSize != 32u && nPixelSize != 24u)
    {
        return false;
    }

    m_strDomFile = strFile;

    if(nPixelSize == 32u)
    {
        m_pDomImage = pImage;
        return true;
    }

    if(!m_pDomImage.valid())
    {
        m_pDomImage = new osg::Image;
    }
    m_pDomImage->allocateImage(pImage->s(), pImage->t(), 1, GL_RGBA, GL_UNSIGNED_BYTE);
    for(int y = 0; y < m_pDomImage->t(); y++)
    {
        unsigned char *pLine0 = m_pDomImage->data(0, y);
        unsigned char *pLine1 = pImage->data(0, y);
        for(int x = 0; x < m_pDomImage->s(); x++)
        {
            pLine0[0] = pLine1[0];
            pLine0[1] = pLine1[1];
            pLine0[2] = pLine1[2];
            pLine0[3] = 0xFF;
            pLine0 += 4;
            pLine1 += 3;
        }
    }
    return true;
}


const std::string &TerrainDomModification::getDomFile(void) const
{
    return m_strDomFile;
}


bool TerrainDomModification::modifyTerrainTile(osg::Node *pTerrainTileNode) const
{
    if(!isApply())  return false;

    if(!m_pDomImage.valid())
    {
        return false;
    }
    osgTerrain::TerrainTile *pTerrainTile = dynamic_cast<osgTerrain::TerrainTile *>(pTerrainTileNode);
    if(pTerrainTile == NULL)
    {
        return false;
    }

    bool bUseShadow = Registry::instance()->getUseShadow();
    osgTerrain::HeightFieldLayer *pHFLayer = dynamic_cast<osgTerrain::HeightFieldLayer *>(pTerrainTile->getElevationLayer());
    osg::HeightField *pHF = pHFLayer->getHeightField();

    const cmm::math::Point2d vecInterval(pHF->getXInterval(), pHF->getYInterval());
    const unsigned int nElevRows = pHF->getNumRows();
    const unsigned int nElevCols = pHF->getNumColumns();
    const cmm::math::Point2d ptTileMin(pHF->getOrigin().x(), pHF->getOrigin().y());
    const cmm::math::Point2d ptTileMax(ptTileMin.x() + (nElevCols - 1u) * vecInterval.x(), ptTileMin.y() + (nElevRows - 1u) * vecInterval.y());
    const cmm::math::Box2d TileBB(ptTileMin, ptTileMax);

    cmm::math::Polygon2  polygonTile;
    polygonTile.addVertex(ptTileMin);
    polygonTile.addVertex(cmm::math::Point2d(ptTileMin.x(), ptTileMax.y()));
    polygonTile.addVertex(ptTileMax);
    polygonTile.addVertex(cmm::math::Point2d(ptTileMax.x(), ptTileMin.y()));
    if(!shouldBeModified(polygonTile, TileBB))
    {
        return false;
    }

    osg::Image *pDomImage = NULL;

    osgTerrain::TextureLayer *pTextureLayer = bUseShadow ? dynamic_cast<osgTerrain::TextureLayer *>(pTerrainTile->getColorLayer(1u)) : dynamic_cast<osgTerrain::TextureLayer *>(pTerrainTile->getColorLayer(7u));

    if(pTextureLayer != NULL)
    {
        osg::Texture2D *pTexture2D = dynamic_cast<osg::Texture2D *>(pTextureLayer->getTexture());
        pDomImage = pTexture2D->getImage();
        pTexture2D->dirtyTextureObject();
        assert(pDomImage);
        assert(pDomImage->getPixelSizeInBits() == 32u); // 第7号纹理一定是我们自己创建的，4通道图片
    }
    else
    {
        pTextureLayer = new osgTerrain::TextureLayer;
        pDomImage = new osg::Image;
        pDomImage->allocateImage(256, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE);
        memset(pDomImage->data(), 0, pDomImage->getImageSizeInBytes());

        osg::ref_ptr<osg::Texture2D> pTexture2D = new osg::Texture2D;
        pTexture2D->setUnRefImageDataAfterApply(false);
        pTexture2D->setImage(pDomImage);
        pTexture2D->setMaxAnisotropy(16.0f);
        pTexture2D->setResizeNonPowerOfTwoHint(false);

        pTexture2D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        pTexture2D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

        pTexture2D->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
        pTexture2D->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);

        pTextureLayer->setTexture(pTexture2D);

        pTextureLayer->setLocator(pTerrainTile->getLocator());
        pTerrainTile->setColorLayer(bUseShadow ? 1u : 7u, pTextureLayer);

        osg::StateSet *pStateSet = pTerrainTile->getOrCreateStateSet();
        if(bUseShadow)
            osgShadow::SoftShadowMap::setSecondTexture(pStateSet, true);
        else
            EarthLightModel::setSampleStatus(pStateSet, 7u, true);
    }

    // 计算瓦片图像分辨率
    const osg::Vec2d vecImageRatio(TileBB.width() / (double)pDomImage->s(), TileBB.height() / (double)pDomImage->t());
    const osg::Vec2d vecSrcImageRatio(m_Box.width() / m_pDomImage->s(), m_Box.height() / m_pDomImage->t());

    const cmm::math::Point2d ptSrcLeftBottom = m_Box.corner(cmm::math::Box2d::LeftBottom);
    cmm::math::Point2d pos(ptTileMin);
    for(int y = 0; y < pDomImage->t(); y++)
    {
        const double dblBiasInSrcY = pos.y() - ptSrcLeftBottom.y();
        const double dblPosInSrcY  = dblBiasInSrcY / vecSrcImageRatio.y();
        const unsigned nTop        = cmm::math::clampBelow((unsigned)ceil(dblPosInSrcY),  m_pDomImage->t() - 1u);
        const unsigned nBottom     = cmm::math::clampBelow((unsigned)floor(dblPosInSrcY), m_pDomImage->t() - 1u);
        const double   dblV        = dblPosInSrcY - nBottom;

        unsigned char *pData = pDomImage->data(0, y);
        for(int x = 0; x < pDomImage->s(); x++, pData += 4u)
        {
            if(!m_Polygon.containsPoint(pos))
            {
                pos.x() += vecImageRatio.x();
                continue;
            }

            const double dblBiasInSrcX = pos.x() - ptSrcLeftBottom.x();
            const double dblPosInSrcX  = dblBiasInSrcX / vecSrcImageRatio.x();
            const unsigned nRight      = cmm::math::clampBelow((unsigned)ceil(dblPosInSrcX),  m_pDomImage->s() - 1u);
            const unsigned nLeft       = cmm::math::clampBelow((unsigned)floor(dblPosInSrcX), m_pDomImage->s() - 1u);
            const double   dblU        = dblPosInSrcX - nLeft;

            const unsigned char *pLB = m_pDomImage->data(nLeft,  nBottom);
            const unsigned char *pRB = m_pDomImage->data(nRight, nBottom);
            const unsigned char *pLT = m_pDomImage->data(nLeft,  nTop);
            const unsigned char *pRT = m_pDomImage->data(nRight, nTop);

            unsigned char r = cmm::image::linearInterpolation(pLB[0], pRB[0], pLT[0], pRT[0], dblU, dblV);
            unsigned char g = cmm::image::linearInterpolation(pLB[1], pRB[1], pLT[1], pRT[1], dblU, dblV);
            unsigned char b = cmm::image::linearInterpolation(pLB[2], pRB[2], pLT[2], pRT[2], dblU, dblV);
            unsigned char a = cmm::image::linearInterpolation(pLB[3], pRB[3], pLT[3], pRT[3], dblU, dblV);

            pData[0] = r;
            pData[1] = g;
            pData[2] = b;
            pData[3] = a;

            pos.x() += vecImageRatio.x();
        }
        pos.x()  = ptTileMin.x();
        pos.y() += vecImageRatio.y();
    }

    return true;
}

