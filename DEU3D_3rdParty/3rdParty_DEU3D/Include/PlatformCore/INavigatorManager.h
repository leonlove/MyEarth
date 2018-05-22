#ifndef I_NAVIGATOR_MANAGER_H_43DD71E0_6F03_48A2_9A8D_1498F60AD09B_INCLUDE
#define I_NAVIGATOR_MANAGER_H_43DD71E0_6F03_48A2_9A8D_1498F60AD09B_INCLUDE

#include <string>
#include <vector>
#include "NavigationParam.h"

class INavigatorManager : virtual public OpenSP::Ref
{
public:
    virtual unsigned                getNavigators(std::vector<std::string> &vecNavigatorNames) const = 0;

    virtual bool                    setActiveNavigator(const std::string &strName) = 0;
    virtual const std::string      &getActiveNavigator(void) const = 0;

    virtual NavigationParam        *getNavigationParam(void) = 0;
    virtual const NavigationParam  *getNavigationParam(void) const = 0;

    virtual void        resetCamera(void) = 0;
    virtual void        stopInertia(void) = 0;

    virtual void        getCameraPose(CameraPose &pose) const = 0;
    virtual void        setCameraPose(const CameraPose &pose) = 0;

    virtual void        setEventOnPoseChanged(bool bPost) = 0;
    virtual bool        hasEventOnPoseChanged(void) const = 0;

	virtual void        enableInertia(const std::string &strNavigatorName, bool bEnable) = 0;
	virtual void		enableUnderGroundViewMode(const std::string &strNavigatorName, bool bEnable) = 0;
};

#endif
