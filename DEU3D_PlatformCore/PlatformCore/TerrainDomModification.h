#ifndef TERRAIN_DOM_MODIFICATION_H_2ECB305E_6016_41EA_95BF_07382F2B5C07_INCLUDE
#define TERRAIN_DOM_MODIFICATION_H_2ECB305E_6016_41EA_95BF_07382F2B5C07_INCLUDE

#include "ITerrainDomModification.h"
#include "TerrainModification.h"

class TerrainDomModification : public ITerrainDomModification, virtual public TerrainModification
{
public:
    explicit TerrainDomModification(const std::string &strType, ea::IEventAdapter *pEventAdapter);
    virtual ~TerrainDomModification(void);

public:
    virtual bool    setDomFile(const std::string &strFilePath);
    virtual const   std::string &getDomFile(void) const;

    virtual bool    modifyTerrainTile(osg::Node *pTerrainTile) const;

protected:
    std::string     m_strDomFile;
    osg::ref_ptr<osg::Image> m_pDomImage;
};


#endif
