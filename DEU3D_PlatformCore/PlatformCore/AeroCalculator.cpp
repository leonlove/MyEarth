#include "AeroCalculator.h"
#include <assert.h>
#include <list>
#include <osgAnimation/Interpolator>
#include <osg/CoordinateSystemNode>
#include "CubicBezierLinearInterpolator.h"

const double    g_dblCubicBezierLeft  = 0.3333333333333333;
const double    g_dblCubicBezierRight = 0.6666666666666667;

AeroCalculator::AeroCalculator(void)
{
    m_dblSmoothLevel        = 0.0;
    m_eNavigationMode       = NM_PARALLEL_SCALE_ALIGNMENT;
    m_dblCubicBezierLeft    = g_dblCubicBezierLeft;
    m_dblCubicBezierRight   = g_dblCubicBezierRight;
}


AeroCalculator::~AeroCalculator(void)
{
}


bool AeroCalculator::initialize(void)
{
    return true;
}


void AeroCalculator::setSmoothLevel(double dblSmoothLevel)
{
    m_dblSmoothLevel = osg::clampBetween(dblSmoothLevel, 0.0, 1.0);
    calcSmoothLevel();
}


void AeroCalculator::calcSmoothLevel(void)
{
    m_dblCubicBezierLeft  = (1.0 - m_dblSmoothLevel) * g_dblCubicBezierLeft;
    m_dblCubicBezierRight = 1.0 - (1.0 - m_dblSmoothLevel) * (1.0 - g_dblCubicBezierRight);
}


void AeroCalculator::setFlyMode(NavigationMode eMode)
{
    if(m_eNavigationMode == eMode)    return;

    m_eNavigationMode = eMode;
}


double AeroCalculator::deltaAngle(const CameraPose &pose1, const CameraPose &pose2) const
{
    const double dblAzimuth  = pose1.m_dblAzimuthAngle - pose2.m_dblAzimuthAngle;
    const double dblPitching = pose1.m_dblPitchAngle - pose2.m_dblPitchAngle;
    const double dblCosAngle = cos(dblAzimuth) * cos(dblPitching);
    const double dblAngle     = acos(dblCosAngle);
    return fabs(dblAngle);
}


double AeroCalculator::deltaDistance_Plane(const CameraPose &pose1, const CameraPose &pose2) const
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const double dblAvgHeight    = (pose1.m_dblHeight + pose2.m_dblHeight) * 0.5;
    const double dblPlanetRadius = pEllipsoidModel->getRadius();
    const double dblRadius       = dblPlanetRadius + dblAvgHeight;
    const double dblCosValue     = sin(pose1.m_dblPositionY) * sin(pose2.m_dblPositionY)
                                 + cos(pose1.m_dblPositionY) * cos(pose2.m_dblPositionY) * cos(pose1.m_dblPositionX - pose2.m_dblPositionX);
    const double dblDistance     = dblRadius * acos(dblCosValue);
    return dblDistance;
}


double AeroCalculator::deltaDistance(const CameraPose &pose1, const CameraPose &pose2) const
{
    const double dblDistance1   = deltaDistance_Plane(pose1, pose2);
    const double dblHeightDelta = fabs(pose2.m_dblHeight - pose1.m_dblHeight);
    const double dblDistance    = sqrt(dblHeightDelta * dblHeightDelta + dblDistance1 * dblDistance1);
    return dblDistance;
}


