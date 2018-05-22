#include "RectPipeConnectorBuilder.h"
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
RectPipeConnectorBuilder::RectPipeConnectorBuilder(void)
{
    setElbowHint(0.50);
    m_dblCornerRadiusRatio = 0.8;
    m_pColorArray = new osg::Vec4Array;
    m_pColorArray->push_back(osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f));
}


RectPipeConnectorBuilder::~RectPipeConnectorBuilder(void)
{
}


void RectPipeConnectorBuilder::setType(const std::string &strType)
{
    m_strType.resize(strType.length());
    std::transform(strType.begin(), strType.end(), m_strType.begin(), ::tolower);
}


void RectPipeConnectorBuilder::setCorner(const osg::Vec3d &ptCorner)
{
    m_ptCorner = ptCorner;
}


void RectPipeConnectorBuilder::setColor(const osg::Vec4 &color)
{
    (*m_pColorArray)[0].set(color.r(), color.g(), color.b(), color.a());
}


void RectPipeConnectorBuilder::setElbowHint(double dblHint)
{
    m_dblElbowHint = osg::clampBetween(dblHint, 0.0, 1.0);

    m_dblElbowHintApply  = m_dblElbowHint * osg::PI_2;
    m_dblElbowHintApply  = osg::clampBelow(m_dblElbowHintApply, osg::PI_2 * 0.999);
    m_dblElbowHintApply  = tan(m_dblElbowHintApply);
    m_dblElbowHintApply *= 16.0;
}


void RectPipeConnectorBuilder::addPort(const osg::Vec3d &ptPosition, double dblWidth, double dblHeight)
{
    m_vecJoints.resize(m_vecJoints.size() + 1u);
    Port &port = m_vecJoints.back();
    port.m_ptPosition    = ptPosition;
    port.m_dblWidth      = dblWidth;
    port.m_dblHeight     = dblHeight;
}


void RectPipeConnectorBuilder::clearJoints(void)
{
    m_vecJoints.clear();
}


osg::Node *RectPipeConnectorBuilder::buildPipeConnector(void)
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


osg::Node *RectPipeConnectorBuilder::createNormalConnector(void) const
{
    osg::ref_ptr<osg::Group>    pGroup = new osg::Group;

    osg::Vec3d  vecWidthDir;
    for(unsigned i = 0u; i < m_vecJoints.size(); i++)
    {
        // 1. Create an Elbow 
        const Port &port0 = m_vecJoints[i];
        for(unsigned j = i + 1u; j < m_vecJoints.size(); j++)
        {
            const Port &port1 = m_vecJoints[j];
            osg::ref_ptr<osg::Geode>    pElbow = createElbow(port0, port1, vecWidthDir);
            pGroup->addChild(pElbow.get());
        }

        // 2. Create a Port
        RectFace rect;
        rect.m_dblWidth      = port0.m_dblWidth * 1.1;
        rect.m_dblHeight     = port0.m_dblHeight * 1.1;
        rect.m_ptCenter      = port0.m_ptPosition;
        rect.m_vecNormal     = rect.m_ptCenter - m_ptCorner;
        rect.m_vecNormal.normalize();

        const double dblLength = (rect.m_dblHeight + rect.m_dblWidth) * 0.25;
        osg::ref_ptr<osg::Node> pNode = createHoop(rect, dblLength, vecWidthDir);
        pGroup->addChild(pNode.get());
    }

    return pGroup.release();
}


osg::Geode *RectPipeConnectorBuilder::createHoop(const RectFace &rect, double dblLength, const osg::Vec3d &vecRecommendWidthDir) const
{
    RectFace rect0 = rect;
    RectFace rect1 = rect;
    rect1.m_ptCenter = rect.m_ptCenter + rect.m_vecNormal * dblLength;


    // 1. Create side of the hoop
    osg::Vec3d vecWidthDir = vecRecommendWidthDir;
    osg::ref_ptr<osg::Drawable> pSideGeom = createElbowSeg(rect0, rect1, vecWidthDir);


    // 2. create cover of the hoop
    rect0.m_vecNormal = -rect0.m_vecNormal;
    osg::ref_ptr<osg::Drawable> pTopGeom    = createRectFace(rect0, vecWidthDir);
    osg::ref_ptr<osg::Drawable> pBottomGeom = createRectFace(rect1, vecWidthDir);


    osg::ref_ptr<osg::Geode>    pGeode = new osg::Geode;
    pGeode->addDrawable(pSideGeom.get());
    pGeode->addDrawable(pTopGeom.get());
    pGeode->addDrawable(pBottomGeom.get());

    return pGeode.release();
}


