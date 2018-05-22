#include <osg/ref_ptr>
#include <osg/Hint>
#include <osg/StateSet>
#include <osg/PolygonMode>
#include <osgViewer/api/Win32/GraphicsWindowWin32>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Renderer>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osg/CullFace>
#include <osg/ProxyNode>
#include <osg/Math>
#include <osg/GLExtensions>
#include <sstream>

#include "Registry.h"
#include "DEUSceneViewer.h"
#include "FrameImageFetcher.h"

#include <Common/Common.h>
#include <osgUtil/FindNodeVisitor.h>
#include "NavigatorManager.h"
#include "AnimationModel.h"
#include <IDProvider/Definer.h>


DEUSceneViewer::DEUSceneViewer(ea::IEventAdapter *pEventAdapter) :
    osgViewer::CompositeViewer(),
    m_pEventAdapter(pEventAdapter),
    m_pToolManager(NULL),
    m_bSyncNavi(false),
    m_bSyncSky(false),
    m_bInitialized(false)
{
}


DEUSceneViewer::~DEUSceneViewer(void)
{
    unInitialize();
}


bool DEUSceneViewer::initialize(const ViewerParam &viewerParam, osg::Group *pRootGroup, SceneGraphOperator *pSceneGraphOperator)
{
    if(m_bInitialized)  return true;
    if(viewerParam.m_hWnd == NULL)
    {
        return false;
    }

    m_pSceneGraphOperator = pSceneGraphOperator;

    m_ViewerParam = viewerParam;
    osg::ref_ptr<osg::GraphicsContext::Traits> pTraits = new osg::GraphicsContext::Traits;
    pTraits->x                             = 0;
    pTraits->y                             = 0;
    pTraits->width                         = osg::clampAbove(m_ViewerParam.m_nInitializationWndWidth, 8u);
    pTraits->height                        = osg::clampAbove(m_ViewerParam.m_nInitializationWndHeight, 8u);
    pTraits->windowDecoration              = false;
    pTraits->doubleBuffer                  = true;
    pTraits->sharedContext                 = 0;
    pTraits->setInheritedWindowPixelFormat = true;
    pTraits->stencil                       = 8u;
    pTraits->inheritedWindowData           = new osgViewer::GraphicsWindowWin32::WindowData((HWND)viewerParam.m_hWnd);

    const Capabilities &cap = Registry::instance()->getCapabilities();
    osg::GraphicsContext *pGraphicsContext = NULL;
    if(cap.supportsMultiSample() && viewerParam.m_nAntiAliasTimes > 0u)
    {
        pTraits->samples       = osg::clampBelow(viewerParam.m_nAntiAliasTimes, 16u);
        pTraits->sampleBuffers = 1u;

        do{
            pGraphicsContext  = osg::GraphicsContext::createGraphicsContext(pTraits.get());
            if(pGraphicsContext)    break;

            pTraits->samples /= 2u;
        }while(pTraits->samples > 0u);
    }

    if(pGraphicsContext == NULL)
    {
        pTraits->samples       = 0u;
        pTraits->sampleBuffers = 0u;
        pGraphicsContext       = osg::GraphicsContext::createGraphicsContext(pTraits.get());
    }

    pGraphicsContext->setClearColor(osg::Vec4(0.5, 0.5, 0.5, 0.0));
    pGraphicsContext->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    osg::DisplaySettings::instance()->setCompileContextsHint(true);
    osgUtil::IncrementalCompileOperation *pOperation = new osgUtil::IncrementalCompileOperation;
    setIncrementalCompileOperation(pOperation); 

    m_pSkyDome = new SkyDome();     // 天空球
    m_pSkyDome->initialize();

    m_vecViewData.resize(4);

    osg::ref_ptr<osgViewer::View> pActiveView = new osgViewer::View;
    pActiveView->setName("1");
    addView(pActiveView.get());

    osg::Camera *pActiveCamera = pActiveView->getCamera();
    pActiveCamera->setName("DU");

    pActiveCamera->setGraphicsContext(pGraphicsContext);
    pActiveCamera->setViewport(pTraits->x, pTraits->y, pTraits->width, pTraits->height);

    m_pRootGroup = pRootGroup;

    {
        const ID idPlanet(0, TERRAIN_TILE, 0, 0, 0);
        osg::ref_ptr<osgUtil::FindNodeByNameVisitor>  pFinder = new osgUtil::FindNodeByNameVisitor("TerrainRootNode");
        m_pRootGroup->accept(*pFinder);

        m_pProjMatrixFixer = new ProjectionMatrixFixer;
        m_pProjMatrixFixer->setPlanetNode(pFinder->getTargetNode());
    }

    //工具管理器
    {
        m_pToolManager = new ToolManager(m_pEventAdapter.get());

        osg::ref_ptr<osgUtil::FindNodeByNameVisitor>    pFinder1 = new osgUtil::FindNodeByNameVisitor("ElementRootGroup");
        m_pRootGroup->accept(*pFinder1);
        m_pToolManager->setOperationTargetNode(pFinder1->getTargetNode());

        m_pToolManager->setSceneGraphOperator(m_pSceneGraphOperator.get());
        m_pToolManager->setSceneView(this);

        osg::ref_ptr<osgUtil::FindNodeByNameVisitor>    pFinder2 = new osgUtil::FindNodeByNameVisitor("TempElementRootGroup");
        m_pRootGroup->accept(*pFinder2);

        osg::Group *pTempElementRootGroup = dynamic_cast<osg::Group *>(pFinder2->getTargetNode());
        pTempElementRootGroup->addChild(m_pToolManager->getToolNode());
    }

    initViewData(0);

    //创建场景根节点和数据根节点
    {
        osg::ref_ptr<osgUtil::FindNodeByNameVisitor>   pFinder1 = new osgUtil::FindNodeByNameVisitor("CultureRootNode");
        m_pRootGroup->accept(*pFinder1);
        osg::Group *pCultureRootNode = dynamic_cast<osg::Group *>(pFinder1->getTargetNode());

        osg::ref_ptr<osgUtil::FindNodeByNameVisitor>   pFinder2 = new osgUtil::FindNodeByNameVisitor("ExtraElementRootGroup");
        m_pRootGroup->accept(*pFinder2);
        osg::Group *pExtraElementRootNode = dynamic_cast<osg::Group *>(pFinder2->getTargetNode());

        osg::ref_ptr<osgUtil::FindNodeByNameVisitor>   pFinder3 = new osgUtil::FindNodeByNameVisitor("TempElementRootGroup");
        m_pRootGroup->accept(*pFinder3);
        osg::Group *pTempElementRootNode = dynamic_cast<osg::Group *>(pFinder3->getTargetNode());

        //m_pFluoroScope = new FluoroScope;   //透视镜
        //m_pFluoroScope->init();
        //m_pFluoroScope->setFlatMode(true);

        //m_pFluoroScope->addToWhiteList(pCultureRootNode);
        //m_pFluoroScope->addToWhiteList(pExtraElementRootNode);
        //m_pFluoroScope->addToWhiteList(pTempElementRootNode);

        //m_pFluoroScope->setStatusBar(m_vecViewData[0].m_pStatusBar);
        //m_vecViewData[0].m_pViewSceneData->addChild(m_pFluoroScope.get());
    }

    applyMultyViewMode("default");

    pActiveView->setSceneData(m_vecViewData[0].m_pViewSceneData);
    setThreadingModel(osgViewer::ViewerBase::CullDrawThreadPerContext);
    realize();

    setKeyEventSetsDone(0);
    m_bInitialized = true;
    return true;
}


