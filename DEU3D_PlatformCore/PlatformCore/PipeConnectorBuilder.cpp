#include "PipeConnectorBuilder.h"
#include <assert.h>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/Quat>
#include <osg/Material>
#include <iostream>
#include <osg/ShapeDrawable>
#include <osg/Shape>
#include <osg/MatrixTransform>

PipeConnectorBuilder::PipeConnectorBuilder(void)
    : m_ptCorner(0.0, 0.0, 0.0)
{
    setPortHint(0.50);
    setElbowHint(0.50);
    m_dblCornerRadiusRatio = 0.8;
    m_pColorArray = new osg::Vec4Array;
    m_pColorArray->push_back(osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f));
}


PipeConnectorBuilder::~PipeConnectorBuilder(void)
{
}


void PipeConnectorBuilder::setPortHint(double dblHint)
{
    m_dblPortHint = osg::clampBetween(dblHint, 0.0, 1.0);

    m_dblPortHintApply  = m_dblPortHint * osg::PI_2;
    m_dblPortHintApply  = osg::clampBelow(m_dblPortHintApply, osg::PI_2 * 0.9999);
    m_dblPortHintApply  = tan(m_dblPortHintApply);
    m_dblPortHintApply *= 36.0;
}


void PipeConnectorBuilder::setElbowHint(double dblHint)
{
    m_dblElbowHint = osg::clampBetween(dblHint, 0.0, 1.0);

    m_dblElbowHintApply  = m_dblElbowHint * osg::PI_2;
    m_dblElbowHintApply  = osg::clampBelow(m_dblElbowHintApply, osg::PI_2 * 0.999);
    m_dblElbowHintApply  = tan(m_dblElbowHintApply);
    m_dblElbowHintApply *= 16.0;
}



void PipeConnectorBuilder::setCorner(const osg::Vec3d &ptCorner)
{
    m_ptCorner = ptCorner;
}


void PipeConnectorBuilder::addJoint(const osg::Vec3d &ptPort, double dblRadius)
{
    const Port port = {ptPort, dblRadius};
    m_vecJoints.push_back(port);
}


void PipeConnectorBuilder::clearJoints(void)
{
    m_vecJoints.clear();
}


void PipeConnectorBuilder::setColor(const osg::Vec4 &color)
{
    (*m_pColorArray)[0].set(color.r(), color.g(), color.b(), color.a());
}

osg::Node *PipeConnectorBuilder::createEndBlocker(const Port &port)const
{
    CircleFace circle;
    circle.m_dblRadius = port.m_dblRadius;
    circle.m_vecNormal = m_ptCorner - port.m_ptPosition;
    circle.m_vecNormal.normalize();
    circle.m_ptCenter  = m_ptCorner - circle.m_vecNormal * port.m_dblRadius * m_dblCornerRadiusRatio * 0.1;
    return createHoop(circle, 1.05, 0.8);
}

osg::Geode *PipeConnectorBuilder::createPipeHat(const Port &port)const
{
    osg::Vec3d normal = port.m_ptPosition - m_ptCorner;

    Port begin = port;
    begin.m_ptPosition = m_ptCorner + normal * port.m_dblRadius * m_dblCornerRadiusRatio;

    Port end = port;
    end.m_ptPosition = m_ptCorner - normal * port.m_dblRadius * m_dblCornerRadiusRatio * 0.01;

    return createElbow(begin, end);
}

osg::Node *PipeConnectorBuilder::createSpecial(void)
{
    osg::ref_ptr<osg::Group> pGroup = new osg::Group;

    if (m_vecJoints.size() == 1u)
    {
        if (m_strType == "endblock")//盲板
        {
            pGroup->addChild(createEndBlocker(m_vecJoints[0]));
        }
        else if(m_strType == "pipehat")//管帽
        {
            pGroup->addChild(createEndBlocker(m_vecJoints[0]));
            pGroup->addChild(createPipeHat(m_vecJoints[0]));
        }
        else
        {
            std::cout<<"错误：只有一个接头的连接器必须是盲板或者管帽！"<<std::endl;
            return NULL;
        }
    }
    else//焊头
    {
        double old = m_dblCornerRadiusRatio;
        m_dblCornerRadiusRatio = 1.0;

        Port port0 = m_vecJoints[0];
        port0.m_ptPosition = port0.m_ptPosition + m_ptCorner;

        Port port1 = m_vecJoints[1];
        port1.m_ptPosition = port1.m_ptPosition + m_ptCorner;

        osg::ref_ptr<osg::Node> pNode = createElbow(port0, port1);
        pGroup->addChild(pNode.get());

        m_dblCornerRadiusRatio = old;
    }   

    return pGroup.release();
}