void AeroCalculator::convertFrameSpeed2Time(NavigationKeyframe &frameFrom, NavigationKeyframe &frameTo) const
{
    if(frameFrom.m_bArgForTime)
    {
        return;
    }

    // 1 convert translation speed to time
    const double dblTransDistance = deltaDistance(frameFrom.m_CameraPose, frameTo.m_CameraPose);
    if(dblTransDistance < FLT_EPSILON)
    {
        frameFrom.m_dblTrans_TimeOrSpeed = 0.0;
    }
    else
    {
        if(frameFrom.m_dblTrans_TimeOrSpeed < FLT_EPSILON)
        {
            frameFrom.m_dblTrans_TimeOrSpeed = FLT_MAX;
        }
        else
        {
            frameFrom.m_dblTrans_TimeOrSpeed = dblTransDistance / frameFrom.m_dblTrans_TimeOrSpeed;
        }
    }

    // 2 convert rotation speed to time
    const double dblDeltaAngle = deltaAngle(frameFrom.m_CameraPose, frameTo.m_CameraPose);
    if(dblDeltaAngle < FLT_EPSILON)
    {
        frameFrom.m_dblRotate_TimeOrSpeed = 0.0;
    }
    else
    {
        if(frameFrom.m_dblRotate_TimeOrSpeed < FLT_EPSILON)
        {
            frameFrom.m_dblRotate_TimeOrSpeed = FLT_MAX;
        }
        else
        {
            frameFrom.m_dblRotate_TimeOrSpeed = dblDeltaAngle / frameFrom.m_dblRotate_TimeOrSpeed;
        }
    }

    frameFrom.m_bArgForTime = true;
}


void AeroCalculator::setFlyFromTo(const NavigationKeyframe *pFrameFrom, const NavigationKeyframe *pFrameTo, bool bCurve)
{
    OpenSP::sp<NavigationKeyframe> pFrameFrom_1 = new NavigationKeyframe(*pFrameFrom);
    OpenSP::sp<NavigationKeyframe> pFrameTo_1   = new NavigationKeyframe(*pFrameTo);
    convertFrameSpeed2Time(*pFrameFrom_1, *pFrameTo_1);

    if(bCurve)
    {
        if(pFrameFrom_1->m_dblRotate_TimeOrSpeed < FLT_EPSILON && pFrameFrom_1->m_dblTrans_TimeOrSpeed < FLT_EPSILON)
        {
            bCurve = false;
        }
    }

    OpenSP::sp<NavigationPath> pNavigationPath = new NavigationPath;

    OpenSP::sp<NavigationKeyframe> pFrameMid1 = new NavigationKeyframe;
    OpenSP::sp<NavigationKeyframe> pFrameMid2 = new NavigationKeyframe;
    pFrameMid1->m_bArgForTime = true;
    pFrameMid2->m_bArgForTime = true;
    if(bCurve)
    {
        const double dblMidRatio = 0.1;
        const double dblDistancePlane = deltaDistance_Plane(pFrameFrom->m_CameraPose, pFrameTo->m_CameraPose);

        // 曲线方式漂移过去，中间需要插入关键帧
        pFrameMid1->m_CameraPose.m_dblAzimuthAngle = pFrameFrom_1->m_CameraPose.m_dblAzimuthAngle;
        pFrameMid1->m_CameraPose.m_dblPitchAngle   = pFrameFrom_1->m_CameraPose.m_dblPitchAngle;

        pFrameMid1->m_CameraPose.m_dblPositionX = pFrameFrom_1->m_CameraPose.m_dblPositionX * (1.0 - dblMidRatio) + pFrameTo_1->m_CameraPose.m_dblPositionX * dblMidRatio;
        pFrameMid1->m_CameraPose.m_dblPositionY = pFrameFrom_1->m_CameraPose.m_dblPositionY * (1.0 - dblMidRatio) + pFrameTo_1->m_CameraPose.m_dblPositionY * dblMidRatio;
        pFrameMid1->m_CameraPose.m_dblHeight    = pFrameFrom_1->m_CameraPose.m_dblHeight * (1.0 - dblMidRatio) + pFrameTo_1->m_CameraPose.m_dblHeight * dblMidRatio;
        pFrameMid1->m_CameraPose.m_dblHeight   += dblDistancePlane * dblMidRatio;

        pFrameMid2->m_CameraPose.m_dblAzimuthAngle = pFrameTo_1->m_CameraPose.m_dblAzimuthAngle;
        pFrameMid2->m_CameraPose.m_dblPitchAngle   = pFrameTo_1->m_CameraPose.m_dblPitchAngle;

        pFrameMid2->m_CameraPose.m_dblPositionX = pFrameFrom_1->m_CameraPose.m_dblPositionX * dblMidRatio + pFrameTo_1->m_CameraPose.m_dblPositionX * (1.0 - dblMidRatio);
        pFrameMid2->m_CameraPose.m_dblPositionY = pFrameFrom_1->m_CameraPose.m_dblPositionY * dblMidRatio + pFrameTo_1->m_CameraPose.m_dblPositionY * (1.0 - dblMidRatio);
        pFrameMid2->m_CameraPose.m_dblHeight    = pFrameMid1->m_CameraPose.m_dblHeight;// * (1.0 - dblMidRatio) + frameTo_1.m_CameraPose.m_dblHeight * dblMidRatio;


        const double dblTransTime  = pFrameFrom_1->m_dblTrans_TimeOrSpeed * 0.5;
        const double dblRotateTime = pFrameFrom_1->m_dblRotate_TimeOrSpeed * 0.5;

        pFrameMid2->m_dblTrans_TimeOrSpeed    = dblTransTime;
        pFrameMid2->m_dblRotate_TimeOrSpeed   = dblRotateTime;

        pFrameFrom_1->m_dblTrans_TimeOrSpeed  = dblTransTime  * (dblMidRatio / (1.0 - dblMidRatio));
        pFrameFrom_1->m_dblRotate_TimeOrSpeed = dblRotateTime * (dblMidRatio / (1.0 - dblMidRatio));

        pFrameMid1->m_dblTrans_TimeOrSpeed    = dblRotateTime * ((1.0 - 2.0 * dblMidRatio) / (1.0 - dblMidRatio));
        pFrameMid1->m_dblRotate_TimeOrSpeed   = dblTransTime  * ((1.0 - 2.0 * dblMidRatio) / (1.0 - dblMidRatio));
    }

    pNavigationPath->appendItem(pFrameFrom_1);
    if(bCurve)
    {
        pNavigationPath->appendItem(pFrameMid1);
        pNavigationPath->appendItem(pFrameMid2);
    }
    pNavigationPath->appendItem(pFrameTo_1);

    setNavigationPath(pNavigationPath);
}