void DEUSceneViewer::initViewData(unsigned int nIndex)
{
    if(nIndex >= 4)
    {
        return;
    }

    ViewData &vData = m_vecViewData[nIndex];
    vData.m_pViewSceneData = m_pRootGroup;

    if(!m_bSyncNavi || nIndex == 0)
    {
        vData.m_pNavigatorManager = new NavigatorManager(m_pEventAdapter.get());
        vData.m_pNavigatorManager->initialize();
        
        osg::ref_ptr<osgUtil::FindNodeByNameVisitor>    pFinder = new osgUtil::FindNodeByNameVisitor("ElementRootGroup");
        m_pRootGroup->accept(*pFinder);
        vData.m_pNavigatorManager->setNaviNode(pFinder->getTargetNode());
        m_pNavigationNode = pFinder->getTargetNode();

        vData.m_pNavigationPathPlayer = new NavigationPathPlayer(m_pEventAdapter.get());
        vData.m_pNavigationPathPlayer->initialize(vData.m_pNavigatorManager.get(), m_pSceneGraphOperator.get());
    }
    else if(nIndex > 0 && m_bSyncNavi)
    {
        vData.m_pNavigatorManager = m_vecViewData[0].m_pNavigatorManager;
        vData.m_pNavigationPathPlayer = m_vecViewData[0].m_pNavigationPathPlayer;
    }

    osg::ref_ptr<osgUtil::FindNodeByNameVisitor>    pFinder1 = new osgUtil::FindNodeByNameVisitor("TempElementRootGroup");
    m_pRootGroup->accept(*pFinder1);
    osg::Group *pTempElementRootGroup = dynamic_cast<osg::Group *>(pFinder1->getTargetNode());
    pTempElementRootGroup->addChild(vData.m_pNavigationPathPlayer->getPlayerNode());

    osg::ref_ptr<osgUtil::FindNodeByNameVisitor>    pFinder2 = new osgUtil::FindNodeByNameVisitor("ExtraElementRootGroup");
    m_pRootGroup->accept(*pFinder2);
    osg::Group *pExtraElementRootGroup = dynamic_cast<osg::Group *>(pFinder2->getTargetNode());

    osg::ref_ptr<osgUtil::FindNodeByNameVisitor>    pFinder3 = new osgUtil::FindNodeByNameVisitor("TerrainRootNode");
    m_pRootGroup->accept(*pFinder3);
    osg::Node *pTerrainNode = pFinder3->getTargetNode();

    vData.m_pStatusBar = new StatusBar;   // 场景状态条
    vData.m_pStatusBar->initialize();

    vData.m_pStatusBar->setTargetNode(pTerrainNode);
    pExtraElementRootGroup->addChild(vData.m_pStatusBar.get());

    m_pTerrainNode = pTerrainNode;

    vData.m_pCompass = new Compass;       // 罗盘
    vData.m_pCompass->initialize();
    vData.m_pCompass->setNavigatorManager(vData.m_pNavigatorManager.get());

    pExtraElementRootGroup->addChild(vData.m_pCompass.get());
    pExtraElementRootGroup->addChild(m_pSkyDome.get());
}


