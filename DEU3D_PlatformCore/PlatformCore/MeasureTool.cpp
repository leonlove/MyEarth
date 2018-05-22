#include "MeasureTool.h"

#include <stdlib.h>
#include <iomanip>
#include <sstream>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osgText/Text>
#include <osg/AutoTransform>
#include <osg/MatrixTransform>
#include <osg/LineStipple>
#include <Common/Common.h>
#include "Utility.h"
#include <osgUtil/CommonOSG.h>

MeasureTool::MeasureTool(const std::string &strName)
    : PolylineTool(strName)
{
    m_eMeasureType  = IMeasureTool::Spatial;
    m_bMap2Screen   = false;
}


MeasureTool::~MeasureTool(void)
{
}

void MeasureTool::setMeasureType(IMeasureTool::MeasureType eType)
{
    if(m_eMeasureType == eType) return;
    //m_pCurrentArtifactNode->removeChildren(0u, m_pCurrentArtifactNode->getNumChildren());
    m_eMeasureType = eType;
}

IMeasureTool::MeasureType MeasureTool::getMeasureType(void) const
{
    return m_eMeasureType;
}


void MeasureTool::clearArtifact(void)
{
    PointTool::clearArtifact();
    m_pCurrentNode = NULL;
}


bool MeasureTool::operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    return false;
}

bool MeasureTool::operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    const osgGA::GUIEventAdapter::EventType eEventType = ea.getEventType();
    if(eEventType != osgGA::GUIEventAdapter::PUSH)
    {
        if(m_pVertexArray->empty())
        {
            return false;
        }
    }

    osg::Vec3d vIntersect(0.0, 0.0, 0.0);
    osg::View *pView = dynamic_cast<osg::View *>(&aa);
    osg::Camera *pCamera = pView->getCamera();
    const osg::Vec2d ptMouse(ea.getXnormalized(), ea.getYnormalized());
    if(!computeIntersection(m_pOperationTargetNode, pCamera, ptMouse, vIntersect))
    {
        return false;
    }

    if(!m_pVertexArray->empty() || m_bAutoClear)
    {
        if(m_pCurrentNode.valid())
        {
            m_pCurrentNode->removeChildren(0u, m_pCurrentNode->getNumChildren());
        }
    }

    switch(eEventType)
    {
    case osgGA::GUIEventAdapter::PUSH:
        {
            const int nButton = ea.getButton();
            if(nButton == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                m_pVertexArray->push_back(vIntersect);

                if(m_pVertexArray->size() < 2u)
                {
                    return false;
                }


                osg::ref_ptr<osg::Node> pNode = createMeasureGeode(m_pVertexArray.get(), true, false);
                if(!m_pCurrentNode.valid())
                {
                    m_pCurrentNode = new osg::Group;
                    m_pCurrentArtifactNode->addChild(m_pCurrentNode.get());
                }
                m_pCurrentNode->addChild(pNode.get());

                return true;
            }

            else return false;
            break;
        }
    case osgGA::GUIEventAdapter::DOUBLECLICK:
        {
            if(m_pVertexArray->size() < 2u)
            {
                return false;
            }

            osg::ref_ptr<osg::Node> pNode = createMeasureGeode(m_pVertexArray.get(), true, true);
            if(!m_bAutoClear)
            {
                m_pCurrentNode->addChild(pNode.get());
                m_pCurrentNode = NULL;
                m_pVertexArray->clear();
                break;
            }
            m_pVertexArray->clear();
            return true;
        }
    case osgGA::GUIEventAdapter::MOVE:
    case osgGA::GUIEventAdapter::DRAG:
        {
            break;
        }
    default: return false;
    }

    if(m_pVertexArray.valid() && !m_pVertexArray->empty())
    {
        osg::ref_ptr<osg::Vec3dArray>   pVtxArray = new osg::Vec3dArray(*m_pVertexArray);
        pVtxArray->push_back(vIntersect);

        osg::ref_ptr<osg::Node> pNode = createMeasureGeode(pVtxArray.get(), false, false);
        if(!m_pCurrentNode.valid())
        {
            m_pCurrentNode = new osg::Group;
            m_pCurrentArtifactNode->addChild(m_pCurrentNode.get());
        }
        m_pCurrentNode->addChild(pNode.get());
    }

    return true;
}