void AeroCalculator::setFlyFromTo(const NavigationKeyframe *pFrameFrom, const osg::BoundingSphere &sphere, bool bCurve)
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    OpenSP::sp<NavigationKeyframe> pFrameTo = new NavigationKeyframe(*pFrameFrom);

    const double dblHorzDistance = sphere.radius() * sin(pFrameTo->m_CameraPose.m_dblPitchAngle);
    const double dblVertDistance = sphere.radius() * cos(pFrameTo->m_CameraPose.m_dblPitchAngle);

    pFrameTo->m_CameraPose.m_dblHeight = sphere.center().z() + dblVertDistance;

    osg::Vec2d vecHorzDelta;
    vecHorzDelta.x() = dblHorzDistance * cos(pFrameTo->m_CameraPose.m_dblAzimuthAngle) / cos(sphere.center().y());
    vecHorzDelta.y() = dblHorzDistance * sin(pFrameTo->m_CameraPose.m_dblAzimuthAngle);

    const double dblPolarRadius   = pEllipsoidModel->getRadiusPolar()   + pFrameTo->m_CameraPose.m_dblHeight;
    const double dblEquatorRadius = pEllipsoidModel->getRadiusEquator() + pFrameTo->m_CameraPose.m_dblHeight;

    const double dblCurEquatorRadius = dblEquatorRadius * cos(sphere.center().x());
    vecHorzDelta.x() =  asin(vecHorzDelta.x() / (2.0 * dblCurEquatorRadius));
    vecHorzDelta.y() = -asin(vecHorzDelta.y() / (2.0 * dblPolarRadius));

    pFrameTo->m_CameraPose.m_dblPositionX = sphere.center().x() + vecHorzDelta.x();
    pFrameTo->m_CameraPose.m_dblPositionY = sphere.center().y() + vecHorzDelta.y();

    setFlyFromTo(pFrameFrom, pFrameTo, bCurve);
}


