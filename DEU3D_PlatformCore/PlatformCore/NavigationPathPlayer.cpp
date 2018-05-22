#include "NavigationPathPlayer.h"
#include <osgViewer/View>
#include <algorithm>
#include <assert.h>
#include <EventAdapter/IEventObject.h>
#include "NavigatorManager.h"
#include "AddOrRemove_Operation.h"

NavigationPathPlayer::NavigationPathPlayer(ea::IEventAdapter *pEventAdapter)
    : m_pEventAdapter(pEventAdapter)
{
    m_eNavPathType      = NPT_NavigationPath;
    m_ePlayingState     = PS_STOP;
    m_dblDurationTime   = 0.0;
    m_dblTimeSpeed      = 1.0;
    m_dblSmoothLevel    = 1.0;
    m_bCurveFromTo      = true;
    m_dblLastNavigationTime = 0.0;
    m_dblLastTimeEventTime  = 0.0;
    m_pAnimationNavPath     = NULL;
    m_bRButtonDown          = false;
    m_pLastMouseEvent       = NULL;
    m_bFixCameraDirWhilePlaying = false;
    m_blockHandleFinish.set(true);
    m_bIsFinishing.exchange(0u);
}


NavigationPathPlayer::~NavigationPathPlayer(void)
{
}


bool NavigationPathPlayer::initialize(INavigatorManager *pNavigatorManager, SceneGraphOperator *pSceneGraphOperator)
{
    assert(pNavigatorManager != NULL);

    m_pNavigatorManager = pNavigatorManager;
    m_pSceneGraphOperator = pSceneGraphOperator;

    m_pNavigationParam = m_pNavigatorManager->getNavigationParam();
    m_pAnimationNavPath = NULL;
    m_pPlayerNode = new osg::Group;

    //setNavigationModel(new AnimationModel);
    return true;
}


void NavigationPathPlayer::setNavigationPath(const INavigationPath *pPath)
{
    if((unsigned)m_bIsFinishing)    return;

    if(m_ePlayingState != PS_STOP)
    {
        m_bIsFinishing.exchange(1u);
        m_ePlayingState = PS_STOP;
        m_blockHandleFinish.reset();
        m_blockHandleFinish.block();
    }

    stop();

    m_eNavPathType = NPT_NavigationPath;
    m_pNavigationPath = dynamic_cast<const NavigationPath *>(pPath);
    m_pAnimationNavPath = NULL;

    prepare();
}


void NavigationPathPlayer::setNavigationPathByFromTo(const INavigationKeyframe *pFrameFrom, const INavigationKeyframe *pFrameTo, bool bCurve)
{
    if((unsigned)m_bIsFinishing)    return;

    if(m_ePlayingState != PS_STOP)
    {
        m_bIsFinishing.exchange(1u);
        m_ePlayingState = PS_STOP;
        m_blockHandleFinish.reset();
        m_blockHandleFinish.block();
    }

    stop();

    m_eNavPathType      = NPT_Keyframe2Keyframe;

    m_bCurveFromTo      = bCurve;
    m_pNavKeyframeFrom  = dynamic_cast<const NavigationKeyframe *>(pFrameFrom);
    m_pNavKeyframeTo    = dynamic_cast<const NavigationKeyframe *>(pFrameTo);
    m_pAnimationNavPath = NULL;

    prepare();
}


void NavigationPathPlayer::setNavigationPathByFromTo(const INavigationKeyframe *pFrameFrom, double dblPosX, double dblPosY, double dblPosH, double dblRadius, bool bCurve)
{
    if((unsigned)m_bIsFinishing)    return;

    if(m_ePlayingState != PS_STOP)
    {
        m_bIsFinishing.exchange(1u);
        m_ePlayingState = PS_STOP;
        m_blockHandleFinish.reset();
        m_blockHandleFinish.block();
    }

    stop();

    m_eNavPathType      = NPT_Keyframe2Sphere;

    m_bCurveFromTo      = bCurve;
    m_pNavKeyframeFrom  = dynamic_cast<const NavigationKeyframe *>(pFrameFrom);
    m_NavSphereTo.set(osg::Vec3d(dblPosX, dblPosY, dblPosH), dblRadius);
    m_pAnimationNavPath = NULL;

    prepare();
}


