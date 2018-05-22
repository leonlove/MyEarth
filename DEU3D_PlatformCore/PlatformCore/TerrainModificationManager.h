#ifndef TERRAIN_MODIFICATION_MANAGER_H_244E51A7_3EB4_45BE_A4ED_27CC3FFC933B_INCLUDE
#define TERRAIN_MODIFICATION_MANAGER_H_244E51A7_3EB4_45BE_A4ED_27CC3FFC933B_INCLUDE

#include "ITerrainModificationManager.h"
#include <vector>
#include <OpenSP/sp.h>
#include <EventAdapter/IEventAdapter.h>
#include "TerrainModification.h"

class TerrainModificationManager : public ITerrainModificationManager
{
public:
    explicit TerrainModificationManager(ea::IEventAdapter *pEventAdapter);
    virtual ~TerrainModificationManager(void);

public:
    virtual ITerrainModification             *createModification(const std::string &strType);
    virtual unsigned                          getModificationCount(void) const;
    virtual ITerrainModification             *getModification(unsigned nIndex);
    virtual const ITerrainModification       *getModification(unsigned nIndex) const;
    virtual ITerrainModification             *findModificationByName(const std::string &strName);
    virtual const ITerrainModification       *findModificationByName(const std::string &strName) const;
    virtual bool                              removeModification(unsigned nIndex);
    virtual bool                              removeModification(ITerrainModification *pModification);

public:
    virtual bool    modifyTerrainTile(osg::Node *pTerrainTile) const;

public:
    bool            modifyTerrainTile(osg::Node *pTerrainTile, bool bModifyTexture) const;

protected:
    mutable OpenThreads::Mutex                      m_mtxTerrainModifications;
    std::vector<OpenSP::sp<TerrainModification> >   m_vecTerrainModifications;
    OpenSP::sp<ea::IEventAdapter>                   m_pEventAdapter;
};


#endif
