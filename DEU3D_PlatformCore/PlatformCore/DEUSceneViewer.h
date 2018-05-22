#ifndef DEU_SCENE_VIEWER_H_AB94D729_671C_46F6_8C88_4034ADB9EA85_INCLUDE
#define DEU_SCENE_VIEWER_H_AB94D729_671C_46F6_8C88_4034ADB9EA85_INCLUDE

#include <osg/ref_ptr>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>
#include <osgViewer/CompositeViewer>

#include <string>

#include "ISceneViewer.h"
#include "ViewerParam.h"
#include "StatusBar.h"
#include "NavigatorManager.h"
#include "ToolManager.h"
#include "NavigationPathPlayer.h"
#include "ProjectionMatrixFixer.h"
#include "Compass.h"
#include "SkyDome.h"
#include "FluoroScope.h"
#include <ParameterSys\IParameter.h>
#include "SceneGraphOperator.h"

class DEUSceneViewer : public ISceneViewer, public osgViewer::CompositeViewer
{
public:
    explicit DEUSceneViewer(ea::IEventAdapter *pEventAdapter = NULL);
             DEUSceneViewer(const DEUSceneViewer &viewer, const osg::CopyOp &copyop = osg::CopyOp::SHALLOW_COPY) : osgViewer::CompositeViewer(viewer, copyop) {}
    virtual ~DEUSceneViewer(void);

    META_Object(osgViewer, DEUSceneViewer);

public:     // Methods from ISceneViewer
    virtual void                         doFrame(void);

    virtual void                         resetCameraPose(unsigned int nIndex);

    virtual IToolManager                *getToolManager(void)         {   return m_pToolManager.get();            }
    virtual INavigatorManager           *getNavigatorManager(unsigned int nIndex);
    virtual INavigationPathPlayer       *getNavigationPathPlayer(unsigned int nIndex);
    virtual IStatusBar                  *getStatusBar(unsigned int nIndex);
    virtual ICompass                    *getCompass(unsigned int nIndex);
    virtual ISkyDome                    *getSkyDome(void);

    virtual const IToolManager          *getToolManager(void) const   {   return m_pToolManager.get();            }
    virtual const INavigatorManager     *getNavigatorManager(unsigned int nIndex) const;
    virtual const IStatusBar            *getStatusBar(unsigned int nIndex) const;
    virtual const INavigationPathPlayer *getNavigationPathPlayer(unsigned int nIndex) const;
    virtual const ICompass              *getCompass(unsigned int nIndex) const;
    virtual const ISkyDome              *getSkyDome(void) const;

    virtual cmm::image::IDEUImage       *snapshot(unsigned int nIndex);

    virtual bool                         setMultyViewMode(const std::string &strMode);
    virtual const std::string           &getMultyViewMode(void) const  {   return m_strMultyViewMode;    }

    virtual cmm::image::IDEUImage       *snapshotFacadeSection(unsigned int nIndex, double dblPoxX1, double dblPosY1, double dblPoxX2, double dblPosY2, double dblHeight);

    virtual const IFluoroScope          *getFluoroScope(void) const;
    virtual IFluoroScope                *getFluoroScope(void);

    virtual void                         acceleratingNavigate(unsigned nIndex, const std::string &strNavMode, double dblTime);
	virtual double						 getHeightAt(double lon,double lat,bool bTerrainOnly);

public:
    bool            initialize(const ViewerParam &viewerParam, osg::Group *pRootGroup, SceneGraphOperator *pSceneGraphOperator);
    void            unInitialize(void);
    double          calcCameraPoseByFacadeSection(double dblPoxX1, double dblPosY1, double dblPoxX2, double dblPosY2, double dblHeight, CameraPose &pose);

protected:
    void            applyMultyViewMode(const std::string &strMode);
    void            applyEventHandlers(void);
    void            applyViewStyle(void);
	bool			hitScene(const osgUtil::Radial3 &ray, osg::Vec3d &ptHitTest, bool bTerrainOnly);

    struct ViewData
    {
        //视图场景数据
        osg::ref_ptr<osg::Group>            m_pViewSceneData;
        //视图状态条
        osg::ref_ptr<StatusBar>             m_pStatusBar;
        //视图罗盘
        osg::ref_ptr<Compass>               m_pCompass;
        //视图漫游器
        osg::ref_ptr<NavigatorManager>      m_pNavigatorManager;
        //视图漫游路径播放器
        osg::ref_ptr<NavigationPathPlayer>  m_pNavigationPathPlayer;
    };

    void initViewData(unsigned int nIndex);

protected:
    bool                                    m_bInitialized;
    std::vector<ViewData>                   m_vecViewData;

    //场景数据节点
    osg::ref_ptr<osg::Group>                m_pRootGroup;

    //视图天空球
    osg::ref_ptr<SkyDome>                   m_pSkyDome;

    //初始化参数
    ViewerParam                             m_ViewerParam;

    //挂接工具管理器
    osg::ref_ptr<ToolManager>               m_pToolManager;

    //事件适配器
    OpenSP::sp<ea::IEventAdapter>           m_pEventAdapter;

    //特效根节点
    osg::ref_ptr<osg::MatrixTransform>      m_pEffectData;

    osg::ref_ptr<ProjectionMatrixFixer>     m_pProjMatrixFixer;

    osg::ref_ptr<FluoroScope>               m_pFluoroScope;

    std::string                             m_strMultyViewMode;

    bool                                    m_bSyncNavi;
    bool                                    m_bSyncSky;

    osg::ref_ptr<SceneGraphOperator>        m_pSceneGraphOperator;
	osg::ref_ptr<osg::Node>					m_pTerrainNode;
	osg::ref_ptr<osg::Node>                 m_pNavigationNode;
};

#endif