osg::Group *MeasureTool::createMeasureGeode(const osg::Vec3dArray *pLinePoints, bool bNeedSend, bool bFinished)
{
    std::vector<double> vecLens;

    osg::ref_ptr<osg::Group> pGroup = new osg::Group;

    osg::ref_ptr<osgText::Font>     pFont          = osgText::readFontFile("SIMSUN.TTC");

    osg::ref_ptr<osg::LineWidth>    pLineWidth     = new osg::LineWidth;
    pLineWidth->setWidth(m_dblLineWidth);

    osg::ref_ptr<osg::Vec4Array>    pColorArray    = new osg::Vec4Array;
    pColorArray->push_back(osg::Vec4(m_clrLineColor.m_fltR, m_clrLineColor.m_fltG, m_clrLineColor.m_fltB, m_clrLineColor.m_fltA));

    unsigned int nSize = pLinePoints->size();
    for(unsigned int i = 0; i < nSize - 1; i++)
    {
        osg::Vec3d vPoint1 = (*pLinePoints)[i];
        osg::Vec3d vPoint2 = (*pLinePoints)[i + 1];

        osg::ref_ptr<osg::MatrixTransform> pMT          = new osg::MatrixTransform;
        osg::Matrix mtx;
        osg::ref_ptr<osg::Geode>        pLineGeo         = new osg::Geode;
        osg::ref_ptr<osg::Geode>        pTextGeo         = new osg::Geode;

        osg::ref_ptr<osg::Geometry>     pGeometry     = new osg::Geometry;

        osg::ref_ptr<osgText::Text>     pDisText       = new osgText::Text;
        pDisText->setDataVariance(osg::Object::DYNAMIC);
        pDisText->setFont(pFont.get());
        pDisText->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
        pDisText->setColor(osg::Vec4(0.0, 0.0, 1.0, 1.0));
        pDisText->setCharacterSize(32.0);
        pDisText->setAutoRotateToScreen(true);
        pDisText->setLayout(osgText::Text::LEFT_TO_RIGHT);
        pDisText->setAlignment(osgText::Text::CENTER_CENTER);

        osg::StateSet                   *pStateSet                   = pGeometry->getOrCreateStateSet();
        pStateSet->setAttribute(pLineWidth.get());

        pGeometry->setColorArray(pColorArray.get());
        pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

        switch(m_eMeasureType)
        {
        case IMeasureTool::Spatial:
            {
                osg::Vec3d vCenter = vPoint1 + vPoint2;
                vCenter *= 0.5;
                mtx.setTrans(vCenter);
                pMT->setMatrix(mtx);

                vPoint1 -= vCenter;
                vPoint2 -= vCenter;

                osg::ref_ptr<osg::Vec3dArray> pVertex = new osg::Vec3dArray;
                pVertex->push_back(vPoint1);
                pVertex->push_back(vPoint2);

                pGeometry->setVertexArray(pVertex.get());
                pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, pVertex->size()));

                osg::Vec3d vTemp = (*pVertex)[1] - (*pVertex)[0];

                double dblDistance = vTemp.length();

                vecLens.push_back(dblDistance);

                vTemp = ((*pVertex)[0] + (*pVertex)[1]) * 0.5;

                std::stringstream ss;
                ss << std::setprecision(2) << std::setiosflags(std::ios::fixed) << dblDistance << "m";
                //ss << std::setprecision(2) << dblDistance << "m";
                const std::wstring strStringW = cmm::ANSIToUnicode(ss.str());

                pDisText->setPosition(vTemp);
                pDisText->setText(osgText::String(strStringW.c_str()));

                pLineGeo->addDrawable(pDisText.get());
                pLineGeo->addDrawable(pGeometry.get());

                break;
            }
        case IMeasureTool::Horizontal:
            {
                osg::Vec3d vTemp1 = vPoint2 - vPoint1;
                double dblLen = vTemp1.length();
                vTemp1.normalize();
                osg::Vec3d vTemp2 = vPoint2;
                vTemp2.normalize();

                double dblTemp1 = vTemp1 * vTemp2;
                dblLen *= dblTemp1;
                osg::Vec3d vPoint3 = vPoint2 - vTemp2 * dblLen;

                osg::Vec3d vCenter = (vPoint1 + vPoint2 +vPoint3) / 3.0;

                mtx.setTrans(vCenter);
                pMT->setMatrix(mtx);

                vPoint1 -= vCenter;
                vPoint2 -= vCenter;
                vPoint3 -= vCenter;

                osg::Vec3d vTemp = vPoint3 - vPoint1;

                double dblDistance = vTemp.length();

                vecLens.push_back(dblDistance);

                vTemp = (vPoint3 + vPoint1) * 0.5;

                osg::ref_ptr<osg::Vec3dArray> pVertex = new osg::Vec3dArray;
                pVertex->push_back(vPoint1);
                pVertex->push_back(vPoint2);
                pGeometry->setVertexArray(pVertex);
                pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 2));

                std::stringstream ss;
                ss << std::setprecision(2) << std::setiosflags(std::ios::fixed) << dblDistance << "m";
                const std::wstring strStringW = cmm::ANSIToUnicode(ss.str());

                pDisText->setPosition(vTemp);
                pDisText->setText(osgText::String(strStringW.c_str()));

                pLineGeo->addDrawable(pGeometry.get());
                pLineGeo->addDrawable(pDisText.get());

                pGeometry = new osg::Geometry;

                osg::ref_ptr<osg::Vec3dArray> pNewArray = new osg::Vec3dArray;
                pNewArray->push_back(vPoint2);
                pNewArray->push_back(vPoint3);
                pNewArray->push_back(vPoint1);
                pGeometry->setVertexArray(pNewArray);
                pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 3));

                osg::ref_ptr<osg::Vec4Array> pClrArray = new osg::Vec4Array(1);
                (*pClrArray)[0].set(0.0, 1.0, 0.0, 1.0);
                pGeometry->setColorArray(pClrArray);
                pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

                osg::ref_ptr<osg::LineStipple> pLineStipple = new osg::LineStipple;
                pLineStipple->setFactor(3);
                pLineStipple->setPattern(0xAAAA);
                pGeometry->getOrCreateStateSet()->setAttributeAndModes(pLineStipple, osg::StateAttribute::ON);
                pLineGeo->addDrawable(pGeometry.get());

                break;
            }
        case IMeasureTool::Vertical:
            {
                osg::Vec3d vTemp1 = vPoint2 - vPoint1;
                double dblLen = vTemp1.length();
                vTemp1.normalize();
                osg::Vec3d vTemp2 = vPoint2;
                vTemp2.normalize();

                double dblTemp1 = vTemp1 * vTemp2;
                dblLen *= dblTemp1;
                osg::Vec3d vPoint3 = vPoint2 - vTemp2 * dblLen;

                osg::Vec3d vCenter = (vPoint1 + vPoint2 +vPoint3) / 3.0;

                mtx.setTrans(vCenter);
                pMT->setMatrix(mtx);

                vPoint1 -= vCenter;
                vPoint2 -= vCenter;
                vPoint3 -= vCenter;

                osg::Vec3d vTemp = vPoint3 - vPoint2;

                double dblDistance = vTemp.length();

                vecLens.push_back(dblDistance);

                vTemp = (vPoint3 + vPoint2) * 0.5;

                osg::ref_ptr<osg::Vec3dArray> pVertex = new osg::Vec3dArray;
                pVertex->push_back(vPoint1);
                pVertex->push_back(vPoint2);
                pGeometry->setVertexArray(pVertex);
                pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 2));

                std::stringstream ss;
                ss << std::setprecision(2) << std::setiosflags(std::ios::fixed) << dblDistance << "m";
                const std::wstring strStringW = cmm::ANSIToUnicode(ss.str());

                pDisText->setPosition(vTemp);
                pDisText->setText(osgText::String(strStringW.c_str()));

                pLineGeo->addDrawable(pGeometry.get());
                pLineGeo->addDrawable(pDisText.get());

                pGeometry = new osg::Geometry;

                osg::ref_ptr<osg::Vec3dArray> pNewArray = new osg::Vec3dArray;
                pNewArray->push_back(vPoint2);
                pNewArray->push_back(vPoint3);
                pNewArray->push_back(vPoint1);
                pGeometry->setVertexArray(pNewArray);
                pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 3));

                osg::ref_ptr<osg::Vec4Array> pClrArray = new osg::Vec4Array(1);
                (*pClrArray)[0].set(0.0, 1.0, 0.0, 1.0);
                pGeometry->setColorArray(pClrArray);
                pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

                osg::ref_ptr<osg::LineStipple> pLineStipple = new osg::LineStipple;
                pLineStipple->setFactor(3);
                pLineStipple->setPattern(0xAAAA);
                pGeometry->getOrCreateStateSet()->setAttributeAndModes(pLineStipple, osg::StateAttribute::ON);
                pLineGeo->addDrawable(pGeometry.get());

                break;
            }
        }

        pMT->addChild(pLineGeo);
        pGroup->addChild(pMT);
    }
    pGroup->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

    if(m_pEventAdapter && bNeedSend)
    {
        OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
        pEventObject->setAction(ea::ACTION_MEASURE_TOOL);

        pEventObject->putExtra("ToolName", m_strName);

        std::vector<double> vecPoints;
        vecPoints.reserve(m_pVertexArray->size() * 3u);
        osg::Vec3dArray::const_iterator itorVtx = m_pVertexArray->begin();
        for( ; itorVtx != m_pVertexArray->end(); ++itorVtx)
        {
            const osg::Vec3d point = osgUtil::convertWorld2Globe(*itorVtx);
            vecPoints.push_back(point.x());
            vecPoints.push_back(point.y());
            vecPoints.push_back(point.z());
        }
        pEventObject->putExtra("Vertices", vecPoints);
        pEventObject->putExtra("Distance", vecLens);
        pEventObject->putExtra("Finished", bFinished);
        m_pEventAdapter->sendBroadcast(pEventObject);
    }

    return pGroup.release();
}