void AeroCalculator::setNavigationPath(const NavigationPath *pNavigationPath)
{
    if(pNavigationPath->getItemCount() < 2u)
    {
        return;
    }

    m_pNavigationPath = new NavigationPath;
    for(unsigned n = 0u; n < pNavigationPath->getItemCount(); n++)
    {
        const NavigationKeyframe *pKeyframeSrc = dynamic_cast<const NavigationKeyframe *>(pNavigationPath->getItem(n));
        OpenSP::sp<NavigationKeyframe>  pKeyframe = new NavigationKeyframe(*pKeyframeSrc);
        m_pNavigationPath->appendItem(pKeyframe);
    }

    for(unsigned i = 0u, j = 1u; j < m_pNavigationPath->getItemCount(); i++, j++)
    {
        NavigationKeyframe *pFrame0 = dynamic_cast<NavigationKeyframe *>(m_pNavigationPath->getItem(i));
        NavigationKeyframe *pFrame1 = dynamic_cast<NavigationKeyframe *>(m_pNavigationPath->getItem(j));
        convertFrameSpeed2Time(*pFrame0, *pFrame1);
    }

    for(unsigned i = 0u, j = 1u; j < m_pNavigationPath->getItemCount(); i++, j++)
    {
        NavigationKeyframe *pFrame0 = dynamic_cast<NavigationKeyframe *>(m_pNavigationPath->getItem(i));
        NavigationKeyframe *pFrame1 = dynamic_cast<NavigationKeyframe *>(m_pNavigationPath->getItem(j));
        fixRound(pFrame0->m_CameraPose, pFrame1->m_CameraPose);
    }
}


void AeroCalculator::fixRound(CameraPose &pose0, CameraPose &pose1)
{
    const double dblPI_M2 = osg::PI + osg::PI;
    const double dblDeltaAzimuth = pose1.m_dblAzimuthAngle - pose0.m_dblAzimuthAngle;
    if(dblDeltaAzimuth > osg::PI)
    {
        pose1.m_dblAzimuthAngle -= dblPI_M2;
    }
    else if(dblDeltaAzimuth < -osg::PI)
    {
        pose1.m_dblAzimuthAngle += dblPI_M2;
    }

    const double dblDeltaX = pose1.m_dblPositionX - pose0.m_dblPositionX;
    if(dblDeltaX > osg::PI)
    {
        pose1.m_dblPositionX -= dblPI_M2;
    }
    else if(dblDeltaX < -osg::PI)
    {
        pose1.m_dblPositionX += dblPI_M2;
    }
}


osgAnimation::Animation *AeroCalculator::calcAnimationPath(std::vector<double> &vecTimeStamp)
{
    if(m_pNavigationPath->getItemCount() < 2u)
    {
        return NULL;
    }

    m_vecTimeStamp.clear();


    // translation channel
    osg::ref_ptr<osgAnimation::Vec2dCubicBezierChannel>        pTranslationChannel = new osgAnimation::Vec2dCubicBezierChannel;
    pTranslationChannel->setName("Translation");
    osgAnimation::Vec2dCubicBezierSampler    *pTranslationSampler = pTranslationChannel->getOrCreateSampler();
    m_pTranslationKeyframes = pTranslationSampler->getOrCreateKeyframeContainer();


    // height channel
    osg::ref_ptr<osgAnimation::DoubleCubicBezierLinearChannel>    pHeightChannel = new osgAnimation::DoubleCubicBezierLinearChannel;
    pHeightChannel->setName("Height");
    osgAnimation::DoubleCubicBezierLinearSampler    *pHeightSampler = pHeightChannel->getOrCreateSampler();
    m_pHeightKeyframes = pHeightSampler->getOrCreateKeyframeContainer();


    // azimuth channel
    osg::ref_ptr<osgAnimation::DoubleCubicBezierLinearChannel>    pAzimuthChannel = new osgAnimation::DoubleCubicBezierLinearChannel;
    pAzimuthChannel->setName("Azimuth");
    osgAnimation::DoubleCubicBezierLinearSampler    *pAzimuthSampler = pAzimuthChannel->getOrCreateSampler();
    m_pAzimuthKeyframes = pAzimuthSampler->getOrCreateKeyframeContainer();


    // pitch channel
    osg::ref_ptr<osgAnimation::DoubleCubicBezierLinearChannel>    pPitchChannel = new osgAnimation::DoubleCubicBezierLinearChannel;
    pPitchChannel->setName("Pitch");
    osgAnimation::DoubleCubicBezierLinearSampler    *pPitchSampler = pPitchChannel->getOrCreateSampler();
    m_pPitchKeyframes = pPitchSampler->getOrCreateKeyframeContainer();

    convertToAnimation();

    osg::ref_ptr<osgAnimation::Animation> pAnimationNavPath = new osgAnimation::Animation;
    pAnimationNavPath->addChannel(pTranslationChannel.get());
    pAnimationNavPath->addChannel(pHeightChannel.get());
    pAnimationNavPath->addChannel(pAzimuthChannel.get());
    pAnimationNavPath->addChannel(pPitchChannel.get());

    vecTimeStamp.assign(m_vecTimeStamp.begin(), m_vecTimeStamp.end());
    return pAnimationNavPath.release();
}


