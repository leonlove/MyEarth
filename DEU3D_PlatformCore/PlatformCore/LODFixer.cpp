#include "LODFixer.h"
#include "SmartLOD.h"
#include <osg/LOD>
#include <osgViewer/View>
#include <osgViewer/CompositeViewer>
#include <iostream>
#include "CameraInfo.h"

LODFixer::LODFixer(double dblVTileRangeRatio)
{
    m_fltCameraCosFovy   = cos(osg::DegreesToRadians(45.0f));
    m_fltCameraCosFovy_2 = cos(osg::DegreesToRadians(22.5f));
    m_fltMinRangeRatio   = float(1.0 / dblVTileRangeRatio);
}


LODFixer::~LODFixer(void)
{
}


void LODFixer::operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor)
{
    const osg::NodeVisitor::VisitorType eType = pNodeVisitor->getVisitorType();
    if(eType != osg::NodeVisitor::EVENT_VISITOR)
    {
        __super::operator()(pNode, pNodeVisitor);
        return;
    }

    osgGA::EventVisitor *pEventVisitor = dynamic_cast<osgGA::EventVisitor *>(pNodeVisitor);
    if(!pEventVisitor || pEventVisitor->getEvents().empty())
    {
        return;
    }

    const osgGA::GUIEventAdapter::EventType eEventType = pEventVisitor->getEvents().front()->getEventType();
    if(eEventType != osgGA::GUIEventAdapter::FRAME)
    {
        __super::operator()(pNode, pNodeVisitor);
        return;
    }

    osgGA::GUIActionAdapter *pActionAdapter = pEventVisitor->getActionAdapter();
    osgViewer::View *pView = dynamic_cast<osgViewer::View *>(pActionAdapter);
    if(!pView)  return;

    SmartLOD *pSmartLOD = dynamic_cast<SmartLOD *>(pNode);
    if(!pSmartLOD)
    {
        __super::operator()(pNode, pNodeVisitor);
        return;
    }

    const unsigned nViewIndex = pView->getViewIndex();
    const osg::FrameStamp *pFrameStamp = pEventVisitor->getFrameStamp();
    const unsigned nCurFrameNum = pFrameStamp->getFrameNumber();
    const CameraInfo *pCamInfo = CameraInfo::instance();
    const unsigned nPoseFrameNum = pCamInfo->getFrameNumber(nViewIndex);
    if(nCurFrameNum > nPoseFrameNum + 1u)
    {
        __super::operator()(pNode, pNodeVisitor);
        return;
    }

    const osg::Vec3  &ptCenter = pSmartLOD->getCenter();
    osg::Vec3 vecCam2Center = ptCenter - pCamInfo->getCameraPosInWorld(nViewIndex);
    vecCam2Center.normalize();
    const float fltCosTheta = (vecCam2Center * pCamInfo->getCameraDirection(nViewIndex));

    float fltRangeRatio = 1.0f;
    if(fltCosTheta < m_fltCameraCosFovy_2)
    {
        fltRangeRatio = osg::clampAbove(fltCosTheta, m_fltMinRangeRatio);
    }
    pSmartLOD->setPositionBiasRatio(fltRangeRatio);

    __super::operator()(pNode, pNodeVisitor);
}


