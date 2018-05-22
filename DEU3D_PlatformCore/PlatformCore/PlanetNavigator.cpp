#include "PlanetNavigator.h"
#include <osgUtil/LineSegmentIntersector>
#include <osgViewer/View>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osgViewer/CompositeViewer>
#include <osgUtil/FindNodeVisitor.h>
#include <assert.h>
#include "Common/Common.h"
#include "SmartLOD.h"
#include <fstream>
#include <iostream>
#include "CameraInfo.h"
#include "Utility.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//    
///////////////////////////////////////////////////////////////////////////////////////////////////
#define DEBUG_LOG   1

const double g_dblMaxElevAngle = osg::PI / 1.2;     // the maximize elevation angle is 75 in degree.
const double g_dblMaxDistance = 3.0;

PlanetNavigator::PlanetNavigator(void)
    : m_vecHomeDirection(0.0, -1.0, 0.0)
{
    m_bMouseDownHitScene = false;
    m_bLButtonDown = false;
    m_bMButtonDown = false;
    m_bRButtonDown = false;

    m_nCameraPoseChangedByExternal = 0;

    m_pNavigationParam = new NavigationParam;
    m_pNavigationParam->Default();

    m_dblLastFrameTime  = DBL_MAX;
    m_dblSubGroundElev  = 400.0;
    //m_dblCamera2Ground  = FLT_MAX;
    m_bCameraUnderGround = false;
	m_bCameraUnderGroundEx = false;

    m_CurrentCameraPose.m_dblPositionX    = 0.0;
    m_CurrentCameraPose.m_dblPositionY    = 0.0;
    m_CurrentCameraPose.m_dblHeight       = 0.0;
    m_CurrentCameraPose.m_dblAzimuthAngle = osg::PI_2;
    m_CurrentCameraPose.m_dblPitchAngle   = 0.0;
}


PlanetNavigator::~PlanetNavigator(void)
{
}

bool hitScene2(osg::Node *pHitTargetNode, osg::Camera *pCamera, osgUtil::Radial3 &ray, osg::Vec3d &ptHitTest/*, osg::Node **ppHitNode*/)
{
	const osgUtil::LineSegmentIntersector::CoordinateFrame cf = osgUtil::Intersector::WINDOW;
	osg::ref_ptr<osgUtil::LineSegmentIntersector>   pPicker  = new osgUtil::LineSegmentIntersector(cf, ray.getOrigin(), ray.getPoint(1e9));
	//pPicker->set
	osg::ref_ptr<osgUtil::IntersectionVisitor>      pVisitor = new osgUtil::IntersectionVisitor(pPicker.get());
	pVisitor->setTraversalMask(0x00FFFFFF);
	const_cast<osg::Node *>(pHitTargetNode)->accept(*pVisitor);

	if(!pPicker->containsIntersections())
	{
		return false;
	}

	osg::Vec3d ptCameraPos, ptCenter, vecUp;
	pCamera->getViewMatrixAsLookAt(ptCameraPos, ptCenter, vecUp);

	const osgUtil::LineSegmentIntersector::Intersections &inters = pPicker->getIntersections();
	osgUtil::LineSegmentIntersector::Intersections::const_iterator itor = inters.begin();
	osg::Vec3d ptHit;
	double dblMin = FLT_MAX;
	osg::Node *pHitNode = NULL;
	for( ; itor != inters.end(); ++itor)
	{
		const osgUtil::LineSegmentIntersector::Intersection &inter = *itor;
		osg::Vec3d point = inter.getWorldIntersectPoint();
		const double dbl = (ptCameraPos - point).length();
		if(dbl < dblMin)
		{
			dblMin = dbl;
			ptHit = point;
			//pHitNode = inter.nodePath[inter.nodePath.size() - 1];
		}
	}
	ptHitTest = ptHit;
	/*if(ppHitNode != NULL)
		*ppHitNode = pHitNode;*/
	return true;
}


void PlanetNavigator::calculateHomePose(void)
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const double dblRadius = pEllipsoidModel->getRadius();
    m_CurrentCameraPose.m_dblPositionX    = osg::PI_2;
    m_CurrentCameraPose.m_dblPositionY    = 0.0;
    m_CurrentCameraPose.m_dblHeight       = dblRadius * g_dblMaxDistance;
    m_CurrentCameraPose.m_dblAzimuthAngle = osg::PI_2;
    m_CurrentCameraPose.m_dblPitchAngle   = 0.0;
}


osg::Matrixd PlanetNavigator::getCameraMatrix(void) const
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    const double dblRadius = pEllipsoidModel->getRadius();
    if(m_CurrentCameraPose.m_dblHeight > dblRadius * g_dblMaxDistance)
    {
        CameraPose &pose = const_cast<CameraPose &>(m_CurrentCameraPose);
        pose.m_dblHeight = dblRadius * g_dblMaxDistance;
        SmartLOD::setSightBiasRatio(1.0f);
    }
    else
    {
        const static float fltCos   = 0.5f;//cos(osg::PI_4);
        const float fltCosPitch     = fabs(cos(m_CurrentCameraPose.m_dblPitchAngle));
        const float fltRangeRatio   = osg::clampAbove(fltCosPitch, fltCos);
        SmartLOD::setSightBiasRatio(fltRangeRatio);
    }

    const osg::Vec3d vecCurCamDir = getCurrentCameraDir();
    const osg::Vec3d vecCurCamUp  = getCurrentCameraUp();
    const osg::Vec3d ptCameraPos  = getCurrentCameraPos();
    osg::Matrixd  mtxCamera;
    mtxCamera.makeLookAt(ptCameraPos, ptCameraPos + vecCurCamDir * dblRadius, vecCurCamUp);

    return mtxCamera;
}


osg::Vec3d PlanetNavigator::getCurrentCameraPos(void) const
{
    return calcWorldCoordByCameraPose(m_CurrentCameraPose);
}


osg::Vec3d PlanetNavigator::getCurrentCameraDir(void) const
{
    return calcCamDirByCameraPose(m_CurrentCameraPose);
}


osg::Vec3d PlanetNavigator::getCurrentCameraUp(void) const
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const osg::Vec3d vecEastern = pEllipsoidModel->computeLocalEastern(m_CurrentCameraPose.m_dblPositionY, m_CurrentCameraPose.m_dblPositionX);
    const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(m_CurrentCameraPose.m_dblPositionY, m_CurrentCameraPose.m_dblPositionX);

    const osg::Quat qtAzimuth(m_CurrentCameraPose.m_dblAzimuthAngle, -vecPlumbLine);
    const osg::Vec3d vecForward = qtAzimuth * vecEastern;

    const osg::Vec3d vecSouthern = vecPlumbLine ^ vecEastern;
    const osg::Vec3d vecAxis = qtAzimuth * vecSouthern;
    const osg::Quat qtPitch(m_CurrentCameraPose.m_dblPitchAngle, vecAxis);

    const osg::Vec3d vecCameraUp = qtPitch * vecForward;
    return vecCameraUp;
}


osg::Vec3d PlanetNavigator::getCurrentCameraRight(void) const
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const osg::Vec3d vecEastern = pEllipsoidModel->computeLocalEastern(m_CurrentCameraPose.m_dblPositionY, m_CurrentCameraPose.m_dblPositionX);
    const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(m_CurrentCameraPose.m_dblPositionY, m_CurrentCameraPose.m_dblPositionX);
    const osg::Vec3d vecSouthern = vecPlumbLine ^ vecEastern;

    const osg::Quat qtAzimuth(m_CurrentCameraPose.m_dblAzimuthAngle, -vecPlumbLine);
    const osg::Vec3d vecRight = qtAzimuth * vecSouthern;

    return vecRight;
}