void AeroCalculator::convertToAnimation(void)
{
    const unsigned nFrameCount = m_pNavigationPath->getItemCount();
    assert(nFrameCount >= 2u);

    m_pTranslationKeyframes->clear();
    m_pHeightKeyframes->clear();
    m_pAzimuthKeyframes->clear();
    m_pPitchKeyframes->clear();

    double            dblBeginTime = 0.0;
    DurationTime    lastDuration(-1.0, -1.0);
    for(unsigned nIndex = 0u; nIndex < nFrameCount; nIndex++)
    {
        const NavigationKeyframe *pCurFrame = dynamic_cast<const NavigationKeyframe *>(m_pNavigationPath->getItem(nIndex));
        const CameraPose *pPoseCurrent = &pCurFrame->m_CameraPose;

        const CameraPose *pPoseLast = NULL;
        if(nIndex > 0u)
        {
            const NavigationKeyframe *pLastFrame = dynamic_cast<const NavigationKeyframe *>(m_pNavigationPath->getItem(nIndex - 1u));
            pPoseLast = &pLastFrame->m_CameraPose;
        }

        const CameraPose *pPoseNext = NULL;
        if(nIndex < nFrameCount - 1u)
        {
            const NavigationKeyframe *pNextFrame = dynamic_cast<const NavigationKeyframe *>(m_pNavigationPath->getItem(nIndex + 1u));
            pPoseNext = &pNextFrame->m_CameraPose;
        }

        const CameraPose *pPoseNext2 = NULL;
        if(nIndex < nFrameCount - 2u)
        {
            const NavigationKeyframe *pNextFrame2 = dynamic_cast<const NavigationKeyframe *>(m_pNavigationPath->getItem(nIndex + 2u));
            pPoseNext2 = &pNextFrame2->m_CameraPose;
        }


        // 1. 计算平移关键帧
        osgAnimation::Vec2dCubicBezier cbTranslation;
        {
            const osg::Vec2d  ptCurPos(pPoseCurrent->m_dblPositionX, pPoseCurrent->m_dblPositionY);

            osg::Vec2d ptLastPos,  *pLastPos  = NULL;
            osg::Vec2d ptNextPos,  *pNextPos  = NULL;
            osg::Vec2d ptNextPos2, *pNextPos2 = NULL;

            if(nIndex > 0u)
            {
                ptLastPos.set(pPoseLast->m_dblPositionX, pPoseLast->m_dblPositionY);
                pLastPos = &ptLastPos;
            }

            if(nIndex < nFrameCount - 1u)
            {
                ptNextPos.set(pPoseNext->m_dblPositionX, pPoseNext->m_dblPositionY);
                pNextPos = &ptNextPos;
            }

            if(nIndex < nFrameCount - 2u)
            {
                ptNextPos2.set(pPoseNext2->m_dblPositionX, pPoseNext2->m_dblPositionY);
                pNextPos2 = &ptNextPos2;
            }

            osg::Vec2d ptTangentPoint1, ptTangentPoint2;
            calculateTranslationCubicBezier(pLastPos, &ptCurPos, pNextPos, pNextPos2, ptTangentPoint1, ptTangentPoint2);

            cbTranslation.setPosition(ptCurPos);
            cbTranslation.setControlPointIn(ptTangentPoint1);
            cbTranslation.setControlPointOut(ptTangentPoint2);
        }


        // 2. 计算高度关键帧
        const osgAnimation::DoubleCubicBezier cbHeight(pPoseCurrent->m_dblHeight, m_dblCubicBezierLeft, m_dblCubicBezierRight);

        // 3. 计算水平旋转关键帧
        const osgAnimation::DoubleCubicBezier cbAzimuth(pPoseCurrent->m_dblAzimuthAngle, m_dblCubicBezierLeft, m_dblCubicBezierRight);

        // 4. 计算垂直旋转关键帧
        const osgAnimation::DoubleCubicBezier cbPitch(pPoseCurrent->m_dblPitchAngle, m_dblCubicBezierLeft, m_dblCubicBezierRight);

        // 5. 插入关键帧
        const double dblTransTime = osg::clampAbove(pCurFrame->m_dblTrans_TimeOrSpeed, 0.01);
        const double dblRotateTime = osg::clampAbove(pCurFrame->m_dblRotate_TimeOrSpeed, 0.01);
        DurationTime duration(dblTransTime, dblRotateTime);
        if(nIndex == nFrameCount - 1u)
        {
            duration.first = -1.0;
            duration.second = -1.0;
        }

        const double dblDurationTime = insertKeyframes
        (
            dblBeginTime,
            duration,
            lastDuration,
            cbTranslation,
            cbHeight,
            cbAzimuth,
            cbPitch
        );
        m_vecTimeStamp.push_back(dblBeginTime);
        lastDuration.swap(duration);
        dblBeginTime += dblDurationTime;
    }
}