osg::Drawable *RectPipeConnectorBuilder::createRectFace(const RectFace &rect, const osg::Vec3d &vecRecommendWidthDir) const
{
    const osg::Vec3d vecUpDir(0.0, 0.0, 1.0);
    const double dblDirectionX[4] = {-0.5, 0.5,  0.5, -0.5};
    const double dblDirectionY[4] = { 0.5, 0.5, -0.5, -0.5};

    osg::Vec3d  vecVertices[4];

    osg::Vec3d  vecWidth  = rect.m_vecNormal ^ vecUpDir;
    const bool  bParallel = (vecWidth.normalize() < 1e-8);
    if(bParallel)
    {
        vecWidth = vecRecommendWidthDir;
    }

    osg::Vec3d  vecHeight = vecWidth ^ rect.m_vecNormal;
    vecHeight.normalize();

    osg::ref_ptr<osg::Vec3Array>    pVtxArray = new osg::Vec3Array(4);
    for(unsigned i = 0u; i < 4u; i++)
    {
        (*pVtxArray)[i]  = vecWidth * rect.m_dblWidth * dblDirectionX[i] + vecHeight * rect.m_dblHeight * dblDirectionY[i];
        (*pVtxArray)[i] += rect.m_ptCenter;
    }

    osg::ref_ptr<osg::Vec3Array>    pNormalArray = new osg::Vec3Array;
    {
        const osg::Vec3d vec0 = pVtxArray->at(0) - pVtxArray->at(1);
        const osg::Vec3d vec1 = pVtxArray->at(2) - pVtxArray->at(1);
        osg::Vec3d vecNormal = vec0 ^ vec1;
        vecNormal.normalize();
        pNormalArray->push_back(vecNormal);
    }

    osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
    pGeometry->setVertexArray(pVtxArray.get());
    pGeometry->setNormalArray(pNormalArray.get());
    pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
    osg::ref_ptr<osg::DrawArrays>   pDrawArrays = new osg::DrawArrays(GL_QUADS, 0u, pVtxArray->size());
    pGeometry->addPrimitiveSet(pDrawArrays.get());
    return pGeometry.release();
}


osg::Node *RectPipeConnectorBuilder::createEndBlocker(const Port &port) const
{
    RectFace rect;
    rect.m_dblWidth = port.m_dblWidth * 1.1;
    rect.m_dblHeight = port.m_dblHeight * 1.1;
    rect.m_vecNormal = port.m_ptPosition - m_ptCorner;
    const double dblLen = rect.m_vecNormal.normalize();
    rect.m_ptCenter = m_ptCorner - rect.m_vecNormal * dblLen * 0.5;

    return createHoop(rect, dblLen);
}


osg::Node *RectPipeConnectorBuilder::createPipeHat(const Port &port) const
{
    osg::Vec3d vecNormal = port.m_ptPosition - m_ptCorner;
    const double dblLen = vecNormal.normalize();

    RectFace rect1;
    rect1.m_dblWidth      = port.m_dblWidth * 1.1;
    rect1.m_dblHeight     = port.m_dblHeight * 1.1;
    rect1.m_vecNormal     = vecNormal;
    rect1.m_ptCenter      = m_ptCorner - vecNormal * dblLen * 0.5;
    osg::ref_ptr<osg::Node> pPipeHatNode = createHoop(rect1, dblLen);

    RectFace rect2;
    rect2.m_dblWidth      = port.m_dblWidth * 1.1;
    rect2.m_dblHeight     = port.m_dblHeight * 1.1;
    rect2.m_vecNormal     = vecNormal;
    rect2.m_ptCenter      = rect1.m_ptCenter;
    const double dblSize  = (rect2.m_dblHeight + rect2.m_dblWidth) * 0.5;
    osg::ref_ptr<osg::Node> pHoopNode = createHoop(rect2, dblSize * 0.5);

    osg::ref_ptr<osg::Group>    pGroup = new osg::Group;
    pGroup->addChild(pPipeHatNode.get());
    pGroup->addChild(pHoopNode.get());
    return pGroup.release();
}


osg::Node *RectPipeConnectorBuilder::createWeld(void) const
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

    osg::Vec3d vecLastWidthDir;
    osg::ref_ptr<osg::Node> pNode = createElbow(port0, port1, vecLastWidthDir);

    const_cast<double &>(m_dblCornerRadiusRatio) = old;
    return pNode.release();
}


