#include "DEUPlatformCore.h"

#include <osgDB/Registry>
#include <osg/Math>
#include <osg/CullFace>
#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/CommonOSG.h>
#include <Common/Common.h>
#include "ICompass.h"
#include <EventAdapter/IEventAdapter.h>
#include <EventAdapter/IEventFilter.h>
#include <osgDB/ReadFile>
#include <osgViewer/CompositeViewer>
#include <osgUtil/IntersectionVisitor.h>
#include <osgUtil/FindNodeVisitor.h>
#include <osg/PolygonMode>
#include <osg/Stencil>
#include <osgTerrain/TerrainTile>
#include <osg/ProxyNode>
#include <osg/SharedObjectPool>
#include <osg/MultiLineDrawable>
#include <osgShadow/ShadowedScene>
#include <osgShadow/SoftShadowMap>
#include <map>
#include <IDProvider/Definer.h>

#include "InitializationParam.h"
#include "Utility.h"
#include "StateBase.h"
#include "Registry.h"
#include "ParmRectifyThreadPool.h"
#include "LogicalManager/ILayerManager.h"
#include "StateChanged_Operation.h"
#include "AddOrRemove_Operation.h"
#include "TerrainModification_Operation.h"
#include "RemoveTileByLevel_Operation.h"
#include "Selecting_Operation.h"
#include "FetchingElevation_Operation.h"
#include "Intersect_Operation.h"

#include <osg/CullFace>
#include <osg/BlendFunc>
#include <osg/ShapeDrawable>

#include "DEUCheck.h"
DEUCheck checker(18, 4);


IPlatformCore *createPlatformCore(ea::IEventAdapter *pEventAdapter)
{
    OpenSP::sp<DEUPlatformCore> pPlatformCore = new DEUPlatformCore(pEventAdapter);
    return pPlatformCore.release();
}


void DEUPlatformCore::MemeryCheckerThread::run(void)
{
    double dblMemoryLimitedCuttingBeginRatio = 0.9;
    char *pszLimitedParam = ::getenv("MemoryLimitedParam");
    if(pszLimitedParam)
    {
        const std::string strParam = pszLimitedParam;
        std::istringstream iss(strParam);

        unsigned nMemoryLimited = ~0u;
        double dblBeginRatio = -1.0;
        iss >> nMemoryLimited >> dblBeginRatio;
        if(nMemoryLimited != ~0u)
        {
            m_nMemoryLimited = nMemoryLimited;
            m_nMemoryLimited *= 1024ui64;
            m_nMemoryLimited *= 1024ui64;
        }
        if(dblBeginRatio > 0.0)
        {
            dblMemoryLimitedCuttingBeginRatio = dblBeginRatio;
        }
    }

    const double dblMemoryLimited = m_nMemoryLimited;

    const double dblBeginCutting = dblMemoryLimited * dblMemoryLimitedCuttingBeginRatio;
    const double dblBeginCuttingingDelta = dblMemoryLimited - dblBeginCutting;

    bool bAllow = true;
    double dblLastMemRange = 1.0;
    while(0u == (unsigned)m_MissionFinished)
    {
        if(m_nMemoryLimited != 0ui64)
        {
            const double dblCurMemory = (double)cmm::getProcessMemory();

            if(dblCurMemory > dblBeginCutting)
            {
                // 缩短PagedLOD的视距，让更多的节点能够卸载
                const double dblBeyond = dblCurMemory - dblBeginCutting;
                const double dblRatio = dblBeyond / dblBeginCuttingingDelta;
                const double dblMemRange = clampValue(dblRatio);

                const double dblSpeed = ((dblMemRange < dblLastMemRange) ? 0.01 : 0.001);
                const double dblActMemRange = dblLastMemRange + (dblMemRange - dblLastMemRange) * dblSpeed;
                osg::PagedLOD::setMemRangeRatio(dblActMemRange);

                if(m_pDatabasePager.valid())
                {
                    if(dblActMemRange < 0.5)
                    {
                        //std::cout << __LINE__ << "disable response." << std::endl;
                        m_pDatabasePager->setAcceptNewDatabaseRequests(false);
                        bAllow = false;
                    }
                    else if(dblActMemRange > dblLastMemRange)
                    {
                        //std::cout << __LINE__ << "enable response." << std::endl;
                        bAllow = true;
                        m_pDatabasePager->setAcceptNewDatabaseRequests(true);
                    }
                }

                dblLastMemRange = dblActMemRange;
            }
            else
            {
                dblLastMemRange = 1.0;
                osg::PagedLOD::setMemRangeRatio(1.0f);
                bAllow = true;
                m_pDatabasePager->setAcceptNewDatabaseRequests(true);
            }
        }
        m_blockWait.block(50u);
    }
}


