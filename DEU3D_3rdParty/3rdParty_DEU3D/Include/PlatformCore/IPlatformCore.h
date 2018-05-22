#ifndef I_PLATFORM_CORE_H_B2FF0C32_45C9_4782_BA48_FF008EBDFBC5_INCLUDE
#define I_PLATFORM_CORE_H_B2FF0C32_45C9_4782_BA48_FF008EBDFBC5_INCLUDE

#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>
#include <Common/Common.h>
#include <Common/deuMath.h>
#include <Common/IStateImplementer.h>

#include "Export.h"

class ISceneViewer;
class IEventAdapter;
class InitializationParam;
class IUtility;
class IStateManager;
class ITerrainModificationManager;

namespace deudbProxy
{
    class IDEUDBProxy;
}

namespace ea
{
    class IEventAdapter;
}

namespace cmm
{
    class IStateQuerier;
}

namespace deues
{
    class ITileSet;
};

typedef enum tagRefreshType
{
    RT_Refresh_Terrain = 0x1,
    RT_Refresh_Culture = 0x2,
    RT_Refresh_All = 0x3
}RefreshType;


typedef enum tagEffectType
{
    EF_TYPE_RAIN = 0x1,
    EF_TYPE_SNOW = 0x2
}EffectType;

typedef enum tagEffectLevel
{
    EF_LEVEL_LOW = 0x1,
    EF_LEVEL_MIDDLE = 0x2,
    EF_LEVEL_HIGH = 0x3
}EffectLevel;


class IPlatformCore : public cmm::IStateImplementer
{
public:
    virtual bool                        initialize(const InitializationParam *pInitParam, const std::string &strHost, const std::string &strPort, const std::string &strPSHost,
                                                   const std::string &strPSPort, const std::string &strUserName, const std::string &strPassword, const std::string &strLocalCache, cmm::IStateQuerier *pStateQuerier) = 0;
    virtual void                        unInitialize(void)                                              = 0;
    virtual const ISceneViewer         *getSceneViewer(void) const                                      = 0;
    virtual ISceneViewer               *getSceneViewer(void)                                            = 0;
    virtual const IUtility             *getUtility(void) const                                          = 0;
    virtual IUtility                   *getUtility(void)                                                = 0;
    virtual const IStateManager        *getStateManager(void) const                                     = 0;
    virtual IStateManager              *getStateManager(void)                                           = 0;

    virtual const ITerrainModificationManager *getTerrainModificationManager(void) const                = 0;
    virtual ITerrainModificationManager *getTerrainModificationManager(void)                            = 0;

    virtual void            setDEMOrder(const IDList &vecOrder)     = 0;
    virtual void            getDEMOrder(IDList &vecOrder) const     = 0;
    virtual void            setDOMOrder(const IDList &vecOrder)     = 0;
    virtual void            getDOMOrder(IDList &vecOrder) const     = 0;

    virtual bool            addLocalDatabase(const std::string &strDB)                                  = 0;
    virtual bool            removeLocalDatabase(const std::string &strDB)                               = 0;

    //SelectType»°÷µ£∫All,Terrain,Model
    virtual ID              selectByScreenPoint(const cmm::math::Point2i &point) = 0;
    virtual IDList          selectByScreenRect(const cmm::math::Point2i &ptLeftTop, const cmm::math::Point2i &ptRightBottom) = 0;
	virtual bool            selectByScreenPoint(const cmm::math::Point2i &point, cmm::math::Point3d &inter_point) = 0;

    virtual void                setMemoryLimited(unsigned __int64 nMemLimited)                        = 0;
    virtual unsigned __int64    getMemoryLimited(void) const                                          = 0;

    virtual void            setTerrainOpacity(double dblOpacity)                                    = 0;
    virtual double          getTerrainOpacity(void) const                                           = 0;

    virtual void                        setSubGroundColor(const cmm::FloatColor &color)             = 0;
    virtual const cmm::FloatColor      &getSubGroundColor(void) const                               = 0;
    virtual void            setSubGroundDepth(double dblDepth)                                      = 0;
    virtual double          getSubGroundDepth(void) const                                           = 0;
    virtual void            setVisibleRangeRatio(double dblRatio)                                   = 0;
    virtual double          getVisibleRangeRatio(void) const                                        = 0;

    virtual void            addTempModel(const ID &id)                              = 0;
    virtual void            removeTempModel(const ID &id)                           = 0;
    virtual double          fetchElevationInView(unsigned nIndex, const cmm::math::Point2d &position) const  = 0;
    virtual bool            addWMTSTileSet(deues::ITileSet *pTileSet)               = 0;
    virtual bool            removeWMTSTileSet(deues::ITileSet *pTileSet)            = 0;
    virtual bool            createGlobalEffectNode(EffectType nEffectType)                                                      = 0;
    virtual bool            showGlobalEffectNode(EffectType nEffectType, bool bIsShow)                                          = 0;
    virtual bool            setGlobalEffectLevel(EffectType nEffectType, EffectLevel nEffectLevel)                              = 0;

    virtual bool            createEffectNode(const std::string& strEffectName, EffectType effectType, EffectLevel effectLevel, 
                                             double dLat, double dLong, double dHeight, double dRadius)                         = 0;

    virtual bool            removeEffectNode(const std::string& strEffectName, EffectType nEffectType)                          = 0;
    virtual bool            setEffectLevel(const std::string& strEffectName, EffectType nEffectType, EffectLevel nEffectLevel)  = 0;

    virtual void            useShadow(bool bOpenFlag)                                                                           = 0;


    virtual void    setDensity(double dVal) = 0;
    virtual void    setNearTransition(double dVal) = 0;
    virtual void    setFarTransition(double dVal) = 0;
    virtual void    setParticleSpeed(double dVal) = 0;
    virtual void    setParticleSize(double dVal) = 0;
    virtual void    setParticleColor(double dR, double dG, double dB, double dA) = 0;
    virtual void    setUseFarLineSegments(bool bVal) = 0;
    virtual void    getRainParams(double& dDensity, double& dNear, double& dFar, double& dSpeed, double& dSize, double& dColor) = 0;
};


PLATFORM_EXPORT IPlatformCore *createPlatformCore(ea::IEventAdapter *pEventAdapter = NULL);

#endif