void DEUSceneViewer::unInitialize(void)
{
    if(!m_bInitialized) return;

    m_pToolManager = NULL;

    unsigned int nViewCount = getNumViews();
    for(unsigned int i = 0u; i < nViewCount; i++)
    {
        osgViewer::View *pView = getView(i);
        removeView(pView);
    }
    setDone(true);
    m_bInitialized = false;
}


void DEUSceneViewer::doFrame(void)
{
    if(!m_bInitialized) return;
    if(done())          return;
    frame();
}


void DEUSceneViewer::applyViewStyle(void)
{
    for(unsigned int i = 0; i < getNumViews(); i++)
    {
        osgViewer::View *pView = getView(i);
        osg::Camera *pCamera = pView->getCamera();
        osg::Viewport *pViewport = pCamera->getViewport();

        const double dblWidth  = osg::clampAbove(double(pViewport->width()), 1.0);
        const double dblHeight = osg::clampAbove(double(pViewport->height()), 1.0);

        osg::ref_ptr<osg::Hint> pHint = new osg::Hint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        pCamera->getOrCreateStateSet()->setAttributeAndModes(pHint.get());
        pCamera->setProjectionMatrixAsPerspective(45.0, dblWidth / dblHeight, 1.0, 1e6);
        pCamera->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 1.0));

        //const osg::CullSettings::CullingMode nCullingMode = pCamera->getCullingMode();
        //pCamera->setCullingMode(nCullingMode & ~osg::CullSettings::SMALL_FEATURE_CULLING);

        osgViewer::Renderer *pRenderer = dynamic_cast<osgViewer::Renderer*>(pCamera->getRenderer());
        for(unsigned n = 0u; n < 2u; n++)
        {
            osgUtil::SceneView *pSceneView = pRenderer->getSceneView(n);
            osgUtil::CullVisitor *pCullVisitor = pSceneView->getCullVisitor();
            pCullVisitor->setClampProjectionMatrixCallback(m_pProjMatrixFixer.get());
        }
    }
}


void DEUSceneViewer::applyEventHandlers(void)
{
    int nDebugSceneViewer = 0;
    const char *ptr = ::getenv("DEU_DEBUG_SCENE_VIEWER");
    if(ptr)
    {
        nDebugSceneViewer = ::atoi(ptr);
        if((nDebugSceneViewer & 0x02) != 0x02)
        {
            setViewerStats(NULL);
        }
    }

    unsigned int nViewCount = getNumViews();
    for(unsigned int i = 0u; i < nViewCount; i++)
    {
        osgViewer::View *pView = getView(i);

        if(m_vecViewData[i].m_pNavigatorManager.valid())
        {
            pView->setCameraManipulator(m_vecViewData[i].m_pNavigatorManager.get());
        }

        if(m_pToolManager.valid())
        {
            pView->addEventHandler(m_pToolManager);
        }

        if(m_vecViewData[i].m_pNavigationPathPlayer.valid())
        {
            pView->addEventHandler(m_vecViewData[i].m_pNavigationPathPlayer.get());
        }

        // add the state manipulator
        if(ptr)
        {
            if((nDebugSceneViewer & 0x01) == 0x01)
            {
                pView->addEventHandler(new osgGA::StateSetManipulator(pView->getCamera()->getOrCreateStateSet()));
            }
            if((nDebugSceneViewer & 0x02) == 0x02)
            {
                pView->addEventHandler(new osgViewer::StatsHandler);
            }
        }
    }
}


