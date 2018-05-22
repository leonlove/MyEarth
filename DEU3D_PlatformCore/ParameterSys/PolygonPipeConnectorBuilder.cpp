#include "PolygonPipeConnectorBuilder.h"
#include <assert.h>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/Quat>
#include <osg/Material>
#include <iostream>
#include <osg/ShapeDrawable>
#include <osg/Shape>
#include <osg/MatrixTransform>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
PolygonPipeConnectorBuilder::PolygonPipeConnectorBuilder(void)
{
    setElbowHint(0.50);
    m_dblCornerRadiusRatio = 0.8;
    m_pColorArray = new osg::Vec4Array;
    m_pColorArray->push_back(osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f));
}


PolygonPipeConnectorBuilder::~PolygonPipeConnectorBuilder(void)
{
}


void PolygonPipeConnectorBuilder::setType(const std::string &strType)
{
    m_strType.resize(strType.length());
    std::transform(strType.begin(), strType.end(), m_strType.begin(), ::tolower);
}


void PolygonPipeConnectorBuilder::setCorner(const osg::Vec3d &ptCorner)
{
    m_ptCorner = ptCorner;
}


void PolygonPipeConnectorBuilder::setColor(const osg::Vec4 &color)
{
    (*m_pColorArray)[0].set(color.r(), color.g(), color.b(), color.a());
}


void PolygonPipeConnectorBuilder::setElbowHint(double dblHint)
{
    m_dblElbowHint = osg::clampBetween(dblHint, 0.0, 1.0);

    m_dblElbowHintApply  = m_dblElbowHint * osg::PI_2;
    m_dblElbowHintApply  = osg::clampBelow(m_dblElbowHintApply, osg::PI_2 * 0.999);
    m_dblElbowHintApply  = tan(m_dblElbowHintApply);
    m_dblElbowHintApply *= 16.0;
}


void PolygonPipeConnectorBuilder::addPort(const osg::Vec3d &ptPosition, const cmm::math::Polygon2 &polygon)
{
    m_vecJoints.resize(m_vecJoints.size() + 1u);
    Port &port = m_vecJoints.back();
    port.m_ptPosition  = ptPosition;
    port.m_polygon = polygon;
}


void PolygonPipeConnectorBuilder::clearJoints(void)
{
    m_vecJoints.clear();
}


osg::Node *PolygonPipeConnectorBuilder::buildPipeConnector(void)
{
    if(m_strType.compare("weld") == 0)
    {
        // 焊接头
        return createWeld();
    }
    else if(m_strType.compare("endblock") == 0)
    {
        // 盲板
        return createEndBlocker(m_vecJoints[0]);
    }
    else if(m_strType.compare("pipehat") == 0)
    {
        // 管帽
        return createPipeHat(m_vecJoints[0]);
    }

    // 普通的弯头
    return createNormalConnector();
}


osg::Node *PolygonPipeConnectorBuilder::createNormalConnector(void) const
{
    osg::ref_ptr<osg::Group>    pGroup = new osg::Group;

    for(unsigned i = 0u; i < m_vecJoints.size(); i++)
    {
        // 1. Create an Elbow 
        const Port &port0 = m_vecJoints[i];
        for(unsigned j = i + 1u; j < m_vecJoints.size(); j++)
        {
            const Port &port1 = m_vecJoints[j];
            osg::ref_ptr<osg::Geode>    pElbow = createElbow(port0, port1);
            pGroup->addChild(pElbow.get());
        }

        // 2. Create a Port
        //PolygonFace polygon;
        //polygon.m_vecVertices   = port0.m_vecVertices;
        //polygon.m_ptCenter      = port0.m_ptPosition;
        //polygon.m_vecNormal     = polygon.m_ptCenter - m_ptCorner;
        //polygon.m_vecNormal.normalize();

        //const double dblLength = (polygon.m_dblHeight + rect.m_dblWidth) * 0.25;
        //osg::ref_ptr<osg::Node> pNode = createHoop(rect, dblLength, vecWidthDir);
        //pGroup->addChild(pNode.get());
    }

    return pGroup.release();
}


osg::Geode *PolygonPipeConnectorBuilder::createHoop(const PolygonFace &rect, double dblLength, const osg::Vec3d &vecRecommendWidthDir) const
{
    return NULL;
}


