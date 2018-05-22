#include "PolygonTool.h"

#include <EventAdapter/IEventObject.h>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include "Utility.h"
#include <osgUtil/CommonOSG.h>

PolygonTool::PolygonTool(const std::string &strName)
    : FaceTool(strName)
{
    m_bMap2Screen = false;
    m_pCurrentArtifactNode = m_pArtifactOnTerrainNode;
    m_pToolNode->addChild(m_pCurrentArtifactNode);
}


PolygonTool::~PolygonTool(void)
{
}


osg::Node *PolygonTool::createPolygonGeode(const osg::Vec3Array *pVertexArray)
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
    osg::ref_ptr<osg::LineWidth> pLineWidth  = new osg::LineWidth(m_dblLineWidth);
    osg::ref_ptr<osg::Depth>     pDepth      = new osg::Depth(osg::Depth::ALWAYS);
    osg::ref_ptr<osg::Vec4Array> pColorArray = new osg::Vec4Array;
    osg::StateSet *pStateSet = pGeometry->getOrCreateStateSet();

    pGeometry->setVertexArray(pVertex);
    pGeometry->setColorArray(pColorArray.get());
    pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    pStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    pStateSet->setAttribute(pLineWidth.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    pStateSet->setAttribute(pDepth.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, pVertexArray->size()));
    pGeode->addDrawable(pGeometry.get());

    pMatrixTransform->addChild(pGeode);
    return pMatrixTransform.release();
}


bool PolygonTool::operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    return false;
}


bool PolygonTool::operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
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
                pEventObject->setAction(ea::ACTION_POLYGON_TOOL);

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
                m_pEventAdapter->sendBroadcast(pEventObject);
                m_pVertexArray->clear();
            }
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
        if(pVtxArray->size() >= 3u)
        {
            pVtxArray->push_back(pVtxArray->front());
        }

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