osg::Vec3d PlanetNavigator::getCurrentCameraForward(void) const
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const osg::Vec3d vecEastern = pEllipsoidModel->computeLocalEastern(m_CurrentCameraPose.m_dblPositionY, m_CurrentCameraPose.m_dblPositionX);
    const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(m_CurrentCameraPose.m_dblPositionY, m_CurrentCameraPose.m_dblPositionX);

    const osg::Quat qtAzimuth(m_CurrentCameraPose.m_dblAzimuthAngle, -vecPlumbLine);
    const osg::Vec3d vecForward = qtAzimuth * vecEastern;
    return vecForward;
}


osg::Vec3d PlanetNavigator::getCurrentPlumbLine(void) const
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(m_CurrentCameraPose.m_dblPositionY, m_CurrentCameraPose.m_dblPositionX);
    return vecPlumbLine;
}


void PlanetNavigator::setNaviNode(osg::Node *pNode)
{
    m_pNavigationNode = pNode;

    osg::ref_ptr<osgUtil::FindNodeByNameVisitor>    pFinder = new osgUtil::FindNodeByNameVisitor("TerrainRootNode");
    m_pNavigationNode->accept(*pFinder);
    m_pTerrainNode = pFinder->getTargetNode();
}


bool PlanetNavigator::handleEvent(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    CameraInfo *pCamInfo = CameraInfo::instance();

    osgViewer::View *pView = dynamic_cast<osgViewer::View *>(&aa);
    const unsigned nViewIndex = pView->getViewIndex();

    if(ea.getHandled())
    {
        pCamInfo->setCameraInfo
        (
            nViewIndex,
            pView->getFrameStamp()->getFrameNumber(),
            m_CurrentCameraPose,
            m_bCameraUnderGround,
            m_ptCameraGroundPos
        );
        return false;
    }

    m_pCurrentCamera = pView->getCamera();

    bool bHandled = false;
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::PUSH):
            bHandled = OnMouseDown(ea);
            break;
        case(osgGA::GUIEventAdapter::RELEASE):
            bHandled = OnMouseUp(ea);
            break;
        case(osgGA::GUIEventAdapter::DRAG):
            bHandled = OnMouseDrag(ea);
            break;
        case(osgGA::GUIEventAdapter::MOVE):
            bHandled = OnMouseMove(ea);
            break;
        case(osgGA::GUIEventAdapter::SCROLL):
            bHandled = OnScroll(ea);
            break;
        case(osgGA::GUIEventAdapter::KEYDOWN):
            bHandled = OnKeyDown(ea);
            break;
        case(osgGA::GUIEventAdapter::KEYUP):
            bHandled = OnKeyUp(ea);
            break;
        case(osgGA::GUIEventAdapter::FRAME):
            bHandled = OnFrame(ea);
            break;
        default:    break;
    }

    if(hasKeyDown())
    {
        const bool bNav = doKeyNavigation(ea.getTime());
        bHandled = bNav || bHandled;
    }

    if(bHandled || pView->getFrameStamp()->getFrameNumber() < 100u || m_nCameraPoseChangedByExternal > 0)
    {
        aa.requestRedraw();
        aa.requestContinuousUpdate(false);

		//原始代码
		if (m_pNavigationParam->m_bUnderGroundViewMode)
		{
			calcGroundPosInfo();
			pCamInfo->setCameraInfo
			(
				nViewIndex,
				pView->getFrameStamp()->getFrameNumber(),
				m_CurrentCameraPose,
				m_bCameraUnderGround,
				m_ptCameraGroundPos
			);
		}
		//修改后代码
		else
		{
			if (m_CurrentCameraPose.m_dblHeight < 2.0)
			{
				m_CurrentCameraPose.m_dblHeight = 2.0;
			}
			//构建射线与地形瓦片求交取得高程，通过交点高程和相机高程比对来决定，是否碰到了地形。
			else
			{
				const osg::Vec3d ptPlanetCenter(0.0, 0.0, 0.0);
				osg::Vec3d ptHitPos;
				osg::Vec3d camPos =calcWorldCoordByCameraPose(m_CurrentCameraPose);
				osgUtil::Radial3 ray(ptPlanetCenter, camPos);
				if(!hitScene2(m_pTerrainNode,pView->getCamera(),ray,ptHitPos))			
				{
					return bHandled;
				}

				osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
				double dblLatitude = 0.0, dblLongitude = 0.0, dblHeight = 0.0;
				pEllipsoidModel->convertXYZToLatLongHeight(ptHitPos.x(), ptHitPos.y(), ptHitPos.z(), dblLatitude, dblLongitude, dblHeight);
				if(dblHeight > m_CurrentCameraPose.m_dblHeight-2.0)
				{
					m_bCameraUnderGroundEx = true;
				}
				else
				{
					m_bCameraUnderGroundEx = false;
					m_vec3dTerrainHitTestPos = ptHitPos;
					m_tmpTerrainHitTestCamPos.m_dblHeight = dblHeight;
					m_tmpTerrainHitTestCamPos.m_dblPositionX = dblLongitude;
					m_tmpTerrainHitTestCamPos.m_dblPositionY = dblLatitude;

				}
				dblLatitude  = osg::RadiansToDegrees(dblLatitude);
				dblLongitude = osg::RadiansToDegrees(dblLongitude);

				//如果在地上按正常镜头操作
				if(!m_bCameraUnderGroundEx)
				{
					pCamInfo->setCameraInfo(
						nViewIndex,
						pView->getFrameStamp()->getFrameNumber(),
						m_CurrentCameraPose,
						m_bCameraUnderGroundEx,
						ptHitPos);
				}
				else 
				{
					//如果在地下但是没开启了地下模式，需要将相机弹起
					m_CurrentCameraPose.m_dblPositionX = m_tmpTerrainHitTestCamPos.m_dblPositionX;
					m_CurrentCameraPose.m_dblPositionY = m_tmpTerrainHitTestCamPos.m_dblPositionY;
					m_tmpTerrainHitTestCamPos.m_dblHeight += 0.1;
					m_CurrentCameraPose.m_dblHeight = m_tmpTerrainHitTestCamPos.m_dblHeight;

					pCamInfo->setCameraInfo(
						nViewIndex,
						pView->getFrameStamp()->getFrameNumber(),
						m_CurrentCameraPose,
						false,
						m_vec3dTerrainHitTestPos);
				}
			}

			if (m_CurrentCameraPose.m_dblPitchAngle > osg::PI_2)
			{
				m_CurrentCameraPose.m_dblPitchAngle = osg::PI_2;
			}
		}
        m_nCameraPoseChangedByExternal--;
    }

    m_dblLastFrameTime = ea.getTime();
    return bHandled;
}


void PlanetNavigator::calcGroundPosInfo(void)
{
    const osg::Vec3d ptPlanetCenter(0.0, 0.0, 0.0);
    const osg::Vec3d vecCurPlumbLine = getCurrentPlumbLine();

    const osgUtil::Radial3 ray(ptPlanetCenter, -vecCurPlumbLine);
    osg::Vec3d ptHitTest;
    if(!hitScene(ray, ptHitTest, true))
    {
        return;
    }

    m_ptCameraGroundPos = ptHitTest;
    const osg::Vec3d ptCurCamPos = getCurrentCameraPos();
    osg::Vec3d vecCam2Hit  = ptHitTest - ptCurCamPos;
    vecCam2Hit.normalize();
    const double dblDot = vecCam2Hit * vecCurPlumbLine;
    m_bCameraUnderGround = (dblDot < 0.0);
	m_bCameraUnderGround = false;
}