osg::Drawable *PolygonPipeConnectorBuilder::createPolygonFace(const PolygonFace &rect, const osg::Vec3d &vecRecommendWidthDir) const
{
    return NULL;
}


osg::Node *PolygonPipeConnectorBuilder::createEndBlocker(const Port &port) const
{
    return NULL;
}


osg::Node *PolygonPipeConnectorBuilder::createPipeHat(const Port &port) const
{
    return NULL;
}


osg::Node *PolygonPipeConnectorBuilder::createWeld(void) const
{
    if(m_vecJoints.size() < 2u)
    {
        return NULL;
    }

    const double old = m_dblCornerRadiusRatio;
    const_cast<double &>(m_dblCornerRadiusRatio) = 1.0;

    Port port0 = m_vecJoints[0];
    port0.m_ptPosition = port0.m_ptPosition + m_ptCorner;

    Port port1 = m_vecJoints[1];
    port1.m_ptPosition = port1.m_ptPosition + m_ptCorner;

    osg::ref_ptr<osg::Node> pNode = createElbow(port0, port1);

    const_cast<double &>(m_dblCornerRadiusRatio) = old;
    return pNode.release();
}


osg::Geode *PolygonPipeConnectorBuilder::createElbow(const Port &port0, const Port &port1) const
{
    assert(port0.m_polygon.getVerticesCount() == port1.m_polygon.getVerticesCount());

    PolygonFace polygonBegin;
    polygonBegin.m_ptCenter  = port0.m_ptPosition;
    polygonBegin.m_vecNormal = m_ptCorner - port0.m_ptPosition;
    polygonBegin.m_vecNormal.normalize();
    for(unsigned i = 0u; i < port0.m_polygon.getVerticesCount(); i++)
    {
        const cmm::math::Point2d &vtx = port0.m_polygon.getSafeVertex(i);
        polygonBegin.m_vecVertices.push_back(osg::Vec3d(vtx.x(), vtx.y(), 0.0));
    }

    const cmm::math::Box2d bb0    = port0.m_polygon.getBound();
    const double dblPolygonSize0  = sqrt(bb0.width() * bb0.width() + bb0.height() * bb0.height());
    const double dblCornerRadius0 = dblPolygonSize0 * m_dblCornerRadiusRatio;
    const bool   bHasStraight0    = ((m_ptCorner - port0.m_ptPosition).length() > dblCornerRadius0);

    PolygonFace polygonEnd;
    polygonEnd.m_ptCenter  = port1.m_ptPosition;
    polygonEnd.m_vecNormal = port1.m_ptPosition - m_ptCorner;
    polygonEnd.m_vecNormal.normalize();
    for(unsigned i = 0u; i < port1.m_polygon.getVerticesCount(); i++)
    {
        const cmm::math::Point2d &vtx = port1.m_polygon.getSafeVertex(i);
        polygonEnd.m_vecVertices.push_back(osg::Vec3d(vtx.x(), vtx.y(), 0.0));
    }

    const cmm::math::Box2d bb1    = port1.m_polygon.getBound();
    const double dblPolygonSize1  = sqrt(bb1.width() * bb1.width() + bb1.height() * bb1.height());
    const double dblCornerRadius1 = dblPolygonSize1 * m_dblCornerRadiusRatio;
    const bool   bHasStraight1    = ((m_ptCorner - port1.m_ptPosition).length() > dblCornerRadius1);
    if(bHasStraight1)
    {
        polygonEnd.m_ptCenter = m_ptCorner + polygonEnd.m_vecNormal * dblCornerRadius1;
    }
    else
    {
        polygonEnd.m_ptCenter = port1.m_ptPosition;
    }

    PolygonFace polygonMid;
    polygonMid.m_vecNormal = (polygonBegin.m_vecNormal + polygonEnd.m_vecNormal) * 0.5;
    polygonMid.m_vecNormal.normalize();
    polygonMid.m_ptCenter  = m_ptCorner;
    for(unsigned i = 0u; i < port0.m_polygon.getVerticesCount(); i++)
    {
        const cmm::math::Point2d &vtx0 = port0.m_polygon.getSafeVertex(i);
        const cmm::math::Point2d &vtx1 = port1.m_polygon.getSafeVertex(i);
        const cmm::math::Point2d  vtx  = (vtx0 + vtx1) * 0.5;
        polygonMid.m_vecVertices.push_back(osg::Vec3d(vtx.x(), vtx.y(), 0.0));
    }

    std::vector<PolygonFace> vecFaces;

    const double  dblHint = osg::clampAbove(ceil(m_dblElbowHintApply), 1.0);
    const unsigned  nHint = (unsigned)dblHint;
    const double dblDelta = 1.0 / dblHint;
    vecFaces.reserve(nHint + 3u);

    // 1. Backup the straight seg of begin
    PolygonFace polygonBegin_BAK = polygonBegin;
    if(bHasStraight0)
    {
        polygonBegin.m_ptCenter = m_ptCorner - polygonBegin.m_vecNormal * dblCornerRadius0;
    }

    // 2. Create the elbow seg
    double t = 0.0;
    for(unsigned n = 0u; n <= nHint; n++)
    {
        const double one_minus_t  = 1.0 - t;
        const double one_minus_t2 = one_minus_t * one_minus_t;
        const double t_one_minus_t = one_minus_t * t * 2.0;
        const double t2 = t * t;

        PolygonFace polygon;
        polygon.m_ptCenter = (polygonBegin.m_ptCenter * one_minus_t2) + (polygonMid.m_ptCenter * t_one_minus_t) + (polygonEnd.m_ptCenter * t2);

        polygon.m_vecVertices.resize(polygonBegin.m_vecVertices.size());
        for(unsigned i = 0u; i < polygon.m_vecVertices.size(); i++)
        {
            const osg::Vec3d &vtx0 = polygonBegin.m_vecVertices[i];
            const osg::Vec3d &vtx1 = polygonMid.m_vecVertices[i];
            const osg::Vec3d &vtx2 = polygonEnd.m_vecVertices[i];
            polygon.m_vecVertices[i] = (vtx0 * one_minus_t2) + (vtx1 * t_one_minus_t) + (vtx2 * t2);
        }

        polygon.m_vecNormal = (polygonBegin.m_vecNormal * one_minus_t2) + (polygonMid.m_vecNormal * t_one_minus_t) + (polygonEnd.m_vecNormal * t2);
        polygon.m_vecNormal.normalize();

        vecFaces.push_back(polygon);

        t += dblDelta;
    }

    // 3. Create hte stright seg of end
    if(bHasStraight1)
    {
        polygonEnd.m_ptCenter = port1.m_ptPosition;
        vecFaces.push_back(polygonEnd);
    }

    // 4. Builder render node by faces
    osg::Vec3d  vecRightDir, vecFirstRightDir;
    osg::ref_ptr<osg::Geode>    pGeode = new osg::Geode;
    for(unsigned i = 0u; i < vecFaces.size() - 1u; i++)
    {
        const PolygonFace &face0 = vecFaces[i];
        const PolygonFace &face1 = vecFaces[i + 1u];

        osg::ref_ptr<osg::Drawable> pDrawable = createElbowSeg(face0, face1, vecRightDir);
        if(i == 0u)
        {
            vecFirstRightDir = vecRightDir;
        }

        pGeode->addDrawable(pDrawable.get());
    }

    // 5. Add the straight seg of begin
    if(bHasStraight0)
    {
        const PolygonFace &face0 = vecFaces[0];
        osg::ref_ptr<osg::Drawable> pDrawable = createElbowSeg(polygonBegin_BAK, face0, vecFirstRightDir);
        pGeode->addDrawable(pDrawable.get());
    }

    return pGeode.release();
}


