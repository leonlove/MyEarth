#ifndef I_INSTANCE_MANAGER_H_974990BE_65F0_4916_B035_5D9C243F4FD1_INCLUDE
#define I_INSTANCE_MANAGER_H_974990BE_65F0_4916_B035_5D9C243F4FD1_INCLUDE

#include "Export.h"
#include <string>
#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>
#include <set>
#include <common/deuMath.h>

namespace vtm
{

class IVirtualTile : public OpenSP::Ref
{
public:
    virtual unsigned    getFragmentObjectsCount(unsigned nRow, unsigned nCol) const = 0;
    virtual void        getFragmentObjects(unsigned nRow, unsigned nCol, IDList &vecObjectIDs) const = 0;
    virtual void        getChildTileState(bool bValid[4]) const = 0;
    virtual void        mergeVTile(const IVirtualTile *pVTile) = 0;
};


class IVirtualTileManager : public OpenSP::Ref
{
public:
    virtual bool            save2DB(const std::string &strTargetDB)                                               = 0;

    virtual bool            addObject(const ID &id, double dblX, double dblY, double dblHeight, double dblRadius)     = 0;
    virtual bool            removeObject(const ID &id, double dblX, double dblY, double dblHeight, double dblRadius)  = 0;

    //virtual unsigned        getTilesCount(void) const = 0;
    //virtual void            getAllTileIDs(IDList &idTiles) const = 0;
    virtual const           IVirtualTile *getVirtualTile(const ID &idTile) const = 0;
    virtual IVirtualTile   *copyVirtualTile(const ID &idTile) const = 0;

    virtual unsigned        getMaxGridLevel(void) const      = 0;
    virtual unsigned        getTileFragmentCount(void) const = 0;
    virtual double          getTileRangeRatio(void) const = 0;
    virtual bool            takeChangedVTile(IDList &idList) = 0;
    virtual void            stopChangingListening(void) = 0;
    virtual void            startChangingListening(void) = 0;
};


VIRTUALTILE_MANAGER_EXPORT IVirtualTileManager *createVirtualTileManager(void);
VIRTUALTILE_MANAGER_EXPORT IVirtualTile *createVirtualTileByStream(const void *pBuffer, unsigned nLength);


}

#endif