osg::Node *PipeConnectorBuilder::buildPipeConnector(void)
{
    if(m_vecJoints.size() == 0u)
    {
        return NULL;
    }

    if (m_strType == "weld" && m_vecJoints.size() == 2u || m_vecJoints.size() == 1)
    {
        //盲板、管帽、焊头
        return createSpecial();
    }

    if(m_strType == "weld")
    {
        std::cout<<"错误：焊头的接头不是两个！"<<std::endl;
    }

    osg::ref_ptr<osg::Group> pGroup = new osg::Group;

    double dblCenterRadius = 0.0;
    std::vector<Port>::const_iterator itorPort0 = m_vecJoints.begin();
    for( ; itorPort0 != m_vecJoints.end(); ++itorPort0)
    {
        Port port0 = *itorPort0;
        port0.m_ptPosition = port0.m_ptPosition + m_ptCorner;

        // 1. Create an Elbow 
        std::vector<Port>::const_iterator itorPort1 = itorPort0 + 1u;
        for(; itorPort1 != m_vecJoints.end(); ++itorPort1)
        {
            Port port1 = *itorPort1;
            port1.m_ptPosition = port1.m_ptPosition + m_ptCorner;

            osg::ref_ptr<osg::Node> pNode = createElbow(port0, port1);
            pGroup->addChild(pNode.get());
        }

        // 2. Create a Port
        CircleFace circle;
        circle.m_dblRadius = port0.m_dblRadius;
        circle.m_ptCenter  = port0.m_ptPosition;
        circle.m_vecNormal = circle.m_ptCenter - m_ptCorner;
        circle.m_vecNormal.normalize();
        osg::ref_ptr<osg::Node> pNode = createHoop(circle);
        pGroup->addChild(pNode.get());
    }

    return pGroup.release();
}


