#ifndef LOGICAL_OBJECT_H_4FE484A0_B863_4782_AF32_EF3CB5F58C3D_INCLUDE
#define LOGICAL_OBJECT_H_4FE484A0_B863_4782_AF32_EF3CB5F58C3D_INCLUDE

#include <vector>
#include <map>
#include <IDProvider/ID.h>

#include "IObject.h"
#include "PropertyManager.h"
#include <EventAdapter/IEventAdapter.h>
#include <OpenSP/op.h>

namespace logical
{

class LayerFinder;
class Layer;

class Object : virtual public IObject
{
    friend class LayerManager;
public:
    explicit Object(const ID &id);
    virtual ~Object(void) = 0;

public:
    virtual const ID           &getID(void) const                   {   return m_id;                }
    virtual DeuObjectIDType     getType(void) const                 {   return (DeuObjectIDType)m_id.TileID.m_nType;    }

    virtual void                addState(const std::string &strStateName)           {   return;         }
    virtual bool                removeState(const std::string &strStateName) const  {   return false;   }
    virtual bool                hasState(const std::string &strStateName) const     {   return false;   }

    virtual const std::string  &getName(void) const;
    virtual void                setName(const std::string &strName) {   m_strName = strName;        }
    virtual unsigned            getParentCount(void) const;
    virtual ILayer             *getParent(unsigned nIndex);
    virtual const ILayer       *getParent(unsigned nIndex) const;
    virtual IInstance           *asInstance(void) {return NULL;}
    virtual ILayer              *asLayer(void)    {return NULL;}
    virtual ITerrainInstance    *asTerrainInstance(void)    {return NULL;}
    virtual const cmm::math::Sphered &getBound(void) const  {   return m_BoundingSphere;    };

public:
    virtual bool    init(void);
    virtual bool    fromBson(const bson::bsonDocument &bsonDoc);

public:
    void addParent(Layer *pLayer);
    void removeParent(Layer *pLayer);
    void getName2(void);

public:
    static OpenSP::op<LayerManager>     ms_pLayerManager;

protected:
    ID                      m_id;
    std::string             m_strName;
    std::vector<Layer *>    m_vecParents;

    mutable cmm::math::Sphered  m_BoundingSphere;
};

bool fetchBoundInfo(const void *pBuffer, unsigned nLength, cmm::math::Sphered &sphere, bool bConsiderRange = true);

}

#endif