DEUPlatformCore::DEUPlatformCore(ea::IEventAdapter *pEventAdapter) :
    m_bInitialized(false),
    m_pEventAdapter(pEventAdapter),
    m_pMemCheckerThread(NULL),
    m_bUseShadow(false)
{
}


DEUPlatformCore::~DEUPlatformCore(void)
{
    if(m_bInitialized)
    {
        unInitialize();
    }
}


bool DEUPlatformCore::initialize(const InitializationParam *pInitParam,
                                 const std::string &strHost,
                                 const std::string &strPort,
                                 const std::string &strPSHost,
                                 const std::string &strPSPort,
                                 const std::string &strUserName,
                                 const std::string &strPassword,
                                 const std::string &strLocalCache,
                                 cmm::IStateQuerier *pStateQuerier)
{
    if(m_bInitialized)  return true;
    if(!pInitParam)     return false;

    m_pFileReadInterceptor = new FileReadInterceptor;
    bool bSucceed = m_pFileReadInterceptor->login(strPSHost, strPSPort, strUserName, strPassword);
    //- 添加空球程序崩溃，现暂时将该逻辑判断屏蔽
//     if (!bSucceed)
//     {
//         return false;
//     }
    bSucceed = m_pFileReadInterceptor->initialize(pInitParam->m_bBifurcateFetchingThread, strHost, strPort, strLocalCache, pStateQuerier);
    if(!bSucceed)
    {
        return false;
    }

    VCubeFragmentNode::setStateQuerier(pStateQuerier);
    VCubeFragmentNode::setLoadingCountPerFrame(pInitParam->m_nLoadingCountPerFrame);

    logical::ILayerManager *pLayerManager = dynamic_cast<logical::ILayerManager *>(pStateQuerier);
    if(pLayerManager)
    {
        m_pVCubeManager = pLayerManager->getVirtualCubeManager();
        m_pVCubeManager->startChangingListening();
    }

    m_pSceneGraphOperator = new SceneGraphOperator;
    m_pStateManager = new StateManager(m_pSceneGraphOperator);
    m_pFileReadInterceptor->setStateManager(m_pStateManager); 

    m_pTerrainModificationManager   = new TerrainModificationManager(m_pEventAdapter.get());
    m_pFileReadInterceptor->setTerrainModificationManager(m_pTerrainModificationManager);

    OpenSP::Ref *pRefReadFile = m_pFileReadInterceptor.get();
    osgDB::ReadFileCallback *pReadFileCallback = dynamic_cast<osgDB::ReadFileCallback *>(pRefReadFile);
    osgDB::Registry::instance()->addReadFileCallback(pReadFileCallback);

    const unsigned nThreadCount = osg::clampAbove(pInitParam->m_nFetchingThreadCount, 1u);
    osg::DisplaySettings::instance()->setNumOfDatabaseThreadsHint(nThreadCount);

    //创建三维视图
    m_pSceneViewer = new DEUSceneViewer(m_pEventAdapter);

    ViewerParam viewerParam;
    viewerParam.m_hWnd                     = pInitParam->m_hWnd;
    viewerParam.m_dblFovy                  = 45.0;
    viewerParam.m_nAntiAliasTimes          = pInitParam->m_nAntiAliasTimes;
    viewerParam.m_nInitializationWndWidth  = pInitParam->m_nInitializationWndWidth;
    viewerParam.m_nInitializationWndHeight = pInitParam->m_nInitializationWndHeight;
    viewerParam.m_idTerrain.set(0, TERRAIN_TILE, 0, 0, 0);

    //m_bUseShadow = pInitParam->m_bUseShadow;
    m_bUseShadow = false;
    Registry::instance()->setUseShadow(m_bUseShadow);
    initSceneGraph();

    m_pSceneViewer->initialize(viewerParam, m_pRootGroup.get(), m_pSceneGraphOperator.get());

    if(m_pVCubeManager.valid())
    {
        m_pVCubeChangingListener = new VCubeChangingListener;
        m_pVCubeChangingListener->initialize(m_pVCubeManager.get(), m_pFileReadInterceptor, m_pSceneGraphOperator.get());
        m_pVCubeChangingListener->startThread();
    }

    m_pUtility = new Utility(this);

    m_pPlatformReceiver = new PlatformReceiver(this);
    OpenSP::sp<ea::IEventFilter> pEventFilter = ea::createEventFilter();
    pEventFilter->addAction("RefreshTerrainTile");
    m_pEventAdapter->registerReceiver(m_pPlatformReceiver.get(), pEventFilter.get());

    osgDB::DatabasePager *pDatabasePager = m_pSceneViewer->getView(0u)->getDatabasePager();
    m_pMemCheckerThread = new MemeryCheckerThread(pDatabasePager);
    m_pMemCheckerThread->startThread();

    m_bInitialized = true;

    return true;
}


