#ifndef I_LAYER_MANAGER_H_D318B953_7CC4_42B2_A5E5_89FFC663AF42_INCLUDE
#define I_LAYER_MANAGER_H_D318B953_7CC4_42B2_A5E5_89FFC663AF42_INCLUDE


#include "Export.h"

#include <string>
#include <vector>
#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>
#include <Common/IStateQuerier.h>


namespace ea
{
    class IEventAdapter;
}

namespace vcm
{
    class IVirtualCubeManager;
}

namespace logical
{

class IPropertyManager;
class IObject;
class ILayer;
class IInstance;


class ILayerManager : public cmm::IStateQuerier
{
public:
    virtual bool        login(const std::string& strAuthHost, const std::string& strAuthPort, const std::string& strUserName, const std::string& strUserPwd) = 0;
    virtual bool        logout()                                            = 0;
    virtual bool        initialize(const std::string &strHost, const std::string &strPort, const std::string &strLocalCache) = 0;
    virtual void        unInitialize(void) = 0;
    virtual ILayer     *getCultureRootLayer(void)                           = 0;
    virtual ILayer     *getTerrainDEMRootLayer(void)                        = 0;
    virtual ILayer     *getTerrainDOMRootLayer(void)                        = 0;
    virtual IObject    *findObject(const ID &id)                            = 0;

    virtual IPropertyManager *getPropertyManager(void)                      = 0;
    virtual const IPropertyManager *getPropertyManager(void) const          = 0;
    virtual vcm::IVirtualCubeManager *getVirtualCubeManager(void)           = 0;
};

LOGICAL_EXPORT ILayerManager *createLayerManager(void);

}
#endif