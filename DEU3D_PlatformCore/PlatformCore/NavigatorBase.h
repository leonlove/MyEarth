#ifndef NAVIGATION_BASE_H_4C7D218B_BB04_496B_8C3A_26B8EB857E50_INCLUDE
#define NAVIGATION_BASE_H_4C7D218B_BB04_496B_8C3A_26B8EB857E50_INCLUDE

#include <osg/Vec3d>
#include <map>
#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>
#include "NavigationParam.h"
#include <EventAdapter/IEventAdapter.h>
#include <EventAdapter/IEventObject.h>
#include <osg/CoordinateSystemNode>

class NavigatorBase : public OpenSP::Ref
{
public:
    explicit    NavigatorBase(void);
    virtual    ~NavigatorBase(void) = 0;

public:
    virtual bool    initialize(NavigationParam *pNavParam);
    virtual bool    handleEvent(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

    virtual osg::Matrixd getCameraMatrix(void) const = 0;
    virtual void    resetCamera(void) = 0;
    virtual void    stopInertia(void) = 0;
    virtual void    getCameraPose(CameraPose &pose) const = 0;
    virtual void    setCameraPose(const CameraPose &pose) = 0;
    virtual void    setNaviNode(osg::Node *pTargetNode) = 0;

	virtual void    enableInertia(bool bEnable) = 0;
	virtual void    enableUnderGroundViewMode(bool bEnable) = 0;

protected:
    virtual void    calculateHomePose(void) = 0;

protected:
    //osg::Vec3d      m_ptHomeSceneCenter;
    //osg::Vec3d      m_ptHomeCameraPos;
    //osg::Vec3d      m_vecHomeCameraUp;
    std::map<NavigationParam::NavKeyboard, bool> m_mapKeyDownState;

    OpenSP::sp<NavigationParam>                 m_pNavigationParam;
};

#endif

