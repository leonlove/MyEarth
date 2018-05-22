#ifndef I_OBJECT_H_4FE484A0_B863_4782_AF32_EF3CB5F58C3D_INCLUDE
#define I_OBJECT_H_4FE484A0_B863_4782_AF32_EF3CB5F58C3D_INCLUDE

#include "Export.h"
#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>
#include <Common/StateDefiner.h>
#include <Common/deuMath.h>
#include <map>
#pragma warning (disable : 4250)
namespace logical
{
class IPropertyManager;
class IProperty;

class ILayer;
class IInstance;
class ITerrainInstance;

class IObject : public OpenSP::Ref
{
public:
    virtual const ID           &getID(void)                                                        const    = 0;
    virtual DeuObjectIDType     getType(void)                                                      const    = 0;

    virtual ILayer              *asLayer(void) = 0;
    virtual IInstance           *asInstance(void) = 0;
    virtual ITerrainInstance    *asTerrainInstance(void)=0;

    virtual void                setState(const std::string &strStateName, bool bState)                      = 0;
    virtual bool                getState(const std::string &strStateName)                          const    = 0;

    virtual const std::string  &getName(void)                                                      const    = 0;
    virtual void                setName(const std::string &strName)                                         = 0;
    virtual unsigned            getParentCount(void)                                               const    = 0;
    virtual ILayer             *getParent(unsigned nIndex)                                                  = 0;
    virtual const ILayer       *getParent(unsigned nIndex)                                         const    = 0;
    virtual const cmm::math::Sphered &getBound(void)                                               const    = 0;
};

}
#endif