void NavigationPathPlayer::setSmoothLevel(double dblSmoothLevel)
{
    if((unsigned)m_bIsFinishing)    return;

    if(m_ePlayingState != PS_STOP)
    {
        m_bIsFinishing.exchange(1u);
        m_ePlayingState = PS_STOP;
        m_blockHandleFinish.reset();
        m_blockHandleFinish.block();
    }

    stop();

    m_dblSmoothLevel = osg::clampBetween(dblSmoothLevel, 0.0, 1.0);
    m_pAnimationNavPath = NULL;

    prepare();
}


void NavigationPathPlayer::setNavigationModel(IAnimationModel *pNaviModel)
{
    if((unsigned)m_bIsFinishing)    return;

    if(m_ePlayingState != PS_STOP)
    {
        m_bIsFinishing.exchange(1u);
        m_ePlayingState = PS_STOP;
        m_blockHandleFinish.reset();
        m_blockHandleFinish.block();
    }

    stop();

    m_pNavigationModel = dynamic_cast<AnimationModel *>(pNaviModel);
    if(m_pPlayerNode->getNumChildren() > 0u)
    {
        const unsigned nCount = m_pPlayerNode->getNumChildren();
        for(unsigned n = 0; n < nCount; n++)
        {
            OpenSP::sp<RemoveChildFromParent_Operation>    pOperation = new RemoveChildFromParent_Operation;
            pOperation->addRemovePair(m_pPlayerNode.get(), m_pPlayerNode->getChild(0u));
            m_pSceneGraphOperator->pushOperation(pOperation.get());
        }
    }

    if(m_pNavigationModel)
    {
        OpenSP::sp<AddChildToParent_Operation>    pOperation = new AddChildToParent_Operation;
        pOperation->setAddPair(m_pPlayerNode.get(), m_pNavigationModel->getModelNode());
        m_pSceneGraphOperator->pushOperation(pOperation.get());
    }
}


void NavigationPathPlayer::prepare(void)
{
    buildAnimationNavPath();
    m_pAnimationNavPath->computeDuration();
}


std::string NavigationPathPlayer::getPlayingState(void) const
{
    std::string strState;
    switch(m_ePlayingState)
    {
    case PS_PLAY:
        strState = "play";
        break;
    case PS_PAUSE:
        strState = "pause";
        break;
    case PS_STOP:
        strState = "stop";
        break;
    }
    return strState;
}


void NavigationPathPlayer::play(void)
{
    //if(!m_pAnimationNavPath.valid())    return;
    if(m_ePlayingState == PS_PLAY)    return;

    if(m_ePlayingState == PS_PAUSE)
    {
        const double dblCurTime = osg::Timer::instance()->time_s();
        m_dblLastNavigationTime = dblCurTime;
    }
    else
    {
        m_dblLastNavigationTime = osg::Timer::instance()->time_s();
        m_dblLastTimeEventTime  = m_dblLastNavigationTime;
        m_dblDurationTime       = 0.0;
        m_dblTimeSpeed          = 1.0;
        m_pAnimationNavPath->setStartTime(0.0);
        m_itorTimeStamp = m_vecTimeStamp.end();

        m_poseNavigatedBias.m_dblPositionX     = 0.0;
        m_poseNavigatedBias.m_dblPositionY     = 0.0;
        m_poseNavigatedBias.m_dblHeight        = 0.0;
        m_poseNavigatedBias.m_dblAzimuthAngle  = 0.0;
        m_poseNavigatedBias.m_dblPitchAngle    = 0.0;
    }

    m_ePlayingState = PS_PLAY;
}


