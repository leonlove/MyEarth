#ifndef PLANET_NAVIGATOR_H_4A188A42_DCEB_43F2_9B31_7410580F39F3_INCLUDE
#define PLANET_NAVIGATOR_H_4A188A42_DCEB_43F2_9B31_7410580F39F3_INCLUDE

#include <osgGA/CameraManipulator>
#include <map>
#include <string>
#include "osgUtil/Radial.h"
#include "NavigatorBase.h"

class PlanetNavigator : public NavigatorBase
{
public:
    explicit                    PlanetNavigator(void);
    virtual                    ~PlanetNavigator(void);

protected:
    virtual osg::Matrixd        getCameraMatrix(void) const;

    virtual bool                handleEvent(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

    virtual void                setNaviNode(osg::Node *pNode);
    virtual const osg::Node    *getTargetNode(void) const    { return m_pNavigationNode.get();   }
    virtual osg::Node          *getNode(void)                { return m_pNavigationNode.get();   }

    virtual void                calculateHomePose(void);
    virtual void                resetCamera(void);
    virtual void                stopInertia(void);
    virtual void                getCameraPose(CameraPose &pose) const;
    virtual void                setCameraPose(const CameraPose &pose);

	virtual void				enableInertia(bool bEnable);
	virtual void				enableUnderGroundViewMode(bool bEnable);

protected:
    bool            getScreenRadial(const osg::Vec2d &ptPosNormalize, osgUtil::Radial3 &ray) const;
    bool            getHitTestPoint(const osgUtil::Radial3 &ray, osg::Vec3d &ptHitTest, bool bAlwaysSphere = false) const;
    bool            hitScene(const osgUtil::Radial3 &ray, osg::Vec3d &ptHitTest, bool bTerrainOnly) const;
    bool            getCameraCorrection(osg::Quat &qtCorrection, osg::Vec3d &vecCorrection) const;
    bool            calcNavBasePlane(osg::Plane &plane, bool bHorz = true) const;

    bool            OnKeyDown(const osgGA::GUIEventAdapter &ea);
    bool            OnKeyUp(const osgGA::GUIEventAdapter &ea);
    bool            OnMouseDown(const osgGA::GUIEventAdapter &ea);
    bool            OnMouseUp(const osgGA::GUIEventAdapter &ea);
    bool            OnMouseMove(const osgGA::GUIEventAdapter &ea);
    bool            OnMouseDrag(const osgGA::GUIEventAdapter &ea);
    bool            OnScroll(const osgGA::GUIEventAdapter &ea);
    bool            OnFrame(const osgGA::GUIEventAdapter &ea);

    bool            doLMouseDragNavigation(const osgGA::GUIEventAdapter &ea);
    bool            doRMouseDragNavigation(const osgGA::GUIEventAdapter &ea);
    bool            doMMouseDragNavigation(const osgGA::GUIEventAdapter &ea);
    bool            doKeyNavigation(double dblCurTime);

    bool            hasKeyDown(void) const;

    bool            setKeyboardState(bool bKeyDown, const osgGA::GUIEventAdapter &ea);

    osg::Vec3d      getCurrentCameraPos(void) const;
    osg::Vec3d      getCurrentCameraDir(void) const;
    osg::Vec3d      getCurrentCameraUp(void) const;
    osg::Vec3d      getCurrentCameraRight(void) const;
    osg::Vec3d      getCurrentCameraForward(void) const;
    osg::Vec3d      getCurrentPlumbLine(void) const;

    void            calcGroundPosInfo(void);

    bool            doInertiaNavigation(double dblCurTime);

protected:
    osg::ref_ptr<const osgGA::GUIEventAdapter>  m_pLastMouseEvent;      // 前一个鼠标消息
    osg::ref_ptr<const osgGA::GUIEventAdapter>  m_pLastMouseDownEvent;  // 前一个鼠标按下消息

    const osg::Vec3d                m_vecHomeDirection;

    osg::ref_ptr<osg::Node>         m_pTerrainNode;
    osg::ref_ptr<osg::Node>         m_pNavigationNode;
    osg::ref_ptr<osg::Camera>       m_pCurrentCamera;

    osg::Vec3d                      m_ptMouseDownHitScene;
    osg::Vec3d                      m_ptMouseDownHitScenePos;

    bool                            m_bMouseDownHitScene;
    bool                            m_bLButtonDown;
    bool                            m_bMButtonDown;
    bool                            m_bRButtonDown;
    bool                            m_bCameraUnderGround;
	bool                            m_bCameraUnderGroundEx;
    int                             m_nCameraPoseChangedByExternal;

    osg::Vec2d                      m_vecInertiaSpeed;      // 惯性速度

    CameraPose                      m_CurrentCameraPose;    // 当前相机位置

    double                          m_dblLastFrameTime;
    double                          m_dblSubGroundElev;     // 地下漫游时的极限值

    osg::Vec3d                      m_ptCameraGroundPos;

	osg::Vec3d						m_vec3dTerrainHitTestPos;
	CameraPose						m_tmpTerrainHitTestCamPos;
};

#endif