osg::Geode *PipeConnectorBuilder::createElbow(const Port &port0, const Port &port1) const
{
    CircleFace circleBegin;
    circleBegin.m_vecNormal = m_ptCorner - port0.m_ptPosition;
    circleBegin.m_vecNormal.normalize();
    circleBegin.m_dblRadius = port0.m_dblRadius;
    circleBegin.m_ptCenter  = port0.m_ptPosition;

    const double dblCornerRadius0 = port0.m_dblRadius * m_dblCornerRadiusRatio;
    const bool   bHasStraight0    = ((m_ptCorner - port0.m_ptPosition).length() > dblCornerRadius0);

    CircleFace circleEnd;
    circleEnd.m_vecNormal = port1.m_ptPosition - m_ptCorner;
    circleEnd.m_vecNormal.normalize();
    circleEnd.m_dblRadius = port1.m_dblRadius;
    const double dblCornerRadius1 = port1.m_dblRadius * m_dblCornerRadiusRatio;
    const bool   bHasStraight1    = ((m_ptCorner - port1.m_ptPosition).length() > dblCornerRadius1);
    if(bHasStraight1)
    {
        circleEnd.m_ptCenter = m_ptCorner + circleEnd.m_vecNormal * dblCornerRadius1;
    }
    else
    {
        circleEnd.m_ptCenter = port1.m_ptPosition;
    }

    const double dblCosConerAngle = osg::clampBetween(circleBegin.m_vecNormal * -circleEnd.m_vecNormal, -1.0, 1.0);
    const double dblConerAngle    = acos(dblCosConerAngle);
    CircleFace circleMid;
    circleMid.m_vecNormal  = (circleBegin.m_vecNormal + circleEnd.m_vecNormal) * 0.5;
    circleMid.m_vecNormal.normalize();
    circleMid.m_dblRadius  = (circleBegin.m_dblRadius + circleEnd.m_dblRadius) * 0.5;
    circleMid.m_dblRadius *= 1.0 + 0.2 * (-dblConerAngle + osg::PI);    // 对于弯头，拐弯的时候半径稍微扩大一点，这样看起来效果好一些
    circleMid.m_ptCenter   = m_ptCorner;

    std::vector<CircleFace> vecFaces;

    // 1. Create the straight seg of begin
    if(bHasStraight0)
    {
        vecFaces.push_back(circleBegin);
        circleBegin.m_ptCenter = m_ptCorner - circleBegin.m_vecNormal * dblCornerRadius0;
    }

    // 2. Create the elbow seg
    const double  dblHint = osg::clampAbove(ceil(m_dblElbowHintApply), 1.0);
    const unsigned  nHint = (unsigned)dblHint;
    const double dblDelta = 1.0 / dblHint;
    double t = 0.0;
    for(unsigned n = 0u; n <= nHint; n++)
    {
        const double one_minus_t  = 1.0 - t;
        const double one_minus_t2 = one_minus_t * one_minus_t;
        const double t_one_minus_t = one_minus_t * t * 2.0;
        const double t2 = t * t;
        CircleFace circle;
        circle.m_dblRadius = circleBegin.m_dblRadius * one_minus_t2 + circleMid.m_dblRadius * t_one_minus_t + circleEnd.m_dblRadius * t2;
        circle.m_ptCenter  = circleBegin.m_ptCenter  * one_minus_t2 + circleMid.m_ptCenter  * t_one_minus_t + circleEnd.m_ptCenter  * t2;
        circle.m_vecNormal = circleBegin.m_vecNormal * one_minus_t2 + circleMid.m_vecNormal * t_one_minus_t + circleEnd.m_vecNormal * t2;
        circle.m_vecNormal.normalize();
        vecFaces.push_back(circle);

        t += dblDelta;
    }

    // 3. Create hte stright seg of end
    if(bHasStraight1)
    {
        circleEnd.m_ptCenter = port1.m_ptPosition;
        vecFaces.push_back(circleEnd);
    }

    // 4. Builder render node by faces
    osg::ref_ptr<osg::Geode>    pGeode = new osg::Geode;
    std::vector<CircleFace>::const_iterator itor0 = vecFaces.begin();
    std::vector<CircleFace>::const_iterator itor1 = vecFaces.begin() + 1u;
    for( ; itor1 != vecFaces.end(); ++itor0, ++itor1)
    {
        const CircleFace &face0 = *itor0;
        const CircleFace &face1 = *itor1;
        osg::ref_ptr<osg::Drawable> pPipeSeg = createElbowSeg(face0, face1);
        pGeode->addDrawable(pPipeSeg.get());
    }

    return pGeode.release();
}


osg::Geode *PipeConnectorBuilder::createHoop(const CircleFace &circle, double dbRadiusRatio, double lengthRatio) const
{
    CircleFace circle0, circle1;
    circle0.m_dblRadius = circle.m_dblRadius * dbRadiusRatio;
    circle0.m_ptCenter  = circle.m_ptCenter;
    circle0.m_vecNormal = circle.m_vecNormal;

    circle1.m_dblRadius = circle.m_dblRadius * dbRadiusRatio;
    circle1.m_ptCenter  = circle.m_ptCenter + circle.m_vecNormal * circle.m_dblRadius * lengthRatio;
    circle1.m_vecNormal = circle.m_vecNormal;


    // 1. Create side of the hoop
    osg::ref_ptr<osg::Drawable> pSideGeom = createElbowSeg(circle0, circle1);


    // 2. create cover of the hoop
    circle0.m_vecNormal = -circle0.m_vecNormal;
    osg::ref_ptr<osg::Drawable> pTopGeom    = createCircleFace(circle0);

    osg::ref_ptr<osg::Drawable> pBottomGeom = createCircleFace(circle1);


    osg::ref_ptr<osg::Geode>    pGeode = new osg::Geode;
    pGeode->addDrawable(pSideGeom.get());
    pGeode->addDrawable(pTopGeom.get());
    pGeode->addDrawable(pBottomGeom.get());

    return pGeode.release();
}