void DEUPlatformCore::initSceneGraph(void)
{
    m_pRootGroup = new osg::Group;
    m_pRootGroup->setName("RootGroup");

    // 1、地形和人文的共同根节点
    {
        m_pElementRootGroup = new osg::Group;
        m_pElementRootGroup->setName("ElementRootGroup");
        m_pRootGroup->addChild(m_pElementRootGroup);

        m_pCultureRootSwitch = new osg::Switch();
        m_pElementRootGroup->addChild(m_pCultureRootSwitch.get());
        
        //- 设置渲染状态
        {
            osg::ref_ptr<osg::CullFace> pCullFace = new osg::CullFace(osg::CullFace::BACK);
            osg::StateSet *pStateSet = m_pElementRootGroup->getOrCreateStateSet();
            pStateSet->setAttributeAndModes(pCullFace.get(), osg::StateAttribute::ON);
        }

        // 1.1、地形根节点
        {
            const ID idPlanet(0u, TERRAIN_TILE, 0u, 0u, 0, 0ui64);
            m_pTerrainRootNode = osgDB::readNodeFile(idPlanet);
            m_pTerrainRootNode->setID(idPlanet);
            m_pTerrainRootNode->setName("TerrainRootNode");

            osg::StateSet *pStateSet = m_pTerrainRootNode->getOrCreateStateSet();
            pStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

            m_pElementRootGroup->addChild(m_pTerrainRootNode.get());
            EarthLightModel::bindEarthLightModel(pStateSet);
        }

        // 1.2、虚拟瓦片根节点
        {
            const ID &idVirtualTile = ID::getVirtualTileRootID();
            m_pCultureRootNode = osgDB::readNodeFile(idVirtualTile);
            m_pCultureRootNode->setName("CultureRootNode");

            //- shadow
            {
                //创建一个阴影节点，并标识接收对象和阴影对象
                m_pShadowedScene = new osgShadow::ShadowedScene();
                m_pShadowedScene->setReceivesShadowTraversalMask(0x01);
                m_pShadowedScene->setCastsShadowTraversalMask(0x02);
                //关联阴影纹理
                osg::ref_ptr<osgShadow::SoftShadowMap> softs = new osgShadow::SoftShadowMap;
                m_pShadowedScene->setShadowTechnique(softs);
                
                m_pCultureRootNode->setNodeMask(0x02);
                m_pShadowedScene->addChild(m_pCultureRootNode.get());

                m_pCultureRootSwitch->addChild(m_pShadowedScene, false);
            }
            
            //- no shadow
            {
                m_pCultureRootSwitch->addChild(m_pCultureRootNode, true);
            }
        }

        // 1.3、地下底板根节点
        {
            m_pSubGround = new SubGround;
            m_pSubGround->initialize(m_pTerrainRootNode.get());
            m_pElementRootGroup->addChild(m_pSubGround.get());
        }
    }

    // 2、附加元素根节点
    {
        m_pExtraElementRootGroup = new osg::Group;
        m_pExtraElementRootGroup->setName("ExtraElementRootGroup");
        m_pRootGroup->addChild(m_pExtraElementRootGroup);

        osg::ref_ptr<osg::Geode>    pMultiLineGeode = new osg::Geode;
        pMultiLineGeode->addDrawable(osg::MultiLineDrawable::instance());
        m_pExtraElementRootGroup->addChild(pMultiLineGeode.get());
    }

    // 3、临时数据根节点
    {
        m_pTempElementRootGroup = new osg::Group;
        m_pTempElementRootGroup->setName("TempElementRootGroup");
        m_pRootGroup->addChild(m_pTempElementRootGroup.get());
    }
	
    //- 4、特效节点
	{
        m_pEffectGroupBox = new osg::Group();
        m_pRootGroup->addChild(m_pEffectGroupBox.get());
        
        m_pEffectGroupSwitch = new osg::Switch();
        m_pRootGroup->addChild(m_pEffectGroupSwitch.get());

        m_pEffectGlobalSwitch = new osg::Switch();
        m_pRootGroup->addChild(m_pEffectGlobalSwitch.get());
    }

    m_pSceneGraphOperator->initialize(m_pRootGroup.get());
    m_pRootGroup->addUpdateCallback(m_pSceneGraphOperator.get());
}


