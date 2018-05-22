#include "RectTool.h"
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Depth>
#include <osgViewer/View>
#include <osgUtil/CommonOSG.h>
#include "Utility.h"

RectTool::RectTool(const std::string &strName)
    : FaceTool(strName)
{
    m_vNormal = osg::Vec3(0.0f, 0.0f, 1.0f);

    m_bMap2Screen   = true;
    m_bDrawPoint    = false;
    m_pCurrentArtifactNode = m_pArtifactOnScreenNode;
    m_pToolNode->addChild(m_pCurrentArtifactNode);
}


RectTool::~RectTool(void)
{
}


osg::Node *RectTool::createRectNode(const osg::Vec3dArray *pVertexArray)
{
    if(pVertexArray->size() != 2)
    {
        return NULL;
    }
    osg::ref_ptr<osg::Geode>        pGeode          = new osg::Geode;
    osg::ref_ptr<osg::Geometry>     pGeometry       = new osg::Geometry;
    pGeode->addDrawable(pGeometry.get());

    osg::Vec3d vPoint1 = osgUtil::convertWorld2Globe((*pVertexArray)[0]);
    osg::Vec3d vPoint2 = osgUtil::convertWorld2Globe((*pVertexArray)[1]);

    osg::Vec3d vCenterNormal = ((*pVertexArray)[0] + (*pVertexArray)[1]) * 0.5;
    double dblCenterLen = vCenterNormal.normalize();

    osg::Vec2d vMin(std::min(vPoint1[0], vPoint2[0]), std::min(vPoint1[1], vPoint2[1]));
    osg::Vec2d vMax(std::max(vPoint1[0], vPoint2[0]), std::max(vPoint1[1], vPoint2[1]));

    osg::ref_ptr<osg::Vec3Array> pRectVertexArray = new osg::Vec3Array;

    osg::Vec3d vCoord;
    osg::Vec3d vPt;
    double dblAngle;

    //1
    vCoord.set(vMin[0], vMin[1], 0.0);
    vPt = osgUtil::convertGlobe2World(vCoord);
    vPt.normalize();
    dblAngle = vCenterNormal * vPt;
    pRectVertexArray->push_back(vPt * dblCenterLen / dblAngle);

    //2
    vCoord.set(vMin[0], vMax[1], 0.0);
    vPt = osgUtil::convertGlobe2World(vCoord);
    vPt.normalize();
    dblAngle = vCenterNormal * vPt;
    pRectVertexArray->push_back(vPt * dblCenterLen / dblAngle);

    //3
    vCoord.set(vMax[0], vMax[1], 0.0);
    vPt = osgUtil::convertGlobe2World(vCoord);
    vPt.normalize();
    dblAngle = vCenterNormal * vPt;
    pRectVertexArray->push_back(vPt * dblCenterLen / dblAngle);

    //4
    vCoord.set(vMax[0], vMin[1], 0.0);
    vPt = osgUtil::convertGlobe2World(vCoord);
    vPt.normalize();
    dblAngle = vCenterNormal * vPt;
    pRectVertexArray->push_back(vPt * dblCenterLen / dblAngle);

    pGeometry->setVertexArray(pRectVertexArray.get());
    osg::ref_ptr<osg::Vec4Array> pColorArray = new osg::Vec4Array;
    pColorArray->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 0.5f));
    pGeometry->setColorArray(pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, 4));

    osg::StateSet *pStateSet = pGeode->getOrCreateStateSet();

	osg::ref_ptr<osg::Depth>  pDepth  = new osg::Depth(osg::Depth::ALWAYS);
	pStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	pStateSet->setAttribute(pDepth.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

    pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    pStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    return pGeode.release();
}


bool RectTool::operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    const osgGA::GUIEventAdapter::EventType eEventType = ea.getEventType();
    if(eEventType != osgGA::GUIEventAdapter::PUSH && !m_bLButtonDown)
    {
        return false;
    }

    m_pCurrentArtifactNode->removeChildren(0u, m_pCurrentArtifactNode->getNumChildren());

    const osg::Vec3d ptMouse(ea.getX(), ea.getY(), 0.0f);
    switch(eEventType)
    {
        case osgGA::GUIEventAdapter::PUSH:
        {
            if(ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                return false;
            }
            m_pVertexArray->clear();
            m_pVertexArray->push_back(ptMouse);
            m_pVertexArray->push_back(ptMouse);
            m_bLButtonDown = true;
            break;
        }
        case osgGA::GUIEventAdapter::MOVE:
        case osgGA::GUIEventAdapter::DRAG:
        {
            if(!m_bLButtonDown)     return false;
            if(m_pVertexArray->size() != 2u)    return false;
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
                pEventObject->setAction(ea::ACTION_RECT_TOOL);

                pEventObject->putExtra("ToolName", m_strName);

                const osg::Vec3d &point0 = m_pVertexArray->front();
                const osg::Vec3d &point1 = m_pVertexArray->back();
                std::vector<double> vecPointList;
                vecPointList.clear();
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


    if(m_pVertexArray->size() != 2u)    return false;
    const osg::Vec3d &point0 = m_pVertexArray->front();
    const osg::Vec3d &point1 = m_pVertexArray->back();

    osg::ref_ptr<osg::Vec3dArray>    pVertexArray = new osg::Vec3dArray;
    pVertexArray->push_back(point0);
    pVertexArray->push_back(osg::Vec3d(point0.x(), point1.y(), 0.0));
    pVertexArray->push_back(point1);
    pVertexArray->push_back(osg::Vec3d(point1.x(), point0.y(), 0.0));
    pVertexArray->push_back(point0);

    osg::ref_ptr<osg::Node> pLineNode = createLineNode(pVertexArray.get());
    m_pCurrentArtifactNode->addChild(pLineNode.get());
    if(m_bDrawPoint)
    {
        osg::ref_ptr<osg::Node> pPointNode = createPointNode(pVertexArray.get());
        m_pCurrentArtifactNode->addChild(pPointNode.get());
    }

    return true;
}


bool RectTool::operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
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
    case(osgGA::GUIEventAdapter::PUSH):
        {
            if(ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                return false;
            }
            m_pVertexArray->clear();
            m_pVertexArray->push_back(vIntersect);
            m_pVertexArray->push_back(vIntersect);
            m_bLButtonDown = true;
            break;
        }
    case osgGA::GUIEventAdapter::MOVE:
    case osgGA::GUIEventAdapter::DRAG:
        {
            if(!m_bLButtonDown)     return false;
            if(m_pVertexArray->size() != 2u)    return false;
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
                pEventObject->setAction(ea::ACTION_RECT_TOOL);

                pEventObject->putExtra("ToolName", m_strName);

                std::vector<double> vecPointList;
                vecPointList.clear();
                for(unsigned int i = 0; i < m_pVertexArray->size(); i++)
                {
                    const osg::Vec3d point = osgUtil::convertWorld2Globe((*m_pVertexArray)[i]);
                    vecPointList.push_back(point.x());
                    vecPointList.push_back(point.y());
                    vecPointList.push_back(point.z());
                }

                pEventObject->putExtra("Vertices", vecPointList);
                m_pEventAdapter->sendBroadcast(pEventObject);
            }
            return true;
        }
    default:    return false;
    }


    if(m_pVertexArray->size() != 2u)    return false;

    osg::ref_ptr<osg::Node> pPointNode = createRectNode(m_pVertexArray.get());
    m_pCurrentArtifactNode->addChild(pPointNode.get());

    return true;
}

