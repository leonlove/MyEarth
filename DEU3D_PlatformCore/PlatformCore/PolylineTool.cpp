#include "PolylineTool.h"
#include <osg/Geode>
#include <osgViewer/View>
#include <osgUtil/CommonOSG.h>
#include "Utility.h"

PolylineTool::PolylineTool(const std::string &strName)
    : LineTool(strName)
{
    m_bRButtonDown = false;

    m_bMap2Screen = false;
    m_pCurrentArtifactNode = m_pArtifactOnTerrainNode;
    m_pToolNode->addChild(m_pCurrentArtifactNode);
}


PolylineTool::~PolylineTool(void)
{
}


bool PolylineTool::operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    return false;
}


bool PolylineTool::operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
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

    m_pCurrentArtifactNode->removeChildren(0u, m_pCurrentArtifactNode->getNumChildren());

    switch(eEventType)
    {
        case osgGA::GUIEventAdapter::PUSH:
        {
            const int nButton = ea.getButton();
            if(nButton == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                m_pVertexArray->push_back(vIntersect);
                sendEvent(false);
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

            sendEvent(true);
            m_pVertexArray->clear();
            break;
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

        osg::ref_ptr<osg::Node> pLineNode = createLineNode(pVtxArray.get());
        m_pCurrentArtifactNode->addChild(pLineNode.get());

        if(m_bDrawPoint)
        {
            osg::ref_ptr<osg::Node> pPointNode = createPointNode(pVtxArray.get());
            m_pCurrentArtifactNode->addChild(pPointNode.get());
        }
    }

    return true;
}

void PolylineTool::sendEvent(bool bFinishOperation)
{
    if(!m_pEventAdapter.valid())    return;
    if(m_pVertexArray->empty())     return;

    OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
    pEventObject->setAction(ea::ACTION_POLYLINE_TOOL);

    pEventObject->putExtra("ToolName", m_strName);

    std::vector<double> vecPoints;
    if(bFinishOperation)
    {
        vecPoints.reserve(m_pVertexArray->size() * 3u);
        osg::Vec3dArray::const_iterator itorVtx = m_pVertexArray->begin();
        for( ; itorVtx != m_pVertexArray->end(); ++itorVtx)
        {
            const osg::Vec3d point = osgUtil::convertWorld2Globe(*itorVtx);
            vecPoints.push_back(point.x());
            vecPoints.push_back(point.y());
            vecPoints.push_back(point.z());
        }
    }
    else
    {
        const osg::Vec3d point = osgUtil::convertWorld2Globe(m_pVertexArray->back());
        vecPoints.push_back(point.x());
        vecPoints.push_back(point.y());
        vecPoints.push_back(point.z());
    }

    pEventObject->putExtra("Vertices", vecPoints);
    m_pEventAdapter->sendBroadcast(pEventObject);
}