void DEUPlatformCore::unInitialize(void)
{
    if(!m_bInitialized) return;

    if(m_pVCubeManager.valid())
    {
        m_pVCubeManager->stopChangingListening();
        m_pVCubeManager = NULL;
    }

    if(m_pVCubeChangingListener.valid())
    {
        m_pVCubeChangingListener->finishMission();
        m_pVCubeChangingListener = NULL;
    }

    if(m_pMemCheckerThread.valid())
    {
        m_pMemCheckerThread->finishMission();
        m_pMemCheckerThread = NULL;
    }

    if(m_pSceneViewer.valid())
    {
        m_pSceneViewer->unInitialize();
        m_pSceneViewer = NULL;
    }

    if(m_pFileReadInterceptor.valid())
    {
        m_pFileReadInterceptor->logout();
        osgDB::Registry::instance()->removeReadFileCallback(m_pFileReadInterceptor.get());
        m_pFileReadInterceptor = NULL;
    }

    m_pUtility = NULL;

    m_pEventAdapter->unregisterReceiver(m_pPlatformReceiver.get());

    m_pStateManager = NULL;
    if (m_pEffectGroupBox.valid())
    {
        m_pEffectGroupBox->removeChildren(0, m_pEffectGroupBox->getNumChildren());
        m_pEffectGroupBox = NULL;
    }

    if (m_pEffectGlobalSwitch.valid())
    {
        m_pEffectGlobalSwitch->removeChildren(0, m_pEffectGlobalSwitch->getNumChildren());
        m_pEffectGlobalSwitch = NULL;
    }

    m_bInitialized = false;
}


const IUtility *DEUPlatformCore::getUtility(void) const
{
    if(!m_bInitialized)     return NULL;

    return m_pUtility.get();
}


IUtility *DEUPlatformCore::getUtility(void)
{
    if(!m_bInitialized)     return NULL;

    return m_pUtility.get();
}


const IStateManager *DEUPlatformCore::getStateManager(void) const
{
    if(!m_bInitialized)     return NULL;

    return m_pStateManager.get();
}


IStateManager *DEUPlatformCore::getStateManager(void)
{
    if(!m_bInitialized)     return NULL;

    return m_pStateManager.get();
}


const ITerrainModificationManager *DEUPlatformCore::getTerrainModificationManager(void) const
{
    if(!m_bInitialized)     return NULL;

    return m_pTerrainModificationManager.get();
}


ITerrainModificationManager *DEUPlatformCore::getTerrainModificationManager(void)
{
    if(!m_bInitialized)     return NULL;

    return m_pTerrainModificationManager.get();
}


const ISceneViewer *DEUPlatformCore::getSceneViewer(void) const
{
    if(!m_bInitialized)     return NULL;

    return m_pSceneViewer.get();
}


ISceneViewer *DEUPlatformCore::getSceneViewer(void)
{
    if(!m_bInitialized)     return NULL;

    return m_pSceneViewer.get();
}


void DEUPlatformCore::refreshObjectState(const IDList &objects, const std::string &strStateName, bool bState)
{
    if(!m_bInitialized)     return;
    assert(m_pSceneGraphOperator.valid());

    OpenSP::sp<StateChanged_Operation>  pStateChanged = new StateChanged_Operation;
    pStateChanged->setState(m_pStateManager.get(), strStateName, bState);
    pStateChanged->setChangedIDs(objects);
    m_pSceneGraphOperator->pushOperation(pStateChanged.get());
}


void DEUPlatformCore::updateTerrainModification(TerrainModification *pModification, bool bApply)
{
    TerrainModification_Operation *pOperation = new TerrainModification_Operation(m_pTerrainRootNode.get(), m_pTerrainModificationManager.get(), pModification, bApply);
    m_pSceneGraphOperator->pushOperation(pOperation);
}

osg::ref_ptr<osgParticle::PrecipitationEffect> DEUPlatformCore::createGlobalRain()
{
    if (m_pPrecipitationRain.valid())
    {
        return m_pPrecipitationRain.get();
    }

    m_pPrecipitationRain = new osgParticle::PrecipitationEffect();
    m_pPrecipitationRain->rain(0.5f);
    m_dDensity = 0.5;
    setParam(m_pPrecipitationRain, EF_TYPE_RAIN, 30.0, 30.0, -10.0, 2.2/100.0);

    return m_pPrecipitationRain.get();
}

osg::ref_ptr<osgParticle::PrecipitationEffect> DEUPlatformCore::createGlobalSnow()
{
    if (m_pPrecipitationSnow.valid())
    {
        return m_pPrecipitationSnow.get();
    }

    m_pPrecipitationSnow = new osgParticle::PrecipitationEffect();
    m_pPrecipitationSnow->snow(0.5f);
    setParam(m_pPrecipitationSnow, EF_TYPE_SNOW, 15.0, 15.0, -2.0, 4/100.0);

    return m_pPrecipitationSnow.get();
}

void DEUPlatformCore::setDEMOrder(const IDList &vecOrder)
{
    if(!m_bInitialized)     return;
    assert(m_pFileReadInterceptor.valid());
    assert(m_pTerrainRootNode.valid());
    assert(m_pSceneGraphOperator.valid());

    IDList   vecOldOrder;
    m_pFileReadInterceptor->getTerrainLayersOrder(true, vecOldOrder);
    if(vecOldOrder == vecOrder)
    {
        return;
    }
    m_pFileReadInterceptor->setTerrainLayersOrder(true, vecOrder);

    osg::ref_ptr<RemoveTileByLevel_Operation> pRemoveTileOperation = new RemoveTileByLevel_Operation(3);
    m_pSceneGraphOperator->pushOperation(pRemoveTileOperation);
}


