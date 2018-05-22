#include "LineTool.h"
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgViewer/View>
#include "Utility.h"
#include <osgUtil/CommonOSG.h>

LineTool::LineTool(const std::string &strName)
    : PointTool(strName)
{
    m_dblPointSize = 1.0;
    m_bDrawPoint = false;

    m_dblLineWidth = 1.0;
    m_clrLineColor.m_fltA = 1.0;
    m_clrLineColor.m_fltR = 1.0;
    m_clrLineColor.m_fltG = 1.0;
    m_clrLineColor.m_fltB = 1.0;

    m_bMap2Screen = false;
    m_pCurrentArtifactNode = m_pArtifactOnTerrainNode;
    m_pToolNode->addChild(m_pCurrentArtifactNode);
}


LineTool::~LineTool(void)
{
}


void LineTool::setLineWidth(double dblPointlSize)
{
    m_dblLineWidth = osg::clampAbove(dblPointlSize, 1.0);
}


void LineTool::setLineColor(const cmm::FloatColor &color)
{
    m_clrLineColor = color;
    m_clrLineColor.m_fltA = 1.0;
}


osg::Node *LineTool::createLineNode(const osg::Vec3dArray *pLinePoints)
{
    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
    osg::ref_ptr<osg::Vec3dArray> pVertex = new osg::Vec3dArray;
    osg::Vec3d vCenter;
    for(unsigned int i = 0; i < pLinePoints->size(); i++)
    {
        pVertex->push_back((*pLinePoints)[i]);
        vCenter += (*pLinePoints)[i];
    }

    vCenter /= (double)pVertex->size();

    for(unsigned int i = 0; i < pVertex->size(); i++)
    {
        (*pVertex)[i] -= vCenter;
    }

    osg::Matrix matrix;
    matrix.setTrans(vCenter);
    pMatrixTransform->setMatrix(matrix);

    osg::ref_ptr<osg::Geode>     pGeode      = new osg::Geode;
    osg::ref_ptr<osg::Geometry>  pGeometry   = new osg::Geometry;
    osg::ref_ptr<osg::LineWidth> pLineWidth  = new osg::LineWidth(m_dblLineWidth);
    osg::ref_ptr<osg::Depth>     pDepth      = new osg::Depth(osg::Depth::ALWAYS);
    osg::ref_ptr<osg::Vec4Array> pColorArray = new osg::Vec4Array;
    osg::StateSet *pStateSet = pGeometry->getOrCreateStateSet();

    pColorArray->push_back(osg::Vec4(m_clrLineColor.m_fltR, m_clrLineColor.m_fltG, m_clrLineColor.m_fltB, m_clrLineColor.m_fltA));

    pGeometry->setVertexArray(pVertex);
    pGeometry->setColorArray(pColorArray.get());
    pStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    pStateSet->setAttribute(pLineWidth.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    pStateSet->setAttribute(pDepth.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, pLinePoints->size()));
    pGeode->addDrawable(pGeometry.get());

    pMatrixTransform->addChild(pGeode);
    return pMatrixTransform.release();
}