osg::Geode *RectPipeConnectorBuilder::createElbow(const Port &port0, const Port &port1, osg::Vec3d &vecLastWidthDir) const
{
    RectFace rectBegin;
    rectBegin.m_vecNormal     = m_ptCorner - port0.m_ptPosition;
    rectBegin.m_vecNormal.normalize();
    rectBegin.m_dblWidth      = port0.m_dblWidth;
    rectBegin.m_dblHeight     = port0.m_dblHeight;
    rectBegin.m_ptCenter      = port0.m_ptPosition;

    const double dblCornerRadius0 = std::max(port0.m_dblWidth, port0.m_dblHeight) * m_dblCornerRadiusRatio;
    const bool   bHasStraight0   = ((m_ptCorner - port0.m_ptPosition).length() > dblCornerRadius0);

    RectFace rectEnd;
    rectEnd.m_vecNormal      = port1.m_ptPosition - m_ptCorner;
    rectEnd.m_vecNormal.normalize();
    rectEnd.m_dblWidth       = port1.m_dblWidth;
    rectEnd.m_dblHeight      = port1.m_dblHeight;
    rectEnd.m_ptCenter       = port1.m_ptPosition;

    const double dblCornerRadius1 = std::max(port1.m_dblWidth, port1.m_dblHeight) * m_dblCornerRadiusRatio;
    const bool   bHasStraight1   = ((m_ptCorner - port1.m_ptPosition).length() > dblCornerRadius1);
    if(bHasStraight1)
    {
        rectEnd.m_ptCenter = m_ptCorner + rectEnd.m_vecNormal * dblCornerRadius1;
    }
    else
    {
        rectEnd.m_ptCenter = port1.m_ptPosition;
    }

    RectFace rectMid;
    rectMid.m_vecNormal = (rectBegin.m_vecNormal + rectEnd.m_vecNormal) * 0.5;
    rectMid.m_vecNormal.normalize();
    rectMid.m_dblWidth  = (rectBegin.m_dblWidth + rectEnd.m_dblWidth) * 0.5;
    rectMid.m_dblHeight = (rectBegin.m_dblHeight + rectEnd.m_dblHeight) * 0.5;
    rectMid.m_ptCenter  = m_ptCorner;

    const double  dblHint = osg::clampAbove(ceil(m_dblElbowHintApply), 1.0);
    const unsigned  nHint = (unsigned)dblHint;
    std::vector<RectFace> vecFaces;
    vecFaces.reserve(nHint + 3u);

    // 1. Backup the straight seg of begin
    RectFace rectBegin_BAK = rectBegin;
    if(bHasStraight0)
    {
        rectBegin.m_ptCenter = m_ptCorner - rectBegin.m_vecNormal * dblCornerRadius0;
    }

    // 2. Create the elbow seg
    const double dblDelta = 1.0 / dblHint;

    double t = 0.0;
    for(unsigned n = 0u; n <= nHint; n++)
    {
        const double one_minus_t  = 1.0 - t;
        const double one_minus_t2 = one_minus_t * one_minus_t;
        const double t_one_minus_t = one_minus_t * t * 2.0;
        const double t2 = t * t;

        RectFace rect;
        rect.m_ptCenter  = (rectBegin.m_ptCenter  * one_minus_t2) + (rectMid.m_ptCenter  * t_one_minus_t) + (rectEnd.m_ptCenter  * t2);
        rect.m_dblWidth  = (rectBegin.m_dblWidth  * one_minus_t2) + (rectMid.m_dblWidth  * t_one_minus_t) + (rectEnd.m_dblWidth  * t2);
        rect.m_dblHeight = (rectBegin.m_dblHeight * one_minus_t2) + (rectMid.m_dblHeight * t_one_minus_t) + (rectEnd.m_dblHeight * t2);
        rect.m_vecNormal = (rectBegin.m_vecNormal * one_minus_t2) + (rectMid.m_vecNormal * t_one_minus_t) + (rectEnd.m_vecNormal * t2);
        rect.m_vecNormal.normalize();

        vecFaces.push_back(rect);

        t += dblDelta;
    }

    // 3. Create hte stright seg of end
    if(bHasStraight1)
    {
        rectEnd.m_ptCenter = port1.m_ptPosition;
        vecFaces.push_back(rectEnd);
    }

    // 4. Builder render node by faces
    osg::Vec3d  vecWidthDir, vecFirstWidthDir;
    osg::ref_ptr<osg::Geode>    pGeode = new osg::Geode;
    for(unsigned i = 0u; i < vecFaces.size() - 1u; i++)
    {
        const RectFace &face0 = vecFaces[i];
        const RectFace &face1 = vecFaces[i + 1u];

        //if(i != 0u) continue;
        osg::ref_ptr<osg::Drawable> pDrawable = createElbowSeg(face0, face1, vecWidthDir);
        if(i == 0u)
        {
            vecFirstWidthDir = vecWidthDir;
        }

        pGeode->addDrawable(pDrawable.get());
    }

    // 5. Add the straight seg of begin
    if(bHasStraight0)
    {
        const RectFace &face0 = vecFaces[0];
        osg::ref_ptr<osg::Drawable> pDrawable = createElbowSeg(rectBegin_BAK, face0, vecFirstWidthDir);
        pGeode->addDrawable(pDrawable.get());
    }

    vecLastWidthDir = vecWidthDir;
    return pGeode.release();
}