void NavigationPathPlayer::stop(void)
{
    if(m_ePlayingState == PS_STOP)    return;

    m_dblLastNavigationTime = 0.0;
    m_dblLastTimeEventTime  = 0.0;
    m_dblDurationTime       = 0.0;
    m_dblTimeSpeed          = 1.0;
    m_ePlayingState         = PS_STOP;
    m_poseNavigatedBias.m_dblAzimuthAngle  = 0.0;
    m_poseNavigatedBias.m_dblPitchAngle    = 0.0;
    m_poseNavigatedBias.m_dblHeight        = 0.0;
    m_poseNavigatedBias.m_dblPositionX     = 0.0;
    m_poseNavigatedBias.m_dblPositionY     = 0.0;
}


void NavigationPathPlayer::pause(void)
{
    if(m_ePlayingState != PS_PLAY)    return;

    m_ePlayingState = PS_PAUSE;
}


double NavigationPathPlayer::getNavigationPathLength(void) const
{
    if(!m_pAnimationNavPath.valid())    return 0.0;
    return m_pAnimationNavPath->getDuration();
}


double NavigationPathPlayer::getPlayingPosition(void) const
{
    return m_dblDurationTime;
}


void NavigationPathPlayer::setPlayingPosition(double dblPos)
{
    if(!m_pAnimationNavPath.valid())    return;

    m_dblDurationTime = osg::clampBetween(dblPos, 0.0, getNavigationPathLength());

    CameraPose pose;
    getCameraPoseByTime(m_dblDurationTime, pose);

    fixPose(pose);

    if(m_pNavigationModel.valid())
    {
        m_pNavigationModel->setPose(pose);
        pose = m_pNavigationModel->getObserverPose();
    }
    m_pNavigatorManager->setCameraPose(pose);
}


bool NavigationPathPlayer::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    if(!m_pAnimationNavPath.valid())
    {
        m_bIsFinishing.exchange(0u);
        m_blockHandleFinish.release();
        return false;
    }
    if(m_ePlayingState == PS_STOP)
    {
        m_bIsFinishing.exchange(0u);
        m_blockHandleFinish.release();
        return false;
    }

    bool bProcessed = false;
    switch(ea.getEventType())
    {
        case osgGA::GUIEventAdapter::KEYDOWN:
            bProcessed = OnKeyDown(ea);
            break;
        case osgGA::GUIEventAdapter::KEYUP:
            bProcessed = OnKeyUp(ea);
            break;
        case osgGA::GUIEventAdapter::PUSH:
            bProcessed = OnMouseDown(ea);
            break;
        case osgGA::GUIEventAdapter::RELEASE:
            bProcessed = OnMouseUp(ea);
            break;
        case osgGA::GUIEventAdapter::MOVE:
            bProcessed = OnMouseMove(ea);
            break;
        case osgGA::GUIEventAdapter::DRAG:
            bProcessed = OnMouseDrag(ea);
            break;
    }

    if(m_ePlayingState == PS_PLAY)
    {
        CameraPose pose = Navigate();
        processPose(pose);
        m_pNavigatorManager->setCameraPose(pose);
    }
    else if(m_ePlayingState == PS_PAUSE)
    {
        if(bProcessed)
        {
            CameraPose pose = Navigate();
            processPose(pose);
            m_pNavigatorManager->setCameraPose(pose);
        }
        m_blockHandleFinish.release();
        m_bIsFinishing.exchange(0u);
        return bProcessed;
    }

    m_bIsFinishing.exchange(0u);
    m_blockHandleFinish.release();
    return true;
}


