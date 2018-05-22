#ifndef DEU_PLATFORM_CORE_H_5AC048D5_4835_4C2A_953A_1A2EB7AC20AD_INCLUDE
#define DEU_PLATFORM_CORE_H_5AC048D5_4835_4C2A_953A_1A2EB7AC20AD_INCLUDE
#include "Export.h"
#include "IPlatformCore.h"
#include <osg/MatrixTransform>
#include <EventAdapter/EventAdapter.h>
#include <osgShadow/ShadowedScene>
#include <osgParticle/PrecipitationEffect>

#include "VirtualCubeNode.h"
#include "FileReadInterceptor.h"

#include "Utility.h"
#include "DEUSceneViewer.h"
#include "StateManager.h"
#include "RefreshingTerrainThread.h"
#include "SubGround.h"
#include "SceneGraphOperator.h"
#include "VCubeChangingListener.h"
#include "EffectPagedLOD.h"


class DEUPlatformCore : public IPlatformCore
{
public:
    double m_dDensity;
    virtual void    setDensity(double dVal){m_dDensity=dVal; m_pPrecipitationRain->rain(dVal);}
    virtual void    setNearTransition(double dVal){m_pPrecipitationRain->setNearTransition(dVal);}
    virtual void    setFarTransition(double dVal){m_pPrecipitationRain->setFarTransition(dVal);}
    virtual void    setParticleSpeed(double dVal){m_pPrecipitationRain->setParticleSpeed(dVal);}
    virtual void    setParticleSize(double dVal){m_pPrecipitationRain->setParticleSize(dVal);}
    virtual void    setParticleColor(double dR, double dG, double dB, double dA){m_pPrecipitationRain->setParticleColor(osg::Vec4(dR,dG,dB,dA));}
    virtual void    setUseFarLineSegments(bool bVal){m_pPrecipitationRain->setUseFarLineSegments(bVal);}

    virtual void    getRainParams(double& dDensity, double& dNear, double& dFar, double& dSpeed, double& dSize, double& dColor)
    {
        if (m_pPrecipitationRain == NULL)
        {
            return;
        }

        dDensity    = m_dDensity;
        dNear       = m_pPrecipitationRain->getNearTransition();
        dFar        = m_pPrecipitationRain->getFarTransition();
        dSpeed      = m_pPrecipitationRain->getParticleSpeed();
        dSize       = m_pPrecipitationRain->getParticleSize();

        osg::Vec4 v4 = m_pPrecipitationRain->getParticleColor(); 
        dColor       = v4.x();
    }

public:
    DEUPlatformCore(ea::IEventAdapter *pEventAdapter = NULL);
    virtual ~DEUPlatformCore(void);

public:     // Methods from IPlatformCore
    virtual bool                        initialize(const InitializationParam *pInitParam, const std::string &strHost, const std::string &strPort, const std::string &strPSHost,
                                                   const std::string &strPSPort, const std::string &strUserName, const std::string &strPassword, const std::string &strLocalCache, cmm::IStateQuerier *pStateQuerier);
    virtual void                        unInitialize(void);
    virtual const ISceneViewer         *getSceneViewer(void) const;
    virtual ISceneViewer               *getSceneViewer(void);

    virtual const IUtility             *getUtility(void) const;
    virtual IUtility                   *getUtility(void);

    virtual const IStateManager        *getStateManager(void) const;
    virtual IStateManager              *getStateManager(void);

    virtual const ITerrainModificationManager *getTerrainModificationManager(void) const;
    virtual ITerrainModificationManager *getTerrainModificationManager(void);

    virtual void                        setDEMOrder(const IDList &vecOrder);
    virtual void                        getDEMOrder(IDList &vecOrder) const;
    virtual void                        setDOMOrder(const IDList &vecOrder);
    virtual void                        getDOMOrder(IDList &vecOrder) const;

    virtual bool                        addLocalDatabase(const std::string &strDB);
    virtual bool                        removeLocalDatabase(const std::string &strDB);

    virtual ID                          selectByScreenPoint(const cmm::math::Point2i &point);
    virtual IDList                      selectByScreenRect(const cmm::math::Point2i &ptLeftTop, const cmm::math::Point2i &ptRightBottom);
	virtual bool                        selectByScreenPoint(const cmm::math::Point2i &point, cmm::math::Point3d &inter_point);

    virtual void                        setMemoryLimited(unsigned __int64 nMemLimited);
    virtual unsigned __int64            getMemoryLimited(void) const;

    virtual void                        setTerrainOpacity(double dblOpacity);
    virtual double                      getTerrainOpacity(void) const;

    virtual void                        setSubGroundColor(const cmm::FloatColor &color);
    virtual const cmm::FloatColor      &getSubGroundColor(void) const;

    virtual void                        setSubGroundDepth(double dblDepth);
    virtual double                      getSubGroundDepth(void) const;

    //IStateImplementer
    virtual void                        refreshObjectState(const IDList &objects, const std::string &strStateName, bool bState);

    virtual void                        setVisibleRangeRatio(double dblRatio);
    virtual double                      getVisibleRangeRatio(void) const;

    virtual void                        addTempModel(const ID &id);
    virtual void                        removeTempModel(const ID &id);
    virtual double                      fetchElevationInView(unsigned nIndex, const cmm::math::Point2d &position) const;
    virtual bool                        addWMTSTileSet(deues::ITileSet *pTileSet);
    virtual bool                        removeWMTSTileSet(deues::ITileSet *pTileSet);

    virtual bool                        createGlobalEffectNode(EffectType nEffectType);
    virtual bool                        showGlobalEffectNode(EffectType nEffectType, bool bIsShow);
    virtual bool                        setGlobalEffectLevel(EffectType nEffectType, EffectLevel nEffectLevel);