osg::Drawable *RectPipeConnectorBuilder::createElbowSeg(const RectFace &rect0, const RectFace &rect1, osg::Vec3d &vecWidthDir/* = osg::Vec3d()*/) const
{
    const osg::Vec3d vecUpDir(0.0, 0.0, 1.0);
    const double dblDirectionX[4] = {-0.5,  0.5, 0.5, -0.5};
    const double dblDirectionY[4] = {-0.5, -0.5, 0.5,  0.5};

    osg::Vec3d  vecVertices0[4];
    osg::Vec3d  vecVertices1[4];

    osg::Vec3d  vecWidth0  = rect0.m_vecNormal ^ vecUpDir;
    const bool  bParallel0 = (vecWidth0.normalize() < 1e-8);
    osg::Vec3d  vecWidth1  = rect1.m_vecNormal ^ vecUpDir;
    const bool  bParallel1 = (vecWidth1.normalize() < 1e-8);
    if(bParallel0 || bParallel1)
    {
        if(bParallel0 && bParallel1)
        {
            if(vecWidthDir != osg::Vec3d())
            {
                vecWidth0 = vecWidthDir;
                vecWidth1 = vecWidthDir;
            }
            else
            {
                vecWidth0.set(1.0, 0.0, 0.0);
                vecWidth1.set(1.0, 0.0, 0.0);
            }
        }
        else if(bParallel0)
        {
            vecWidth0 = vecWidth1;
        }
        else if(bParallel1)
        {
            vecWidth1 = vecWidth0;
        }
    }
    vecWidthDir = vecWidth1;

    osg::Vec3d  vecHeight0 = vecWidth0 ^ rect0.m_vecNormal;
    vecHeight0.normalize();

    osg::Vec3d  vecHeight1 = vecWidth1 ^ rect1.m_vecNormal;
    vecHeight1.normalize();

    for(unsigned i = 0u; i < 4u; i++)
    {
        vecVertices0[i]  = vecWidth0 * rect0.m_dblWidth * dblDirectionX[i] + vecHeight0 * rect0.m_dblHeight * dblDirectionY[i];
        vecVertices1[i]  = vecWidth1 * rect1.m_dblWidth * dblDirectionX[i] + vecHeight1 * rect1.m_dblHeight * dblDirectionY[i];

        vecVertices0[i] += rect0.m_ptCenter;
        vecVertices1[i] += rect1.m_ptCenter;
    }

    osg::ref_ptr<osg::Vec3Array>    pVtxArray = new osg::Vec3Array;
    pVtxArray->reserve(8u);
    pVtxArray->push_back(vecVertices0[0]);
    pVtxArray->push_back(vecVertices1[0]);
    pVtxArray->push_back(vecVertices0[1]);
    pVtxArray->push_back(vecVertices1[1]);
    pVtxArray->push_back(vecVertices0[2]);
    pVtxArray->push_back(vecVertices1[2]);
    pVtxArray->push_back(vecVertices0[3]);
    pVtxArray->push_back(vecVertices1[3]);

    const unsigned nVtxIndicesArray[24u] = {0,1,3, 0,3,2, 2,3,5, 2,5,4, 4,5,7, 4,7,6, 6,7,1, 6,1,0};

    osg::ref_ptr<osg::Vec3Array>    pNormalArray = new osg::Vec3Array;
    pNormalArray->reserve(8u);
    for(unsigned i = 0u; i < 24u; i += 3u)
    {
        const unsigned index0 = nVtxIndicesArray[i];
        const unsigned index1 = nVtxIndicesArray[i + 1u];
        const unsigned index2 = nVtxIndicesArray[i + 2u];
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
    osg::ref_ptr<osg::UByteArray>   pVtxIndicesArray = new osg::UByteArray(nVtxIndicesArray, nVtxIndicesArray + 24u);
    pGeometry->setVertexIndices(pVtxIndicesArray.get());

    pGeometry->setNormalArray(pNormalArray.get());
    pGeometry->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);

    pGeometry->setColorArray(m_pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::DrawArrays>   pDrawArrays = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0u, pVtxIndicesArray->size());
    pGeometry->addPrimitiveSet(pDrawArrays.get());
    return pGeometry.release();
}

