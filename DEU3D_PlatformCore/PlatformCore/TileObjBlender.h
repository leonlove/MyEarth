#ifndef TILE_OBJ_BLENDER_H_77176D19_7B3D_491A_804D_13DD7C5F3A07_INCLUDE
#define TILE_OBJ_BLENDER_H_77176D19_7B3D_491A_804D_13DD7C5F3A07_INCLUDE

#include <IDProvider/ID.h>
#include <osgDB/ReaderWriter>
#include <osg/NodeVisitor>
#include <osgTerrain/TerrainTile>
#include <osg/BoundingBox>
#include <osg/Vec2s>

namespace osg
{
    class Image;
    class Node;
    class HeightField;
}

osgTerrain::TerrainTile *buildTerrainTile(const ID &id, std::vector<std::pair<osg::ref_ptr<osg::Texture2D>, osg::ref_ptr<osg::TexMat> > > &vecTexture, osg::Image *pDemImage);
osgTerrain::TerrainTile *buildTerrainTile(const ID &id, osg::Texture2D *pTexture, osg::Image *pDemImage);
osg::Node* buildTerrainNode(const ID &id, osgTerrain::TerrainTile *pTerrainTile);

osg::Image *floodImage(const ID &idSrc, const ID &idDes, const osg::Image *pImage);
//bool floodImage(const cmm::Pyramid &pyramid, const ID &idSrc, const ID &idDes, osg::Image *pImage, bool bUseTexMat);
osg::Image *combineImage(osg::Image *pImageDes, osg::Image *pImageSrc, bool bImageTile);
bool scaleImageByTileInfo(osg::Image *pSrcImage, const cmm::math::Box2d &bbFrom, const cmm::math::Box2d &bbTo);
bool doesImageHaveAlpha(const osg::Image *pImage);
bool blendImage(osg::Image *pDesImage, const osg::Image *pSrcImage);
void clearAlphaAsColor(osg::Image *pImage, unsigned char r, unsigned char g, unsigned char b);
void clearAlphaAsColor(osg::Image *pImage, float fltValue);
void smoothHeightField(osg::Image *pImage, unsigned nCount = 1u);
osg::Image *getSubImage(const osg::Image *pImage, const osg::Vec2s &ptOffset, const osg::Vec2s &vecSize);
void findNearestImageSize(const osg::Vec2s &sizeOrg, osg::Vec2s &sizeResult);

bool attachOsgImage(osg::Image *pOsgImage, cmm::image::Image *pImage);

#endif
