#ifndef I_TERRAIN_DOM_MODIFICATION_H_6EA7C8AD_0F02_4104_90F7_BDE2768B5CF5_INCLUDE
#define I_TERRAIN_DOM_MODIFICATION_H_6EA7C8AD_0F02_4104_90F7_BDE2768B5CF5_INCLUDE

#include "ITerrainModification.h"

class ITerrainDomModification : virtual public ITerrainModification
{
public:
    virtual bool    setDomFile(const std::string &strFilePath) = 0;
    virtual const   std::string &getDomFile(void) const = 0;

};

#endif
