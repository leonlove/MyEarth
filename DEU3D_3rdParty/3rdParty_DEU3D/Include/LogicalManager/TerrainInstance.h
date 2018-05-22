#ifndef LOGICAL_TERRAININSTANCE_H_8FF5891A_7ED0_4C4D_AD4C_11C26FA99884_INCLUDE
#define LOGICAL_TERRAININSTANCE_H_8FF5891A_7ED0_4C4D_AD4C_11C26FA99884_INCLUDE

#include "Object.h"
#include "ITerrainInstance.h"

namespace logical
{

class TerrainInstance : public Object, virtual public ITerrainInstance
{
public:
    explicit TerrainInstance(const ID &id);
    virtual ~TerrainInstance(void);
    virtual bool init(void);
    virtual bool fromBson(const bson::bsonDocument &bsonDoc);
    virtual ITerrainInstance *asTerrainInstance(void){ return this;}

    virtual bool                getState(const std::string &strStateName) const         { return false; }

    unsigned getCode(void) const;
    unsigned getTilesCount(void) const;
    bool getTileInfo( int index , ID& id, int& lLevel);

protected:
    virtual void  setState(const std::string &strStateName, bool bState);

private:
    std::map<ID, unsigned>  m_mapTerrainData;
    unsigned                m_nDataSetCode;
};

}

#endif