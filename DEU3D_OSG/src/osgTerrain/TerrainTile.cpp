/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osgTerrain/TerrainTile>
#include <osgTerrain/GeometryTechnique>

#include <osg/ClusterCullingCallback>

#include <osgDB/ReadFile>


using namespace osg;
using namespace osgTerrain;

/////////////////////////////////////////////////////////////////////////////////
//
// TileID
//
TileID::TileID():
    level(-1),
    x(-1),
    y(-1)
{
}

TileID::TileID(int in_level, int in_x, int in_y):
    level(in_level),
    x(in_x),
    y(in_y)
{
}

/////////////////////////////////////////////////////////////////////////////////
//
// TerrainTile
//
TerrainTile::TerrainTile():
    _dirtyMask(NOT_DIRTY),
    _hasBeenTraversal(false)
{
    setThreadSafeRefUnref(true);
}

TerrainTile::TerrainTile(const TerrainTile& terrain,const osg::CopyOp& copyop):
    Group(terrain,copyop),
    _dirtyMask(NOT_DIRTY),
    _hasBeenTraversal(false),
    _elevationLayer(terrain._elevationLayer),
    _colorLayers(terrain._colorLayers)
{
    if (terrain.getTerrainTechnique()) 
    {
        setTerrainTechnique(dynamic_cast<TerrainTechnique*>(terrain.getTerrainTechnique()->cloneType()));
    }
}

TerrainTile::~TerrainTile()
{
    if (_terrainTechnique.valid())
    {
        _terrainTechnique->setTerrainTile(0);
    }
}

void TerrainTile::setTileID(const TileID& tileID)
{
    if (_tileID == tileID) return;
    _tileID = tileID;
}


void TerrainTile::traverse(osg::NodeVisitor& nv)
{
    if (!_hasBeenTraversal)
    {
        init(getDirtyMask(), false);

        _hasBeenTraversal = true;
    }

    if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
        const unsigned nNum = getNumCullCallback();
        for(unsigned n = 0u; n < nNum; n++)
        {
            osg::ClusterCullingCallback* ccc = dynamic_cast<osg::ClusterCullingCallback*>(getCullCallback(n));
            if (ccc)
            {
                if (ccc->cull(&nv,0,static_cast<State *>(0)))
                {
                    return;
                }
            }
        }
    }

    if (_terrainTechnique.valid())
    {
        _terrainTechnique->traverse(nv);
    }
    else
    {
        osg::Group::traverse(nv);
    }
}

void TerrainTile::init(int dirtyMask, bool assumeMultiThreaded)
{
    if (!_terrainTechnique)
    {
        setTerrainTechnique(new GeometryTechnique);
    }

    if (_terrainTechnique.valid())
    {
        _terrainTechnique->init(dirtyMask, assumeMultiThreaded);
    }
}

void TerrainTile::setTerrainTechnique(TerrainTechnique* terrainTechnique)
{
    if (_terrainTechnique == terrainTechnique) return; 

    int dirtyDelta = (_dirtyMask==NOT_DIRTY) ? 0 : -1;

    if (_terrainTechnique.valid()) 
    {
        _terrainTechnique->setTerrainTile(0);
    }

    _terrainTechnique = terrainTechnique;
    
    if (_terrainTechnique.valid()) 
    {
        _terrainTechnique->setTerrainTile(this);
        ++dirtyDelta;        
    }
    
    if (dirtyDelta>0) setDirtyMask(ALL_DIRTY);
    else if (dirtyDelta<0) setDirtyMask(NOT_DIRTY);
}

void TerrainTile::setDirtyMask(int dirtyMask)
{
    _dirtyMask = dirtyMask;
}

void TerrainTile::setElevationLayer(HeightFieldLayer* layer)
{
    _elevationLayer = layer;
}

void TerrainTile::setColorLayer(unsigned int i, TextureLayer* layer)
{
    if (_colorLayers.size() <= i) _colorLayers.resize(i+1);
    
    _colorLayers[i] = layer;
}

osg::BoundingSphere TerrainTile::computeBound() const
{
    osg::BoundingSphere bs;
    if(_elevationLayer == NULL) return bs;

    bs.expandBy(_elevationLayer->computeBound(true));

    return bs;
}


void TerrainTile::releaseGLObjects(osg::State* state) const
{
    Group::releaseGLObjects(state);

    if (_terrainTechnique.valid())
    {
        _terrainTechnique->releaseGLObjects( state );
    }
}