CameraPose NavigationPathPlayer::Navigate(void)
{
    // 1. 计算当前的播放时间
    bool    bShouldFireTimeEvent = false;
    if(m_ePlayingState == PS_PLAY)
    {
        const double dblCurTime = osg::Timer::instance()->time_s();

        if(!m_mapKeyDownState[NavigationParam::NK_NavPath_StepForward]
            && !m_mapKeyDownState[NavigationParam::NK_NavPath_StepBackward])
        {
            const double dblDuration = dblCurTime - m_dblLastNavigationTime;
            m_dblDurationTime += dblDuration * m_dblTimeSpeed;
            if(m_dblDurationTime < 0.0)
            {
                m_dblDurationTime = 0.0;
            }
            else if(m_dblDurationTime > m_vecTimeStamp.back())
            {
                m_dblDurationTime = m_vecTimeStamp.back();
            }
        }

        m_dblLastNavigationTime = dblCurTime;

        if(m_dblLastNavigationTime - m_dblLastTimeEventTime >= 0.0)
        {
            m_dblLastTimeEventTime = m_dblLastNavigationTime;
            bShouldFireTimeEvent   = true;
        }
    }

    // 2. 根据当前播放时间来获取相机姿态
    CameraPose pose;
    getCameraPoseByTime(m_dblDurationTime, pose);

    // 3. 检测换帧事件
    std::vector<double>::const_iterator itorStamp = std::upper_bound(m_vecTimeStamp.begin(), m_vecTimeStamp.end(), m_dblDurationTime);
    if(m_eNavPathType != NPT_NavigationPath || !m_pEventAdapter.valid() )
    {
        // 只是一个飞行转到，不需要任何换帧事件
        // 获取本来就没有事件适配器，发不了事件

        if(itorStamp == m_vecTimeStamp.end())
        {
            if(m_ePlayingState == PS_PLAY)
            {
                // 播放结束
                stop();
            }
        }
        return pose;
    }

    bool        bShouldFireKeyEvent = false;
    unsigned    nKeyframeIndex = 0u;
    if(itorStamp == m_vecTimeStamp.end())
    {
        if(m_ePlayingState == PS_PLAY)
        {
            // 播放结束
            stop();

            osg::notify(osg::NOTICE) << "play finished." << std::endl;
            bShouldFireKeyEvent = true;

            nKeyframeIndex = m_vecTimeStamp.size() - 1u;
        }
    }
    else if(itorStamp != m_itorTimeStamp)
    {
        if(m_itorTimeStamp == m_vecTimeStamp.end())
        {
            // 这是整个路径的第一个关键帧
            osg::notify(osg::NOTICE) << "this is the first key frame." << std::endl;
            nKeyframeIndex = 0u;
        }
        else
        {
            // 换帧m_listNavKeyframes
            osg::notify(osg::NOTICE) << "change key frame." << std::endl;
            nKeyframeIndex = m_itorTimeStamp - m_vecTimeStamp.begin();
        }

        bShouldFireKeyEvent = true;
        m_itorTimeStamp = itorStamp;
    }

    if(bShouldFireKeyEvent && (m_ePlayingState != PS_PAUSE))
    {
        OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
        pEventObject->setAction(ea::ACTION_NAVPATH_KEYFRAME);

        const std::string strType = "Key";
        pEventObject->putExtra("Type", strType);
        pEventObject->putExtra("FrameIndex", nKeyframeIndex);
        pEventObject->putExtra("Time", m_dblDurationTime);
        pEventObject->putExtra("UserDefinitionData", m_pNavigationPath->getItem(nKeyframeIndex)->getUserDefinitionData());

        m_pEventAdapter->sendBroadcast(pEventObject.get());
    }

    if(bShouldFireTimeEvent && (m_ePlayingState != PS_PAUSE) && (nKeyframeIndex < m_pNavigationPath->getItemCount()))
    {
        OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
        pEventObject->setAction(ea::ACTION_NAVPATH_KEYFRAME);

        const std::string strType = "Time";
        pEventObject->putExtra("Type", strType);
        pEventObject->putExtra("FrameIndex", nKeyframeIndex);
        pEventObject->putExtra("Time", m_dblDurationTime);
        pEventObject->putExtra("UserDefinitionData", m_pNavigationPath->getItem(nKeyframeIndex)->getUserDefinitionData());
        m_pEventAdapter->sendBroadcast(pEventObject.get());
    }

    return pose;
}