void DEUSceneViewer::resetCameraPose(unsigned int nIndex)
{
    if(!m_bInitialized) return;
    if(nIndex >= getNumViews())
    {
        return;
    }
    //m_vecViewData[nIndex].m_pNavigatorManager->resetCamera();
    //getView(nIndex)->requestRedraw();
    //getView(nIndex)->requestContinuousUpdate();

    osgViewer::View *pView = getView(nIndex);
    osgGA::EventQueue *pEventQueue = pView->getEventQueue();
    const int nSpaceKey = 32;
    const double dblTime = pEventQueue->getTime();

    osgGA::GUIEventAdapter *pResetEvent0 = new osgGA::GUIEventAdapter();
    pResetEvent0->setEventType(osgGA::GUIEventAdapter::KEYDOWN);
    pResetEvent0->setKey(nSpaceKey);
    pResetEvent0->setUnmodifiedKey(nSpaceKey);
    pResetEvent0->setTime(dblTime);
    pEventQueue->addEvent(pResetEvent0);

    osgGA::GUIEventAdapter *pResetEvent1 = new osgGA::GUIEventAdapter();
    pResetEvent1->setEventType(osgGA::GUIEventAdapter::KEYUP);
    pResetEvent1->setKey(nSpaceKey);
    pResetEvent1->setUnmodifiedKey(nSpaceKey);
    pResetEvent1->setTime(dblTime);
    pEventQueue->addEvent(pResetEvent1);
}


cmm::image::IDEUImage *DEUSceneViewer::snapshot(unsigned int nIndex)
{
    if(!m_bInitialized) return NULL;

    if(nIndex >= getNumViews())
    {
        return NULL;
    }

    osgViewer::View *pView = getView(nIndex);
    osg::Camera *pCamera = pView->getCamera();
    if(pCamera->getFinalDrawCallback())
    {
        return NULL;
    }

    osg::ref_ptr<FrameImageFetcher>  pFrameImageFetcher = new FrameImageFetcher;

    pCamera->setFinalDrawCallback(pFrameImageFetcher.get());
    pFrameImageFetcher->pulseSnapshot();
    pFrameImageFetcher->waitForFrame();
    pCamera->setFinalDrawCallback(NULL);

    const osg::Image *pImage = pFrameImageFetcher->getImage();

    cmm::image::Image *pDeuImage = new cmm::image::Image;
    pDeuImage->allocImage(pImage->s(), pImage->t(), cmm::image::PF_RGBA);
    memcpy(pDeuImage->data(), pImage->data(), pImage->getImageSizeInBytes());
    return pDeuImage;
}


cmm::image::IDEUImage *DEUSceneViewer::snapshotFacadeSection(unsigned int nIndex, double dblPoxX1, double dblPosY1, double dblPoxX2, double dblPosY2, double dblHeight)
{
    if(!m_bInitialized) return NULL;

    if(nIndex >= getNumViews())
    {
        return NULL;
    }

    CameraPose cpos;
    const double dblDistance = calcCameraPoseByFacadeSection(dblPoxX1, dblPosY1, dblPoxX2, dblPosY2, dblHeight, cpos);
    m_vecViewData[nIndex].m_pNavigatorManager->setCameraPose(cpos);
    m_pProjMatrixFixer->clampNearAbove(dblDistance);
    cmm::image::IDEUImage *pImageObj = snapshot(nIndex);
    m_pProjMatrixFixer->clampNearAbove(-1.0);

    return pImageObj;
}