bool PlanetNavigator::OnKeyDown(const osgGA::GUIEventAdapter &ea)
{
    return setKeyboardState(true, ea);
}


bool PlanetNavigator::OnKeyUp(const osgGA::GUIEventAdapter &ea)
{
    return setKeyboardState(false, ea);
}


bool PlanetNavigator::setKeyboardState(bool bKeyDown, const osgGA::GUIEventAdapter &ea)
{
    int nKey = ea.getKey();
    if(nKey >= 'A' && nKey <= 'Z')
    {
        nKey += 'a' - 'A';
    }

    bool bKeySet = false;
    if(nKey == osgGA::GUIEventAdapter::KEY_Shift_L)
    {
        m_mapKeyDownState[NavigationParam::NK_Shift_L] = bKeyDown;
        bKeySet = true;
    }

    else if(nKey == osgGA::GUIEventAdapter::KEY_Shift_R)
    {
        m_mapKeyDownState[NavigationParam::NK_Shift_R] = bKeyDown;
        bKeySet = true;
    }

    else if(nKey == osgGA::GUIEventAdapter::KEY_Control_L)
    {
        m_mapKeyDownState[NavigationParam::NK_Control_L] = bKeyDown;
        bKeySet = true;
    }

    else if(nKey == osgGA::GUIEventAdapter::KEY_Control_R)
    {
        m_mapKeyDownState[NavigationParam::NK_Control_R] = bKeyDown;
        bKeySet = true;
    }

    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_ResetCamera])
    {
        m_mapKeyDownState[NavigationParam::NK_ResetCamera] = bKeyDown;
        bKeySet = true;
    }

    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_Forward])
    {
        m_mapKeyDownState[NavigationParam::NK_Forward] = bKeyDown;
        bKeySet = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_Backward])
    {
        m_mapKeyDownState[NavigationParam::NK_Backward] = bKeyDown;
        bKeySet = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_Left])
    {
        m_mapKeyDownState[NavigationParam::NK_Left] = bKeyDown;
        bKeySet = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_Right])
    {
        m_mapKeyDownState[NavigationParam::NK_Right] = bKeyDown;
        bKeySet = true;
    }

    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_Up])
    {
        m_mapKeyDownState[NavigationParam::NK_Up] = bKeyDown;
        bKeySet = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_Down])
    {
        m_mapKeyDownState[NavigationParam::NK_Down] = bKeyDown;
        bKeySet = true;
    }

    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_LookUp])
    {
        m_mapKeyDownState[NavigationParam::NK_LookUp] = bKeyDown;
        bKeySet = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_LookDown])
    {
        m_mapKeyDownState[NavigationParam::NK_LookDown] = bKeyDown;
        bKeySet = true;
    }

    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_RotateRight])
    {
        m_mapKeyDownState[NavigationParam::NK_RotateRight] = bKeyDown;
        bKeySet = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_RotateLeft])
    {
        m_mapKeyDownState[NavigationParam::NK_RotateLeft] = bKeyDown;
        bKeySet = true;
    }

    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_RotateRight_V])
    {
        m_mapKeyDownState[NavigationParam::NK_RotateRight_V] = bKeyDown;
        bKeySet = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_RotateLeft_V])
    {
        m_mapKeyDownState[NavigationParam::NK_RotateLeft_V] = bKeyDown;
        bKeySet = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_StopInertia])
    {
        m_mapKeyDownState[NavigationParam::NK_StopInertia] = bKeyDown;
        bKeySet = true;
    }

    return bKeySet;
}