void DEUPlatformCore::getDEMOrder(IDList &vecOrder) const
{
    if(!m_bInitialized)     return;
    assert(m_pFileReadInterceptor.valid());
    m_pFileReadInterceptor->getTerrainLayersOrder(true, vecOrder);
}


void DEUPlatformCore::setDOMOrder(const IDList &vecOrder)
{
    if(!m_bInitialized)     return;
    assert(m_pFileReadInterceptor.valid());

    IDList   vecOldOrder;
    m_pFileReadInterceptor->getTerrainLayersOrder(false, vecOldOrder);
    if(vecOldOrder == vecOrder)
    {
        return;
    }
    m_pFileReadInterceptor->setTerrainLayersOrder(false, vecOrder);

    refreshTerrainWithMinimalChange();
}


void DEUPlatformCore::getDOMOrder(IDList &vecOrder) const
{
    if(!m_bInitialized)     return;
    assert(m_pFileReadInterceptor.valid());

    m_pFileReadInterceptor->getTerrainLayersOrder(false, vecOrder);
}


void DEUPlatformCore::refreshTerrainWithMinimalChange(void)
{
    if(!m_pRefreshingTerrainThread.valid())
    {
        m_pRefreshingTerrainThread = new RefreshingTerrainThread;
        m_pRefreshingTerrainThread->initialize(m_pSceneViewer.get(), m_pTerrainRootNode.get(), m_pFileReadInterceptor.get(), m_pSceneGraphOperator.get());
    }
    m_pRefreshingTerrainThread->doRefresh();
}


bool DEUPlatformCore::addLocalDatabase(const std::string &strDB)
{
    if(!m_bInitialized)     return false;
    assert(m_pFileReadInterceptor.valid());

    return m_pFileReadInterceptor->addLocalDatabase(strDB);
}


bool DEUPlatformCore::removeLocalDatabase(const std::string &strDB)
{
    if(!m_bInitialized)     return false;

    assert(m_pFileReadInterceptor.valid());
    return m_pFileReadInterceptor->removeLocalDatabase(strDB);
}


ID DEUPlatformCore::selectByScreenPoint(const cmm::math::Point2i &point)
{
    if(!m_bInitialized)     return ID();

    osg::ref_ptr<Selecting_Operation>   pSelOperation = new Selecting_Operation;
    pSelOperation->selectByScreenPoint(m_pSceneViewer->getView(0), osg::Vec2s(point.x(), point.y()));
    m_pSceneGraphOperator->pushOperation(pSelOperation.get());

    IDList result;
	osg::Vec3d minIntersectPoint;
    pSelOperation->waitForFinishing(result, minIntersectPoint);

    if(result.empty())
    {
        return ID();
    }

    return result.front();
}


bool DEUPlatformCore::selectByScreenPoint(const cmm::math::Point2i &point, cmm::math::Point3d &inter_point)
{
    /*osg::ref_ptr<Intersect_Operation>   pIntOperation = new Intersect_Operation;
    pIntOperation->selectByScreenPoint(m_pSceneViewer->getView(0), osg::Vec2s(point.x(), point.y()));
    m_pSceneGraphOperator->pushOperation(pIntOperation.get());

    std::vector<osg::Vec3d> vecReslut;
    pIntOperation->waitForFinishing(vecReslut);

    if(vecReslut.empty())
    {
        return false;
    };


    inter_point = vecReslut.front();*/

	osg::ref_ptr<Selecting_Operation>   pSelOperation = new Selecting_Operation;
	pSelOperation->selectByScreenRect(m_pSceneViewer->getView(0), osg::Vec2s(point.x(), point.y()), osg::Vec2s(point.x()+5, point.y()+5));
	m_pSceneGraphOperator->pushOperation(pSelOperation.get());

	IDList result;
	osg::Vec3d minIntersectPoint;
	pSelOperation->waitForFinishing(result, minIntersectPoint);

	if (minIntersectPoint == osg::Vec3d(0.0, 0.0, 0.0))
	{
		return false;
	}

	osg::Vec3d minPoint = osgUtil::convertWorld2Globe(minIntersectPoint);
	inter_point.x() = minPoint.x();
	inter_point.y() = minPoint.y();
	inter_point.z() = minPoint.z();

    return true;
}

bool DEUPlatformCore::selectByScreenPoint(const cmm::math::Point2i &point, osg::Vec3d &inter_point)
{
	osg::ref_ptr<Intersect_Operation>   pIntOperation = new Intersect_Operation;
	pIntOperation->selectByScreenPoint(m_pSceneViewer->getView(0), osg::Vec2s(point.x(), point.y()));
	m_pSceneGraphOperator->pushOperation(pIntOperation.get());

	std::vector<osg::Vec3d> vecReslut;
	pIntOperation->waitForFinishing(vecReslut);

	if(vecReslut.empty())
	{
		return false;
	}

	inter_point = vecReslut.front();
	return true;
}


