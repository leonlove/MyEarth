#ifndef I_TERRAIN_MODIFICATION_MANAGER_H_CAF5F280_ABF5_4180_B08B_831374AAA7CC_INCLUDE
#define I_TERRAIN_MODIFICATION_MANAGER_H_CAF5F280_ABF5_4180_B08B_831374AAA7CC_INCLUDE

#include <OpenSP/Ref.h>
#include <string>
#include "ITerrainModification.h"

class ITerrainModificationManager : public OpenSP::Ref
{
public:
    virtual ITerrainModification             *createModification(const std::string &strType)       = 0;
    virtual unsigned                          getModificationCount(void) const       = 0;
    virtual ITerrainModification             *getModification(unsigned nIndex)       = 0;
    virtual const ITerrainModification       *getModification(unsigned nIndex) const = 0;
    virtual ITerrainModification             *findModificationByName(const std::string &strName)       = 0;
    virtual const ITerrainModification       *findModificationByName(const std::string &strName) const = 0;
    virtual bool                              removeModification(unsigned nIndex)    = 0;
    virtual bool                              removeModification(ITerrainModification *pModification)  = 0;
};


#endif


