#ifndef INSTANCE_MANAGER_H_2CBFBCA5_3D59_4AA4_8E1E_D1A217386AA3_INCLUDE
#define INSTANCE_MANAGER_H_2CBFBCA5_3D59_4AA4_8E1E_D1A217386AA3_INCLUDE

#include "IVirtualTileManager.h"

#include <math.h>
#include <OpenSP/sp.h>
#include <Common/Pyramid.h>
#include <Common/deuMath.h>
#include <DEUDBProxy/IDEUDBProxy.h>
#include <map>
#include <list>
#include <Common/DEUBson.h>
#include <algorithm>
#include <Common/MemPool.h>
#include <OpenThreads/Mutex>
#include <deque>
#include <iostream>

namespace vtm
{

UINT_64 getRefTime(void);

struct TileFragment
{
public:
    explicit TileFragment(void)
    {
    }
    TileFragment(const TileFragment &param)
    {
        operator=(param);
    }
    virtual ~TileFragment(void){}

    const TileFragment &operator=(const TileFragment &param)
    {
        if(this == &param)  return *this;
        m_listObjects = param.m_listObjects;
        return *this;
    }

    bool operator==(const TileFragment &param) const
    {
        if(this == &param)  return true;
        return (m_listObjects == param.m_listObjects);
    }

    bool operator!=(const TileFragment &param) const
    {
        return !(operator==(param));
    }

    bool isExist(const ID &id) const
    {
        IDList::const_iterator itorFind = std::lower_bound(m_listObjects.begin(), m_listObjects.end(), id);
        if(itorFind == m_listObjects.end())
        {
            return false;
        }

        const ID &idFind = *itorFind;
        return (idFind == id);
    }

    unsigned getObjectsCount(void) const
    {
        return m_listObjects.size();
    }

    void getObjects(IDList &vecObjectIDs) const
    {
        vecObjectIDs.insert(vecObjectIDs.end(), m_listObjects.begin(), m_listObjects.end());
    }

    bool addObject(const ID &id)
    {
        IDList::const_iterator itorFind = std::lower_bound(m_listObjects.begin(), m_listObjects.end(), id);
        if(itorFind == m_listObjects.end())
        {
            m_listObjects.push_back(id);
            return true;
        }

        const ID &idFind = *itorFind;
        if(idFind == id)
        {
            return false;
        }

        m_listObjects.insert(itorFind, id);
        return true;
    }

    bool removeObject(const ID &id)
    {
        IDList::const_iterator itorFind = std::lower_bound(m_listObjects.begin(), m_listObjects.end(), id);
        if(itorFind == m_listObjects.end())
        {
            return false;
        }

        const ID &idFind = *itorFind;
        if(idFind == id)
        {
            m_listObjects.erase(itorFind);
            return true;
        }

        return false;
    }

    IDList      m_listObjects;
};

const unsigned int g_nFragmentCount = 5u;
class VirtualTile : public IVirtualTile
{
public:
    explicit VirtualTile(void)
    {
        m_bChildValid[0] = false;
        m_bChildValid[1] = false;
        m_bChildValid[2] = false;
        m_bChildValid[3] = false;
    }

    VirtualTile(const VirtualTile &param)
    {
        operator=(param);
    }

    virtual ~VirtualTile(void){}

    bool isExist(const ID &id) const
    {
        for(unsigned y = 0u; y < g_nFragmentCount; y++)
        {
            for(unsigned x = 0u; x < g_nFragmentCount; x++)
            {
                if(m_mtxFragments[y][x].isExist(id))
                {
                    return true;
                }
            }
        }
        return false;
    }

    unsigned getObjectsCount(void) const
    {
        unsigned nCount = 0;
        for(unsigned y = 0u; y < g_nFragmentCount; y++)
        {
            for(unsigned x = 0u; x < g_nFragmentCount; x++)
            {
                nCount += m_mtxFragments[y][x].getObjectsCount();
            }
        }
        return nCount;
    }


    void getObjects(IDList &vecObjectIDs) const
    {
        for(unsigned y = 0u; y < g_nFragmentCount; y++)
        {
            for(unsigned x = 0u; x < g_nFragmentCount; x++)
            {
                m_mtxFragments[y][x].getObjects(vecObjectIDs);
            }
        }
    }


