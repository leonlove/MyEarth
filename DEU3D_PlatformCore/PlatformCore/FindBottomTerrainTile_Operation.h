#ifndef FIND_BOTTOM_TERRAIN_TILE_OPERATION_H_F0FEB03D_CDBD_410D_9492_95E40446ED7A_INCLUDE
#define FIND_BOTTOM_TERRAIN_TILE_OPERATION_H_F0FEB03D_CDBD_410D_9492_95E40446ED7A_INCLUDE

#include "SceneGraphOperationBase.h"
#include <osg/NodeVisitor>
#include <osgTerrain/TerrainTile>
#include <OpenThreads/Block>

class FindBottomTerrainTile_Operation : public SceneGraphOperationBase
{
public:
    explicit FindBottomTerrainTile_Operation(void);
    virtual ~FindBottomTerrainTile_Operation(void) {}

public:
    typedef std::vector<osg::ref_ptr<osgTerrain::TerrainTile> >     TerrainTileList;
    typedef std::map<unsigned char, TerrainTileList>                TerrainTileMap;

protected:
    class BottomTerrainTileFinder : public osg::NodeVisitor
    {
    public:
        BottomTerrainTileFinder()
            : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        {
        }

        void apply(osg::Group &node)
        {
            osgTerrain::TerrainTile *pTile = dynamic_cast<osgTerrain::TerrainTile *>(&node);
            if(!pTile)
            {
                osg::NodeVisitor::apply(node);
                return;
            }

            m_mapTiles[pTile->getID().TileID.m_nLevel].push_back(pTile);
        }

    public:
        TerrainTileMap      m_mapTiles;
    };

    osg::ref_ptr<BottomTerrainTileFinder>       m_pFinder;
    OpenThreads::Block                          m_block;

public:
    void  waitResult(void)                  {   m_block.block();                }
    TerrainTileMap &getTerrainTiles(void)   {   return m_pFinder->m_mapTiles;   }

protected:
    virtual bool doAction(SceneGraphOperator *pOperator);
};


#endif

