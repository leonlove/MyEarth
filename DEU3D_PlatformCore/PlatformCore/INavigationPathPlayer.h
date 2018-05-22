#ifndef I_NAVIGATION_PATH_PLAYER_H_70939CA7_91B0_4736_8EE3_D05E6B2DEBFB_INCLUDE
#define I_NAVIGATION_PATH_PLAYER_H_70939CA7_91B0_4736_8EE3_D05E6B2DEBFB_INCLUDE

#include <OpenSP/Ref.h>
#include "NavigationParam.h"
#include "IAnimationModel.h"
#include "INavigationPath.h"


class INavigationPathPlayer : virtual public OpenSP::Ref
{
public:
    virtual void    setNavigationPath(const INavigationPath * pPath) = 0;
    virtual void    setNavigationPathByFromTo(const INavigationKeyframe * pFrameFrom, const INavigationKeyframe * pFrameTo, bool bCurve) = 0;
    virtual void    setNavigationPathByFromTo(const INavigationKeyframe * pFrameFrom, double dblPosX, double dblPosY, double dblPosH, double dblRadius, bool bCurve) = 0;
    virtual void    setSmoothLevel(double dblSmoothLevel) = 0;
    virtual void    play(void) = 0;
    virtual void    stop(void) = 0;
    virtual void    pause(void) = 0;
    virtual std::string getPlayingState(void) const = 0;
    virtual double  getNavigationPathLength(void) const = 0;
    virtual double  getPlayingPosition(void) const = 0;
    virtual void    setPlayingPosition(double dblPos) = 0;
    virtual void    setNavigationModel(IAnimationModel *pNaviModel) = 0;
    virtual IAnimationModel *getNavigationModel(void) = 0;
    virtual const   IAnimationModel *getNavigationModel(void) const = 0;
    virtual void    setFixCamera(bool bFixCamera = true) = 0;
    virtual bool    getFixCamera(void) = 0;
};

#endif