bool PlanetNavigator::OnMouseDown(const osgGA::GUIEventAdapter &ea)
{
    m_bMouseDownHitScene = false;

    stopInertia();

    osgUtil::Radial3 ray;
    const bool bHasRadial = getScreenRadial(osg::Vec2d(ea.getXnormalized(), ea.getYnormalized()), ray);
    if(!bHasRadial)
    {
        return true;
    }

    {
        osg::Vec3d ptHitTest;
        if(!getHitTestPoint(ray, ptHitTest))
        {
            return true;
        }
        m_ptMouseDownHitScene = ptHitTest;

        osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
        pEllipsoidModel->convertXYZToLatLongHeight(m_ptMouseDownHitScene.x(), m_ptMouseDownHitScene.y(), m_ptMouseDownHitScene.z(), m_ptMouseDownHitScenePos.y(), m_ptMouseDownHitScenePos.x(), m_ptMouseDownHitScenePos.z());
        m_bMouseDownHitScene = true;
    }

    if(ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        m_bLButtonDown = true;
    }
    if(ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
    {
        m_bMButtonDown = true;
    }
    if(ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {
        m_bRButtonDown = true;
    }

    m_pLastMouseEvent = &ea;
    m_pLastMouseDownEvent = &ea;

    return true;
}


bool PlanetNavigator::OnMouseUp(const osgGA::GUIEventAdapter &ea)
{
    if(m_bLButtonDown && m_pLastMouseEvent.valid())
    {
        const double dbl1 = m_pLastMouseEvent->getTime();
        const double dbl2 = ea.getTime();

        // 鼠标拖拽平移的惯性速度，根据时间，以余弦曲线递减；最长1秒钟衰减为0
        const double dblDeltaTime = osg::clampBetween(dbl2 - dbl1, 0.0, 1.0);

        const double dblRatio = (cos(dblDeltaTime * osg::PI) + 1.0) * 0.5;
        m_vecInertiaSpeed *= dblRatio;
		m_vecInertiaSpeed *= 0.4;
    }

    if(ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        m_bLButtonDown = false;
    }
    if(ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
    {
        m_bMButtonDown = false;
    }
    if(ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {
        m_bRButtonDown = false;
    }

    if(!m_bLButtonDown && !m_bRButtonDown && !m_bMButtonDown)
    {
        m_pLastMouseEvent = NULL;
        m_pLastMouseDownEvent = NULL;
    }

    return true;
}


bool PlanetNavigator::OnMouseMove(const osgGA::GUIEventAdapter &ea)
{
    return false;
}


bool PlanetNavigator::OnMouseDrag(const osgGA::GUIEventAdapter &ea)
{
    if(!m_bLButtonDown && !m_bRButtonDown && !m_bMButtonDown)
    {
        // 鼠标三个键都没有按下，那么这样的DRAG事件没有处理的价值
        return false;
    }

    if(!m_pLastMouseEvent.valid())  return false;

    const osg::Vec2d vecMouseMove
    (
        ea.getXnormalized() - m_pLastMouseEvent->getXnormalized(),
        ea.getYnormalized() - m_pLastMouseEvent->getYnormalized()
    );
    const double fltDelta = vecMouseMove.length();
    if(fltDelta > 0.5)
    {
        // 移动太快，多半会是用户的鼠标跳帧导致，不予处理
        return false;
    }
    else if(fltDelta < 0.001)
    {
        // 移动太慢，多半是用户的鼠标发生了漂移，不予处理
        return false;
    }

    bool bDone = false;
    if(m_bLButtonDown)    bDone = doLMouseDragNavigation(ea);

    if(m_bRButtonDown)    bDone = doRMouseDragNavigation(ea);

    if(m_bMButtonDown)    bDone = doMMouseDragNavigation(ea);

    m_pLastMouseEvent = &ea;

    return bDone;
}


bool PlanetNavigator::doLMouseDragNavigation(const osgGA::GUIEventAdapter &ea)
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    // 1. Check if the sight line intersect to the planet
    //    if not, move the camera to make sure the sight line intersect to the planet
    const osg::Vec3d vecCurCamDir = getCurrentCameraDir();
    const osg::Vec3d vecCurCamUp  = getCurrentCameraUp();
    const osg::Vec3d vecCurPlumbLine = getCurrentPlumbLine();


    // 2. Calculate the mouse-drag plane, this plane pass the mouse hitting point, and its normal is the reverse of current plumb line
    const osg::Plane planeBase(-vecCurPlumbLine, m_ptMouseDownHitScene);


    // 3. Calculate the begin mouse-drag point
    // 3.1. Calculate the begin mouse-drag point
    osg::Vec3d ptPlaneBegin;
    {
        const osg::Vec2d ptMouseBegin(m_pLastMouseEvent->getXnormalized(), m_pLastMouseEvent->getYnormalized());
        osgUtil::Radial3  rayPrev;
        if(!getScreenRadial(ptMouseBegin, rayPrev))
        {
            return false;
        }
        if(!rayPrev.hitPlane(planeBase, ptPlaneBegin))
        {
            return false;
        }
    }

    // 3.2. Do some special things, if the line between camera and hitting point is nearly perpendicular to the plumbline,
    // we cannot calculate the dragging-distance directly
    const osg::Vec3d ptCurCamPos = getCurrentCameraPos();
    double dblRatio  = 1.0;
    {
        osg::Vec3d vecCam2Point = ptPlaneBegin - ptCurCamPos;
        vecCam2Point.normalize();
        const double dblDot = fabs(vecCurPlumbLine * vecCam2Point);
        if(dblDot < 0.1)
        {
            dblRatio *= dblDot * 10.0;
        }
    }


    // 4. Calculate the end mouse-drag point
    // 4.1. Calculate the end mouse-drag point
    osg::Vec3d ptPlaneEnd;
    {
        osgUtil::Radial3 rayCur;
        const osg::Vec2d ptMouseEnd(ea.getXnormalized(), ea.getYnormalized());
        if(!getScreenRadial(ptMouseEnd, rayCur))
        {
            return false;
        }
        if(!rayCur.hitPlane(planeBase, ptPlaneEnd))
        {
            return false;
        }
    }

    // 4.2. Do the mouse drag correction
    osg::Vec3d vecCamBias = ptPlaneBegin - ptPlaneEnd;
    const osg::Vec3d vecHitPlumbLine = pEllipsoidModel->computeLocalPlumbLine(m_ptMouseDownHitScenePos.y(), m_ptMouseDownHitScenePos.x());
    const double dblCosBias = vecHitPlumbLine * vecCurPlumbLine;
    vecCamBias /= dblCosBias;


    // Decompose vecBias to two elements: front-back element and left-right element,
    // for the front-back element, we should compress it suitably
    const osg::Vec3d vecCurCamRight = vecCurCamUp ^ vecCurCamDir;

    const double dblLeftRight     = vecCamBias * vecCurCamRight;
    const osg::Vec3d vecLeftRight = vecCurCamRight * dblLeftRight;    // left-right element
    const osg::Vec3d vecFrontBack = vecCamBias - vecLeftRight;        // front-back element
    vecCamBias = vecLeftRight + vecFrontBack * dblRatio;


    // 5. Apply this mouse dragging
    const osg::Vec3d ptNewCamPos = ptCurCamPos + vecCamBias;
    osg::Vec3d ptNewCamCoord;
    pEllipsoidModel->convertXYZToLatLongHeight
    (
        ptNewCamPos.x(), ptNewCamPos.y(), ptNewCamPos.z(),
        ptNewCamCoord.y(), ptNewCamCoord.x(), ptNewCamCoord.z()
    );

    if(m_pNavigationParam->m_bMouseInertia)
    {
        m_vecInertiaSpeed.set(ptNewCamCoord.x() - m_CurrentCameraPose.m_dblPositionX, ptNewCamCoord.y() - m_CurrentCameraPose.m_dblPositionY);
        const double dblCurTime = osg::Timer::instance()->time_s();
        const double dblDeltaTime = (dblCurTime > m_dblLastFrameTime ? (dblCurTime - m_dblLastFrameTime) : 0.0);
        m_vecInertiaSpeed /= dblDeltaTime;
		m_vecInertiaSpeed *= 0.5;
    }

    m_CurrentCameraPose.m_dblPositionX = ptNewCamCoord.x();
    m_CurrentCameraPose.m_dblPositionY = ptNewCamCoord.y();
    m_CurrentCameraPose.m_dblHeight    = ptNewCamCoord.z();

    return true;
}


bool PlanetNavigator::doMMouseDragNavigation(const osgGA::GUIEventAdapter &ea)
{
    const osg::Vec3d  ptCurCamPos = getCurrentCameraPos();
    osg::Vec3d  ptHitTest;
    osg::Vec3d  vecDirection;
    if(m_bMouseDownHitScene)
    {
        vecDirection = m_ptMouseDownHitScene - ptCurCamPos;
        vecDirection.normalize();
        ptHitTest = m_ptMouseDownHitScene;
    }
    else
    {
        const osg::Vec3d vecCurCamDir = getCurrentCameraDir();
        const osgUtil::Radial3 ray(ptCurCamPos, vecCurCamDir);
        if(!getHitTestPoint(ray, ptHitTest))
        {
            return false;
        }

        vecDirection = vecCurCamDir;
    }

    const double dblDistance = (ptCurCamPos - ptHitTest).length();
    if(dblDistance < 0.1)
    {
        return false;
    }

    const double dbl0 = m_pLastMouseEvent->getY();
    const double dbl1 = ea.getY();
    const double dblDeltaDistance  = dblDistance * (dbl0 - dbl1) * 0.001;
    const osg::Vec3d vecCameraBias = vecDirection * dblDeltaDistance;
    const osg::Vec3d ptNewCamPos   = ptCurCamPos + vecCameraBias;
    if(dblDeltaDistance > 0.0)
    {
        // Limit the camera, don't let it cross to the earth
        osg::Vec3d vecNewPlumbLine = -ptNewCamPos;
        vecNewPlumbLine.normalize();

        const osgUtil::Radial3   rayPlumbline(ptNewCamPos, vecNewPlumbLine);
        osg::Vec3d               ptHitTest;
        if(getHitTestPoint(rayPlumbline, ptHitTest))
        {
            const double dbl = (ptHitTest - ptNewCamPos).length();
            if(dbl < 500.0)
            {
                // I don't think that doing this limit here is correct.
                // If the user has a requirement which needs to see underground objects, the camera has to be moved to underground
                return false;
            }
        }
    }

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    double dblLat, dblLon, dblHeight;
    pEllipsoidModel->convertXYZToLatLongHeight
    (
        ptNewCamPos.x(), ptNewCamPos.y(), ptNewCamPos.z(),
        dblLat, dblLon, dblHeight
    );
    m_CurrentCameraPose.m_dblPositionX = dblLon;
    m_CurrentCameraPose.m_dblPositionY = dblLat;

    const double dblPlanetRadius = pEllipsoidModel->getRadius();
    m_CurrentCameraPose.m_dblHeight = std::min(dblHeight, dblPlanetRadius * g_dblMaxDistance);
    return false;
}


bool PlanetNavigator::doRMouseDragNavigation(const osgGA::GUIEventAdapter &ea)
{
    if(!m_bMouseDownHitScene)    return false;

    const osg::Vec3d ptCurCamPos     = getCurrentCameraPos();
    const osg::Vec3d vecCurPlumbLine = getCurrentPlumbLine();

    osg::Vec3d  vecCameraBias;
    double  dblAzimuthBias = 0.0;
    double  dblPitchBias = 0.0;

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const bool bControl = (m_mapKeyDownState[NavigationParam::NK_Control_L] || m_mapKeyDownState[NavigationParam::NK_Control_R]);
    if(!bControl)
    {
        // 旋转中心为鼠标命中点，这种漫游会导致视点位置饶旋转中心转
        // 也就是说，视点位置会发生变化

        // 1. 计算相机的旋转量
        // 1.1. 水平方向的旋转量：水平方向的旋转，旋转轴就是从球心到命中点的向量
        const osg::Vec3d  vecHitPlumbLine = pEllipsoidModel->computeLocalPlumbLine(m_ptMouseDownHitScenePos.y(), m_ptMouseDownHitScenePos.x());
        const double  dblHorzRotation = 0.1 * osg::DegreesToRadians(ea.getX() - m_pLastMouseEvent->getX());
        dblAzimuthBias += dblHorzRotation;
        const osg::Quat qtHorz(-dblHorzRotation, vecHitPlumbLine);


        // 1.2. 计算垂直方向的旋转量：垂直方向的旋转，旋转轴就是相机向右的向量
        double dblVertRotation = -0.1 * osg::DegreesToRadians(ea.getY() - m_pLastMouseEvent->getY());
        const double dblExpectedRotation = m_CurrentCameraPose.m_dblPitchAngle + dblVertRotation;
        if(dblVertRotation > 0.0)
        {
            // LookUp
            if(m_CurrentCameraPose.m_dblPitchAngle >= g_dblMaxElevAngle)
            {
                dblVertRotation = 0.0;
            }
            else if(dblExpectedRotation > g_dblMaxElevAngle)
            {
                dblVertRotation = g_dblMaxElevAngle - m_CurrentCameraPose.m_dblPitchAngle;
            }
        }
        else
        {
            // LookDown
            if(m_CurrentCameraPose.m_dblPitchAngle <= 0.0)
            {
                dblVertRotation = 0.0;
            }
            else if(dblExpectedRotation < 0.0)
            {
                dblVertRotation = -m_CurrentCameraPose.m_dblPitchAngle;
            }
        }

        dblPitchBias += dblVertRotation;

        osg::Vec3d vecRadial = ptCurCamPos - m_ptMouseDownHitScene;
        const double dblRotationRadius = vecRadial.normalize();

        const osg::Vec3d vecRight = getCurrentCameraRight();
        const osg::Quat qtVert(dblVertRotation, vecRight);


        // 2. 计算相机的平移量
        const osg::Quat   qtRotation  = qtHorz * qtVert;
        const osg::Vec3d  vecRadial1  = qtRotation * vecRadial;
        const osg::Vec3d  ptNewCamPos = m_ptMouseDownHitScene + vecRadial1 * dblRotationRadius;
        vecCameraBias = ptNewCamPos - ptCurCamPos;
    }
    else
    {
        // 旋转中心就是视点位置，此种漫游，视点位置是不会变化的，只会调整视线方向

        // 1. 计算相机的旋转量
        // 1.1. 水平方向的旋转量：水平方向的旋转，旋转轴就是当前视点位置的铅垂线
        const double dblMouseHorzBias = ea.getX() - m_pLastMouseEvent->getX();
        dblAzimuthBias += osg::DegreesToRadians(dblMouseHorzBias * 0.1);

        // 1.2. 计算垂直方向的旋转量：垂直方向的旋转，旋转轴就是相机向右的向量
        const osg::Vec3d  vecCurCamDir = getCurrentCameraDir();
        const osg::Vec3d  vecCurCamUp  = getCurrentCameraUp();
        const osg::Vec3d  vecHorzAxis  = vecCurCamDir ^ vecCurCamUp;
        const double dblMouseVertBias  = m_pLastMouseEvent->getY() - ea.getY();
        double dblVertRotation = osg::DegreesToRadians(dblMouseVertBias * 0.1);
        const double dblExpectedRotation = m_CurrentCameraPose.m_dblPitchAngle + dblVertRotation;
        if(dblVertRotation > 0.0)
        {
            // LookUp
            if(m_CurrentCameraPose.m_dblPitchAngle >= g_dblMaxElevAngle)
            {
                dblVertRotation = 0.0;
            }
            else if(dblExpectedRotation > g_dblMaxElevAngle)
            {
                dblVertRotation = g_dblMaxElevAngle - m_CurrentCameraPose.m_dblPitchAngle;
            }
        }
        else
        {
            // LookDown
            if(m_CurrentCameraPose.m_dblPitchAngle <= 0.0)
            {
                dblVertRotation = 0.0;
            }
            else if(dblExpectedRotation < 0.0)
            {
                dblVertRotation = -m_CurrentCameraPose.m_dblPitchAngle;
            }
        }

        dblPitchBias += dblVertRotation;
    }

    const osg::Vec3d ptNewCamPos = ptCurCamPos + vecCameraBias;

    double dblLat, dblLon, dblHeight;
    pEllipsoidModel->convertXYZToLatLongHeight
    (
        ptNewCamPos.x(), ptNewCamPos.y(), ptNewCamPos.z(),
        dblLat, dblLon, dblHeight
    );

    const double dblPlanetRadius = pEllipsoidModel->getRadius();
    m_CurrentCameraPose.m_dblPositionX     = dblLon;
    m_CurrentCameraPose.m_dblPositionY     = dblLat;
    m_CurrentCameraPose.m_dblHeight        = std::min(dblHeight, dblPlanetRadius * g_dblMaxDistance);
    m_CurrentCameraPose.m_dblAzimuthAngle += dblAzimuthBias;
    m_CurrentCameraPose.m_dblPitchAngle   += dblPitchBias;

    return true;
}


bool PlanetNavigator::OnScroll(const osgGA::GUIEventAdapter &ea)
{
    stopInertia();

    bool bMouseHitScene = false;
    osgUtil::Radial3 rayMouse;
    osg::Vec3d  ptHitTest;
    const bool bHasRadial = getScreenRadial(osg::Vec2d(ea.getXnormalized(), ea.getYnormalized()), rayMouse);
    if(bHasRadial)
    {
        if(getHitTestPoint(rayMouse, ptHitTest))
        {
            bMouseHitScene = true;
        }
    }

    const osg::Vec3d ptCurCamPos = getCurrentCameraPos();
    osg::Vec3d  vecDirection;
    if(bMouseHitScene)
    {
        vecDirection = ptHitTest - ptCurCamPos;
        vecDirection.normalize();
    }
    else
    {
        const osg::Vec3d vecCurCamDir = getCurrentCameraDir();
        const osgUtil::Radial3 raySightLine(ptCurCamPos, vecCurCamDir);
        if(!getHitTestPoint(raySightLine, ptHitTest))
        {
            return false;
        }

        vecDirection = vecCurCamDir;
    }

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const double dblDistance = (ptCurCamPos - ptHitTest).length();

    const bool   bScrollDown = (ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_DOWN);
    if(bScrollDown)
    {
        const double dblRadius = pEllipsoidModel->getRadius();
        const double dblMaxDistance = dblRadius * g_dblMaxDistance;
        if(dblDistance > dblMaxDistance)
        {
            return false;
        }
    }

    bool bvalid = false;
    double dblDeltaDistance = dblDistance * (bScrollDown ? -0.05 : 0.05);
    if(bScrollDown)
    {
        dblDeltaDistance = osg::clampBelow(dblDeltaDistance, -0.5);
    }
    else
    {
        dblDeltaDistance = osg::clampAbove(dblDeltaDistance, 0.5);
        if(m_bCameraUnderGround && dblDeltaDistance <= 0.5)
        {
            return false;
        }
    }

    const osg::Vec3d ptNewCamPos = ptCurCamPos + vecDirection * dblDeltaDistance;

    double dblLat, dblLon, dblHeight;
    pEllipsoidModel->convertXYZToLatLongHeight
    (
        ptNewCamPos.x(), ptNewCamPos.y(), ptNewCamPos.z(),
        dblLat, dblLon, dblHeight
    );
    m_CurrentCameraPose.m_dblPositionX = dblLon;
    m_CurrentCameraPose.m_dblPositionY = dblLat;

    const double dblPlanetRadius    = pEllipsoidModel->getRadius();
    m_CurrentCameraPose.m_dblHeight = std::min(dblHeight, dblPlanetRadius * g_dblMaxDistance);
    return true;
}


bool PlanetNavigator::OnFrame(const osgGA::GUIEventAdapter &ea)
{
    bool bHandled = false;
    if(doInertiaNavigation(ea.getTime()))
    {
        bHandled = true;
    }
    return bHandled;
}


bool PlanetNavigator::doInertiaNavigation(double dblCurTime)
{
    if(!m_pNavigationParam->m_bKeyboardInertia && !m_pNavigationParam->m_bMouseInertia)
    {
        return false;
    }

    if(m_bLButtonDown || m_bMButtonDown || m_bRButtonDown || hasKeyDown())
    {
        return false;
    }

    const double dblEpsilon = DBL_EPSILON * 10.0;
    if(m_vecInertiaSpeed.length2() < dblEpsilon)
    {
        m_vecInertiaSpeed.set(0.0, 0.0);
        return false;
    }

    const double dblDeltaTime = (dblCurTime > m_dblLastFrameTime ? dblCurTime - m_dblLastFrameTime : 0.0);

    m_CurrentCameraPose.m_dblPositionX += m_vecInertiaSpeed.x() * dblDeltaTime;
    m_CurrentCameraPose.m_dblPositionY += m_vecInertiaSpeed.y() * dblDeltaTime;
    m_vecInertiaSpeed *= 1.0 - m_pNavigationParam->m_dblFrictionalFactor;
    //std::cout << "deltaTime = " << dblDeltaTime << std::endl;

    return true;
}


bool PlanetNavigator::doKeyNavigation(double dblCurTime)
{
    // 1. Process the keys which cannot be combined
    // 1.1. Reset the camera
    if(m_mapKeyDownState[NavigationParam::NK_ResetCamera])
    {
        resetCamera();
        return true;
    }

    // 1.2. stop the inertia
    if(m_mapKeyDownState[NavigationParam::NK_StopInertia])
    {
        stopInertia();
        return true;
    }

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    // 2. Process the normal keys
    // 2.1. Calculate the base camera info
    bool bNavigated = false;

    const osg::Vec3d ptCurCamPos       = getCurrentCameraPos();
    const osg::Vec3d vecCurCamDir      = getCurrentCameraDir();
    const osg::Vec3d vecCurPlumbLine   = getCurrentPlumbLine();
    const osg::Vec3d vecCurCamUp       = getCurrentCameraUp();
    const osg::Vec3d vecCurCameraRight = vecCurCamDir ^ vecCurCamUp;
    const double dblPlanetRadius       = pEllipsoidModel->getRadius();

    osg::Vec3d  vecCameraBias;
    double      dblAzimuthBias = 0.0;
    double      dblPitchBias = 0.0;

    const double dblDeltaTime = (dblCurTime > m_dblLastFrameTime ? dblCurTime - m_dblLastFrameTime : 0.0f);

    // 2.2. Forword / Backword / Left / Right keys
    if(m_mapKeyDownState[NavigationParam::NK_Forward] || m_mapKeyDownState[NavigationParam::NK_Backward] || m_mapKeyDownState[NavigationParam::NK_Left] || m_mapKeyDownState[NavigationParam::NK_Right])
    {
        if(!m_pNavigationParam->m_bKeyboardInertia)
        {
            stopInertia();
        }

        bool bConflictLeftRight = true;
        osg::Vec3d vecRightLeftDir;
        if(m_mapKeyDownState[NavigationParam::NK_Left] || m_mapKeyDownState[NavigationParam::NK_Right])
        {
            bConflictLeftRight = (m_mapKeyDownState[NavigationParam::NK_Left] && m_mapKeyDownState[NavigationParam::NK_Right]);
            if(!bConflictLeftRight)
            {
                if(m_mapKeyDownState[NavigationParam::NK_Left])
                {
                    vecRightLeftDir = -vecCurCameraRight;
                }
                else
                {
                    vecRightLeftDir =  vecCurCameraRight;
                }
            }
        }

        const osg::Vec3d vecCurCamForward = vecCurCameraRight ^ vecCurPlumbLine;

        bool bConflictUpDown = true;
        osg::Vec3d vecFrontBackDir;
        if(m_mapKeyDownState[NavigationParam::NK_Forward] || m_mapKeyDownState[NavigationParam::NK_Backward])
        {
            bConflictUpDown = (m_mapKeyDownState[NavigationParam::NK_Forward] && m_mapKeyDownState[NavigationParam::NK_Backward]);
            if(!bConflictUpDown)
            {
                if(m_mapKeyDownState[NavigationParam::NK_Backward])
                {
                    vecFrontBackDir = -vecCurCamForward;
                }
                else
                {
                    vecFrontBackDir =  vecCurCamForward;
                }
            }
        }

        if(!bConflictUpDown || !bConflictLeftRight)
        {
            double dblActualSpeed = m_pNavigationParam->m_dblKeyboardTranslationSpeed;
            {
                const osgUtil::Radial3   rayPlumbLine(osg::Vec3d(0.0, 0.0, 0.0), -vecCurPlumbLine);
                osg::Vec3d              ptHitTest;
                getHitTestPoint(rayPlumbLine, ptHitTest);

                const double    dblCurCamHeight = (ptCurCamPos - ptHitTest).length();
                dblActualSpeed *= dblCurCamHeight / m_pNavigationParam->m_dblKeyboardSpeedBindHeight;
                dblActualSpeed /= osg::clampAbove(fabs(cos(m_CurrentCameraPose.m_dblPitchAngle)), 0.001);
            }
            if(m_mapKeyDownState[NavigationParam::NK_Shift_L])
            {
                dblActualSpeed *= m_pNavigationParam->m_dblShiftLMag;
            }
            if(m_mapKeyDownState[NavigationParam::NK_Shift_R])
            {
                dblActualSpeed *= m_pNavigationParam->m_dblShiftRMag;
            }


            osg::Vec3d vecMoveDir = vecFrontBackDir + vecRightLeftDir;
            vecMoveDir.normalize();
            vecMoveDir *= dblActualSpeed;

            vecCameraBias += vecMoveDir * dblDeltaTime;

            bNavigated = true;
        }
    }

    // 2.3. Lift or lower the camera position
    if(m_mapKeyDownState[NavigationParam::NK_Up] || m_mapKeyDownState[NavigationParam::NK_Down])
    {
        stopInertia();
        const bool bConflict = (m_mapKeyDownState[NavigationParam::NK_Up] && m_mapKeyDownState[NavigationParam::NK_Down]);
        if(!bConflict)
        {
            const double dblTranslateRatio = osg::clampAbove(fabs(m_CurrentCameraPose.m_dblHeight), 0.1);

            // 根据人的眼观感觉：
            //    1、若是10米/秒的速度水平运动，人感觉不是很快；
            //    2、若是以这个速度垂直上升，人会感觉非常快；
            //    3、若是人本身就在很高的高空，那么也不会感觉到这个速度很快；
            // 注意，这里的0.005是经验值！！
            double dblTranslation = m_pNavigationParam->m_dblKeyboardTranslationSpeed * dblDeltaTime;
            dblTranslation *= dblTranslateRatio * 0.005;

            if(m_mapKeyDownState[NavigationParam::NK_Shift_L])
            {
                dblTranslation *= m_pNavigationParam->m_dblShiftLMag;
            }
            if(m_mapKeyDownState[NavigationParam::NK_Shift_R])
            {
                dblTranslation *= m_pNavigationParam->m_dblShiftRMag;
            }

            if(m_mapKeyDownState[NavigationParam::NK_Up])
            {
                dblTranslation = -dblTranslation;
            }

            const osg::Vec3d vecMoveDir = vecCurPlumbLine * dblTranslation;
            vecCameraBias += vecMoveDir;

            bNavigated = true;
        }
    }

    // 2.4. Lift or lower the camera sight line
    if(m_mapKeyDownState[NavigationParam::NK_LookUp] || m_mapKeyDownState[NavigationParam::NK_LookDown])
    {
        stopInertia();
        // I should do some limited things here:
        //      1. If current action is LookUp, the sight line cannot over to the top of sky
        //      2. If current action is LookDown, the sight line cannot over to the bottom of planet center
        const bool bLookUp   = m_mapKeyDownState[NavigationParam::NK_LookUp];
        const bool bLookDown = m_mapKeyDownState[NavigationParam::NK_LookDown];
        const bool bConflict = (bLookUp && bLookDown);
        const bool bInvalid  = ((bLookUp && m_CurrentCameraPose.m_dblPitchAngle >= osg::PI) || (bLookDown && m_CurrentCameraPose.m_dblPitchAngle <= 0.0));
        if(!bInvalid && !bConflict)
        {
            double dblRotateAngle = m_pNavigationParam->m_dblKeyboardRotationSpeed * dblDeltaTime;
            if(m_mapKeyDownState[NavigationParam::NK_Shift_L])
            {
                dblRotateAngle *= m_pNavigationParam->m_dblShiftLMag;
            }
            if(m_mapKeyDownState[NavigationParam::NK_Shift_R])
            {
                dblRotateAngle *= m_pNavigationParam->m_dblShiftRMag;
            }

            if(bLookDown)   dblRotateAngle = -dblRotateAngle;

            const double dblExpectedAngle = m_CurrentCameraPose.m_dblPitchAngle + dblRotateAngle;
            if(bLookDown)
            {
                if(dblExpectedAngle <= 0.0)
                {
                    dblRotateAngle = -m_CurrentCameraPose.m_dblPitchAngle;
                }
            }
            else if(bLookUp)
            {
                if(dblExpectedAngle >= g_dblMaxElevAngle)
                {
                    dblRotateAngle = g_dblMaxElevAngle - m_CurrentCameraPose.m_dblPitchAngle;
                }
            }

            dblPitchBias += dblRotateAngle;
            bNavigated = true;
        }
    }

    // 2.5. Rotate camera sight line (rotate to left or right), the rotation center is the camera position
    if(m_mapKeyDownState[NavigationParam::NK_RotateRight] || m_mapKeyDownState[NavigationParam::NK_RotateLeft])
    {
        stopInertia();
        const bool bConflict = (m_mapKeyDownState[NavigationParam::NK_RotateRight] && m_mapKeyDownState[NavigationParam::NK_RotateLeft]);
        if(!bConflict)
        {
            double dblRotateAngle = m_pNavigationParam->m_dblKeyboardRotationSpeed * dblDeltaTime;
            if(m_mapKeyDownState[NavigationParam::NK_Shift_L])
            {
                dblRotateAngle *= m_pNavigationParam->m_dblShiftLMag;
            }
            if(m_mapKeyDownState[NavigationParam::NK_Shift_R])
            {
                dblRotateAngle *= m_pNavigationParam->m_dblShiftRMag;
            }
            if(m_mapKeyDownState[NavigationParam::NK_RotateLeft])
            {
                dblRotateAngle = -dblRotateAngle;
            }

            dblAzimuthBias += dblRotateAngle;
        }

        bNavigated = true;
    }

    // 2.6. Rotate camera sight line (rotate to left or right), the rotation center is the intersection point of sight line to planet
    if(m_mapKeyDownState[NavigationParam::NK_RotateLeft_V] || m_mapKeyDownState[NavigationParam::NK_RotateRight_V])
    {
        stopInertia();
        const bool bConflict = (m_mapKeyDownState[NavigationParam::NK_RotateLeft_V] && m_mapKeyDownState[NavigationParam::NK_RotateRight_V]);
        if(!bConflict)
        {
            double dblRotationAngle = m_pNavigationParam->m_dblKeyboardRotationSpeed * dblDeltaTime;
            if(m_mapKeyDownState[NavigationParam::NK_Shift_L])
            {
                dblRotationAngle *= m_pNavigationParam->m_dblShiftLMag;
            }
            if(m_mapKeyDownState[NavigationParam::NK_Shift_R])
            {
                dblRotationAngle *= m_pNavigationParam->m_dblShiftRMag;
            }
            if(m_mapKeyDownState[NavigationParam::NK_RotateLeft_V])
            {
                dblRotationAngle = -dblRotationAngle;
            }


            const osgUtil::Radial3  raySightline(ptCurCamPos, vecCurCamDir);
            osg::Vec3d              ptCircleCenter;
            if(getHitTestPoint(raySightline, ptCircleCenter))
            {
                // 视线的旋转量
                dblAzimuthBias += dblRotationAngle;

                // 计算出视点的旋转中心和旋转半径
                osg::Vec3d vecRadial = ptCurCamPos - ptCircleCenter;
                const double dblRotationRadius = vecRadial.normalize();

                osg::Vec3d ptCircleCenterPos;
                pEllipsoidModel->convertXYZToLatLongHeight(ptCircleCenter.x(), ptCircleCenter.y(), ptCircleCenter.z(), ptCircleCenterPos.y(), ptCircleCenterPos.x(), ptCircleCenterPos.z());
                const osg::Vec3d vecCenterPlumbLine = pEllipsoidModel->computeLocalPlumbLine(ptCircleCenterPos.y(), ptCircleCenterPos.x());

                const osg::Quat qtRotate(-dblRotationAngle, vecCenterPlumbLine);
                const osg::Vec3d vecRadial1 = qtRotate * vecRadial;

                // 视点的平移量
                const osg::Vec3d ptNewCameraPos = ptCircleCenter + vecRadial1 * dblRotationRadius;
                const osg::Vec3d vecCameraDetal = ptNewCameraPos - ptCurCamPos;
                vecCameraBias += vecCameraDetal;

                bNavigated = true;
            }
        }
    }

    // 2.7. Apply the navigation
    if(bNavigated)
    {
        const osg::Vec3d ptNewCamPos = ptCurCamPos + vecCameraBias;

        double dblNewLat, dblNewLon, dblNewHeight;
        pEllipsoidModel->convertXYZToLatLongHeight
        (
            ptNewCamPos.x(), ptNewCamPos.y(), ptNewCamPos.z(),
            dblNewLat, dblNewLon, dblNewHeight
        );

        if(m_pNavigationParam->m_bKeyboardInertia)
        {
            m_vecInertiaSpeed.set(dblNewLon - m_CurrentCameraPose.m_dblPositionX, dblNewLat - m_CurrentCameraPose.m_dblPositionY);
            m_vecInertiaSpeed /= dblDeltaTime;
        }

        m_CurrentCameraPose.m_dblPositionX     = dblNewLon;
        m_CurrentCameraPose.m_dblPositionY     = dblNewLat;
        m_CurrentCameraPose.m_dblHeight        = std::min(dblNewHeight, dblPlanetRadius * g_dblMaxDistance);
        m_CurrentCameraPose.m_dblAzimuthAngle += dblAzimuthBias;
        m_CurrentCameraPose.m_dblPitchAngle   += dblPitchBias;
    }

    return bNavigated;
}


bool PlanetNavigator::getScreenRadial(const osg::Vec2d &ptPosNormalize, osgUtil::Radial3 &ray) const
{
    const osg::Matrixd &mtxProj      = m_pCurrentCamera->getProjectionMatrix();
    const osg::Matrixd &mtxView      = m_pCurrentCamera->getViewMatrix();
    const osg::Matrixd &mtxInverseVP = osg::Matrixd::inverse(mtxView * mtxProj);

    const osg::Vec3d ptNear(ptPosNormalize.x(), ptPosNormalize.y(), -1.0f);
    const osg::Vec3d ptMid(ptPosNormalize.x(), ptPosNormalize.y(), 0.0f);

    const osg::Vec3d ptRayBegin = ptNear * mtxInverseVP;
    const osg::Vec3d ptRayEnd   = ptMid  * mtxInverseVP;
    osg::Vec3d       vecRayDir  = ptRayEnd - ptRayBegin;
    const double     fltLen     = vecRayDir.normalize();
    if(!vecRayDir.valid())      return false;
    if(fltLen <= FLT_EPSILON)   return false;

    ray.setOrigin(ptRayBegin);
    ray.setDirection(vecRayDir);

    return true;
}


bool PlanetNavigator::getHitTestPoint(const osgUtil::Radial3 &ray, osg::Vec3d &ptHitTest, bool bAlwaysSphere/* = false*/) const
{
    //if(m_dblCamera2Ground < FLT_EPSILON)
    if(m_bCameraUnderGround)
    {
        //std::cout << "under ground\n";
        if(bAlwaysSphere)
        {
            return false;
        }

        const osg::Vec3d ptCameraPos  = getCurrentCameraPos();
        const osg::Vec3d vecPlumbLine = getCurrentPlumbLine();
        const osgUtil::Radial3 raySpoke(ptCameraPos, vecPlumbLine);
        osg::Vec3d ptGround;
        if(!hitScene(raySpoke, ptGround, false))
        {
            return false;
        }

        ptHitTest = ptGround;
    }
    else if(bAlwaysSphere)
    {
        osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
        //std::cout << "upon ground\n";
        const double dblPlanetRadius = pEllipsoidModel->getRadius();
        const osg::BoundingSphere bs(osg::Vec3d(0.0, 0.0, 0.0), dblPlanetRadius);
        osg::Vec3d point1, point2;
        if(ray.hitSphere(bs, point1, point2) < 1u)
        {
            return false;
        }

        if(!point1.valid())
        {
            return false;
        }
        ptHitTest = point1;
    }
    else
    {
        osg::Vec3d point;
        if(!hitScene(ray, point, false))
        {
            return false;
        }

        const osg::Vec3d ptCamera = getCurrentCameraPos();
        osg::Vec3d vec1 = point;
        osg::Vec3d vec2 = ptCamera;
        vec1.normalize();
        vec2.normalize();
        const double dbl = vec1 * vec2;
        if(dbl < 0.01)
        {
            return false;
        }

        ptHitTest = point;
    }

    return true;
}


bool PlanetNavigator::hitScene(const osgUtil::Radial3 &ray, osg::Vec3d &ptHitTest, bool bTerrainOnly) const
{
    const osgUtil::LineSegmentIntersector::CoordinateFrame cf = osgUtil::Intersector::WINDOW;
    osg::ref_ptr<osgUtil::LineSegmentIntersector>   pPicker  = new osgUtil::LineSegmentIntersector(cf, ray.getOrigin(), ray.getPoint(1e9));
    osg::ref_ptr<osgUtil::IntersectionVisitor>      pVisitor = new osgUtil::IntersectionVisitor(pPicker.get());
    //pVisitor->setTraversalMask(0x00FFFFFF);
    if(bTerrainOnly)
    {
        m_pTerrainNode->accept(*pVisitor);
    }
    else
    {
        m_pNavigationNode->accept(*pVisitor);
    }

    if(!pPicker->containsIntersections())
    {
        return false;
    }

    const osg::Vec3d ptCameraPos = getCurrentCameraPos();
    const osgUtil::LineSegmentIntersector::Intersections &inters = pPicker->getIntersections();
    //osgUtil::LineSegmentIntersector::Intersections::const_iterator itor = inters.begin();
    //osg::Vec3d ptHit;
    //double dblMin = FLT_MAX;
    //for( ; itor != inters.end(); ++itor)
    //{
    //    const osgUtil::LineSegmentIntersector::Intersection &inter = *itor;
    //    osg::Vec3d point = inter.getWorldIntersectPoint();
    //    const double dbl = (ptCameraPos - point).length();
    //    if(dbl < dblMin)
    //    {
    //        dblMin = dbl;
    //        ptHit = point;
    //    }
    //}
    //ptHitTest = ptHit;
    ptHitTest = pPicker->getFirstIntersection().getWorldIntersectPoint();
    return true;
}


void PlanetNavigator::stopInertia(void)
{
    m_vecInertiaSpeed.set(0.0, 0.0);
}


void PlanetNavigator::resetCamera(void)
{
    stopInertia();
    calculateHomePose();
}


bool PlanetNavigator::hasKeyDown(void) const
{
    std::map<NavigationParam::NavKeyboard, bool>::const_iterator itor = m_mapKeyDownState.begin();
    for(; itor != m_mapKeyDownState.end(); ++itor)
    {
        if(itor->second)
        {
            if(itor->first != NavigationParam::NK_Shift_L
                && itor->first != NavigationParam::NK_Shift_R
                && itor->first != NavigationParam::NK_Control_L
                && itor->first != NavigationParam::NK_Control_R)
            {
                return true;
            }
        }
    }
    return false;
}


bool PlanetNavigator::calcNavBasePlane(osg::Plane &plane, bool bHorz/* = true*/) const
{
    if(!m_bMouseDownHitScene)
    {
        return false;
    }

    if(bHorz)
    {
        osg::Vec3d vecNormal = m_ptMouseDownHitScene;
        vecNormal.normalize();
        plane.set(vecNormal, m_ptMouseDownHitScene);
    }
    else
    {
        const osg::Vec3d vecCurCamDir = getCurrentCameraDir();
        plane.set(-vecCurCamDir, m_ptMouseDownHitScene);
        //const osg::Vec3d ptCurCamPos = getCurrentCameraPos();
        //osg::Vec3d  vecHit2Cam = m_ptMouseDownHitScene - ptCurCamPos;
        //vecHit2Cam.normalize();
        //plane.set(vecHit2Cam, m_ptMouseDownHitScene);
    }

    return true;
}


void PlanetNavigator::getCameraPose(CameraPose &pose) const
{
    pose = m_CurrentCameraPose;
}


void PlanetNavigator::setCameraPose(const CameraPose &pose)
{
    stopInertia();

    m_CurrentCameraPose = pose;

    if(m_nCameraPoseChangedByExternal < 0)
    {
        m_nCameraPoseChangedByExternal = 0;
    }
    m_nCameraPoseChangedByExternal++;
}

void PlanetNavigator::enableInertia(bool bEnable)
{
	m_pNavigationParam->m_bMouseInertia = bEnable;
}

void PlanetNavigator::enableUnderGroundViewMode(bool bEnable)
{
	m_pNavigationParam->m_bUnderGroundViewMode = bEnable;
}
