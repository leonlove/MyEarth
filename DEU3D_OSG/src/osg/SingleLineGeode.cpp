#include <osg/SingleLineGeode>
#include <osg/MultiLineDrawable>

namespace osg {

SingleLineGeode::ShadowDrawable::ShadowDrawable(const osg::Vec3d &point0, const osg::Vec3d &point1, const osg::Vec4 &color, float fltWidth)
    : m_nLineKey(~0ui64),
      m_point0(point0),
      m_point1(point1),
      m_color(color),
      m_fltWidth(fltWidth)
{
}


SingleLineGeode::ShadowDrawable::~ShadowDrawable(void)
{
    if(m_nLineKey != ~0ui64)
    {
        MultiLineDrawable::instance()->removeLine(m_nLineKey);
        m_nLineKey = ~0ui64;
    }
}


osg::BoundingBox SingleLineGeode::ShadowDrawable::computeBound(void) const
{
    osg::BoundingBox box;

    box.xMax() = osg::maximum(m_point0.x(), m_point1.x());
    box.xMin() = osg::minimum(m_point0.x(), m_point1.x());
    box.yMax() = osg::maximum(m_point0.y(), m_point1.y());
    box.yMin() = osg::minimum(m_point0.y(), m_point1.y());
    box.zMax() = osg::maximum(m_point0.z(), m_point1.z());
    box.zMin() = osg::minimum(m_point0.z(), m_point1.z());
    return box;
}


void SingleLineGeode::ShadowDrawable::accept(osg::PrimitiveFunctor &functor) const
{
    osg::ref_ptr<osg::Vec3Array>    pFloatArray = new osg::Vec3Array;
    pFloatArray->push_back(m_point0);
    pFloatArray->push_back(m_point1);
    functor.setVertexArray(2u, (const osg::Vec3 *)pFloatArray->getDataPointer());
    functor.drawArrays(GL_LINES, 0u, 2u);
}


void SingleLineGeode::ShadowDrawable::traverse(osg::NodeVisitor &nv)
{
    if(nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
    {
        return;
    }

    if(m_nLineKey == ~0ui64)
    {
        m_nLineKey = MultiLineDrawable::instance()->addLine(m_point0, m_point1, m_color, m_fltWidth);
    }
}


SingleLineGeode::SingleLineGeode(const osg::Vec3d &point0, const osg::Vec3d &point1, const osg::Vec4 &color, float fltWidth)
{
    m_pShadowDrawable = new ShadowDrawable(point0, point1, color, fltWidth);
    addDrawable(m_pShadowDrawable);
}


SingleLineGeode::~SingleLineGeode(void)
{
}


osg::BoundingSphere SingleLineGeode::computeBound(void) const
{
    osg::BoundingSphere sphere;
    sphere.expandBy(_drawables[0]->computeBound());
    return sphere;
}


void SingleLineGeode::traverse(osg::NodeVisitor &nv)
{
    m_pShadowDrawable->traverse(nv);
}

}
