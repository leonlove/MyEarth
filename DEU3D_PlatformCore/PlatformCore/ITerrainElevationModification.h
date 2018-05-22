#ifndef I_TERRAIN_ELEVATION_MODIFICATION_H_0D30A729_9339_43BC_8F45_1368E5DEA2B1_INCLUDE
#define I_TERRAIN_ELEVATION_MODIFICATION_H_0D30A729_9339_43BC_8F45_1368E5DEA2B1_INCLUDE

#include "ITerrainModification.h"

class ITerrainElevationModification : virtual public ITerrainModification
{
public:
    virtual void    setElevation(double dblElevation) = 0;
    virtual double  getElevation(void) const = 0;
    virtual void    setSmoothInterval(double dblSmooth) = 0;
    virtual double  getSmoothInterval(void) const = 0;

};

#endif