void NavigationPathPlayer::getCameraPoseByTime(double dblTime, CameraPose &pose)
{
    m_pAnimationNavPath->resetTargets();
    m_pAnimationNavPath->update(dblTime + m_pAnimationNavPath->getStartTime());

    const osg::Vec2d ptCameraPos = m_pTranslationTarget->getValue();
    pose.m_dblPositionX     = ptCameraPos.x();
    pose.m_dblPositionY     = ptCameraPos.y();
    pose.m_dblHeight        = m_pHeightTarget->getValue();
    pose.m_dblAzimuthAngle  = m_pAzimuthTarget->getValue();
    pose.m_dblPitchAngle    = m_pPitchTarget->getValue();
}


void NavigationPathPlayer::processPose(CameraPose &pose) const
{
    //播放时可以调整视线
    if(m_bFixCameraDirWhilePlaying)
    {
        if(m_pNavigationModel.valid())
        {
            m_pNavigationModel->setPose(pose);
            pose = m_pNavigationModel->getObserverPose(/*m_vecNavigatedBias*/);
        }

        pose.m_dblPositionX    += m_poseNavigatedBias.m_dblPositionX;
        pose.m_dblPositionY    += m_poseNavigatedBias.m_dblPositionY;
        pose.m_dblHeight       += m_poseNavigatedBias.m_dblHeight;
        pose.m_dblAzimuthAngle += m_poseNavigatedBias.m_dblAzimuthAngle;
        pose.m_dblPitchAngle   += m_poseNavigatedBias.m_dblPitchAngle;
    }
    //播放时不可以调整视线
    else
    {
        if(m_pNavigationModel.valid())
        {
            m_pNavigationModel->setPose(pose);
            pose = m_pNavigationModel->getObserverPose(m_poseNavigatedBias.m_dblPitchAngle, m_poseNavigatedBias.m_dblAzimuthAngle);
        }

    }
    fixPose(pose);
}


void NavigationPathPlayer::fixPose(CameraPose &pose) const
{
    const double dblPI_M2 = osg::PI + osg::PI;
    while(pose.m_dblAzimuthAngle < -osg::PI)    pose.m_dblAzimuthAngle += dblPI_M2;
    while(pose.m_dblAzimuthAngle >  osg::PI)    pose.m_dblAzimuthAngle -= dblPI_M2;

    while(pose.m_dblPositionX < -osg::PI)   pose.m_dblPositionX += dblPI_M2;
    while(pose.m_dblPositionX >  osg::PI)   pose.m_dblPositionX -= dblPI_M2;

    while(pose.m_dblPositionY < -osg::PI)   pose.m_dblPositionY += dblPI_M2;
    while(pose.m_dblPositionY >  osg::PI)   pose.m_dblPositionY -= dblPI_M2;
    if(pose.m_dblPositionY < -osg::PI_2)    pose.m_dblPositionY  = -pose.m_dblPositionY - osg::PI;
    else if(pose.m_dblPositionY > osg::PI_2)pose.m_dblPositionY  = -pose.m_dblPositionY + osg::PI;

    pose.m_dblPitchAngle = osg::clampBetween(pose.m_dblPitchAngle, 0.0, osg::PI);
}


bool NavigationPathPlayer::OnKeyDown(const osgGA::GUIEventAdapter &ea)
{
    return processKeyboard(true, ea);
}


bool NavigationPathPlayer::OnKeyUp(const osgGA::GUIEventAdapter &ea)
{
    return processKeyboard(false, ea);
}