    virtual bool                        createEffectNode(const std::string& strEffectName, EffectType effectType, EffectLevel effectLevel, 
                                                         double dLat, double dLong, double dHeight, double dRadius);

    virtual bool                        removeEffectNode(const std::string& strEffectName, EffectType nEffectType);
    virtual bool                        setEffectLevel(const std::string& strEffectName, EffectType nEffectType, EffectLevel nEffectLevel);

    virtual void                        useShadow(bool bOpenFlag);

public:
    bool                                selectByScreenPoint(const cmm::math::Point2i &point, osg::Vec3d &inter_point);
protected:
    void    initSceneGraph(void);
    
    void    refreshTerrainWithMinimalChange(void);
    void    updateTerrainModification(TerrainModification *pModification, bool bApply);


    osg::ref_ptr<osgParticle::PrecipitationEffect>  createGlobalRain();
    osg::ref_ptr<osgParticle::PrecipitationEffect>  createGlobalSnow();
    bool                                            setParam(osg::ref_ptr<osgParticle::PrecipitationEffect> pPrecipitation, EffectType nEffectType, double dNearTransition, double dFarTransition, double dParticleSpeed, double dParticleSize);
    osg::ref_ptr<EffectPagedLOD>                    getEffectPagedLOD(const std::string& strEffectName, EffectType effectType);

protected:
    //文件拦截器
    osg::ref_ptr<FileReadInterceptor>       m_pFileReadInterceptor;

    osg::ref_ptr<vcm::IVirtualCubeManager>  m_pVCubeManager;

    //刷新地形/影像线程
    OpenSP::sp<RefreshingTerrainThread>     m_pRefreshingTerrainThread;

    // 场景图操纵器
    OpenSP::sp<SceneGraphOperator>          m_pSceneGraphOperator;

    //视图
    osg::ref_ptr<DEUSceneViewer>            m_pSceneViewer;

    // 总根节点
    osg::ref_ptr<osg::Group>                m_pRootGroup;

    // 地形和人文共同的根节点
    osg::ref_ptr<osg::Group>                m_pElementRootGroup;
    osg::ref_ptr<osg::Switch>               m_pCultureRootSwitch;

    // 特效相关
    osg::ref_ptr<osg::Switch>               m_pEffectGroupSwitch;
    osg::ref_ptr<osg::Group>                m_pEffectGroupBox;
    osg::ref_ptr<osg::Switch>               m_pEffectGlobalSwitch;
    osg::ref_ptr<osgParticle::PrecipitationEffect> m_pPrecipitationRain;
    osg::ref_ptr<osgParticle::PrecipitationEffect> m_pPrecipitationSnow;
    // 地形根节点
    osg::ref_ptr<osg::Node>                 m_pTerrainRootNode;

    // 人文根节点
    osg::ref_ptr<osg::Node>                 m_pCultureRootNode;

    // 附加元素根节点
    osg::ref_ptr<osg::Group>                m_pExtraElementRootGroup;

    // 临时数据根节点
    osg::ref_ptr<osg::Group>                m_pTempElementRootGroup;

    //阴影节点
    osg::ref_ptr<osgShadow::ShadowedScene>  m_pShadowedScene;

    // 地下模式的地表
    osg::ref_ptr<SubGround>                 m_pSubGround;

    //事件适配器
    osg::ref_ptr<ea::IEventAdapter>         m_pEventAdapter;

    bool                                    m_bInitialized;

    bool                                    m_bUseShadow;

    OpenSP::sp<Utility>                     m_pUtility;

    OpenSP::sp<StateManager>                m_pStateManager;

    OpenSP::sp<TerrainModificationManager>  m_pTerrainModificationManager;

    osg::Vec4                               m_clrGroud;

protected:
    class MemeryCheckerThread : public OpenThreads::Thread, public OpenSP::Ref
    {
    public:
        MemeryCheckerThread(osgDB::DatabasePager *pDatabasePager)
        {
            m_nMemoryLimited = 900ui64 * 1024ui64 * 1024ui64;
            setStackSize(16u * 1024u);
            m_MissionFinished.exchange(0u);
            m_blockWait.reset();
            m_pDatabasePager = pDatabasePager;
        }

        void finishMission(void)
        {
            m_MissionFinished.exchange(1);
            m_blockWait.release();
            join();
        }

        void setMemoryLimited(unsigned __int64 nMemLimited)
        {
            m_nMemoryLimited = nMemLimited;
        }

        unsigned __int64 getMemoryLimited(void) const
        {
            return m_nMemoryLimited;
        }

        double clampValue(double dbl)
        {
            dbl  = osg::clampBetween(dbl, 0.0, 1.0);
            dbl *= osg::PI_2;
            return cos(dbl);
        }

    protected:
        virtual void run(void);
        OpenThreads::Atomic     m_MissionFinished;
        unsigned __int64        m_nMemoryLimited;
        OpenThreads::Block      m_blockWait;
        osg::ref_ptr<osgDB::DatabasePager>      m_pDatabasePager;
    };
    OpenSP::sp<MemeryCheckerThread>     m_pMemCheckerThread;

    OpenSP::sp<VCubeChangingListener>   m_pVCubeChangingListener;


protected:
    class PlatformReceiver : public ea::IEventReceiver
    {
    public:
        PlatformReceiver(DEUPlatformCore *pPlatformCore) : m_pPlatformCore(pPlatformCore) {}
        ~PlatformReceiver(){}

    protected:
        virtual void onReceive(ea::IEventAdapter *pEventAdapter, ea::IEventObject *pEventObject);
    protected:
        OpenSP::sp<DEUPlatformCore> m_pPlatformCore;
    };

    OpenSP::sp<PlatformReceiver> m_pPlatformReceiver;
};

#endif