template<typename Type>
void AeroCalculator::calculateTranslationCubicBezier(
            const Type *pLastPos,
            const Type *pCurPos,
            const Type *pNextPos,
            const Type *pNextPos2,
            Type &ptCtrl1,
            Type &ptCtrl2) const
{
    assert(pCurPos != NULL);

    Type vecCur2Last;
    if(pLastPos != NULL)
    {
        vecCur2Last = *pLastPos - *pCurPos;
        vecCur2Last.normalize();
    }

    Type ptNextPos, vecCur2Next;
    if(pNextPos != NULL)
    {
        vecCur2Next = *pNextPos - *pCurPos;
        vecCur2Next.normalize();
    }


    if(pLastPos == NULL)
    {
        // 当前点是第一个关键点
        ptCtrl1 = *pCurPos + (*pNextPos - *pCurPos) * m_dblCubicBezierLeft;
    }
    else if(pNextPos != NULL)
    {
        Type vecAngleBisector = vecCur2Last + vecCur2Next;
        vecAngleBisector.normalize();

        const Type   ptLeftPos     = *pCurPos + (*pNextPos - *pCurPos) * g_dblCubicBezierLeft;
        const double dblHypotenuse = (ptLeftPos - *pCurPos).length();
        const double dblCos        = vecCur2Next * vecAngleBisector;
        const double dblFootLen    = dblHypotenuse * dblCos * m_dblSmoothLevel;
        ptCtrl1 = ptLeftPos - vecAngleBisector * dblFootLen;
    }
    else
    {
        ptCtrl1 = *pCurPos;
    }


    if(pNextPos == NULL)
    {
        ptCtrl2 = *pCurPos;
    }
    else if(pNextPos2 != NULL)
    {
        Type vecNext2Next2 = *pNextPos2 - *pNextPos;
        vecNext2Next2.normalize();

        const Type vecInverseCur2Next = -vecCur2Next;

        Type vecAngleBisector = vecInverseCur2Next + vecNext2Next2;
        vecAngleBisector.normalize();

        const Type     ptRightPos        = *pCurPos + (*pNextPos - *pCurPos) * g_dblCubicBezierRight;
        const double dblHypotenuse    = (ptRightPos - *pNextPos).length();
        const double dblCos            = vecInverseCur2Next * vecAngleBisector;
        const double dblFootLen        = dblHypotenuse * dblCos * m_dblSmoothLevel;

        ptCtrl2 = ptRightPos - vecAngleBisector * dblFootLen;
    }
    else
    {
        // 当前点是倒数第二个关键点
        ptCtrl2 = *pCurPos + (*pNextPos - *pCurPos) * m_dblCubicBezierRight;
    }
}


