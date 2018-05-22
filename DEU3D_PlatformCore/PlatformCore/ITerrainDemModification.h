#ifndef I_TERRAIN_DEM_MODIFICATION_H_3C319FE9_1CB2_43E3_9089_8085DF093501_INCLUDE
#define I_TERRAIN_DEM_MODIFICATION_H_3C319FE9_1CB2_43E3_9089_8085DF093501_INCLUDE

#include "ITerrainModification.h"

class ITerrainDemModification : virtual public ITerrainModification
{
public:
    virtual bool    setDemFile(const std::string &strFilePath) = 0;
    virtual const   std::string &getDemFile(void) const = 0;

};

#endif
