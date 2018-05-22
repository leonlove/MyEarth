#include <osg/Math>
#include <osg/Point>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgUtil/CommonOSG.h>
#include "PointTool.h"
#include "Utility.h"

#include <EventAdapter/IEventObject.h>

PointTool::PointTool(const std::string &strName)
    : ToolBase(strName)
{
    m_dblPointSize  = 5.0f;
    m_clrPointColor.m_fltA = 1.0f;
    m_clrPointColor.m_fltR = 1.0f;
    m_clrPointColor.m_fltG = 1.0f;
    m_clrPointColor.m_fltA = 1.0f;

    m_bDrawPoint = true;

    m_pVertexArray = new osg::Vec3dArray;
    m_pVertexArray->reserve(4u);

    m_bMap2Screen = true;
    m_pCurrentArtifactNode = m_pArtifactOnScreenNode;
    m_pToolNode->addChild(m_pCurrentArtifactNode);
}


PointTool::~PointTool(void)
{
}


void PointTool::clearArtifact(void)
{
    ToolBase::clearArtifact();
    m_pVertexArray->clear();
}


void PointTool::setPointSize(double dblPointlSize)
{
    m_dblPointSize = osg::clampAbove(dblPointlSize, 1.0);
}


void PointTool::setPointColor(const cmm::FloatColor &color)
{
    m_clrPointColor = color;
    m_clrPointColor.m_fltA = 1.0;
}


void PointTool::onDeactive(void)
{
    //ToolBase::onDeactive();
    m_pVertexArray->clear();
}


osg::Node *PointTool::createPointNode(const osg::Vec3dArray *pVertexArray)
{
    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
    osg::ref_ptr<osg::Vec3dArray> pVertex = new osg::Vec3dArray;
    osg::Vec3d vCenter;
    for(unsigned int i = 0; i < pVertexArray->size(); i++)
    {
        pVertex->push_back((*pVertexArray)[i]);
        vCenter += (*pVertexArray)[i];
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
    osg::ref_ptr<osg::Vec4Array> pColorArray = new osg::Vec4Array;
    osg::ref_ptr<osg::Point>     pPointSize  = new osg::Point(m_dblPointSize);
    osg::StateSet *pStateSet                 = pGeometry->getOrCreateStateSet();

    pGeometry->setVertexArray(pVertex);
    pColorArray->push_back(osg::Vec4(m_clrPointColor.m_fltR, m_clrPointColor.m_fltG, m_clrPointColor.m_fltB, m_clrPointColor.m_fltA));
    pGeometry->setColorArray(pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, pVertexArray->size()));
    pPointSize->setSize(m_dblPointSize);
    pStateSet->setAttribute(pPointSize);
    pStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    pGeode->addDrawable(pGeometry.get());

    pMatrixTransform->addChild(pGeode);
    return pMatrixTransform.release();
}


bool PointTool::operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    const osgGA::GUIEventAdapter::EventType eEventType = ea.getEventType();
    if(eEventType != osgGA::GUIEventAdapter::PUSH)
    {
        if(m_pVertexArray->empty()) return false;
        if(!m_bLButtonDown)         return false;
    }

    if(m_bDrawPoint)
    {
        m_pCurrentArtifactNode->removeChildren(0u, m_pCurrentArtifactNode->getNumChildren());
    }

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
            m_pVertexArray->clear();
            m_pVertexArray->push_back(ptMouse);
            break;
        }
        case osgGA::GUIEventAdapter::DRAG:
        case osgGA::GUIEventAdapter::MOVE:
        {
            if(!m_bLButtonDown)     return false;
            if(m_pVertexArray->size() != 1u)
            {
                return false;
            }

            m_pVertexArray->at(0) = ptMouse;
            break;
        }
        case osgGA::GUIEventAdapter::RELEASE:
        {
            if(ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                return false;
            }

            m_bLButtonDown = false;
            if(m_pVertexArray->size() != 1u)    return false;
            m_pVertexArray->at(0) = ptMouse;

            // send event
            if(m_pEventAdapter)
            {
                OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
                pEventObject->setAction(ea::ACTION_POINT_TOOL);

                pEventObject->putExtra("ToolName", m_strName);

                const osg::Vec3d &point = m_pVertexArray->front();
                std::vector<double> vecPointList;
                vecPointList.clear();
                vecPointList.push_back(point.x());
                vecPointList.push_back(point.y());
                vecPointList.push_back(point.z());
                pEventObject->putExtra("Vertices", vecPointList);
                m_pEventAdapter->sendBroadcast(pEventObject);
            }
            return true;
        }
        default:    return false;
    }

    if(m_bDrawPoint)
    {
        osg::ref_ptr<osg::Node> pNode = createPointNode(m_pVertexArray.get());
        m_pCurrentArtifactNode->addChild(pNode.get());
    }

    return true;
}


bool PointTool::operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    const osgGA::GUIEventAdapter::EventType eEventType = ea.getEventType();
    if(eEventType != osgGA::GUIEventAdapter::PUSH)
    {
        if(m_pVertexArray->empty()) return false;
        if(!m_bLButtonDown)         return false;
    }

    osg::Vec3d vIntersect(0.0, 0.0, 0.0);
    osg::View *pView = dynamic_cast<osg::View *>(&aa);
    osg::Camera *pCamera = pView->getCamera();
    const osg::Vec2d ptMouse(ea.getXnormalized(), ea.getYnormalized());
    if(!computeIntersection(m_pOperationTargetNode, pCamera, ptMouse, vIntersect))
    {
        return false;
    }

    if(m_bDrawPoint)
    {
        m_pCurrentArtifactNode->removeChildren(0u, m_pCurrentArtifactNode->getNumChildren());
    }
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

            break;
        }
        case osgGA::GUIEventAdapter::MOVE:
        case osgGA::GUIEventAdapter::DRAG:
        {
            if(!m_bLButtonDown)     return false;
            if(m_pVertexArray->size() != 1u)
            {
                return false;
            }

            m_pVertexArray->at(0) = vIntersect;
            break;
        }
        case osgGA::GUIEventAdapter::RELEASE:
        {
            if(ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                return false;
            }
            m_bLButtonDown = false;
            if(m_pVertexArray->size() != 1u)    return false;
            m_pVertexArray->at(0) = vIntersect;

            // send event
            if(m_pEventAdapter)
            {
                OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
                pEventObject->setAction(ea::ACTION_POINT_TOOL);

                pEventObject->putExtra("ToolName", m_strName);

                const osg::Vec3d point = osgUtil::convertWorld2Globe(m_pVertexArray->front());
                std::vector<double> vecPointList;
                vecPointList.push_back(point.x());
                vecPointList.push_back(point.y());
                vecPointList.push_back(point.z());
                pEventObject->putExtra("Vertices", vecPointList);
                m_pEventAdapter->sendBroadcast(pEventObject);
            }
            return true;
        }
        default: return false;
    }

    if(!m_bDrawPoint)   return true;

    if(m_pVertexArray.valid() && !m_pVertexArray->empty())
    {
        osg::ref_ptr<osg::Node> pNode = createPointNode(m_pVertexArray.get());
        m_pCurrentArtifactNode->addChild(pNode.get());
    }

    return true;
}