double DEUSceneViewer::calcCameraPoseByFacadeSection(double dblPoxX1, double dblPosY1, double dblPoxX2, double dblPosY2, double dblHeight, CameraPose &pose)
{
    const osg::Camera *pCmamera = getView(0)->getCamera();
    const osg::Matrixd &projectMat = pCmamera->getProjectionMatrix();
    double fovy, aspectRatio, zNear, zFar;
    projectMat.getPerspective(fovy, aspectRatio, zNear, zFar);

    osg::ref_ptr<osg::EllipsoidModel> pEllipsoidModel = new osg::EllipsoidModel;
    double dP1W_X, dP1W_Y, dP1W_Z;
    pEllipsoidModel->convertLatLongHeightToXYZ(dblPosY1, dblPoxX1, dblHeight, dP1W_X, dP1W_Y, dP1W_Z);

    double dP2W_X,dP2W_Y,dP2W_Z;
    pEllipsoidModel->convertLatLongHeightToXYZ(dblPosY2, dblPoxX2, dblHeight, dP2W_X, dP2W_Y, dP2W_Z);

    const osg::Vec3d startPoint = osg::Vec3d(dP1W_X,dP1W_Y,dP1W_Z);
    const osg::Vec3d endPoint = osg::Vec3d(dP2W_X,dP2W_Y,dP2W_Z);

    const osg::Vec3d midePoint = (startPoint + endPoint)/2.0;

    osg::Vec3d plumbDir = -midePoint;
    plumbDir.normalize();

    osg::Vec3d lineDir = endPoint - startPoint;
    lineDir.normalize();

    osg::Vec3d negativeEyeDir =  plumbDir ^ lineDir;
    negativeEyeDir.normalize();

    double dViewWorldHeight = (endPoint - startPoint).length() / aspectRatio;
    double dViewDistance = (dViewWorldHeight/2.0) / tan(osg::DegreesToRadians(fovy)/2.0);

    osg::Vec3d curEyePos = midePoint + negativeEyeDir * dViewDistance;
                                
    double dNewLatValue,dNewLongValue,dNewHeight;
    pEllipsoidModel->convertXYZToLatLongHeight(curEyePos.x(),curEyePos.y(),curEyePos.z(),dNewLatValue,dNewLongValue,dNewHeight);

    osg::Vec2d easterDir(1.0,0.0);
    osg::Vec2d latlongDir(dblPoxX2-dblPoxX1,dblPosY2-dblPosY1);
    latlongDir.normalize();
    double AzimuthAngle = acos(latlongDir * easterDir);
    if (latlongDir.y() < 0)
    {
        AzimuthAngle = -AzimuthAngle;
    }

    AzimuthAngle += osg::PI_2;
    if (AzimuthAngle > osg::PI)
    {
        AzimuthAngle -= osg::PI * 2;
    }
    pose.m_dblPositionX = dNewLongValue;
    pose.m_dblPositionY = dNewLatValue;
    pose.m_dblHeight = dblHeight;
    pose.m_dblPitchAngle = osg::PI_2;
    pose.m_dblAzimuthAngle = AzimuthAngle;

    return dViewDistance;
}


bool DEUSceneViewer::setMultyViewMode(const std::string &strMode)
{
    if(!m_bInitialized) return false;

    if(strMode.compare("default") == 0
        || strMode.compare("two") == 0
        || strMode.compare("four") == 0)
    {
        applyMultyViewMode(strMode);
        m_strMultyViewMode = strMode;
        return true;
    }
    return false;
}


