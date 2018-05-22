#ifndef I_SCENE_VIEWER_H_A11CC59F_8BF8_450B_B582_68DC2B36A53B_INCLUDE
#define I_SCENE_VIEWER_H_A11CC59F_8BF8_450B_B582_68DC2B36A53B_INCLUDE

#include "Export.h"
#include <string>
#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>
#include "ViewerParam.h"
#include "NavigationParam.h"
#include <set>


class IToolManager;
class INavigatorManager;
class INavigationPathPlayer;
class IStatusBar;
class ICompass;
class IFluoroScope;
class ISkyDome;
class IAnimationModel;

namespace ea
{
    class IEventAdapter;
}

class ISceneViewer : virtual public OpenSP::Ref
{
public:
    virtual void                     doFrame(void)                                                  = 0;

    virtual void                     resetCameraPose(unsigned int nIndex)                           = 0;

    virtual const IToolManager      *getToolManager(void) const                                     = 0;
    virtual IToolManager            *getToolManager(void)                                           = 0;

    virtual const INavigatorManager *getNavigatorManager(unsigned int nIndex) const                 = 0;
    virtual INavigatorManager       *getNavigatorManager(unsigned int nIndex)                       = 0;

    virtual const INavigationPathPlayer *getNavigationPathPlayer(unsigned int nIndex) const         = 0;
    virtual INavigationPathPlayer   *getNavigationPathPlayer(unsigned int nIndex)                   = 0;


    virtual const IStatusBar        *getStatusBar(unsigned int nIndex) const                        = 0;
    virtual IStatusBar              *getStatusBar(unsigned int nIndex)                              = 0;

    virtual const ICompass          *getCompass(unsigned int nIndex) const                          = 0;
    virtual ICompass                *getCompass(unsigned int nIndex)                                = 0;

    virtual const ISkyDome          *getSkyDome(void) const                                         = 0;
    virtual ISkyDome                *getSkyDome(void)                                               = 0;

    virtual bool                     setMultyViewMode(const std::string &strMode)                   = 0;
    virtual const std::string       &getMultyViewMode(void) const                                   = 0;

    virtual cmm::image::IDEUImage   *snapshot(unsigned int nIndex)                                  = 0;
    virtual cmm::image::IDEUImage   *snapshotFacadeSection(unsigned int nIndex, double dblPoxX1, double dblPosY1, double dblPoxX2, double dblPosY2, double dblHeight)    = 0;
    //virtual bool                     calcCameraPoseByFacadeSection(double dblPoxX1, double dblPosY1, double dblPoxX2, double dblPosY2, double dblHeight, CameraPose &pose) = 0;

    //virtual void                    setActiveViewIndex(unsigned nViewIndex)                         = 0;
    //virtual unsigned                getActiveViewIndex(void) const                                  = 0;

    virtual const IFluoroScope     *getFluoroScope(void) const                                      = 0;
    virtual IFluoroScope           *getFluoroScope(void)                                            = 0;

    virtual void                    acceleratingNavigate(unsigned nIndex, const std::string &strNavMode, double dblTime)               = 0;
	virtual double				    getHeightAt(double lon,double lat,bool bTerrainOnly)			=0;
};


#endif