    bool operator==(const VirtualTile &param) const
    {
        if(this == &param)  return true;

        if(m_nLevel != param.m_nLevel)  return false;
        if(m_nRow != param.m_nRow)      return false;
        if(m_nCol != param.m_nCol)      return false;

        if(m_bChildValid[0] != param.m_bChildValid[0])    return false;
        if(m_bChildValid[1] != param.m_bChildValid[1])    return false;
        if(m_bChildValid[2] != param.m_bChildValid[2])    return false;
        if(m_bChildValid[3] != param.m_bChildValid[3])    return false;

        for(unsigned y = 0u; y < g_nFragmentCount; y++)
        {
            for(unsigned x = 0u; x < g_nFragmentCount; x++)
            {
                if(m_mtxFragments[y][x] != param.m_mtxFragments[y][x])
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool operator!=(const VirtualTile &param) const
    {
        return !(operator==(param));
    }

    const VirtualTile &operator=(const VirtualTile &param)
    {
        if(this == &param)   return *this;

        m_bChildValid[0] = param.m_bChildValid[0];
        m_bChildValid[1] = param.m_bChildValid[1];
        m_bChildValid[2] = param.m_bChildValid[2];
        m_bChildValid[3] = param.m_bChildValid[3];

        m_nLevel = param.m_nLevel;
        m_nRow = param.m_nRow;
        m_nCol = param.m_nCol;

        for(unsigned y = 0u; y < g_nFragmentCount; y++)
        {
            for(unsigned x = 0u; x < g_nFragmentCount; x++)
            {
                m_mtxFragments[y][x] = param.m_mtxFragments[y][x];
            }
        }

        return *this;
    }

    virtual unsigned getFragmentObjectsCount(unsigned nRow, unsigned nCol) const
    {
        return m_mtxFragments[nRow][nCol].getObjectsCount();
    }

    virtual void getFragmentObjects(unsigned nRow, unsigned nCol, IDList &vecObjectIDs) const
    {
        m_mtxFragments[nRow][nCol].getObjects(vecObjectIDs);
    }

    virtual void getChildTileState(bool bValid[4]) const
    {
        bValid[0] = m_bChildValid[0];
        bValid[1] = m_bChildValid[1];
        bValid[2] = m_bChildValid[2];
        bValid[3] = m_bChildValid[3];
    }

    virtual void mergeVTile(const IVirtualTile *pTile);

    bool addObject(const ID &id, double dblX, double dblY);
    bool removeObject(const ID &id);

    bool toBson(bson::bsonDocument &bsonDoc) const;
    bool fromBson(const bson::bsonDocument &bsonDoc);

public:
    bool                m_bChildValid[4];
    unsigned int        m_nLevel;
    unsigned int        m_nRow;
    unsigned int        m_nCol;
    TileFragment        m_mtxFragments[g_nFragmentCount][g_nFragmentCount];
};


class VirtualTileManager : public IVirtualTileManager
{
    friend IVirtualTileManager *createVirtualTileManager(void);
public:
    explicit VirtualTileManager(void);
    virtual ~VirtualTileManager(void);

protected:
    virtual bool        save2DB(const std::string &strTargetDB);

    virtual bool        addObject(const ID &id, double dblX, double dblY, double dblHeight, double dblRadius);
    virtual bool        removeObject(const ID &id, double dblX, double dblY, double dblHeight, double dblRadius);

    virtual unsigned    getMaxGridLevel(void) const         {   return m_nMaxLevel;                 }

    virtual void        getObjectsOnTile(const ID &idTile, IDList &vecObjectIDs, bool valid[]) const;
    virtual const       IVirtualTile *getVirtualTile(const ID &idTile) const;
    virtual IVirtualTile   *copyVirtualTile(const ID &idTile) const;

    virtual unsigned    getTileFragmentCount(void) const    {   return g_nFragmentCount;            }

    virtual double      getTileRangeRatio(void) const       {   return 7.0;                         }

    virtual bool        takeChangedVTile(IDList &idList);
    virtual void        stopChangingListening(void);
    virtual void        startChangingListening(void);

protected:
    bool                initialize(void);
    bool                getLevelRowCol(double dblX, double dblY, double dblHeight, double dblRadius, unsigned &nLevel, unsigned &nRow, unsigned &nCol) const;
    VirtualTile        *loadTile(const void *pBuffer, unsigned nLength) const;

    struct VTileInfo
    {
        OpenSP::sp<VirtualTile>     m_pVirtualTile;
        unsigned __int64            m_nLastRefTime;

        VTileInfo(void)
        {
            m_pVirtualTile = NULL;
            m_nLastRefTime = getRefTime();
        }
        VTileInfo(const VTileInfo &info)
        {
            m_pVirtualTile = info.m_pVirtualTile;
            m_nLastRefTime = info.m_nLastRefTime;
        }
        ~VTileInfo(void)
        {
        }

        const VTileInfo &operator=(const VTileInfo &info)
        {
            if(this == &info)   return *this;
            m_pVirtualTile = info.m_pVirtualTile;
            m_nLastRefTime = info.m_nLastRefTime;
            return *this;
        }
    };

    void                reLoadTile(const ID &idTile);
    void                addTileToWindow(const ID &idTile, const VTileInfo &info);
    void                limitWindowSize(void);
    void                removeTile(const ID &idTile);

    void                registerChangedVTile(const ID &id);

protected:
    const unsigned      m_nMaxLevel;
    const unsigned      m_nMemoryLimited;

    OpenSP::sp<deudbProxy::IDEUDBProxy>         m_pVTileDB;

    std::map<ID, VTileInfo>             m_mapVTilesWindow;
    std::map<unsigned __int64, ID>      m_mapBuffer;
    mutable OpenThreads::Mutex          m_mtxVTilesWindow;

    std::set<ID>                        m_setChangedVTile;
    mutable OpenThreads::Mutex          m_mtxChangedVTile;
    OpenThreads::Block                  m_blockChangedVTile;
    bool                                m_bChangingListening;
};

}

#endif