void DEUSceneViewer::applyMultyViewMode(const std::string &strMode)
{
    if(m_strMultyViewMode == strMode)
    {
        return;
    }

    m_strMultyViewMode = strMode;

    //以0号视图为基准
    osgViewer::View *pView0 = getView(0);
    osg::Node *pRootNode = pView0->getSceneData();
    osg::Camera *pCamera0 = pView0->getCamera();

    double dblFovy, dblRatio, dblNear, dblFar;
    pCamera0->getProjectionMatrixAsPerspective(dblFovy, dblRatio, dblNear, dblFar);

    osg::GraphicsContext *pGraphicsContext = pCamera0->getGraphicsContext();
    const osg::Viewport *pViewport0 = pCamera0->getViewport();

    osg::BoundingBox  bb;
    bb.expandBy(pViewport0->x(), pViewport0->y(), 0.0f);
    bb.expandBy(pViewport0->x() + pViewport0->width(), pViewport0->y() + pViewport0->height(), 0.0f);

    unsigned nViewCount = getNumViews();

    //删除0号视图以外的视图
    for(unsigned i = nViewCount - 1u; i >= 1u; i--)
    {
        osgViewer::View *pView = getView(i);

        const osg::Camera *pCamera = pView->getCamera();
        const osg::Viewport *pViewport = pCamera->getViewport();
        bb.expandBy(pViewport->x(), pViewport->y(), 0.0f);
        bb.expandBy(pViewport->x() + pViewport->width(), pViewport->y() + pViewport->height(), 0.0f);

        removeView(pView);
    }

    const osg::Vec2s vecWndSize(short(bb.xMax() - bb.xMin() + 0.5f), short(bb.yMax() - bb.yMin() + 0.5f));

    const unsigned nSeparator = 2u;
    if(m_strMultyViewMode.compare("default") == 0)
    {
        osgViewer::View *pView1 = getView(0u);

        osg::Camera *pCamera1 = pView1->getCamera();
        pCamera1->setViewport(0u, 0u, vecWndSize.x(), vecWndSize.y());
    }
    else if(m_strMultyViewMode.compare("two") == 0)
    {
        const unsigned nViewWidth = (vecWndSize.x() - 2) / 2u;

        // 第一个视图在左边
        osgViewer::View *pView1 = getView(0u);

        osg::Camera *pCamera1 = pView1->getCamera();
        pCamera1->setViewport(0u, 0u, nViewWidth, vecWndSize.y());

        // 第二个视图在右边
        const unsigned nViewPosX = nViewWidth + 2u;

        osgViewer::View *pView2 = new osgViewer::View;

        osg::Camera *pCamera2 = pView2->getCamera();
        pCamera2->setGraphicsContext(pGraphicsContext);
        pCamera2->setViewport(nViewPosX, 0u, nViewWidth, vecWndSize.y());

        if(!m_vecViewData[1].m_pViewSceneData.valid())
        {
            initViewData(1);
        }

        pView2->setSceneData(m_vecViewData[1].m_pViewSceneData);
        addView(pView2);
    }
    else if(m_strMultyViewMode.compare("four") == 0)
    {
        const int nWidth = (vecWndSize.x()  - 2u) / 2u;
        const int nHeight = (vecWndSize.y() - 2u) / 2u;

        // 第一个视图在左上角
        {
            osgViewer::View *pView1 = getView(0);
            osg::Camera *pCamera1 = pView1->getCamera();
            pCamera1->setViewport(0u, nHeight + 2u, nWidth, nHeight);
        }

        // 第二个视图在右上角
        {
            osgViewer::View *pView2 = new osgViewer::View;
            osg::Camera *pCamera2 = pView2->getCamera();
            pCamera2->setViewport(nWidth + 2u, nHeight + 2u, nWidth, nHeight);
            pCamera2->setGraphicsContext(pGraphicsContext);

            if(!m_vecViewData[1].m_pViewSceneData.valid())
            {
                initViewData(1);
            }
            pView2->setSceneData(m_vecViewData[1].m_pViewSceneData);
            addView(pView2);
        }

        // 第三个视图在左下角
        {
            osgViewer::View *pView3 = new osgViewer::View;
            osg::Camera *pCamera3 = pView3->getCamera();
            pCamera3->setViewport(0, 0, nWidth, nHeight);
            pCamera3->setGraphicsContext(pGraphicsContext);

            if(!m_vecViewData[2].m_pViewSceneData.valid())
            {
                initViewData(2);
            }
            pView3->setSceneData(m_vecViewData[2].m_pViewSceneData);
            addView(pView3);
        }

        // 第四个视图在右上角
        {
            osgViewer::View *pView4 = new osgViewer::View;
            osg::Camera *pCamera4 = pView4->getCamera();
            pCamera4->setViewport(nWidth + 2u, 0, nWidth, nHeight);
            pCamera4->setGraphicsContext(pGraphicsContext);

            if(!m_vecViewData[3].m_pViewSceneData.valid())
            {
                initViewData(3);
            }
            pView4->setSceneData(m_vecViewData[3].m_pViewSceneData);
            addView(pView4);
        }
    }

    const unsigned nNumViews = getNumViews();
    for(unsigned nViewIndex = 0u; nViewIndex < nNumViews; nViewIndex++)
    {
        osgViewer::View *pView = getView(nViewIndex);
        pView->setViewIndex(nViewIndex);

        std::ostringstream oss;
        oss << nViewIndex;
        pView->setName(oss.str());

        //pView->getDatabasePager()->setDeleteRemovedSubgraphsInDatabaseThread(false);
        osg::Camera *pCamera = pView->getCamera();
        pCamera->setName(oss.str());

        //pCamera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
        pCamera->setCullingMode(pCamera->getCullingMode() & ~osg::CullStack::SMALL_FEATURE_CULLING);
        pCamera->setNearFarRatio(0.0001f);

        //pCamera->setProjectionResizePolicy(osg::Camera::HORIZONTAL);

        const osg::Viewport *pViewport = pCamera->getViewport();
        const double dblRatioNew = double(pViewport->width()) / double(pViewport->height());
        pCamera->setProjectionMatrixAsPerspective(dblFovy, dblRatioNew, dblNear, dblFar);

        osg::ref_ptr<osg::Hint> pHint = new osg::Hint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        pCamera->getOrCreateStateSet()->setAttributeAndModes(pHint);
        pCamera->setClearColor(osg::Vec4(0.078f, 0.235f, 0.706f, 1.0f));
    }

    applyViewStyle();
    applyEventHandlers();
}


const IFluoroScope *DEUSceneViewer::getFluoroScope(void) const
{
    if(!m_bInitialized) return NULL;
    return m_pFluoroScope.get();
}

IFluoroScope *DEUSceneViewer::getFluoroScope(void)
{
    if(!m_bInitialized) return NULL;
    return m_pFluoroScope.get();
}


INavigatorManager *DEUSceneViewer::getNavigatorManager(unsigned int nIndex)
{
    if(!m_bInitialized) return NULL;
    if(nIndex >= getNumViews())
    {
        return NULL;
    }
    return m_vecViewData[nIndex].m_pNavigatorManager.get();
}


const INavigatorManager *DEUSceneViewer::getNavigatorManager(unsigned int nIndex) const
{
    if(!m_bInitialized) return NULL;
    if(nIndex >= getNumViews())
    {
        return NULL;
    }
    return m_vecViewData[nIndex].m_pNavigatorManager.get();
}