osg::Drawable *PipeConnectorBuilder::createElbowSeg(const CircleFace &circle0, const CircleFace &circle1) const
{
    const double dblHint        = osg::clampAbove(ceil(m_dblPortHintApply), 4.0);
    const double dblAngleDelta  = osg::PI * 2.0 / dblHint;
    const unsigned nHint        = (unsigned)dblHint;

    const osg::Vec3d vecOrgNormal(0.0, 0.0, 1.0);
    const osg::Vec3d vecMidNormal(0.0, 1.0, 0.0);

    osg::Quat   qt0, qt0_Half0, qt0_Half1;
    osg::Quat   qt1, qt1_Half0, qt1_Half1;
    bool bRotateTwice0 = false;
    bool bRotateTwice1 = false;
    if(circle0.m_vecNormal * vecOrgNormal < -0.99)
    {
        bRotateTwice0 = true;
        qt0_Half0.makeRotate(vecOrgNormal, vecMidNormal);
        qt0_Half1.makeRotate(vecMidNormal, circle0.m_vecNormal);
    }
    else
    {
        qt0.makeRotate(vecOrgNormal, circle0.m_vecNormal);
    }

    if(circle1.m_vecNormal * vecOrgNormal < -0.99)
    {
        bRotateTwice1 = true;
        qt1_Half0.makeRotate(vecOrgNormal, vecMidNormal);
        qt1_Half1.makeRotate(vecMidNormal, circle1.m_vecNormal);
    }
    else
    {
        qt1.makeRotate(vecOrgNormal, circle1.m_vecNormal);
    }

    std::vector<osg::Vec3d>  vecVertices0, vecVertices1;
    std::vector<osg::Vec3d>  vecNormals0, vecNormals1;
    double dblAngle = 0.0;
    for(unsigned n = 0u; n < nHint; n++)
    {
        const double dblCos = cos(dblAngle);
        const double dblSin = sin(dblAngle);

        osg::Vec3d  vtx0(circle0.m_dblRadius * dblCos, circle0.m_dblRadius * dblSin, 0.0);
        if(bRotateTwice0)
        {
            vtx0 = qt0_Half1 * (qt0_Half0 * vtx0);
        }
        else
        {
            vtx0  = qt0 * vtx0;
        }

        osg::Vec3d  normal0(vtx0);
        normal0.normalize();
        vecNormals0.push_back(normal0);

        vtx0 += circle0.m_ptCenter;
        vecVertices0.push_back(vtx0);


        osg::Vec3d  vtx1(circle1.m_dblRadius * dblCos, circle1.m_dblRadius * dblSin, 0.0);
        if(bRotateTwice1)
        {
            vtx1 = qt1_Half1 * (qt1_Half0 * vtx1);
        }
        else
        {
            vtx1 = qt1 * vtx1;
        }
        osg::Vec3d  normal1(vtx1);
        normal1.normalize();
        vecNormals1.push_back(normal1);

        vtx1 += circle1.m_ptCenter;
        vecVertices1.push_back(vtx1);


        dblAngle += dblAngleDelta;
    }

    std::vector<osg::Vec3d>::const_iterator itorVtx0 = vecVertices0.begin();
    std::vector<osg::Vec3d>::const_iterator itorVtx1 = vecVertices1.begin();
    if(bRotateTwice0 != bRotateTwice1)
    {
        const osg::Vec3d &point0 = vecVertices0.front();
        double dblDistance = FLT_MAX;
        std::vector<osg::Vec3d>::const_iterator itorFind = vecVertices1.begin();
        for( ; itorFind != vecVertices1.end(); ++itorFind)
        {
            const osg::Vec3d &point1 = *itorFind;
            const double dbl = (point0 - point1).length2();
            if(dbl < dblDistance)
            {
                dblDistance = dbl;
                itorVtx1 = itorFind;
            }
        }
    }

    const unsigned nOffset = itorVtx1 - vecVertices1.begin();

    std::vector<osg::Vec3d>::const_iterator itorNormal0 = vecNormals0.begin();
    std::vector<osg::Vec3d>::const_iterator itorNormal1 = vecNormals1.begin() + nOffset;
    osg::ref_ptr<osg::Vec3Array>   pVtxArray = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array>   pNormalArray = new osg::Vec3Array;
    for( ; itorVtx0 != vecVertices0.end(); ++itorVtx0, ++itorNormal0)
    {
        const osg::Vec3d &vtx0 = *itorVtx0;
        const osg::Vec3d &vtx1 = *itorVtx1;
        pVtxArray->push_back(vtx0);
        pVtxArray->push_back(vtx1);

        const osg::Vec3d &normal0 = *itorNormal0;
        const osg::Vec3d &normal1 = *itorNormal1;
        pNormalArray->push_back(normal0);
        pNormalArray->push_back(normal1);

        if(++itorVtx1    == vecVertices1.end()) itorVtx1    = vecVertices1.begin();
        if(++itorNormal1 == vecNormals1.end())  itorNormal1 = vecNormals1.begin();
    }

    pVtxArray->push_back(pVtxArray->at(0));
    pVtxArray->push_back(pVtxArray->at(1));
    pNormalArray->push_back(pNormalArray->at(0));
    pNormalArray->push_back(pNormalArray->at(1));

    osg::ref_ptr<osg::Geometry>     pGeometry = new osg::Geometry;
    pGeometry->setVertexArray(pVtxArray.get());

    pGeometry->setNormalArray(pNormalArray.get());
    pGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    pGeometry->setColorArray(m_pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::DrawArrays>   pDrawArrays = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, pVtxArray->size());
    pGeometry->addPrimitiveSet(pDrawArrays.get());

    return pGeometry.release();
}


