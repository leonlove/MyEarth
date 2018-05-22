#include "TerrainColorModification.h"
#include <IDProvider/Definer.h>
#include <osg/TexEnv>
#include <assert.h>
#include "Utility.h"
#include <osgShadow/SoftShadowMap>

#include "Registry.h"

TerrainColorModification::TerrainColorModification(const std::string &strType, ea::IEventAdapter *pEventAdapter)
    : TerrainModification(strType, pEventAdapter)
{
}


TerrainColorModification::~TerrainColorModification(void)
{
}


bool TerrainColorModification::modifyTerrainTile(osg::Node *pTerrainTileNode) const
{
    if(!isApply())  return false;

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

    const osg::Vec2d vecImageRatio(TileBB.width() / (double)pDomImage->s(), TileBB.height() / (double)pDomImage->t());
    const unsigned char color[4] = {(unsigned char)(m_color.m_fltR * 255.0f), (unsigned char)(m_color.m_fltG * 255.0f), (unsigned char)(m_color.m_fltB * 255.0f), (unsigned char)(m_color.m_fltA * 255.0f)};

    cmm::math::Point2d pos(ptTileMin);
    for(int y = 0; y < pDomImage->t(); y++)
    {
        unsigned char *pData = pDomImage->data(0, y);
        for(int x = 0; x < pDomImage->s(); x++, pData += 4u)
        {
            if(!m_Polygon.containsPoint(pos))
            {
                pos.x() += vecImageRatio.x();
                continue;
            }

            if(color[3] == 255)
            {
                pData[0] = color[0];
                pData[1] = color[1];
                pData[2] = color[2];
            }
            else
            {
                pData[0] *= (1.0f - m_color.m_fltA);
                pData[0] += color[0] * m_color.m_fltA;

                pData[1] *= (1.0f - m_color.m_fltA);
                pData[1] += color[1] * m_color.m_fltA;

                pData[2] *= (1.0f - m_color.m_fltA);
                pData[2] += color[2] * m_color.m_fltA;
            }
            pData[3] = 255;

            pos.x() += vecImageRatio.x();
        }
        pos.x()  = ptTileMin.x();
        pos.y() += vecImageRatio.y();
    }

    return true;
}