INavigationPathPlayer *DEUSceneViewer::getNavigationPathPlayer(unsigned int nIndex)
{
    if(!m_bInitialized) return NULL;
    if(nIndex >= getNumViews())
    {
        return NULL;
    }
    return m_vecViewData[nIndex].m_pNavigationPathPlayer.get();
}


const INavigationPathPlayer *DEUSceneViewer::getNavigationPathPlayer(unsigned int nIndex) const
{
    if(!m_bInitialized) return NULL;
    if(nIndex >= getNumViews())
    {
        return NULL;
    }
    return m_vecViewData[nIndex].m_pNavigationPathPlayer.get();
}


IStatusBar *DEUSceneViewer::getStatusBar(unsigned int nIndex)
{
    if(!m_bInitialized) return NULL;
    if(nIndex >= getNumViews())
    {
        return NULL;
    }
    return m_vecViewData[nIndex].m_pStatusBar.get();
}


const IStatusBar *DEUSceneViewer::getStatusBar(unsigned int nIndex) const
{
    if(!m_bInitialized) return NULL;
    if(nIndex >= getNumViews())
    {
        return NULL;
    }
    return m_vecViewData[nIndex].m_pStatusBar.get();
}


ICompass *DEUSceneViewer::getCompass(unsigned int nIndex)
{
    if(!m_bInitialized) return NULL;
    if(nIndex >= getNumViews())
    {
        return NULL;
    }
    return m_vecViewData[nIndex].m_pCompass.get();
}


const ICompass *DEUSceneViewer::getCompass(unsigned int nIndex) const
{
    if(!m_bInitialized) return NULL;
    if(nIndex >= getNumViews())
    {
        return NULL;
    }
    return m_vecViewData[nIndex].m_pCompass.get();
}


ISkyDome *DEUSceneViewer::getSkyDome(void)
{
    if(!m_bInitialized) return NULL;
    return m_pSkyDome.get();
}


const ISkyDome *DEUSceneViewer::getSkyDome(void) const
{
    if(!m_bInitialized) return NULL;
    return m_pSkyDome.get();
}


void DEUSceneViewer::acceleratingNavigate(unsigned nIndex, const std::string &strNavMode, double dblTime)
{
    if(!m_bInitialized) return;
    INavigationPathPlayer *pPathPlayer = getNavigationPathPlayer(nIndex);
    if(!pPathPlayer)    return;

    INavigatorManager *pNavManager = getNavigatorManager(nIndex);
    if(!pNavManager)    return;

    if(dblTime < 0.1)  dblTime = 0.1;

    OpenSP::sp<NavigationKeyframe>  pKeyframe0 = new NavigationKeyframe;
    OpenSP::sp<NavigationKeyframe>  pKeyframe1 = new NavigationKeyframe;

    pNavManager->getCameraPose(pKeyframe0->m_CameraPose);
    pKeyframe0->m_bArgForTime = true;
    pKeyframe0->m_dblRotate_TimeOrSpeed = dblTime;
    pKeyframe0->m_dblTrans_TimeOrSpeed  = dblTime;

    CameraPose &pose = pKeyframe1->m_CameraPose;
    pose = pKeyframe0->m_CameraPose;

    if(strNavMode.compare("zoom_in") == 0)
    {
        pose.m_dblHeight -= pose.m_dblHeight * 0.2;
    }
    else if(strNavMode.compare("zoom_out") == 0)
    {
        pose.m_dblHeight += pose.m_dblHeight * 0.2;
    }
    else if(strNavMode.compare("move_left") == 0)
    {
        const double dblDistance = (pose.m_dblHeight * 0.5) / osg::WGS_84_RADIUS_EQUATOR;
        const double dblAzimuth  = pose.m_dblAzimuthAngle - osg::PI_2;
        const double dblDeltaX   = dblDistance * cos(dblAzimuth);
        const double dblDeltaY   = dblDistance * sin(dblAzimuth);
        pose.m_dblPositionX     -= dblDeltaX * cos(pose.m_dblPositionY);
        pose.m_dblPositionY     -= dblDeltaY;
    }
    else if(strNavMode.compare("move_right") == 0)
    {
        const double dblDistance = (pose.m_dblHeight * 0.5) / osg::WGS_84_RADIUS_EQUATOR;
        const double dblAzimuth  = pose.m_dblAzimuthAngle - osg::PI_2;
        const double dblDeltaX   = dblDistance * cos(dblAzimuth);
        const double dblDeltaY   = dblDistance * sin(dblAzimuth);
        pose.m_dblPositionX     += dblDeltaX * cos(pose.m_dblPositionY);
        pose.m_dblPositionY     += dblDeltaY;
    }
    else if(strNavMode.compare("move_front") == 0)
    {
        const double dblDistance = (pose.m_dblHeight * 0.5) / osg::WGS_84_RADIUS_EQUATOR;
        const double dblAzimuth  = pose.m_dblAzimuthAngle - osg::PI_2;
        const double dblDeltaX   = dblDistance * sin(dblAzimuth);
        const double dblDeltaY   = dblDistance * cos(dblAzimuth);
        pose.m_dblPositionX     -= dblDeltaX * cos(pose.m_dblPositionY);
        pose.m_dblPositionY     += dblDeltaY;
    }
    else if(strNavMode.compare("move_back") == 0)
    {
        const double dblDistance = (pose.m_dblHeight * 0.5) / osg::WGS_84_RADIUS_EQUATOR;
        const double dblAzimuth  = pose.m_dblAzimuthAngle - osg::PI_2;
        const double dblDeltaX   = dblDistance * sin(dblAzimuth);
        const double dblDeltaY   = dblDistance * cos(dblAzimuth);
        pose.m_dblPositionX     += dblDeltaX * cos(pose.m_dblPositionY);
        pose.m_dblPositionY     -= dblDeltaY;
    }
    else if(strNavMode.compare("rotate_left") == 0)
    {
        pose.m_dblAzimuthAngle  += osg::DegreesToRadians(5.0);
    }
    else if(strNavMode.compare("rotate_right") == 0)
    {
        pose.m_dblAzimuthAngle  -= osg::DegreesToRadians(5.0);
    }
    else if(strNavMode.compare("rotate_up") == 0)
    {
        pose.m_dblPitchAngle    += osg::DegreesToRadians(5.0);
        if(pose.m_dblPitchAngle > osg::DegreesToRadians(160.0))
        {
            pose.m_dblPitchAngle = osg::DegreesToRadians(160.0);
        }
    }
    else if(strNavMode.compare("rotate_down") == 0)
    {
        pose.m_dblPitchAngle    -= osg::DegreesToRadians(5.0);
        if(pose.m_dblPitchAngle < 0.0)
        {
            pose.m_dblPitchAngle = 0.0;
        }
    }
    else
    {
        return;
    }

    OpenSP::sp<NavigationPath>  pNavigationPath = new NavigationPath;
    pNavigationPath->appendItem(pKeyframe0);
    pNavigationPath->appendItem(pKeyframe1);
    pPathPlayer->setNavigationPath(pNavigationPath);
    pPathPlayer->play();
}

