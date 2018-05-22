#ifndef I_VIRTUAL_CUBE_MANAGER_H_974990BE_65F0_4916_B035_5D9C243F4FD1_INCLUDE
#define I_VIRTUAL_CUBE_MANAGER_H_974990BE_65F0_4916_B035_5D9C243F4FD1_INCLUDE

#include "Export.h"
#include <string>
#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>
#include <set>
#include <common/deuMath.h>

namespace vcm
{

class IVirtualCube : public OpenSP::Ref
{
public:
    virtual unsigned    getFragmentObjectsCount(unsigned nRow, unsigned nCol, unsigned nHeight) const = 0;
    virtual void        getFragmentObjects(unsigned nRow, unsigned nCol, unsigned nHeight, IDList &vecObjectIDs) const = 0;
    virtual void        getChildCubeState(bool bValid[16]) const = 0;
    virtual void        mergeVCube(const IVirtualCube *pVCube) = 0;
};


class IVirtualCubeManager : public OpenSP::Ref
{
public:
    virtual bool            save2DB(const std::string &strTargetDB) = 0;

    virtual bool            addObject(const ID &id, const cmm::math::Sphered &sphere)     = 0;
    virtual bool            removeObject(const ID &id, const cmm::math::Sphered &sphere)  = 0;

    virtual const           IVirtualCube *getVirtualCube(const ID &idCube) const = 0;
    virtual IVirtualCube   *copyVirtualCube(const ID &idCube) const = 0;

    virtual unsigned        getMaxGridLevel(void) const      = 0;
    virtual unsigned        getCubeFragmentCount(void) const = 0;
    virtual double          getCubeRangeRatio(void) const = 0;
    virtual bool            takeChangedVCube(IDList &idList) = 0;
    virtual void            stopChangingListening(void) = 0;
    virtual void            startChangingListening(void) = 0;
};


VIRTUALTILE_MANAGER_EXPORT IVirtualCubeManager *createVirtualCubeManager(void);
VIRTUALTILE_MANAGER_EXPORT IVirtualCube *createVirtualCubeByStream(const void *pBuffer, unsigned nLength);


}

#endif
