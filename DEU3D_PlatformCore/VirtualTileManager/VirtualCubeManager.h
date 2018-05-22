#ifndef VIRTUAL_CUBE_MANAGER_H_2CBFBCA5_3D59_4AA4_8E1E_D1A217386AA3_INCLUDE
#define VIRTUAL_CUBE_MANAGER_H_2CBFBCA5_3D59_4AA4_8E1E_D1A217386AA3_INCLUDE

#include "IVirtualCubeManager.h"

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

namespace vcm
{

UINT_64 getRefTime(void);

struct CubeFragment
{
public:
    explicit CubeFragment(void)
    {
    }
    CubeFragment(const CubeFragment &param)
    {
        operator=(param);
    }
    virtual ~CubeFragment(void){}

    const CubeFragment &operator=(const CubeFragment &param)
    {
        if(this == &param)  return *this;
        m_listObjects = param.m_listObjects;
        return *this;
    }

    bool operator==(const CubeFragment &param) const
    {
        if(this == &param)  return true;
        return (m_listObjects == param.m_listObjects);
    }

    bool operator!=(const CubeFragment &param) const
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

class VirtualCube : public IVirtualCube
{
public:
    explicit VirtualCube(void)
    {
        memset(m_bChildValid, 0, sizeof(m_bChildValid));
    }

    VirtualCube(const VirtualCube &param)
    {
        operator=(param);
    }

    virtual ~VirtualCube(void){}