IDList DEUPlatformCore::selectByScreenRect(const cmm::math::Point2i &ptLeftTop, const cmm::math::Point2i &ptRightBottom)
{
    IDList result;
	osg::Vec3d minIntersectPoint;

    if(!m_bInitialized)     return result;

    osg::ref_ptr<Selecting_Operation>   pSelOperation = new Selecting_Operation;
    pSelOperation->selectByScreenRect(m_pSceneViewer->getView(0), osg::Vec2s(ptLeftTop.x(), ptLeftTop.y()), osg::Vec2s(ptRightBottom.x(), ptRightBottom.y()));
    m_pSceneGraphOperator->pushOperation(pSelOperation.get());

    pSelOperation->waitForFinishing(result, minIntersectPoint);

    return result;
}


double DEUPlatformCore::fetchElevationInView(unsigned nIndex, const cmm::math::Point2d &position) const
{
    if(nIndex >= m_pSceneViewer->getNumViews())
    {
        return 0.0;
    }

    osg::ref_ptr<FetchingElevation_Operation>   pFetcher = new FetchingElevation_Operation;
    osgViewer::View *pView = const_cast<osgViewer::View *>(m_pSceneViewer->getView(nIndex));
    pFetcher->fetchElevation(pView, position);

    m_pSceneGraphOperator->pushOperation(pFetcher.get());

    const double dblElevation = pFetcher->waitForFinishing();
    return dblElevation;
}


void DEUPlatformCore::setMemoryLimited(unsigned __int64 nMemLimited)
{
    if(!m_bInitialized)     return;

    if(m_pMemCheckerThread.valid())
    {
        m_pMemCheckerThread->setMemoryLimited(nMemLimited);
    }
}


unsigned __int64 DEUPlatformCore::getMemoryLimited(void) const
{
    if(!m_bInitialized)     return 0ui64;

    if(m_pMemCheckerThread.valid())
    {
        return m_pMemCheckerThread->getMemoryLimited();
    }
    return 0ui64;
}


void DEUPlatformCore::setTerrainOpacity(double dblOpacity)
{
    if(!m_bInitialized)     return;
    assert(m_pSubGround.valid());

    m_pSubGround->setTerrainOpacity(dblOpacity);
}


double DEUPlatformCore::getTerrainOpacity(void) const
{
    if(!m_bInitialized)     return 1.0;
    assert(m_pSubGround.valid());

    return m_pSubGround->getTerrainOpacity();
}


void DEUPlatformCore::setSubGroundColor(const cmm::FloatColor &color)
{
    if(!m_bInitialized)     return;
    assert(m_pSubGround.valid());

    m_pSubGround->setGroundColor(color);
}


const cmm::FloatColor &DEUPlatformCore::getSubGroundColor(void) const
{
    if(!m_bInitialized)
    {
        const static cmm::FloatColor color;
        return color;
    }
    assert(m_pSubGround.valid());

    return m_pSubGround->getGroundColor();
}


void DEUPlatformCore::setSubGroundDepth(double dblDepth)
{
    if(!m_bInitialized)     return;
    assert(m_pSubGround.valid());

    m_pSubGround->setSubGroundDepth(dblDepth);
}


double DEUPlatformCore::getSubGroundDepth(void) const
{
    if(!m_bInitialized)
    {
        return 0.0;
    }
    assert(m_pSubGround.valid());

    return m_pSubGround->getSubGroundDepth();
}


void DEUPlatformCore::setVisibleRangeRatio(double dblRatio)
{
    //osg::LOD::setRangeRatio(dblRatio);
}


double DEUPlatformCore::getVisibleRangeRatio(void) const
{
    return 0.0;//osg::LOD::getRangeRatio();
}


void DEUPlatformCore::addTempModel(const ID &id)
{
    if(!m_bInitialized)     return;
    assert(m_pSceneGraphOperator.valid());

    OpenSP::sp<AddTempModelByID_Operation> pAddOpration = new AddTempModelByID_Operation(id);
    m_pSceneGraphOperator->pushOperation(pAddOpration);
}


void DEUPlatformCore::removeTempModel(const ID &id)
{
    if(!m_bInitialized)     return;
    assert(m_pSceneGraphOperator.valid());

    OpenSP::sp<RemoveTempModelByID_Operation> pRemoveOpration = new RemoveTempModelByID_Operation(id);
    m_pSceneGraphOperator->pushOperation(pRemoveOpration);
}