bool DEUSceneViewer::hitScene(const osgUtil::Radial3 &ray, osg::Vec3d &ptHitTest, bool bTerrainOnly)
{
	const osgUtil::LineSegmentIntersector::CoordinateFrame cf = osgUtil::Intersector::WINDOW;
	osg::ref_ptr<osgUtil::LineSegmentIntersector>   pPicker  = new osgUtil::LineSegmentIntersector(cf, ray.getOrigin(), ray.getPoint(1e6));
	osg::ref_ptr<osgUtil::IntersectionVisitor>      pVisitor = new osgUtil::IntersectionVisitor(pPicker.get());
	//pVisitor->setTraversalMask(0x00FFFFFF);
	if(bTerrainOnly)
	{
		if(m_pTerrainNode == NULL)
		{
			return false;
		}
		m_pTerrainNode->accept(*pVisitor);
	}
	else
	{
		if(m_pNavigationNode == NULL)
		{
			return false;
		}
		m_pNavigationNode->accept(*pVisitor);
	}



	if(!pPicker->containsIntersections())
	{
		return false;
	}

	const osgUtil::LineSegmentIntersector::Intersections &inters = pPicker->getIntersections();	
	ptHitTest = pPicker->getFirstIntersection().getWorldIntersectPoint();
	return true;

}

double DEUSceneViewer::getHeightAt(double lon,double lat,bool bTerrainOnly)
{
	double dblLatitude  = osg::DegreesToRadians(lat);
	double dblLongitude = osg::DegreesToRadians(lon);
	osg::Vec3d xyzPos;
	osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
	//从海拔10000米高空为起点构建射线
	pEllipsoidModel->convertLatLongHeightToXYZ(dblLatitude, dblLongitude, 10000, xyzPos.x(), xyzPos.y(), xyzPos.z());

	//修改后代码。构建射线与地形瓦片求交取得高程，通过交点高程和相机高程比对来决定，是否碰到了地形。		
	osg::Vec3d direction = xyzPos;
	direction.normalize();
	osgUtil::Radial3 ray(xyzPos,-direction);

	osg::Vec3d ptHitPos;
	if(!hitScene(ray, ptHitPos, bTerrainOnly))			
	{
		return -1;
	}	
	double dblRayLatitude = 0.0, dblRayLongitude = 0.0, dblRayHeight = 0.0;
	pEllipsoidModel->convertXYZToLatLongHeight(ptHitPos.x(), ptHitPos.y(), ptHitPos.z(), dblRayLatitude, dblRayLongitude, dblRayHeight);
	return dblRayHeight;

}