    bool isExist(const ID &id) const
    {
        for(unsigned y = 0u; y < g_nFragmentCount; y++)
        {
            for(unsigned x = 0u; x < g_nFragmentCount; x++)
            {
                for(unsigned z = 0u; z < g_nFragmentCount; z++)
                {
                    if(m_mtxFragments[y][x][z].isExist(id))
                    {
                        return true;
                    }
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
                for(unsigned z = 0u; z < g_nFragmentCount; z++)
                {
                    nCount += m_mtxFragments[y][x][z].getObjectsCount();
                }
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
                for(unsigned z = 0u; z < g_nFragmentCount; z++)
                {
                    m_mtxFragments[y][x][z].getObjects(vecObjectIDs);
                }
            }
        }
    }


    bool operator==(const VirtualCube &param) const
    {
        if(this == &param)  return true;

        if(m_nLevel != param.m_nLevel)      return false;
        if(m_nRow != param.m_nRow)          return false;
        if(m_nCol != param.m_nCol)          return false;
        if(m_nHeight != param.m_nHeight)    return false;

        if(memcmp(m_bChildValid, param.m_bChildValid, sizeof(m_bChildValid)) != 0)
        {
            return false;
        }

        for(unsigned y = 0u; y < g_nFragmentCount; y++)
        {
            for(unsigned x = 0u; x < g_nFragmentCount; x++)
            {
                for(unsigned z = 0u; z < g_nFragmentCount; z++)
                {
                    if(m_mtxFragments[y][x][z] != param.m_mtxFragments[y][x][z])
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    bool operator!=(const VirtualCube &param) const
    {
        return !(operator==(param));
    }

    const VirtualCube &operator=(const VirtualCube &param)
    {
        if(this == &param)   return *this;

        memcpy(m_bChildValid, param.m_bChildValid, sizeof(m_bChildValid));

        m_nLevel = param.m_nLevel;
        m_nRow = param.m_nRow;
        m_nCol = param.m_nCol;
        m_nHeight = param.m_nHeight;

        for(unsigned y = 0u; y < g_nFragmentCount; y++)
        {
            for(unsigned x = 0u; x < g_nFragmentCount; x++)
            {
                for(unsigned z = 0u; z < g_nFragmentCount; z++)
                {
                    m_mtxFragments[y][x][z] = param.m_mtxFragments[y][x][z];
                }
            }
        }

        return *this;
    }

    virtual unsigned getFragmentObjectsCount(unsigned nRow, unsigned nCol, unsigned nHeight) const
    {
        return m_mtxFragments[nRow][nCol][nHeight].getObjectsCount();
    }

    virtual void getFragmentObjects(unsigned nRow, unsigned nCol, unsigned nHeight, IDList &vecObjectIDs) const
    {
        m_mtxFragments[nRow][nCol][nHeight].getObjects(vecObjectIDs);
    }

    virtual void getChildCubeState(bool bValid[8]) const
    {
        memcpy(bValid, m_bChildValid, sizeof(m_bChildValid));
    }

    virtual void mergeVCube(const IVirtualCube *pCube);

    bool addObject(const ID &id, const cmm::math::Point3d &pos);
    bool removeObject(const ID &id);

    bool toBson(bson::bsonDocument &bsonDoc) const;
    bool fromBson(const bson::bsonDocument &bsonDoc);

public:
    bool                m_bChildValid[8];
    unsigned int        m_nLevel;
    unsigned int        m_nRow;
    unsigned int        m_nCol;
    unsigned int        m_nHeight;
    CubeFragment        m_mtxFragments[g_nFragmentCount][g_nFragmentCount][g_nFragmentCount];
};


class VirtualCubeManager : public IVirtualCubeManager
{
    friend IVirtualCubeManager *createVirtualCubeManager(void);
public:
    explicit VirtualCubeManager(void);
    virtual ~VirtualCubeManager(void);

protected:
    virtual bool        save2DB(const std::string &strTargetDB);

    virtual bool        addObject(const ID &id, const cmm::math::Sphered &sphere);
    virtual bool        removeObject(const ID &id, const cmm::math::Sphered &sphere);

    virtual unsigned    getMaxGridLevel(void) const         {   return m_nMaxLevel;                 }

    virtual void        getObjectsOnCube(const ID &idTile, IDList &vecObjectIDs, bool valid[]) const;
    virtual const       IVirtualCube *getVirtualCube(const ID &idTile) const;
    virtual IVirtualCube   *copyVirtualCube(const ID &idCube) const;

    virtual unsigned    getCubeFragmentCount(void) const    {   return g_nFragmentCount;            }

    virtual double      getCubeRangeRatio(void) const       {   return 7.0;                         }

    virtual bool        takeChangedVCube(IDList &idList);
    virtual void        stopChangingListening(void);
    virtual void        startChangingListening(void);

protected:
    bool                initialize(void);
    bool                getLevelRowColHeight(const cmm::math::Sphered &sphere, unsigned &nLevel, unsigned &nRow, unsigned &nCol, unsigned &nHeight) const;
    VirtualCube        *loadCube(const void *pBuffer, unsigned nLength) const;

    struct VCubeInfo
    {
        OpenSP::sp<VirtualCube>     m_pVirtualCube;
        unsigned __int64            m_nLastRefTime;

        VCubeInfo(void)
        {
            m_pVirtualCube = NULL;
            m_nLastRefTime = getRefTime();
        }
        VCubeInfo(const VCubeInfo &info)
        {
            m_pVirtualCube = info.m_pVirtualCube;
            m_nLastRefTime = info.m_nLastRefTime;
        }
        ~VCubeInfo(void)
        {
        }

        const VCubeInfo &operator=(const VCubeInfo &info)
        {
            if(this == &info)   return *this;
            m_pVirtualCube = info.m_pVirtualCube;
            m_nLastRefTime = info.m_nLastRefTime;
            return *this;
        }
    };

    void                reLoadCube(const ID &idTile);
    void                addCubeToWindow(const ID &idCube, const VCubeInfo &info);
    void                limitWindowSize(void);
    void                removeCube(const ID &idCube);

    void                registerChangedVCube(const ID &id);

protected:
    const unsigned      m_nMaxLevel;
    const unsigned      m_nMemoryLimited;

    OpenSP::sp<deudbProxy::IDEUDBProxy>         m_pVCubeDB;

    std::map<ID, VCubeInfo>             m_mapVCubesWindow;
    std::map<unsigned __int64, ID>      m_mapBuffer;
    mutable OpenThreads::Mutex          m_mtxVCubesWindow;

    std::set<ID>                        m_setChangedVCube;
    mutable OpenThreads::Mutex          m_mtxChangedVCube;
    OpenThreads::Block                  m_blockChangedVCube;
    bool                                m_bChangingListening;
};


}

#endif