bool NavigationPathPlayer::processKeyboard(bool bKeyDown, const osgGA::GUIEventAdapter &ea)
{
    bool bProcessed = false;

    int nKey = ea.getKey();
    if(nKey >= 'A' && nKey <= 'Z')
    {
        nKey += 'a' - 'A';
    }

    if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_NavPath_StepForward])
    {
        if(bKeyDown)
        {
            const double dblRemainder = m_pAnimationNavPath->getDuration() - m_dblDurationTime;
            if(dblRemainder >= 0.2)
            {
                m_dblDurationTime += 0.2;
            }
            else
            {
                m_dblDurationTime = m_pAnimationNavPath->getDuration();
            }
        }
        m_mapKeyDownState[NavigationParam::NK_NavPath_StepForward] = bKeyDown;
        bProcessed = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_NavPath_StepBackward])
    {
        if(bKeyDown)
        {
            const double dblRemainder = m_dblDurationTime;
            if(dblRemainder <= 0.2)
            {
                m_dblDurationTime = 0.0;
            }
            else
            {
                m_dblDurationTime -= 0.2;
            }
        }
        m_mapKeyDownState[NavigationParam::NK_NavPath_StepBackward] = bKeyDown;
        bProcessed = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_NavPath_Pause])
    {
        if(bKeyDown && !m_mapKeyDownState[NavigationParam::NK_NavPath_Pause])
        {
            if(m_ePlayingState == PS_PLAY)
            {
                pause();
            }
            else if(m_ePlayingState == PS_PAUSE)
            {
                play();
            }
        }
        m_mapKeyDownState[NavigationParam::NK_NavPath_Pause] = bKeyDown;
        bProcessed = true;
    }

    if(m_ePlayingState != PS_PLAY)    return bProcessed;

    if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_Forward])
    {
        m_mapKeyDownState[NavigationParam::NK_Forward] = bKeyDown;
        bProcessed = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_Backward])
    {
        m_mapKeyDownState[NavigationParam::NK_Backward] = bKeyDown;
        bProcessed = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_Left])
    {
        m_mapKeyDownState[NavigationParam::NK_Left] = bKeyDown;
        bProcessed = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_Right])
    {
        m_mapKeyDownState[NavigationParam::NK_Right] = bKeyDown;
        bProcessed = true;
    }

    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_Up])
    {
        m_mapKeyDownState[NavigationParam::NK_Up] = bKeyDown;
        bProcessed = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_Down])
    {
        m_mapKeyDownState[NavigationParam::NK_Down] = bKeyDown;
        bProcessed = true;
    }

    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_LookUp])
    {
        m_mapKeyDownState[NavigationParam::NK_LookUp] = bKeyDown;
        bProcessed = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_LookDown])
    {
        m_mapKeyDownState[NavigationParam::NK_LookDown] = bKeyDown;
        bProcessed = true;
    }

    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_RotateRight])
    {
        m_mapKeyDownState[NavigationParam::NK_RotateRight] = bKeyDown;
        bProcessed = true;
    }
    else if(nKey == m_pNavigationParam->m_NavKeyboardConfig[NavigationParam::NK_RotateLeft])
    {
        m_mapKeyDownState[NavigationParam::NK_RotateLeft] = bKeyDown;
        bProcessed = true;
    }

    return bProcessed;
}


