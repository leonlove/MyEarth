#ifndef AERO_CALCULATOR_H_EEC69C47_77B0_4611_A054_20C16969AC26_INCLUDE
#define AERO_CALCULATOR_H_EEC69C47_77B0_4611_A054_20C16969AC26_INCLUDE

#include <osg/Referenced>
#include <osgAnimation/KeyFrame>
#include <osgAnimation/Animation>
#include <osgAnimation/Channel>
#include <osg/Matrix>
#include <osg/BoundingSphere>
#include <vector>
#include "NavigationParam.h"
#include "NavigationPath.h"

namespace osgAnimation
{
    typedef TemplateCubicBezier<osg::Vec2d>                 Vec2dCubicBezier;
    typedef TemplateKeyframe<Vec2dCubicBezier>              Vec2dCubicBezierKeyframe;
    typedef TemplateKeyframeContainer<Vec2dCubicBezier>     Vec2dCubicBezierKeyframeContainer;

    typedef TemplateCubicBezier<osg::Vec3d>                 Vec3dCubicBezier;
    typedef TemplateKeyframe<Vec3dCubicBezier>              Vec3dCubicBezierKeyframe;
    typedef TemplateKeyframeContainer<Vec3dCubicBezier>     Vec3dCubicBezierKeyframeContainer;

    typedef TemplateCubicBezierInterpolator<osg::Vec2d, Vec2dCubicBezier>   Vec2dCubicBezierInterpolator;
    typedef TemplateSampler<Vec2dCubicBezierInterpolator>                   Vec2dCubicBezierSampler;
    typedef TemplateChannel<Vec2dCubicBezierSampler>                        Vec2dCubicBezierChannel;

    typedef TemplateCubicBezierInterpolator<osg::Vec3d, Vec3dCubicBezier>   Vec3dCubicBezierInterpolator;
    typedef TemplateSampler<Vec3dCubicBezierInterpolator>                   Vec3dCubicBezierSampler;
    typedef TemplateChannel<Vec3dCubicBezierSampler>                        Vec3dCubicBezierChannel;

    typedef TemplateTarget<osg::Vec3d>        Vec3dTarget;
    typedef TemplateTarget<osg::Vec4d>        Vec4dTarget;
    typedef TemplateTarget<osg::Vec2d>        Vec2dTarget;
};


class AeroCalculator : public osg::Referenced
{
public:
    explicit AeroCalculator(void);
protected:
    virtual ~AeroCalculator(void);

public:
    bool    initialize(void);

    void    setFlyFromTo(const NavigationKeyframe *pFrameFrom, const NavigationKeyframe *pFrameTo, bool bCurve);
    void    setFlyFromTo(const NavigationKeyframe *pFrameFrom, const osg::BoundingSphere &sphere, bool bCurve);

    void    setNavigationPath(const NavigationPath *pNavigationPath);

    void    setSmoothLevel(double dblSmoothLevel);

    void    setFlyMode(NavigationMode eMode);

    osgAnimation::Animation *calcAnimationPath(std::vector<double> &vecTimeStamp);

protected:
    void    convertToAnimation(void);

    void    calcSmoothLevel(void);

    void    fixRound(CameraPose &pose0, CameraPose &pose1);

    typedef std::pair<double, double>    DurationTime;
    double    insertKeyframes(double dblBeginTime, const DurationTime &duration, const DurationTime &lastDuration, const osgAnimation::Vec2dCubicBezier &translation, const osgAnimation::DoubleCubicBezier &height, const osgAnimation::DoubleCubicBezier &azimuth, const osgAnimation::DoubleCubicBezier &pitch);


    template<typename Type>
    void    calculateTranslationCubicBezier
        (const Type *pLastPos, const Type *pCurPos, const Type *pNextPos, const Type *pNextPos2, Type &ptCtrl1, Type &ptCtrl2) const;

    double    deltaAngle(const CameraPose &pose1, const CameraPose &pose2) const;
    double    deltaDistance(const CameraPose &pose1, const CameraPose &pose2) const;
    double    deltaDistance_Plane(const CameraPose &pose1, const CameraPose &pose2) const;
    void    convertFrameSpeed2Time(NavigationKeyframe &frameFrom, NavigationKeyframe &frameTo) const;

protected:
    OpenSP::sp<NavigationPath>  m_pNavigationPath;

    std::vector<double>        m_vecTimeStamp;
    osg::ref_ptr<osgAnimation::Vec2dCubicBezierKeyframeContainer>         m_pTranslationKeyframes;
    osg::ref_ptr<osgAnimation::DoubleCubicBezierKeyframeContainer>        m_pHeightKeyframes;
    osg::ref_ptr<osgAnimation::DoubleCubicBezierKeyframeContainer>        m_pAzimuthKeyframes;
    osg::ref_ptr<osgAnimation::DoubleCubicBezierKeyframeContainer>        m_pPitchKeyframes;

    NavigationMode    m_eNavigationMode;
    double            m_dblSmoothLevel;
    double            m_dblCubicBezierLeft;
    double            m_dblCubicBezierRight;
};


#endif
