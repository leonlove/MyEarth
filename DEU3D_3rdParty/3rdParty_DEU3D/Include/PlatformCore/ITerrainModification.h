#ifndef I_TERRAIN_MODIFICATION_H_31475C38_22BA_491B_81E0_43B473C42737_INCLUDE
#define I_TERRAIN_MODIFICATION_H_31475C38_22BA_491B_81E0_43B473C42737_INCLUDE

#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>
#include <Common/deuMath.h>
#include <vector>

static const std::string TMT_DEM_MODIFICATION           = "dem_modification";
static const std::string TMT_DOM_MODIFICATION           = "dom_modification";
static const std::string TMT_COLOR_MODIFICATION         = "color_modification";
static const std::string TMT_ELEVATION_MODIFICATION     = "elevation_modification";

class ITerrainModification : public OpenSP::Ref
{
public:
    virtual void        setName(const std::string &strName) = 0;
    virtual const std::string &getName(void) const = 0;
    virtual const std::string &getType(void) const = 0;
    virtual unsigned    getVerticesCount(void) const = 0;
    virtual void        addVertex(double dblX, double dblY) = 0;
    virtual bool        removeVertex(unsigned nIndex) = 0;
    virtual void        clearVertices(void) = 0;
    virtual void        setApply(bool bApply) = 0;
    virtual bool        isApply(void) const = 0;
};

#endif