bool NavigationPathPlayer::OnMouseDown(const osgGA::GUIEventAdapter &ea)
{
	if(m_ePlayingState != PS_PLAY)
	{
		return false;
	}

	if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
	{
		pause();
	}

    if(ea.getButton() != osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {
        return false;
    }

    m_bRButtonDown = true;
    m_pLastMouseEvent = &ea;
    return true;
}


bool NavigationPathPlayer::OnMouseUp(const osgGA::GUIEventAdapter &ea)
{
    if(m_ePlayingState != PS_PLAY || ea.getButton() != osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {
        return false;
    }

    m_bRButtonDown = false;
    m_pLastMouseEvent = NULL;
    return true;
}


bool NavigationPathPlayer::OnMouseMove(const osgGA::GUIEventAdapter &ea)
{
    return false;
}


bool NavigationPathPlayer::OnMouseDrag(const osgGA::GUIEventAdapter &ea)
{
    if(m_ePlayingState != PS_PLAY || !m_bRButtonDown || !m_pLastMouseEvent.valid())
    {
        return false;
    }

    const bool bDone = doRMouseDragNavigation(ea);
    m_pLastMouseEvent = &ea;
    return bDone;
}


bool NavigationPathPlayer::doRMouseDragNavigation(const osgGA::GUIEventAdapter &ea)
{
    if(!m_bRButtonDown)     return false;

    const double dblMouseHorzBias = ea.getX() - m_pLastMouseEvent->getX();

    m_poseNavigatedBias.m_dblAzimuthAngle += osg::DegreesToRadians(dblMouseHorzBias * 0.1);

    const double dblMouseVertBias = m_pLastMouseEvent->getY() - ea.getY();
    m_poseNavigatedBias.m_dblPitchAngle   += osg::DegreesToRadians(dblMouseVertBias * 0.1);

    return true;
}


bool NavigationPathPlayer::buildAnimationNavPath(void)
{
    osg::ref_ptr<AeroCalculator>    pAeroCalculator = new AeroCalculator;
    pAeroCalculator->initialize();
    switch(m_eNavPathType)
    {
        case NPT_NavigationPath:
            pAeroCalculator->setNavigationPath(m_pNavigationPath);
            break;
        case NPT_Keyframe2Keyframe:
            pAeroCalculator->setFlyFromTo(m_pNavKeyframeFrom, m_pNavKeyframeTo, m_bCurveFromTo);
            break;
        case NPT_Keyframe2Sphere:
            pAeroCalculator->setFlyFromTo(m_pNavKeyframeFrom, m_NavSphereTo, m_bCurveFromTo);
            break;
        default: assert(false);
    }
    pAeroCalculator->setSmoothLevel(m_dblSmoothLevel);

    m_vecTimeStamp.clear();
    m_pAnimationNavPath = pAeroCalculator->calcAnimationPath(m_vecTimeStamp);
    if(!m_pAnimationNavPath.valid())
    {
        return false;
    }
    m_pAnimationNavPath->setWeight(1.0);
    m_pAnimationNavPath->setPlayMode(osgAnimation::Animation::STAY);


    osgAnimation::Channel *pTranslationChannel = findChannel("Translation");
    m_pTranslationTarget = new osgAnimation::Vec2dTarget;
    pTranslationChannel->setTarget(m_pTranslationTarget.get());

    osgAnimation::Channel *pHeightChannel = findChannel("Height");
    m_pHeightTarget = new osgAnimation::DoubleTarget;
    pHeightChannel->setTarget(m_pHeightTarget.get());

    osgAnimation::Channel *pAzimuthChannel = findChannel("Azimuth");
    m_pAzimuthTarget = new osgAnimation::DoubleTarget;
    pAzimuthChannel->setTarget(m_pAzimuthTarget.get());

    osgAnimation::Channel *pPitchChannel = findChannel("Pitch");
    m_pPitchTarget = new osgAnimation::DoubleTarget;
    pPitchChannel->setTarget(m_pPitchTarget.get());

    return true;
}


osgAnimation::Channel *NavigationPathPlayer::findChannel(const std::string &strName)
{
    osgAnimation::ChannelList &listChannel = m_pAnimationNavPath->getChannels();
    osgAnimation::ChannelList::iterator itorChannel = listChannel.begin();
    for(; itorChannel != listChannel.end(); ++itorChannel)
    {
        osgAnimation::Channel *pChannel = itorChannel->get();
        if(pChannel->getName() == strName)
        {
            return pChannel;
        }
    }

    return NULL;
}
