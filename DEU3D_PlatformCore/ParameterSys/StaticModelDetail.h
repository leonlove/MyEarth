#ifndef STATIC_MODEL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE
#define STATIC_MODEL_H_F8BB9BD0_F78E_4B9F_9D4F_6A65184A836A_INCLUDE

#include "Detail.h"

namespace param
{

class StaticModelDetail : public Detail, public IStaticModelDetail
{
public:
    explicit StaticModelDetail(void);
    explicit StaticModelDetail(unsigned int nDataSetCode);
    virtual ~StaticModelDetail(void);

public:
    virtual osg::Node           *createDetailNode(const CreationInfo *pInfo) const;

public:
    virtual bool        fromBson(bson::bsonDocument &bsonDoc);
    virtual bool        toBson(bson::bsonDocument &bsonDoc) const;
    virtual double      getBoundingSphereRadius(void)const{return -1.0;}

public:
    virtual const std::string  &getStyle(void) const { return STATIC_DETAIL; }

public:
    virtual void        setModelID(const ID &id) { m_ModelID = id; }
    virtual const ID   &getModelID(void) const { return m_ModelID; }

    virtual void        setAsOnGlobe(bool onGlobe){m_bOnGlobe = onGlobe;}
    virtual bool        isOnGlobe(){return m_bOnGlobe;}

protected:
    ID      m_ModelID;
    bool    m_bOnGlobe;
};

}

#endif

