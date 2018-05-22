#ifndef I_TERRAIN_COLOR_MODIFICATION_H_83126982_D888_4D3F_8650_2F1850DCA2E4_INCLUDE
#define I_TERRAIN_COLOR_MODIFICATION_H_83126982_D888_4D3F_8650_2F1850DCA2E4_INCLUDE

#include "ITerrainModification.h"
#include "common/Common.h"

class ITerrainColorModification : virtual public ITerrainModification
{
public:
    virtual void setColor(const cmm::FloatColor &color) = 0;
    virtual const cmm::FloatColor &getColor(void) const = 0;
};

#endif