bool LineTool::operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    const osgGA::GUIEventAdapter::EventType eEventType = ea.getEventType();
    if(eEventType != osgGA::GUIEventAdapter::PUSH && !m_bLButtonDown)
    {
        return false;
    }

    m_pCurrentArtifactNode->removeChildren(0u, m_pCurrentArtifactNode->getNumChildren());

    const osg::Vec3d ptMouse(ea.getX(), ea.getY(), 0.0);
    switch(eEventType)
    {
        case osgGA::GUIEventAdapter::PUSH:
        {
            if(ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                return false;
            }
            m_bLButtonDown = true;
            if(m_pVertexArray->empty())
            {
                m_pVertexArray->push_back(ptMouse);
                m_pVertexArray->push_back(ptMouse);
            }
            else
            {
                m_pVertexArray->clear();
            }
            break;
        }
        case osgGA::GUIEventAdapter::DRAG:
        case osgGA::GUIEventAdapter::MOVE:
        {
            if(!m_bLButtonDown)     return false;
            if(m_pVertexArray->size() != 2u)
            {
                return false;
            }

            m_pVertexArray->back() = ptMouse;
            break;
        }
        case osgGA::GUIEventAdapter::RELEASE:
        {
            if(ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                return false;
            }
            m_bLButtonDown = false;
            if(m_pVertexArray->size() != 2u)    return false;
            m_pVertexArray->back() = ptMouse;

            // send event
            if(m_pEventAdapter)
            {
                OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
                pEventObject->setAction(ea::ACTION_LINE_TOOL);

                pEventObject->putExtra("ToolName", m_strName);

                const osg::Vec3d &point0 = m_pVertexArray->front();
                const osg::Vec3d &point1 = m_pVertexArray->back();
                std::vector<double> vecPointList;
                vecPointList.push_back(point0.x());
                vecPointList.push_back(point0.y());
                vecPointList.push_back(point0.z());
                vecPointList.push_back(point1.x());
                vecPointList.push_back(point1.y());
                vecPointList.push_back(point1.z());
                pEventObject->putExtra("Vertices", vecPointList);
                m_pEventAdapter->sendBroadcast(pEventObject);
            }
            m_pVertexArray->clear();
            return true;
        }
        default:    return false;
    }

    if(m_pVertexArray.valid() && !m_pVertexArray->empty())
    {
        osg::ref_ptr<osg::Node> pLineNode = createLineNode(m_pVertexArray.get());
        m_pCurrentArtifactNode->addChild(pLineNode.get());

        if(m_bDrawPoint)
        {
            osg::ref_ptr<osg::Node> pPointNode = createPointNode(m_pVertexArray.get());
            m_pCurrentArtifactNode->addChild(pPointNode.get());
        }
    }

    return true;
}


bool LineTool::operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    const osgGA::GUIEventAdapter::EventType eEventType = ea.getEventType();
    if(eEventType != osgGA::GUIEventAdapter::PUSH && !m_bLButtonDown)
    {
        return false;
    }

    osg::Vec3d vIntersect(0.0, 0.0, 0.0);
    osg::View *pView = dynamic_cast<osg::View *>(&aa);
    osg::Camera *pCamera = pView->getCamera();
    const osg::Vec2d ptMouse(ea.getXnormalized(), ea.getYnormalized());
    if(!computeIntersection(m_pOperationTargetNode, pCamera, ptMouse, vIntersect))
    {
        return false;
    }

    m_pCurrentArtifactNode->removeChildren(0u, m_pCurrentArtifactNode->getNumChildren());

    switch(eEventType)
    {
        case osgGA::GUIEventAdapter::PUSH:
        {
            if(ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                return false;
            }
            m_bLButtonDown = true;
            m_pVertexArray->clear();
            m_pVertexArray->push_back(vIntersect);
            m_pVertexArray->push_back(vIntersect);
            break;
        }
        case osgGA::GUIEventAdapter::DRAG:
        case osgGA::GUIEventAdapter::MOVE:
        {
            if(!m_bLButtonDown)     return false;
            if(m_pVertexArray->size() != 2u)
            {
                return false;
            }

            m_pVertexArray->back() = vIntersect;
            break;
        }
        case osgGA::GUIEventAdapter::RELEASE:
        {
            if(ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                return false;
            }
            m_bLButtonDown = false;
            if(m_pVertexArray->size() != 2u)    return false;
            m_pVertexArray->back() = vIntersect;

            // send event
            if(m_pEventAdapter)
            {
                OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
                pEventObject->setAction(ea::ACTION_LINE_TOOL);

                pEventObject->putExtra("ToolName", m_strName);

                const osg::Vec3d point0 = osgUtil::convertWorld2Globe(m_pVertexArray->front());
                const osg::Vec3d point1 = osgUtil::convertWorld2Globe(m_pVertexArray->back());
                std::vector<double> vecPointList;
                vecPointList.push_back(point0.x());
                vecPointList.push_back(point0.y());
                vecPointList.push_back(point0.z());
                vecPointList.push_back(point1.x());
                vecPointList.push_back(point1.y());
                vecPointList.push_back(point1.z());
                pEventObject->putExtra("Vertices", vecPointList);
                m_pEventAdapter->sendBroadcast(pEventObject);
            }
            return true;
        }
        default:    return false;
    }

    osg::ref_ptr<osg::Node> pLineNode = createLineNode(m_pVertexArray.get());
    m_pCurrentArtifactNode->addChild(pLineNode.get());

    if(m_bDrawPoint)
    {
        osg::ref_ptr<osg::Node> pPointNode = createPointNode(m_pVertexArray.get());
        m_pCurrentArtifactNode->addChild(pPointNode.get());
    }

    return true;
}