osg::Drawable *PolygonPipeConnectorBuilder::createElbowSeg(const PolygonFace &face0, const PolygonFace &face1, osg::Vec3d &vecRightDir/* = osg::Vec3d()*/) const
{
    const osg::Vec3d vecUpDir(0.0, 0.0, 1.0);

    osg::Quat qtNormal0(vecUpDir, face0.m_vecNormal);
    osg::Quat qtNormal1(vecUpDir, face1.m_vecNormal);

    osg::Vec3d  vecRight0  = face0.m_vecNormal ^ vecUpDir;
    const bool  bParallel0 = (vecRight0.normalize() < 1e-8);
    osg::Vec3d  vecRight1  = face1.m_vecNormal ^ vecUpDir;
    const bool  bParallel1 = (vecRight1.normalize() < 1e-8);
    if(bParallel0 || bParallel1)
    {
        if(bParallel0 && bParallel1)
        {
            if(vecRightDir != osg::Vec3d())
            {
                vecRight0 = vecRightDir;
                vecRight1 = vecRightDir;
            }
            else
            {
                vecRight0.set(1.0, 0.0, 0.0);
                vecRight1.set(1.0, 0.0, 0.0);
            }
        }
        else if(bParallel0)
        {
            vecRight0 = vecRight1;

            double dblAngle;
            osg::Vec3d vecAxis;
            qtNormal1.getRotate(dblAngle, vecAxis);
            qtNormal0.makeRotate(dblAngle, vecAxis);
        }
        else if(bParallel1)
        {
            vecRight1 = vecRight0;

            double dblAngle;
            osg::Vec3d vecAxis;
            qtNormal0.getRotate(dblAngle, vecAxis);
            qtNormal1.makeRotate(dblAngle, vecAxis);
        }
    }
    vecRightDir = vecRight1;

    osg::Quat qtRight0;     qtRight0.makeRotate(osg::Vec3d(-1.0, 0.0, 0.0), vecRight0);
    osg::Quat qtRight1;     qtRight1.makeRotate(osg::Vec3d(-1.0, 0.0, 0.0), vecRight1);

    osg::ref_ptr<osg::Vec3Array>    pVtxArray = new osg::Vec3Array;
    pVtxArray->reserve(face0.m_vecVertices.size() + face1.m_vecVertices.size());
    for(unsigned i = 0u; i < face0.m_vecVertices.size(); i++)
    {
        osg::Vec3d vtx0 = face0.m_vecVertices[i];
        osg::Vec3d vtx1 = face1.m_vecVertices[i];

        vtx0  = qtRight0 * vtx0;
        vtx1  = qtRight1 * vtx1;

        vtx0  = qtNormal0 * vtx0;
        vtx1  = qtNormal1 * vtx1;

        vtx0 += face0.m_ptCenter;
        vtx1 += face1.m_ptCenter;

        pVtxArray->push_back(vtx0);
        pVtxArray->push_back(vtx1);
    }

    std::vector<unsigned>   vecIndicesArray;
    vecIndicesArray.reserve(pVtxArray->size() * 2u);
    for(unsigned i = 0u; i < face0.m_vecVertices.size() - 1u; i++)
    {
        const unsigned n = i + i;
        vecIndicesArray.push_back(n + 2u);
        vecIndicesArray.push_back(n + 3u);
        vecIndicesArray.push_back(n + 1u);
        vecIndicesArray.push_back(n + 0u);
    }

    osg::ref_ptr<osg::Vec3Array>    pNormalArray = new osg::Vec3Array;
    pNormalArray->reserve(face0.m_vecVertices.size());
    for(unsigned i = 0u; i < vecIndicesArray.size(); i += 4u)
    {
        const unsigned index0 = vecIndicesArray[i];
        const unsigned index1 = vecIndicesArray[i + 1u];
        const unsigned index2 = vecIndicesArray[i + 2u];
        const osg::Vec3 &vtx0 = pVtxArray->at(index0);
        const osg::Vec3 &vtx1 = pVtxArray->at(index1);
        const osg::Vec3 &vtx2 = pVtxArray->at(index2);

        osg::Vec3 vec1 = vtx1 - vtx0;
        osg::Vec3 vec2 = vtx2 - vtx0;

        osg::Vec3 vec  = vec1 ^ vec2;
        vec.normalize();
        pNormalArray->push_back(vec);
    }

    osg::ref_ptr<osg::Geometry>     pGeometry = new osg::Geometry;
    pGeometry->setVertexArray(pVtxArray.get());
    osg::ref_ptr<osg::UByteArray>   pVtxIndicesArray = new osg::UByteArray(vecIndicesArray.begin(), vecIndicesArray.end());
    pGeometry->setVertexIndices(pVtxIndicesArray.get());

    pGeometry->setNormalArray(pNormalArray.get());
    pGeometry->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);

    osg::ref_ptr<osg::Vec4Array>    pColorArray = new osg::Vec4Array;
    pColorArray->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    pGeometry->setColorArray(pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::DrawArrays>   pDrawArrays = new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0u, pVtxIndicesArray->size());
    pGeometry->addPrimitiveSet(pDrawArrays.get());
    return pGeometry.release();
}

