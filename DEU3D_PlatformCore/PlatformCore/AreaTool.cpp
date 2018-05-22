#include "AreaTool.h"
#include <Common/deuMath.h>
#include "Utility.h"
#include <osgUtil/CommonOSG.h>

AreaTool::AreaTool(const std::string &strName)
    : PolygonTool(strName)
{
    m_bMap2Screen = false;
}


AreaTool::~AreaTool(void)
{
}

bool AreaTool::operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    return false;
}


bool AreaTool::operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
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
        m_pCurrentArtifactNode->removeChildren(0u, m_pCurrentArtifactNode->getNumChildren());
    }

    switch(eEventType)
    {
    case osgGA::GUIEventAdapter::PUSH:
        {
            const int nButton = ea.getButton();
            if(nButton == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                m_pVertexArray->push_back(vIntersect);
            }
            else return false;
            break;
        }
	case osgGA::GUIEventAdapter::DOUBLECLICK:
    {
        if(m_pVertexArray->size() < 3u)
        {
            return false;
        }

        if(m_pEventAdapter)
        {
            OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
            pEventObject->setAction(ea::ACTION_AREA_TOOL);

            pEventObject->putExtra("ToolName", m_strName);

            std::vector<double> vecPoints;
            vecPoints.reserve(m_pVertexArray->size() * 3u);
            osg::Vec3dArray::const_iterator itorVtx = m_pVertexArray->begin();
            osg::Vec3d vpt;
            for( ; itorVtx != m_pVertexArray->end(); ++itorVtx)
            {
                vpt += *itorVtx;
                const osg::Vec3d point = osgUtil::convertWorld2Globe(*itorVtx);
                vecPoints.push_back(point.x());
                vecPoints.push_back(point.y());
                vecPoints.push_back(point.z());
            }
            pEventObject->putExtra("Vertices", vecPoints);

            vpt /= (double)m_pVertexArray->size();
            vpt.normalize();
            vpt = -vpt;

            osg::Quat quat;
            quat.makeRotate(vpt, osg::Vec3d(0.0, 0.0, 1.0));
            osg::Matrixd mtx = osg::Matrixd::rotate(quat);

            std::vector<cmm::math::Point2d> vecPoly;
            itorVtx = m_pVertexArray->begin();
            for( ; itorVtx != m_pVertexArray->end(); ++itorVtx)
            {
                osg::Vec3d vTemp = *itorVtx;
                mtx.postMult(mtx);
                vecPoly.push_back(cmm::math::Point2d(vTemp.x(), vTemp.y()));
            }

            cmm::math::Polygon2 poly(vecPoly);
            double dblArea = abs(poly.area());

            pEventObject->putExtra("Area", dblArea);
            m_pEventAdapter->sendBroadcast(pEventObject);

            if(!m_bAutoClear)
            {
                osg::ref_ptr<osg::Vec3dArray>   pVtxArray = new osg::Vec3dArray(*m_pVertexArray);
                if(pVtxArray->size() >= 3u)
                {
                    pVtxArray->push_back(pVtxArray->front());
                }
                osg::ref_ptr<osg::Node> pNode = createLineNode(pVtxArray.get());
                m_pCurrentArtifactNode->addChild(pNode.get());
                m_pVertexArray->clear();
                break;
            }
            m_pVertexArray->clear();
        }
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
        if(pVtxArray->size() >= 3u)
        {
            pVtxArray->push_back(pVtxArray->front());
        }

        osg::ref_ptr<osg::Node> pNode = createLineNode(pVtxArray.get());
        m_pCurrentArtifactNode->addChild(pNode.get());
    }
    return true;
}