osg::Drawable *PipeConnectorBuilder::createCircleFace(const CircleFace &circle) const
{
    osg::ref_ptr<osg::Vec3Array>   pVtxArray = new osg::Vec3Array;
    pVtxArray->push_back(circle.m_ptCenter);

    const double dblHint        = osg::clampAbove(ceil(m_dblPortHintApply), 4.0);
    const double dblAngleDelta  = osg::PI * 2.0 / dblHint;
    const unsigned nHint        = (unsigned)dblHint;

    const osg::Vec3d vecOrgNormal(0.0, 0.0, 1.0);
    const osg::Vec3d vecOrgMidNormal(0.0, 1.0, 0.0);

    osg::Quat   qt, qt_Half0, qt_Half1;
    bool bRotateTwice = false;
    if(circle.m_vecNormal * vecOrgNormal < -0.99)
    {
        bRotateTwice = true;
        qt_Half0.makeRotate(osg::Vec3d(vecOrgNormal), vecOrgMidNormal);
        qt_Half1.makeRotate(vecOrgMidNormal, circle.m_vecNormal);
    }
    else
    {
        qt.makeRotate(vecOrgNormal, circle.m_vecNormal);
    }


    double dblAngle = 0.0;
    for(unsigned n = 0u; n <= nHint; n++)
    {
        const double dblCos = cos(dblAngle);
        const double dblSin = sin(dblAngle);

        osg::Vec3d  vtx(circle.m_dblRadius * dblCos, circle.m_dblRadius * dblSin, 0.0);
        if(bRotateTwice)
        {
            vtx = qt_Half1 * (qt_Half0 * vtx);
        }
        else
        {
            vtx  = qt * vtx;
        }

        vtx += circle.m_ptCenter;
        pVtxArray->push_back(vtx);

        dblAngle += dblAngleDelta;
    }

    osg::ref_ptr<osg::Geometry>     pGeometry = new osg::Geometry;
    pGeometry->setVertexArray(pVtxArray.get());

    osg::ref_ptr<osg::Vec3Array>   pNormalArray = new osg::Vec3Array;
    pNormalArray->push_back(circle.m_vecNormal);
    pGeometry->setNormalArray(pNormalArray.get());
    pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

    pGeometry->setColorArray(m_pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::DrawArrays>   pDrawArrays = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, 0, pVtxArray->size());
    pGeometry->addPrimitiveSet(pDrawArrays.get());

    return pGeometry.release();
}

