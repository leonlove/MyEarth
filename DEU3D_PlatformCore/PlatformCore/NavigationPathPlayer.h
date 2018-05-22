#ifndef NAVIGATION_PATH_PLAYER_H_INCLUDE
#define NAVIGATION_PATH_PLAYER_H_INCLUDE

#include <osgGA/GUIEventHandler>
#include "INavigationPathPlayer.h"
#include <osgAnimation/Animation>
#include <vector>
#include "AeroCalculator.h"
#include "INavigatorManager.h"
#include <EventAdapter/IEventAdapter.h>
#include "AnimationModel.h"
#include "SceneGraphOperator.h"

#pragma warning (disable : 4250)

class NavigationPathPlayer : public INavigationPathPlayer, public osgGA::GUIEventHandler
{
public:
    explicit NavigationPathPlayer(ea::IEventAdapter *pEventAdapter = NULL);
    virtual ~NavigationPathPlayer(void);

public:
    bool initialize(INavigatorManager *pNavigatorManager, SceneGraphOperator *pSceneGraphOperator);
    osg::Node *getPlayerNode(void)                  {   return m_pPlayerNode.get(); }
    const osg::Node *getPlayerNode(void) const      {   return m_pPlayerNode.get(); }

protected:  // Methods from INavigationPathPlayer
    virtual void setNavigationPath(const INavigationPath *pNaviPath);
    virtual void setNavigationPathByFromTo(const INavigationKeyframe *pFrameFrom, const INavigationKeyframe *pFrameTo, bool bCurve);
    virtual void setNavigationPathByFromTo(const INavigationKeyframe *pFrameFrom, double dblPosX, double dblPosY, double dblPosH, double dblRadius, bool bCurve);
    virtual void setSmoothLevel(double dblSmoothLevel);

    virtual void play(void);
    virtual void stop(void);
    virtual void pause(void);
    virtual std::string getPlayingState(void) const;

    virtual double getNavigationPathLength(void) const;
    virtual double getPlayingPosition(void) const;
    virtual void   setPlayingPosition(double dblPos);

    virtual void setNavigationModel(IAnimationModel *pNaviModel);
    virtual IAnimationModel *getNavigationModel(void)               {   return m_pNavigationModel;  }
    virtual const   IAnimationModel *getNavigationModel(void) const {   return m_pNavigationModel;  }


    virtual void    setFixCamera(bool bFixCamera = true)    { m_bFixCameraDirWhilePlaying = bFixCamera; }
    virtual bool    getFixCamera(void)  { return m_bFixCameraDirWhilePlaying; }

protected:  // Methods from osgGA::GUIEventHandler
    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

protected:
    void prepare(void);
    bool OnKeyDown(const osgGA::GUIEventAdapter &ea);
    bool OnKeyUp(const osgGA::GUIEventAdapter &ea);
    bool OnMouseDown(const osgGA::GUIEventAdapter &ea);
    bool OnMouseUp(const osgGA::GUIEventAdapter &ea);
    bool OnMouseMove(const osgGA::GUIEventAdapter &ea);
    bool OnMouseDrag(const osgGA::GUIEventAdapter &ea);

    bool buildAnimationNavPath(void);
    CameraPose Navigate(void);
    void getCameraPoseByTime(double dblTime, CameraPose &pose);
    void fixPose(CameraPose &pose) const;

    void processPose(CameraPose &pose) const;

    osgAnimation::Channel *findChannel(const std::string &strName);

    bool processKeyboard(bool bKeyDown, const osgGA::GUIEventAdapter &ea);
    bool doRMouseDragNavigation(const osgGA::GUIEventAdapter &ea);

protected:
    osg::ref_ptr<SceneGraphOperator>        m_pSceneGraphOperator;

    enum NavigationPathType
    {
        NPT_NavigationPath,
        NPT_Keyframe2Keyframe,
        NPT_Keyframe2Sphere
    };
    NavigationPathType                          m_eNavPathType;
    osg::ref_ptr<const NavigationPath>          m_pNavigationPath;
    osg::ref_ptr<const NavigationKeyframe>      m_pNavKeyframeFrom;
    osg::ref_ptr<const NavigationKeyframe>      m_pNavKeyframeTo;
    osg::BoundingSphere                         m_NavSphereTo;
    bool                                        m_bCurveFromTo;


    double                  m_dblDurationTime;
    double                  m_dblLastNavigationTime;
    double                  m_dblLastTimeEventTime;
    CameraPose              m_poseNavigatedBias;

    osg::ref_ptr<osgAnimation::Animation>           m_pAnimationNavPath;
    osg::ref_ptr<osgAnimation::Vec2dTarget>         m_pTranslationTarget;
    osg::ref_ptr<osgAnimation::DoubleTarget>        m_pHeightTarget;
    osg::ref_ptr<osgAnimation::DoubleTarget>        m_pAzimuthTarget;
    osg::ref_ptr<osgAnimation::DoubleTarget>        m_pPitchTarget;

    osg::ref_ptr<INavigatorManager>                 m_pNavigatorManager;
    std::vector<double>                             m_vecTimeStamp;
    std::vector<double>::const_iterator             m_itorTimeStamp;
    osg::ref_ptr<AnimationModel>                    m_pNavigationModel;

    double              m_dblTimeSpeed;
    double              m_dblSmoothLevel;

    typedef enum tagPlayingState
    {
        PS_PLAY,
        PS_PAUSE,
        PS_STOP
    }PlayingState;
    PlayingState        m_ePlayingState;

    OpenSP::sp<NavigationParam>         m_pNavigationParam;
    std::map<NavigationParam::NavKeyboard, bool> m_mapKeyDownState;

    OpenSP::sp<ea::IEventAdapter>       m_pEventAdapter;

    osg::ref_ptr<osg::Group>            m_pPlayerNode;

    bool            m_bRButtonDown;
    osg::ref_ptr<const osgGA::GUIEventAdapter>  m_pLastMouseEvent;      // 前一个鼠标消息

    bool            m_bFixCameraDirWhilePlaying;

    mutable OpenThreads::Block  m_blockHandleFinish;
    OpenThreads::Atomic         m_bIsFinishing;
};


#endif
