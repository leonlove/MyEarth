#ifndef TERRAIN_DEM_MODIFICATION_H_49DCFF6C_7468_4639_B1D6_CDFF631D6DF0_INCLUDE
#define TERRAIN_DEM_MODIFICATION_H_49DCFF6C_7468_4639_B1D6_CDFF631D6DF0_INCLUDE

#include "ITerrainDemModification.h"
#include "TerrainModification.h"

class TerrainDemModification : public ITerrainDemModification, virtual public TerrainModification
{
public:
    explicit TerrainDemModification(const std::string &strType, ea::IEventAdapter *pEventAdapter);
    virtual ~TerrainDemModification(void);

public:
    virtual bool    setDemFile(const std::string &strFilePath);
    virtual const   std::string &getDemFile(void) const;

    virtual bool    modifyTerrainTile(osg::Node *pTerrainTile) const;

protected:
    std::string     m_strDemFile;
    osg::ref_ptr<osg::Image> m_pDemImage;
};


#endif
