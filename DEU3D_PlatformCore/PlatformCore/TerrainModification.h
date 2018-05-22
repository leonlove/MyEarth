#ifndef TERRAIN_MODIFICATION_H_85E17620_EF96_4651_88ED_436EA49130C4_INCLUDE
#define TERRAIN_MODIFICATION_H_85E17620_EF96_4651_88ED_436EA49130C4_INCLUDE

#include "ITerrainModification.h"
#include <vector>
#include <osgTerrain/TerrainTile>
#include <OpenThreads/Atomic>
#include <EventAdapter/IEventAdapter.h>

#pragma warning (disable : 4250)
class TerrainModification : virtual public ITerrainModification
{
public:
    explicit TerrainModification(const std::string &strType, ea::IEventAdapter *pEventAdapter);
    virtual ~TerrainModification(void) = 0;

public:
    virtual void        setName(const std::string &strName) {   m_strName = strName;                    }
    virtual const std::string &getName(void) const          {   return m_strName;                       }
    virtual const std::string &getType(void) const          {   return m_strType;                       }
    virtual unsigned    getVerticesCount(void) const        {   return m_Polygon.getVerticesCount();    }
    virtual void        addVertex(double dblX, double dblY);
    virtual bool        removeVertex(unsigned nIndex);
    virtual void        clearVertices(void)                 {   m_Polygon.clear();                      }
    virtual void        setApply(bool bApply);
    virtual bool        isApply(void) const                 {   return ((unsigned)m_bApply != 0);       }

    virtual bool        modifyTerrainTile(osg::Node *pTerrainTile) const = 0;
    virtual bool        shouldBeModified(const cmm::math::Polygon2 &polygonTile, const cmm::math::Box2d &bbTile) const;
    virtual bool        shouldBeModified(const osgTerrain::TerrainTile *pTerrainTile) const;

protected:
    const std::string       m_strType;
    std::string             m_strName;
    cmm::math::Polygon2     m_Polygon;
    cmm::math::Box2d        m_Box;
    OpenThreads::Atomic     m_bApply;
    OpenSP::sp<ea::IEventAdapter>   m_pEventAdapter;
};

#endif