void DEUPlatformCore::PlatformReceiver::onReceive(ea::IEventAdapter *pEventAdapter, ea::IEventObject *pEventObject)
{
    if(pEventAdapter == NULL || pEventObject == NULL || !m_pPlatformCore.valid())
    {
        return;
    }

    const std::string &strAction = pEventObject->getAction();

    if(strAction.compare("RefreshTerrainTile") == 0)
    {
        cmm::variant_data var_apply;
        pEventObject->getExtra("Apply", var_apply);

        cmm::variant_data var_obj;
        pEventObject->getExtra("Object", var_obj);
        TerrainModification *pTerrainModification = dynamic_cast<TerrainModification *>((OpenSP::Ref *)var_obj);
        if(!pTerrainModification)
        {
            return;
        }

        m_pPlatformCore->updateTerrainModification(pTerrainModification, (bool)var_apply);
    }

    return;
}


bool DEUPlatformCore::addWMTSTileSet(deues::ITileSet *pTileSet)
{
    return m_pFileReadInterceptor->addWMTSTileSet(pTileSet);
}


bool DEUPlatformCore::removeWMTSTileSet(deues::ITileSet *pTileSet)
{
    return m_pFileReadInterceptor->removeWMTSTileSet(pTileSet);
}

void DEUPlatformCore::useShadow(bool bOpenFlag)
{
    if (bOpenFlag)
    {
        m_bUseShadow = true;
        Registry::instance()->setUseShadow(m_bUseShadow);

        m_pCultureRootSwitch->setChildValue(m_pCultureRootNode, false);
        m_pCultureRootSwitch->setChildValue(m_pShadowedScene, true);
    }
    else
    {
        m_bUseShadow = false;
        Registry::instance()->setUseShadow(m_bUseShadow);
        
        m_pCultureRootSwitch->setChildValue(m_pCultureRootNode, true);
        m_pCultureRootSwitch->setChildValue(m_pShadowedScene, false);
    }

    return;
}


bool DEUPlatformCore::setParam(osg::ref_ptr<osgParticle::PrecipitationEffect> pPrecipitation, EffectType nEffectType, double dNearTransition, double dFarTransition, double dParticleSpeed, double dParticleSize)
{
    if (NULL == pPrecipitation)
    {
        return false;
    }

    pPrecipitation->setNearTransition(dNearTransition);
    pPrecipitation->setFarTransition(dFarTransition);
    if (EF_TYPE_RAIN == nEffectType)
    {
        pPrecipitation->setParticleSpeed(dParticleSpeed);
        pPrecipitation->setParticleColor(osg::Vec4(0.7,0.7,0.7,1.0));
    }
    else
    {
        pPrecipitation->setParticleColor(osg::Vec4(1.0,1.0,1.0,1.0));
    }
    pPrecipitation->setParticleSize(dParticleSize);

    return true;
}

bool DEUPlatformCore::createGlobalEffectNode(EffectType nEffectType)
{
    if (NULL == m_pEffectGlobalSwitch)
    {
        return false;
    }

    if (nEffectType == EF_TYPE_RAIN)
    {
        m_pEffectGlobalSwitch->addChild(createGlobalRain(), false);
    }
    else if (nEffectType == EF_TYPE_SNOW)
    {
        m_pEffectGlobalSwitch->addChild(createGlobalSnow(), false);
    }

    return true;
}

bool DEUPlatformCore::showGlobalEffectNode(EffectType nEffectType, bool bIsShow)
{
    if (NULL == m_pEffectGlobalSwitch)
    {
        return false;
    }

    switch (nEffectType)
    {
    case EF_TYPE_RAIN:
        {
            if (bIsShow)
                m_pEffectGlobalSwitch->setChildValue(createGlobalRain(), true);
            else
                m_pEffectGlobalSwitch->setChildValue(createGlobalRain(), false);
            break;
        }
    case EF_TYPE_SNOW:
        {
            if (bIsShow)
                m_pEffectGlobalSwitch->setChildValue(createGlobalSnow(), true);
            else
                m_pEffectGlobalSwitch->setChildValue(createGlobalSnow(), false);
            break;
        }
    default:
        break;
    }
    
    return true;
}

bool DEUPlatformCore::setGlobalEffectLevel(EffectType nEffectType, EffectLevel nEffectLevel)
{
    if (nEffectType == EF_TYPE_RAIN)
    {
        switch (nEffectLevel)
        {
        case EF_LEVEL_LOW:
            setParam(createGlobalRain(), EF_TYPE_RAIN, 30.0, 30.0, -8.0, 1.6/100.0);
            break;
        case EF_LEVEL_MIDDLE:
            setParam(createGlobalRain(), EF_TYPE_RAIN, 30.0, 30.0, -10.0, 2.2/100.0);
            break;
        case EF_LEVEL_HIGH:
            setParam(createGlobalRain(), EF_TYPE_RAIN, 30.0, 30.0, -14.0, 3.1/100.0);
            break;
        default:
            break;
        }        
    }
    else if (nEffectType == EF_TYPE_SNOW)
    {
        switch (nEffectLevel)
        {
        case EF_LEVEL_LOW:
            setParam(createGlobalSnow(), EF_TYPE_SNOW, 15.0, 15.0, -1.0, 2/100.0);
            break;
        case EF_LEVEL_MIDDLE:
            setParam(createGlobalSnow(), EF_TYPE_SNOW, 15.0, 15.0, -2.0, 4/100.0);
            break;
        case EF_LEVEL_HIGH:
            setParam(createGlobalSnow(), EF_TYPE_SNOW, 15.0, 15.0, -3.0, 7/100.0);
            break;
        default:
            break;
        }
    }

    return true;
}

