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

#include <osgTerrain/Layer>
#include <osg/Notify>

using namespace osgTerrain;

Layer::Layer()
{
}

Layer::Layer(const Layer& layer,const osg::CopyOp& copyop) : osg::Object(layer,copyop)
{
}

Layer::~Layer()
{
}

/////////////////////////////////////////////////////////////////////////////
//
// TextureLayer
//

TextureLayer::TextureLayer(osg::Texture *pTexture) : m_pTexture(pTexture)
{
}

TextureLayer::TextureLayer(const TextureLayer &textureLayer, const osg::CopyOp &copyop) :
    Layer(textureLayer, copyop),
    m_pTexture(textureLayer.m_pTexture)
{
}

HeightFieldLayer::HeightFieldLayer(osg::HeightField* hf):
    _modifiedCount(0),
    _heightField(hf)
{
}

HeightFieldLayer::HeightFieldLayer(const HeightFieldLayer& hfLayer,const osg::CopyOp& copyop):
    Layer(hfLayer,copyop),
    _modifiedCount(0),
    _heightField(hfLayer._heightField)
{
    if (_heightField.valid()) ++_modifiedCount;
}


void HeightFieldLayer::setHeightField(osg::HeightField* hf)
{
    _heightField = hf;
    dirty();
}

void HeightFieldLayer::restore(void)
{
    if(!_heightField_bak.valid())
    {
        return;
    }
    else
    {
        _heightField = _heightField_bak;
        {
            _heightField_bak = NULL;
        }
        dirty();
    }
}

void HeightFieldLayer::backup(void)
{
    if(!_heightField_bak.valid())
    {
        _heightField_bak = new osg::HeightField(*_heightField, osg::CopyOp::DEEP_COPY_ALL);
    }
}

bool HeightFieldLayer::getValue(unsigned int i, unsigned int j, float& value) const
{
    value = _heightField->getHeight(i,j);
    return true;
}

void HeightFieldLayer::dirty()
{
    ++_modifiedCount;
}

void HeightFieldLayer::setModifiedCount(unsigned int value)
{
    _modifiedCount = value;
}

unsigned int HeightFieldLayer::getModifiedCount() const
{
    return _modifiedCount;
}


osg::BoundingSphere HeightFieldLayer::computeBound(bool treatAsElevationLayer) const
{
    osg::BoundingSphere bs;
    if (!getLocator()) return bs;

    osg::BoundingBox bb;
    unsigned int numColumns = getNumColumns();
    unsigned int numRows = getNumRows();
    for(unsigned int r=0;r<numRows;++r)
    {
        for(unsigned int c=0;c<numColumns;++c)
        {
            float value = 0.0f;
            bool validValue = getValidValue(c,r, value);
            if (validValue) 
            {
                osg::Vec3d ndc, v;
                ndc.x() = ((double)c)/(double)(numColumns-1), 
                    ndc.y() = ((double)r)/(double)(numRows-1);
                ndc.z() = value;

                if (getLocator()->convertLocalToModel(ndc, v))
                {
                    bb.expandBy(v);
                }
            }
        }
    }
    bs.expandBy(bb);

    return bs;
}