double AeroCalculator::insertKeyframes(
        double dblBeginTime,
        const DurationTime &duration,
        const DurationTime &lastDuration,
        const osgAnimation::Vec2dCubicBezier  &translation,
        const osgAnimation::DoubleCubicBezier &height,
        const osgAnimation::DoubleCubicBezier &azimuth,
        const osgAnimation::DoubleCubicBezier &pitch)
{
    double dblDurationTime = std::max(duration.first, duration.second);

    const osgAnimation::Vec2dCubicBezierKeyframe  frameTranslation(dblBeginTime, translation);
    const osgAnimation::DoubleCubicBezierKeyframe frameHeight(dblBeginTime, height);
    const osgAnimation::DoubleCubicBezierKeyframe frameAzimuth(dblBeginTime, azimuth);
    const osgAnimation::DoubleCubicBezierKeyframe framePitch(dblBeginTime, pitch);

    switch(m_eNavigationMode)
    {
        case NM_PARALLEL_SCALE_ALIGNMENT:
        {
            m_pTranslationKeyframes->push_back(frameTranslation);
            m_pHeightKeyframes->push_back(frameHeight);
            m_pAzimuthKeyframes->push_back(frameAzimuth);
            m_pPitchKeyframes->push_back(framePitch);
            break;
        }
        case NM_PARALLEL_RIGHT_ALIGNMENT:
        {
            m_pTranslationKeyframes->push_back(frameTranslation);
            m_pHeightKeyframes->push_back(frameHeight);
            m_pAzimuthKeyframes->push_back(frameAzimuth);
            m_pPitchKeyframes->push_back(framePitch);

            if(duration.first >= 0.0 && duration.second >= 0.0)
            {
                if(duration.first > duration.second)
                {
                    const double dblDeltaTime = duration.first - duration.second;
                    const osgAnimation::DoubleCubicBezierKeyframe fillAzimuthFrame(dblBeginTime + dblDeltaTime, azimuth);
                    const osgAnimation::DoubleCubicBezierKeyframe fillPitchFrame(dblBeginTime + dblDeltaTime, pitch);

                    m_pAzimuthKeyframes->push_back(fillAzimuthFrame);
                    m_pPitchKeyframes->push_back(fillPitchFrame);
                }
                else if(duration.first < duration.second)
                {
                    const double dblDeltaTime = duration.second - duration.first;

                    const osgAnimation::Vec2dCubicBezierKeyframe  fillTranslationFrame(dblBeginTime + dblDeltaTime, translation);
                    m_pTranslationKeyframes->push_back(fillTranslationFrame);

                    const osgAnimation::DoubleCubicBezierKeyframe fillHeightFrame(dblBeginTime + dblDeltaTime, height);
                    m_pHeightKeyframes->push_back(fillHeightFrame);
                }
            }
            break;
        }
        case NM_PARALLEL_LEFT_ALIGNMENT:
        {
            if(lastDuration.first >= 0.0 && lastDuration.second >= 0.0)
            {
                if(lastDuration.first > lastDuration.second)
                {
                    const double dblLastDeltaTime = lastDuration.first - lastDuration.second;

                    const osgAnimation::DoubleCubicBezierKeyframe  fillAzimuthFrame(dblBeginTime - dblLastDeltaTime, azimuth);
                    m_pAzimuthKeyframes->push_back(fillAzimuthFrame);

                    const osgAnimation::DoubleCubicBezierKeyframe  fillPitchFrame(dblBeginTime - dblLastDeltaTime, pitch);
                    m_pPitchKeyframes->push_back(fillPitchFrame);
                }
                else if(lastDuration.first < lastDuration.second)
                {
                    const double dblLastDeltaTime = lastDuration.second - lastDuration.first;

                    const osgAnimation::Vec2dCubicBezierKeyframe  fillTranslationFrame(dblBeginTime - dblLastDeltaTime, translation);
                    m_pTranslationKeyframes->push_back(fillTranslationFrame);

                    const osgAnimation::DoubleCubicBezierKeyframe  fillHeightFrame(dblBeginTime - dblLastDeltaTime, height);
                    m_pHeightKeyframes->push_back(fillHeightFrame);
                }
            }

            m_pTranslationKeyframes->push_back(frameTranslation);
            m_pHeightKeyframes->push_back(frameHeight);
            m_pAzimuthKeyframes->push_back(frameAzimuth);
            m_pPitchKeyframes->push_back(framePitch);
            break;
        }
        case NM_SERIAL_RT:
        {
            // rotation key frames
            const bool bFirstKeyframe = (lastDuration.first < 0.0 || lastDuration.second < 0.0);
            if(!bFirstKeyframe)
            {
                const osgAnimation::DoubleCubicBezierKeyframe fillAzimuthFrame(dblBeginTime - lastDuration.first, azimuth);
                m_pAzimuthKeyframes->push_back(fillAzimuthFrame);

                const osgAnimation::DoubleCubicBezierKeyframe fillPitchFrame(dblBeginTime - lastDuration.first, pitch);
                m_pPitchKeyframes->push_back(fillPitchFrame);
            }

            m_pAzimuthKeyframes->push_back(frameAzimuth);
            m_pPitchKeyframes->push_back(framePitch);


            // translation key frames
            m_pTranslationKeyframes->push_back(frameTranslation);
            m_pHeightKeyframes->push_back(frameHeight);

            const bool bLastKeyframe = (duration.first < 0.0 || duration.second < 0.0);
            if(!bLastKeyframe)
            {
                const osgAnimation::Vec2dCubicBezierKeyframe  fillTranslationFrame(dblBeginTime + duration.second, translation);
                m_pTranslationKeyframes->push_back(fillTranslationFrame);

                const osgAnimation::DoubleCubicBezierKeyframe fillHeightFrame(dblBeginTime + duration.second, height);
                m_pHeightKeyframes->push_back(fillHeightFrame);
            }

            dblDurationTime = duration.first + duration.second;
            break;
        }
        case NM_SERIAL_TR:
        {
            // translation key frames
            const bool bFirstKeyframe = (lastDuration.first < 0.0 || lastDuration.second < 0.0);
            if(!bFirstKeyframe)
            {
                const osgAnimation::Vec2dCubicBezierKeyframe fillTranslationFrame(dblBeginTime - lastDuration.second, translation);
                m_pTranslationKeyframes->push_back(fillTranslationFrame);

                const osgAnimation::DoubleCubicBezierKeyframe fillHeightFrame(dblBeginTime - lastDuration.second, height);
                m_pHeightKeyframes->push_back(fillHeightFrame);
            }

            m_pTranslationKeyframes->push_back(frameTranslation);
            m_pHeightKeyframes->push_back(frameHeight);


            // rotation key frames
            m_pAzimuthKeyframes->push_back(frameAzimuth);
            m_pPitchKeyframes->push_back(framePitch);

            const bool bLastKeyframe = (duration.first < 0.0 || duration.second < 0.0);
            if(!bLastKeyframe)
            {
                const osgAnimation::DoubleCubicBezierKeyframe fillAzimuthFrame(dblBeginTime + duration.first, azimuth);
                m_pAzimuthKeyframes->push_back(fillAzimuthFrame);

                const osgAnimation::DoubleCubicBezierKeyframe fillPitchFrame(dblBeginTime + duration.first, pitch);
                m_pPitchKeyframes->push_back(fillPitchFrame);
            }

            dblDurationTime = duration.first + duration.second;
            break;
        }
        default: assert(false);
    }
    return dblDurationTime;
}