bool DEUPlatformCore::createEffectNode(const std::string& strEffectName, EffectType effectType, EffectLevel effectLevel, 
                                       double dLat, double dLong, double dHeight, double dRadius)
{
    if (strEffectName == "")
    {
        return false;
    }

//    dHeight = dRadius;

    osg::Vec3d ptCenter;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    pEllipsoidModel->convertLatLongHeightToXYZ(dLat, dLong, dHeight, ptCenter.x(), ptCenter.y(), ptCenter.z());

    //--------------------------------------------------------------------------------------------------------
//     osg::ref_ptr<osg::Box> pBox = new osg::Box(ptCenter, dRadius*2.0f);
//     const osg::Quat qtRotation = osgUtil::calcRotationByCoordAndAngle(osg::Vec2d(dLong, dLat), 0.0, 0.0);
//     pBox->setRotation(qtRotation);
// 
//     osg::ref_ptr<osg::ShapeDrawable> doorShape = new osg::ShapeDrawable(pBox);
//     doorShape->setColor( osg::Vec4(1.0f, 0.0f, 0.0f, 0.4f) );
//     osg::StateSet* stateset = doorShape->getOrCreateStateSet();  
//     osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc();   
//     blendFunc->setSource(osg::BlendFunc::SRC_ALPHA);         
//     blendFunc->setDestination(osg::BlendFunc::ONE_MINUS_SRC_ALPHA);          
//     stateset->setAttributeAndModes( blendFunc );  
//     stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
//     osg::ref_ptr<osg::CullFace> pCullFace = new osg::CullFace(osg::CullFace::BACK);
//     stateset->setAttributeAndModes(pCullFace.get(), osg::StateAttribute::ON);
// 
//     osg::ref_ptr<osg::Geode> geode = new osg::Geode();
//     geode->addDrawable(doorShape);
//     m_pTempElementRootGroup->addChild(geode);
    //--------------------------------------------------------------------------------------------------------

    osg::ref_ptr<EffectPagedLOD> pEftPagedLOD = new EffectPagedLOD(m_pEffectGroupSwitch);
    pEftPagedLOD->setEffectName(strEffectName);
    pEftPagedLOD->setEffectType(effectType);
    pEftPagedLOD->setEffectLevel(effectLevel);
    pEftPagedLOD->setCenter(ptCenter);
    pEftPagedLOD->setRadius(dRadius);
    pEftPagedLOD->setRange(0u, 0.0, dRadius * 7);

    m_pEffectGroupBox->addChild(pEftPagedLOD.get());
    
    return true;
}

bool DEUPlatformCore::removeEffectNode(const std::string& strEffectName, EffectType nEffectType)
{
    osg::ref_ptr<EffectPagedLOD> pEftPagedLOD = getEffectPagedLOD(strEffectName, nEffectType);
    if (!pEftPagedLOD.valid())
    {
        return false;
    }

    unsigned int nPos = m_pEffectGroupBox->getChildIndex(pEftPagedLOD);
    m_pEffectGroupBox->removeChildren(nPos, 1);

    return true;
}

bool DEUPlatformCore::setEffectLevel(const std::string& strEffectName, EffectType nEffectType, EffectLevel nEffectLevel)
{
    osg::ref_ptr<EffectPagedLOD> pEftPagedLOD = getEffectPagedLOD(strEffectName, nEffectType);
    if (!pEftPagedLOD.valid())
    {
        return false;
    }

    pEftPagedLOD->setEffectLevel(nEffectLevel);

    return true;
}

osg::ref_ptr<EffectPagedLOD> DEUPlatformCore::getEffectPagedLOD(const std::string& strEffectName, EffectType effectType)
{
    unsigned int nCnt = m_pEffectGroupBox->getNumChildren();

    for (unsigned int i=0; i<m_pEffectGroupBox->getNumChildren(); i++)
    {
        osg::ref_ptr<EffectPagedLOD> pEffectPagedLOD = dynamic_cast<EffectPagedLOD*>(m_pEffectGroupBox->getChild(i));

        if (pEffectPagedLOD->getEffectName()==strEffectName && pEffectPagedLOD->getEffectType()==effectType)
        {
            return pEffectPagedLOD;
        }
    }

    return NULL;
}

